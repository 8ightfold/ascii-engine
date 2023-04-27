#include <api/console.hpp>
#include <api/framebuffer.hpp>
#include <api/keypress_handler.hpp>
#include <api/timer.hpp>
#include <api/resource_locator.hpp>

#include <ui/strided_memcpy.hpp>
#include <ui/core.hpp>
#include <render/helper.hpp>
#include <audio/audiointerface.hpp>

#include <cstdio>
#include <cstdarg>

#define LINEAR_ALGORITHM(value, max, def, subtract) ((value) > (max)) ? (def) : ((subtract) - (float)(value) / (float)(max))
#define LINEAR_GAIN(value, max) LINEAR_ALGORITHM(value, max, 1.0f, 0.0f)
#define LINEAR_FALLOFF(value, max) LINEAR_ALGORITHM(value, max, 0.0f, 1.0f)
#define VEC4_TO_VEC3(v4) TPE_Vec3{ v4.x, v4.y, v4.z }

using modeASCII = char;
using modeRGBA = void;

inline constexpr double framerate_ms = 1000.0 / DFPS;

static void print_chars();
static int esprintf(char* string, const char* format, ...);

bool valid_distance(TPE_Vec3 v) {
    return not (v.x == TPE_INFINITY or v.y == TPE_INFINITY or v.z == TPE_INFINITY);
}

void buf_print(char* rawbuf, TPE_Vec3 v) {
    esprintf(rawbuf, "X: %i, Y: %i, Z: %i      ", v.x, v.y, v.z);
}

void buf_print(char* rawbuf, const char* str, TPE_Vec3 v) {
    esprintf(rawbuf, "%s: { X: %i, Y: %i, Z: %i }      ", str, v.x, v.y, v.z);
}

TPE_Unit elevatorHeight;
TPE_Unit ramp[6] = { 1600,0, -500,1400, -700,0 };
TPE_Unit ramp2[6] = { 2000,-5000, 1500,1700, -5000,-500 };

TPE_Vec3 environmentDistance(TPE_Vec3 p, TPE_Unit maxD)
{
    // manually created environment to match the 3D model of it
    TPE_ENV_START( TPE_envAABoxInside(p,TPE_vec3(0,2450,-2100),TPE_vec3(12600,5000,10800)),p )
    TPE_ENV_NEXT(  TPE_envAABox(p,TPE_vec3(-5693,0,-6580),TPE_vec3(4307,20000,3420)),p )
    TPE_ENV_NEXT(  TPE_envAABox(p,TPE_vec3(-10000,-1000,-10000),TPE_vec3(11085,2500,9295)),p )
    TPE_ENV_NEXT(  TPE_envAATriPrism(p,TPE_vec3(-5400,0,0),ramp,3000,2), p)
    TPE_ENV_NEXT(  TPE_envAATriPrism(p,TPE_vec3(2076,651,-6780),ramp2,3000,0), p)
    TPE_ENV_NEXT(  TPE_envAABox(p,TPE_vec3(7000,0,-8500),TPE_vec3(3405,2400,3183)),p )
    TPE_ENV_NEXT(  TPE_envSphere(p,TPE_vec3(2521,-100,-3799),1200),p )
    TPE_ENV_NEXT(  TPE_envAABox(p,TPE_vec3(5300,elevatorHeight,-4400),TPE_vec3(1000,elevatorHeight,1000)),p )
    TPE_ENV_NEXT(  TPE_envHalfPlane(p,TPE_vec3(5051,0,1802),TPE_vec3(-255,0,-255)),p )
    TPE_ENV_NEXT(  TPE_envInfiniteCylinder(p,TPE_vec3(320,0,170),TPE_vec3(0,255,0),530),p )
    TPE_ENV_END
}

int jumpCountdown = 0, onGround = 0;
TPE_Unit playerRotation = 0, headAngle = 0, groundDist;
TPE_Vec3 ballRot, ballPreviousPos, playerDirectionVec;
PTPE_Body playerBody = nullptr, ballBody = nullptr, boxBody = nullptr;

void updateDirection() // updates player direction vector
{
    playerDirectionVec.x = TPE_sin(playerRotation);
    playerDirectionVec.z = TPE_cos(playerRotation);
    playerDirectionVec.y = 0;
}

int main(int argc, char* argv[]) {
    api::Console::initialize();
    api::ResourceLocator::initialize();
    audio::XAudioInterface::initialize();

    api::KeyboardInput keys;
    api::MouseInput mouse;
    api::Timer time, seconds;

    api::KeypressHandlerFactory khf(keys);
    const api::KeypressHandler ESCAPE = khf(VK_ESCAPE, api::eSingle);
    const api::KeypressHandler DRAW_FPS = khf('F', api::eSingle);
    const api::KeypressHandler SPRINTING = khf('R', api::eContinuous);

    const api::KeypressHandler FREECAM = khf(VK_TAB, api::eSingle);
    const api::KeypressHandler INC_GRAVITY = khf('M', api::eSingle);
    const api::KeypressHandler DEC_GRAVITY = khf('N', api::eSingle);

    DEBUG_ONLY( print_chars(); )
    std::cout << "Press 'Return' to continue...";
    while(not keys.get_key_pressed(VK_RETURN)) {
        if(ESCAPE()) return 0;
    }

    api::Console window({
            .fullscreen = api::on,
            .font_size = { 16,16 },
            .console_title = "Morpheus, Dorpheus, Orpheus, Go eat some walruses"
    });

    api::Framebuffer<modeASCII> framebuffer;
    auto bufcoords = window.get_buffer_coords();
    framebuffer.set_details(bufcoords, 3);
    framebuffer.initialize_buffers();
    render::initialize_buffer(framebuffer);
    render::initialize_screen(bufcoords * api::Coords{ 1, 2 });

    auto audio_interface = audio::XAudioInterface::create();
    std::string music_track = "cavesong";
    audio_interface.register_source(music_track, audio::eLoopingInstance);
    audio_interface.register_source("stutter", audio::eLoopingInstance);
    audio_interface.register_source("boing", audio::eCircularInstance);
    audio_interface.register_source("thud", audio::eCircularInstance);
    audio_interface.register_source("box", audio::eCircularInstance);

    framebuffer.swap_buffers();

    auto screen_coords = (api::dVec2)(window.get_screen_coords() + 1) / (api::dVec2)framebuffer.get_coords();
    auto screen_middle = (window.get_screen_coords() / 2);
    auto buffer_middle = bufcoords / 2;

    helper_init();
    levelModelInit();
    updateDirection();
    api::CursorHider cur {};

    ballRot = { 0,0,0 };
    tpe_world.environmentFunction = environmentDistance;

    /* normally player bodies are approximated with capsules -- since we don't
    have these, we'll use a body consisting of two spheres: */
    helper_add2Line(400,300,400);
    playerBody = &(tpe_world.bodies[0]);
    TPE_bodyMoveBy(playerBody, TPE_vec3(1000,1000,1500));
    TPE_bodyRotateByAxis(playerBody, TPE_vec3(0,0,TPE_F / 4));
    playerBody->elasticity = 0;
    playerBody->friction = 0;
    playerBody->flags |= TPE_BODY_FLAG_NONROTATING | TPE_BODY_FLAG_ALWAYS_ACTIVE;
    groundDist = TPE_JOINT_SIZE(playerBody->joints[0]) + 30;


    // add two interactive bodies:
    TPE_Unit ball_audio_cooldown = 0;
    TPE_Unit box_audio_cooldown = 0;
    helper_addBall(1000,100);
    ballBody = &(tpe_world.bodies[1]);
    TPE_bodyMoveBy(ballBody,TPE_vec3(-1000,1000,0));
    ballBody->elasticity = 400;
    ballBody->friction = 100;
    ballBody->previouslyCollided = true;
    ballBody->bodyCollisionCallback = [&](TPE_Vec3 velocity){
        static constexpr auto min_vel = 10;
        static constexpr auto down_time = 10;

        TPE_Unit average_vel = (std::abs(velocity.x / 3) + (velocity.y * 2) + std::abs(velocity.z / 3)) / 3;
        if(average_vel > min_vel && not ball_audio_cooldown) {
            float dist = (float)TPE_dist(ballPreviousPos, playerBody->joints[1].position) / TPE_F;
            float volume_modifier = LINEAR_FALLOFF(dist, 25.0f);
            float volume = LINEAR_GAIN(average_vel, 35);
            audio_interface.set_volume("thud", volume * volume_modifier);
            audio_interface.start_source("thud");
            ball_audio_cooldown = down_time;
        }
    };
    ballPreviousPos = ballBody->joints[0].position;

    helper_addCenterRect(600,600,400,50);
    boxBody = &(tpe_world.bodies[2]);
    TPE_bodyMoveBy(boxBody,TPE_vec3(-3000,1000,2000));
    boxBody->elasticity = 100;
    boxBody->friction = 50;
    boxBody->previouslyCollided = true;
    boxBody->bodyCollisionCallback = [&](TPE_Vec3 velocity) {
        static constexpr auto min_vel = 7;
        static constexpr auto down_time = 10;

        TPE_Unit average_vel = (std::abs(velocity.x) + (velocity.y * 2) + std::abs(velocity.z)) / 3;
        if(average_vel > min_vel && not box_audio_cooldown) {
            float dist = (float)TPE_dist(ballPreviousPos, playerBody->joints[1].position) / TPE_F;
            float volume_modifier = LINEAR_FALLOFF(dist, 25.0f);
            float volume = LINEAR_GAIN(average_vel, 35);
            audio_interface.set_volume("box", volume * volume_modifier);
            audio_interface.start_source("box");
            box_audio_cooldown = down_time;
        }
    };

    auto dur = api::ms_duration(framerate_ms);
    TPE_Unit gravity_value = 5;
    double elapsed_time = framerate_ms;
    double average_fps = DFPS;
    bool zero_gs = false;
    bool disp_fps = false;
    bool freecam = false;
    float music_volume = 0.0f;

    auto tick_position = [&] {
        TPE_bodyApplyGravity(&tpe_world.bodies[0], gravity_value);

        if (onGround) {
            if(keys.get_key_held(VK_SPACE) && jumpCountdown == 0) {
                playerBody->joints[0].velocity[1] = 120;
                jumpCountdown = 8;
            }
        }

        static TPE_Unit D = 15, max_v = 60;
        auto& player_velocity = playerBody->joints[0].velocity;

        if(SPRINTING()) {
            D = 10, max_v = 110;
        }
        else {
            D = 15, max_v = 90;
        }

        if(keys.get_key_held('W')) {
            player_velocity[0] += playerDirectionVec.x / D;
            player_velocity[2] += playerDirectionVec.z / D;
        }
        else if(keys.get_key_held('S')) {
            player_velocity[0] -= playerDirectionVec.x / D;
            player_velocity[2] -= playerDirectionVec.z / D;
        }

        if (keys.get_key_held('A')) {
            player_velocity[2] += playerDirectionVec.x / D;
            player_velocity[0] -= playerDirectionVec.z / D;
        }
        else if (keys.get_key_held('D')) {
            player_velocity[2] -= playerDirectionVec.x / D;
            player_velocity[0] += playerDirectionVec.z / D;
        }

        player_velocity[0] = TPE_keepInRange(player_velocity[0], -max_v, max_v);
        player_velocity[2] = TPE_keepInRange(player_velocity[2], -max_v, max_v);
    };
    auto tick_freecam = [&] {
        static constexpr TPE_Unit F = 5;
        static constexpr TPE_Unit D = 2;

        auto& foot_position = playerBody->joints[0].position;
        auto& head_position = playerBody->joints[1].position;

        if(keys.get_key_held(VK_SPACE)) {
            head_position.y += TPE_F / F;
            foot_position.y += TPE_F / F;
        }
        else if(keys.get_key_held(VK_SHIFT)) {
            head_position.y -= TPE_F / F;
            foot_position.y -= TPE_F / F;
        }

        if(keys.get_key_held('W')) {
            foot_position.x += playerDirectionVec.x / D;
            foot_position.z += playerDirectionVec.z / D;
        }
        else if(keys.get_key_held('S')) {
            foot_position.x -= playerDirectionVec.x / D;
            foot_position.z -= playerDirectionVec.z / D;
        }

        if (keys.get_key_held('A')) {
            foot_position.z += playerDirectionVec.x / D;
            foot_position.x -= playerDirectionVec.z / D;
        }
        else if (keys.get_key_held('D')) {
            foot_position.z -= playerDirectionVec.x / D;
            foot_position.x += playerDirectionVec.z / D;
        }

        head_position.x = foot_position.x;
        head_position.z = foot_position.z;
    };
    auto tick_head = [&] {
        static constexpr TPE_Unit movement_unit = 5;
        if(keys.get_key_held(VK_UP))
            headAngle += movement_unit - 1;
        else if(keys.get_key_held(VK_DOWN))
            headAngle -= movement_unit - 1;

        if (keys.get_key_held(VK_LEFT))
            playerRotation -= movement_unit;
        else if (keys.get_key_held(VK_RIGHT))
            playerRotation += movement_unit;
    };

    auto poll_volume = [&] {
        static constexpr float max_volume = 2.0f;
        static constexpr float min_volume = 0.0f;
        if(keys.get_key_held('K')) {
            if(music_volume > min_volume) {
                music_volume -= 0.02f;
                audio_interface.set_volume(music_track, music_volume);
            }
            else music_volume = min_volume;
        }
        else if(keys.get_key_held('L')) {
            if(music_volume < max_volume) {
                music_volume += 0.02f;
                audio_interface.set_volume(music_track, music_volume);
            }
            else music_volume = max_volume;
        }
    };
    auto poll_fps = [&] {
        double measurement = ((double)helper_frame / seconds.elapsed_ms()) * 1000.0;
        if(measurement > 5000.0) measurement = DFPS;

        double smoothing = 0.5;
        average_fps = (measurement * smoothing) + (average_fps * (1.0 - smoothing));
        if(DRAW_FPS()) disp_fps = !disp_fps;
        if(disp_fps) {
            auto* rawbuf = framebuffer.get_active_buffer()->raw_data();
            esprintf(rawbuf, "FPS: %.1f, Music volume: %i%%    ", average_fps, (int)(music_volume * 100));
            rawbuf += framebuffer->get_x();
            auto pos = playerBody->joints[0].position;
            buf_print(rawbuf, "POS", pos);
            rawbuf += framebuffer->get_x();
            esprintf(rawbuf, "X: %.2f, Y: %.1f, Z: %.2f      ",
                     (double)pos.x / TPE_F, (double)pos.y / TPE_F, (double)pos.z / TPE_F);


            auto look_vec = TPE_vec3(playerDirectionVec.x, headAngle, playerDirectionVec.z);
            TPE_Vec3 env_dist = TPE_castEnvironmentRay(
                    playerBody->joints[1].position, look_vec,
                    tpe_world.environmentFunction, 128, 512, 128);

            rawbuf += framebuffer->get_x();
            if(valid_distance(env_dist)) {
                buf_print(rawbuf, "LOOKING AT", env_dist);
                //helper_drawPoint3D(env_dist, 1);
            }
            else esprintf(rawbuf, "LOOKING AT: NULL   ");

            rawbuf += framebuffer->get_x();
            esprintf(rawbuf, "GRAVITY: %i   ", gravity_value);
        }
    };
    auto poll_cursor = [&] {
        static auto sensitivity = 4;
        const auto moved_by = (mouse.get_screen_position() - screen_middle) / sensitivity;
        playerRotation += moved_by.x;
        headAngle -= moved_by.y;
        mouse.set_screen_position(screen_middle);
    };
    auto poll_sleep = [&] {
        time.stop();
        elapsed_time = framerate_ms - time.elapsed_ms();
        if(elapsed_time > 0.0) {
            dur = api::ms_duration(elapsed_time);
            std::this_thread::sleep_for(dur);
        }
        time.restart();
    };

    audio_interface.set_volume(music_track, music_volume);
    audio_interface.start_source(music_track);
    time.start(), seconds.start();
    while(helper_running) {
        helper_frameStart();
        poll_cursor();
        TPE_worldStep(&tpe_world);

        if(ball_audio_cooldown > 0) --ball_audio_cooldown;
        if(box_audio_cooldown > 0) --box_audio_cooldown;
        if(jumpCountdown > 0) --jumpCountdown;

        TPE_Vec3 groundPoint =
                environmentDistance(playerBody->joints[0].position,groundDist);

        onGround = (playerBody->flags & TPE_BODY_FLAG_DEACTIVATED) ||
                   (TPE_DISTANCE(playerBody->joints[0].position,groundPoint)
                    <= groundDist && groundPoint.y < playerBody->joints[0].position.y - groundDist / 2);

        if(not onGround) {
            /* it's possible that the closest point is e.g. was a perpend wall so also
               additionally check directly below */

            onGround = TPE_DISTANCE( playerBody->joints[0].position,
                TPE_castEnvironmentRay(
                    playerBody->joints[0].position,
                    TPE_vec3(0,-1 * TPE_F,0),tpe_world.environmentFunction,
                    128,512,512)) <= groundDist;
        }

        elevatorHeight = (1250 * (TPE_sin(helper_frame * 4) + TPE_F)) / (2 * TPE_F);

        s3l_scene.camera.transform.translation.x = playerBody->joints[0].position.x;
        s3l_scene.camera.transform.translation.z = playerBody->joints[0].position.z;
        s3l_scene.camera.transform.translation.y = TPE_keepInRange(
                s3l_scene.camera.transform.translation.y,
                playerBody->joints[1].position.y,
                playerBody->joints[1].position.y + 10);

        TPE_bodyMultiplyNetSpeed(playerBody, onGround ? 300 : 500);

        static constexpr TPE_Unit max_headAngle = TPE_FRACTIONS_PER_UNIT / 4;
        headAngle = TPE_keepInRange(headAngle, -max_headAngle, max_headAngle);
        s3l_scene.camera.transform.rotation.x = headAngle;
        s3l_scene.camera.transform.rotation.y = -1 * playerRotation;

        // fake the sphere rotation (since a single joint doesn't rotate itself):
        TPE_Vec3 ballRoll = TPE_fakeSphereRotation(ballPreviousPos,
                                                   ballBody->joints[0].position,1000);

        ballRot = TPE_rotationRotateByAxis(ballRot,ballRoll);

        ballPreviousPos = ballBody->joints[0].position;

        for (int i = 1; i < tpe_world.bodyCount; ++i)
            TPE_bodyApplyGravity(&tpe_world.bodies[i], gravity_value);

        if(FREECAM()) {
            freecam = !freecam;
            playerBody->flags ^= TPE_BODY_FLAG_DISABLED;
        }

        if(freecam) tick_freecam();
        else tick_position();

        if(INC_GRAVITY())
            ++gravity_value;
        else if(DEC_GRAVITY())
            --gravity_value;

        if(gravity_value == 0 && not zero_gs) {
            for (int i = 1; i < tpe_world.bodyCount; ++i)
                tpe_world.bodies[i].flags |= TPE_BODY_FLAG_ALWAYS_ACTIVE;
        }
        else if(gravity_value != 0 && zero_gs) {
            for (int i = 1; i < tpe_world.bodyCount; ++i)
                tpe_world.bodies[i].flags &= ~TPE_BODY_FLAG_ALWAYS_ACTIVE;
        }


        updateDirection();

        // Draw functions
        // World color
        helper_set3DColor(0); {
            helper_drawModel(&levelModel, TPE_vec3(0,0,0), TPE_vec3(600,600,600), TPE_vec3(0,0,0));
        }

        // White color
        helper_set3DColor(1); {
            helper_draw3DBox(TPE_vec3(5300,elevatorHeight,-4400),
                             TPE_vec3(2000,2 * elevatorHeight,2000),TPE_vec3(0,0,0));

            helper_draw3DBox(TPE_bodyGetCenterOfMass(boxBody),
                             TPE_vec3(1200,800,1200),TPE_bodyGetRotation(boxBody,0,2,1));

            helper_draw3DSphere(ballBody->joints[0].position,
                                TPE_vec3(1000,1000,1000),ballRot);
        }

        poll_volume();
        poll_fps();
        render::draw_pixel(buffer_middle.x + 1, buffer_middle.y, 3, TO_LUM(15));
        helper_frameEnd();

        if (keys.get_key_pressed(VK_ESCAPE))
            helper_running = false;

        poll_sleep();
    }

    framebuffer.get_active_buffer()->set_buffer_data(255);
    framebuffer.post_buffer();
    window.set_keystate(buffer_middle);
    std::printf("Bye bye!");

    const float fade_rate = music_volume * 0.015;
    while(not audio_interface.fade_out(music_track, fade_rate) and not ESCAPE()) {
        poll_sleep();
    }

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(150ms);
}

static void print_chars() {
    for(unsigned short c = 0; c < 256; ++c) {
        switch(c) {
            case '\a': {
                std::printf("%i: '\\a'\n", c);
                break;
            }
            case '\b': {
                std::printf("%i: '\\b'\n", c);
                break;
            }
            case '\n': {
                std::printf("%i: '\\n'\n", c);
                break;
            }
            case '\r': {
                std::printf("%i: '\\r'\n", c);
                break;
            }
            case '\t': {
                std::printf("%i: '\\t'\n", c);
                break;
            }
            default: {
                std::printf("%i: '%c'\n", c, c);
            }
        }
    }
    std::cout << std::endl;
}

static int esprintf(char* string, const char* format, ...) {
    va_list ls;
    int ret;

    va_start(ls, format);
    ret = vsprintf(string, format, ls);
    va_end(ls);

    return ret;
}