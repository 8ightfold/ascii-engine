#include <api/console.hpp>
#include <api/framebuffer.hpp>
#include <api/keypress_handler.hpp>
#include <api/timer.hpp>
#include <api/resource_locator.hpp>

#include <ui/ui_types.hpp>
#include <render/helper.hpp>
#include <audio/audiointerface.hpp>

#include <engine/game.hpp>

#define ARGUMENTS \
    api::Input& input, \
    api::KeypressHandlerFactory& khf, \
    api::Timer& time, api::Timer& seconds, double& elapsed_time, \
    api::DefaultFramebuffer& framebuffer, \
    api::Console& window,             \
    audio::XAudioInterface& audio_interface, std::string& current_song


inline constexpr double framerate_ms = 1000.0 / DFPS;

static void render_init(api::Framebuffer<ModeASCII>& fb, api::Coords bc);
void audio_init(audio::XAudioInterface& ai);
bool valid_distance(TPE_Vec3 v);
void buf_print(api::DefaultFramebuffer& buffer, TPE_Vec3 v);
void buf_print(api::DefaultFramebuffer& buffer, const char* str, TPE_Vec3 v);

int test_engine(ARGUMENTS);
int test_ui(ARGUMENTS);

TPE_Unit elevatorHeight;
TPE_Unit ramp[6] = { 1600,0, -500,1400, -700,0 };
TPE_Unit ramp2[6] = { 2000,-5000, 1500,1700, -5000,-500 };

TPE_Vec3 environmentDistance(TPE_Vec3 p, TPE_Unit maxD) {
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

void updateDirection() {
    playerDirectionVec.x = TPE_sin(playerRotation);
    playerDirectionVec.z = TPE_cos(playerRotation);
    playerDirectionVec.y = 0;
}

int main() {
    std::cout << "Press 'Return' to continue..." << std::endl;
    while(not WIN_PRESSED(VK_RETURN)) {
        if(WIN_PRESSED(VK_ESCAPE)) return 0;
    }

    api::Console::initialize();
    api::ResourceLocator::initialize();
    audio::XAudioInterface::initialize();

    api::Input input;
    api::KeypressHandlerFactory khf(input);
    api::Timer time, seconds;
    double elapsed_time;

    api::Console window({
        .fullscreen = api::on,
        .console_title = "Physics Engine"
    });

    api::DefaultFramebuffer framebuffer;
    api::Coords bufcoords = window.get_buffer_coords();
    render_init(framebuffer, bufcoords);

    auto audio_interface = audio::XAudioInterface::create();
    audio_init(audio_interface);
    api::on_error = [&] {
        enable_ansi();
        std::cout << "\x1b[0;30;41m";
        window.set_keystate({ 0,0 });
        std::cout << std::string(framebuffer->area(), ' ');
        window.set_keystate({ 0,0 });
        audio_interface.stop_all();
        audio_interface.set_volume("error", 2.0);
        audio_interface.start_source("error");
    };
    std::string current_song = "menusong";

    int return_code = test_ui(input, khf, time, seconds, elapsed_time, framebuffer, window, audio_interface, current_song);
    if(return_code == 1) {
        test_engine(input, khf, time, seconds, elapsed_time, framebuffer, window, audio_interface, current_song);
    }
}

int test_engine(ARGUMENTS) {
    TPE::ObjectModel model("models/level_test.obj", TPE::WindingOrder::eCCW);
    model.compile_model();
    model.set_color(0);

    const api::KeypressHandler ESCAPE = khf(VK_ESCAPE, api::eSingle);
    const api::KeypressHandler DRAW_FPS = khf('F', api::eSingle);
    const api::KeypressHandler SPRINTING = khf('R', api::eContinuous);
    const api::KeypressHandler FREECAM = khf(VK_TAB, api::eSingle);
    const api::KeypressHandler DEBUG = khf('I', api::eSingle);

    auto bufcoords = window.get_buffer_coords();
    auto screen_coords = framebuffer.get_screen_coords(window.get_screen_coords());
    auto screen_middle = (window.get_screen_coords() / 2);
    auto buffer_middle = bufcoords / 2;

    helper_init();
    levelModelInit();
    updateDirection();
    api::CursorHider cur {};

    ballRot = { 0,0,0 };
    tpe_ecs->register_env(environmentDistance);

    /* normally player bodies are approximated with capsules -- since we don't
    have these, we'll use a body consisting of two spheres: */
    auto player_body = tpe_ecs->get_player();
    player_body.move_by(1000, 1000, 1500);
    player_body.rotate_by_axis(0, 0, TPE_F / 4);
    player_body->elasticity = 0;
    player_body->friction = 0;
    player_body->flags |= (TPE_BODY_FLAG_NONROTATING | TPE_BODY_FLAG_ALWAYS_ACTIVE);
    groundDist = TPE_JOINT_SIZE(player_body.foot_joint()) + 30;


    // add two interactive bodies:
    TPE_Unit ball_audio_cooldown = 0;
    auto ball_body = tpe_ecs->bind(tpe_ecs->add_ball(1000, 100));
    ball_body.move_by(-1000, 1000, 0);
    ball_body->elasticity = 400;
    ball_body->friction = 100;
    ball_body->previouslyCollided = true;
    ball_body->bodyCollisionCallback = [&](TPE_Vec3 velocity) {
        static constexpr auto min_vel = 10;
        static constexpr auto down_time = 10;

        TPE_Unit average_vel = (std::abs(velocity.x / 3) + (velocity.y * 2) + std::abs(velocity.z / 3)) / 3;
        if(average_vel > min_vel && not ball_audio_cooldown) {
            float dist = (float)TPE_dist(ballPreviousPos, player_body->joints[1].position) / TPE_F;
            float volume_modifier = LINEAR_FALLOFF(dist, 25.0f);
            float volume = LINEAR_GAIN(average_vel, 35);
            audio_interface.set_volume("thud", volume * volume_modifier);
            audio_interface.start_source("thud");
            ball_audio_cooldown = down_time;
        }
    };
    ballPreviousPos = ball_body->joints[0].position;

    TPE_Unit box_audio_cooldown = 0;
    auto box_body = tpe_ecs->bind(
            tpe_ecs->add_centered_rect(600, 600, 400, 50));
    box_body.move_by(-3000, 1000, 2000);
    box_body->elasticity = 100;
    box_body->friction = 50;
    box_body->previouslyCollided = true;
    box_body->bodyCollisionCallback = [&](TPE_Vec3 velocity) {
        static constexpr auto min_vel = 7;
        static constexpr auto down_time = 10;

        TPE_Unit average_vel = (std::abs(velocity.x) + (velocity.y * 2) + std::abs(velocity.z)) / 3;
        if(average_vel > min_vel && not box_audio_cooldown) {
            float dist = (float)TPE_dist(ballPreviousPos, player_body->joints[1].position) / TPE_F;
            float volume_modifier = LINEAR_FALLOFF(dist, 25.0f);
            float volume = LINEAR_GAIN(average_vel, 35);
            audio_interface.set_volume("box", volume * volume_modifier);
            audio_interface.start_source("box");
            box_audio_cooldown = down_time;
        }
    };

    // TODO: Add Zero-G
    bool zero_gs = false;
    TPE_Unit gravity_value = 5;

    auto dur = api::ms_duration(framerate_ms);
    elapsed_time = framerate_ms;
    double average_fps = DFPS;
    bool disp_fps = false;
    bool freecam = false;
    bool debug_draw = false;
    float music_volume = 1.0f;

    auto tick_position = [&] {
        if (onGround) {
            if(input.get_key_held(VK_SPACE) && jumpCountdown == 0) {
                player_body.foot_velocity()[1] = 140;
                jumpCountdown = 8;
            }
        }

        static TPE_Unit D = 17, max_v = 80;
        auto& player_velocity = player_body.foot_velocity();

        if(SPRINTING()) {
            D = 12, max_v = 100;
        }
        else {
            D = 17, max_v = 80;
        }

        if(input.get_key_held('W')) {
            player_velocity[0] += playerDirectionVec.x / D;
            player_velocity[2] += playerDirectionVec.z / D;
        }
        else if(input.get_key_held('S')) {
            player_velocity[0] -= playerDirectionVec.x / D;
            player_velocity[2] -= playerDirectionVec.z / D;
        }

        if (input.get_key_held('A')) {
            player_velocity[2] += playerDirectionVec.x / D;
            player_velocity[0] -= playerDirectionVec.z / D;
        }
        else if (input.get_key_held('D')) {
            player_velocity[2] -= playerDirectionVec.x / D;
            player_velocity[0] += playerDirectionVec.z / D;
        }

        player_velocity[0] = TPE_keepInRange(player_velocity[0], -max_v, max_v);
        player_velocity[2] = TPE_keepInRange(player_velocity[2], -max_v, max_v);
    };
    auto tick_freecam = [&] {
        static constexpr TPE_Unit F = 5;
        static constexpr TPE_Unit D = 2;

        auto& foot_position = player_body.foot_joint().position;
        auto& head_position = player_body.head_joint().position;

        if(input.get_key_held(VK_SPACE)) {
            head_position.y += TPE_F / F;
            foot_position.y += TPE_F / F;
        }
        else if(input.get_key_held(VK_SHIFT)) {
            head_position.y -= TPE_F / F;
            foot_position.y -= TPE_F / F;
        }

        if(input.get_key_held('W')) {
            foot_position.x += playerDirectionVec.x / D;
            foot_position.z += playerDirectionVec.z / D;
        }
        else if(input.get_key_held('S')) {
            foot_position.x -= playerDirectionVec.x / D;
            foot_position.z -= playerDirectionVec.z / D;
        }

        if (input.get_key_held('A')) {
            foot_position.z += playerDirectionVec.x / D;
            foot_position.x -= playerDirectionVec.z / D;
        }
        else if (input.get_key_held('D')) {
            foot_position.z -= playerDirectionVec.x / D;
            foot_position.x += playerDirectionVec.z / D;
        }

        head_position.x = foot_position.x;
        head_position.z = foot_position.z;
    };
    auto tick_draw = [&] {
        if(debug_draw) {
            helper_debugDraw();
            return;
        }

        // Draw functions
        //TPE_Unit scale = 1200;
        //TPE::draw_model(model, {0,0,0}, {scale,scale,scale}, {0,0,0});

        helper_set3DColor(0); {
            TPE_Unit scale = 600;
            helper_drawModel(&levelModel, TPE_vec3(0,0,0), TPE_vec3(scale,scale,scale), TPE_vec3(0,0,0));
        }

        // White color
        helper_set3DColor(1);
        {
            helper_draw3DBox(TPE_vec3(5300, elevatorHeight, -4400),
                             TPE_vec3(2000, 2 * elevatorHeight, 2000), TPE_vec3(0, 0, 0));

            helper_draw3DBox(box_body.get_center_of_mass(),
                             TPE_vec3(1200, 800, 1200),
                             box_body.get_rotation(0, 2, 1));

            helper_draw3DSphere(ball_body->joints[0].position,
                                TPE_vec3(1000, 1000, 1000), ballRot);
        }
    };

    auto poll_volume = [&] {
        static constexpr float max_volume = 2.0f;
        static constexpr float min_volume = 0.0f;
        if(input.get_key_held('K')) {
            if(music_volume > min_volume) {
                music_volume -= 0.02f;
                audio_interface.set_volume(current_song, music_volume);
            }
            else music_volume = min_volume;
        }
        else if(input.get_key_held('L')) {
            if(music_volume < max_volume) {
                music_volume += 0.02f;
                audio_interface.set_volume(current_song, music_volume);
            }
            else music_volume = max_volume;
        }
    };
    auto poll_fps = [&] {
        double measurement = ((double)helper_framev / seconds.elapsed_ms()) * 1000.0;
        if(measurement > 5000.0) measurement = DFPS;

        if(helper_framev > 30) {
            double smoothing = 0.5;
            average_fps = (measurement * smoothing) + (average_fps * (1.0 - smoothing));
            helper_framev = 0;
            seconds.restart();
        }

        if(DRAW_FPS()) disp_fps = !disp_fps;
        if(disp_fps) {
            framebuffer.write_line("FPS: %.1f, Music volume: %i%%    ", average_fps, (int)(music_volume * 100));
            framebuffer.write_line("NOW PLAYING: %s   ", current_song.c_str());
            buf_print(framebuffer, "POS", player_body.foot_position());

            auto look_vec = TPE_vec3(playerDirectionVec.x, headAngle, playerDirectionVec.z);
            TPE_Vec3 env_dist = TPE_castEnvironmentRay(
                    player_body.head_position(), look_vec,
                    tpe_ecs->get_env(), 128, 512, 128);

            if(valid_distance(env_dist))
                buf_print(framebuffer, "LOOKING AT", env_dist);
            else framebuffer.write_line("LOOKING AT: NULL   ");
        }
    };
    auto poll_cursor = [&] {
        static auto sensitivity = 4;
        const auto moved_by = (input.get_screen_position() - screen_middle) / sensitivity;
        playerRotation += moved_by.x;
        headAngle -= moved_by.y;
        input.set_screen_position(screen_middle);
    };
    auto poll_sleep = [&] {
        elapsed_time = framerate_ms - time.elapsed_ms();
        while(elapsed_time > 0.0) {
            elapsed_time = framerate_ms - time.elapsed_ms();
        }
        time.restart();
    };

    if(freecam) player_body->flags ^= TPE_BODY_FLAG_DISABLED;

    current_song = "intersong";
    audio_interface.set_volume(current_song, music_volume);
    audio_interface.start_source(current_song);
    time.start(), seconds.start();
    while(helper_running) {
        TAG_FRAME("main loop")
        helper_frameStart();
        poll_cursor();
        tpe_ecs->tick();

        if(ball_audio_cooldown > 0) --ball_audio_cooldown;
        if(box_audio_cooldown > 0) --box_audio_cooldown;
        if(jumpCountdown > 0) --jumpCountdown;

        TPE_Vec3 groundPoint =
                environmentDistance(player_body.foot_position(), groundDist);

        onGround = (player_body->flags & TPE_BODY_FLAG_DEACTIVATED) ||
                   (TPE_DISTANCE(player_body.foot_position(),groundPoint)
                    <= groundDist && groundPoint.y < player_body.foot_position().y - groundDist / 2);

        if(not onGround) {
            /* it's possible that the closest point is e.g. was a perpend wall so also
               additionally check directly below */

            onGround = TPE_DISTANCE( player_body.foot_joint().position,
                                     TPE_castEnvironmentRay(
                                             player_body.foot_position(),
                                             TPE_vec3(0,-1 * TPE_F,0), tpe_ecs->get_env(),
                                             128,512,512)) <= groundDist;
        }

        elevatorHeight = (1250 * (TPE_sin(helper_frame * 4) + TPE_F)) / (2 * TPE_F);

        s3l_scene.camera.transform.translation.x = player_body.foot_position().x;
        s3l_scene.camera.transform.translation.z = player_body.foot_position().z;
        s3l_scene.camera.transform.translation.y = TPE_keepInRange(
                s3l_scene.camera.transform.translation.y,
                player_body.head_position().y,
                player_body.head_position().y + 10);

        player_body.multiply_net_speed(onGround ? 300 : 500);

        static constexpr TPE_Unit max_headAngle = TPE_FRACTIONS_PER_UNIT / 4;
        headAngle = TPE_keepInRange(headAngle, -max_headAngle, max_headAngle);
        s3l_scene.camera.transform.rotation.x = headAngle;
        s3l_scene.camera.transform.rotation.y = -1 * playerRotation;

        // fake the sphere rotation (since a single joint doesn't rotate itself):
        TPE_Vec3 ballRoll = TPE_fakeSphereRotation(ballPreviousPos,
                                                   ball_body->joints[0].position,1000);

        ballRot = TPE_rotationRotateByAxis(ballRot,ballRoll);

        ballPreviousPos = ball_body->joints[0].position;

        if(FREECAM()) {
            freecam = !freecam;
            player_body->flags ^= TPE_BODY_FLAG_DISABLED;
        }
        if(freecam) tick_freecam();
        else tick_position();

        updateDirection();

        if(DEBUG()) debug_draw = !debug_draw;
        tick_draw();

        poll_volume();
        poll_fps();
        render::draw_pixel(buffer_middle.x + 1, buffer_middle.y, 3, TO_LUM(15));
        helper_frameEnd();

        if(ESCAPE()) helper_running = false;

        poll_sleep();
    }

    framebuffer.get_active_buffer()->set_buffer_data(255);
    framebuffer.post_buffer();
    window.set_keystate(buffer_middle);
    std::printf("Bye bye!");

    const float fade_rate = music_volume * 0.015;
    while(not audio_interface.fade_out(current_song, fade_rate) and not ESCAPE()) {
        poll_sleep();
    }

    const double exit_time = (music_volume > 0.1) ? 0.5 : (1.5 - music_volume);
    const auto exit_dur = api::ms_duration(exit_time * 400.0);
    std::this_thread::sleep_for(exit_dur);
    return 0;
}


struct Parent {
    ui::UIFrameManager<Parent>* operator->() { return &_ui; }
    bool tick() { return _ui(); }
private:
    ui::UIFrameManager<Parent> _ui {};
};

int test_ui(ARGUMENTS) {
    const api::KeypressHandler ESCAPE = khf(VK_ESCAPE, api::eSingle);
    const api::KeypressHandler DEBUG = khf('F', api::eSingle);

    auto average_fps = DFPS;
    std::uint32_t frame = 0;
    bool do_escape = false;
    bool begin_game = false;

    auto poll_sleep = [&] {
        elapsed_time = framerate_ms - time.elapsed_ms();
        while(elapsed_time > 0.0) {
            elapsed_time = framerate_ms - time.elapsed_ms();
        }
        time.restart();
    };
    auto poll_fps = [&] {
        double measurement = ((double) frame / seconds.elapsed_ms()) * 1000.0;
        if(measurement > 5000.0) measurement = DFPS;

        if(frame > 30) {
            double smoothing = 0.5;
            average_fps = (measurement * smoothing) + (average_fps * (1.0 - smoothing));
            frame = 0;
            seconds.restart();
        }
    };
    auto wipe_screen = [&] {
        framebuffer.swap_buffers();
        framebuffer->set_buffer_data(255);
    };

    wipe_screen();
    framebuffer.post_buffer();

    Parent parent;
    auto* p = parent->add_frame("main");
    p->bind(input);
    p->bind(window);
    p->bind(framebuffer);
    p->callback([&] {
        if(ESCAPE()) { do_escape = true; }
    });

    using ui_type = ui::UIType<Parent>;
    auto* body = p->add_element<ui_type::Body>("body");
    body->align(ui::eCenter);
    body->set_size(.35, .6);
    body->set_color({ .luminance = 176 });

    {
        auto* start = body->add_element<ui_type::Button>("start");
        start->align(ui::eCenterY, ui::eTextLeft, ui::eTextSuspend);
        start->set_size(.2, .1);
        start->set_position(.0, 0.4);
        start->set_scale(1);
        start->set_color({ .luminance = 177 });
        start->on_click([&](ui_type::Button* elem) {
            audio_interface.start_source("click");
            begin_game = true;
            do_escape = true;
        });
        start->set_text("START GAME");

        auto* options = body->add_element<ui_type::Button>("options");
        options->align(ui::eCenterY, ui::eTextCenter, ui::eTextSuspend);
        options->set_size(.2, .1);
        options->set_position(.0, 0.1);
        options->set_scale(1);
        options->set_color({ .luminance = 177 });
        options->on_click([&](ui_type::Button* elem) {
            audio_interface.start_source("click");
            elem->push("options");
        });
        options->set_text("OPTIONS");

        auto* exit = body->add_element<ui_type::Button>("  exit  ");
        exit->align(ui::eCenterY, ui::eTextRight, ui::eTextSuspend);
        exit->set_size(.2, .1);
        exit->set_position(.0, -0.2);
        exit->set_scale(1);
        exit->set_color({ .luminance = 177 });
        exit->on_click([](ui_type::Button* elem) {
            elem->pop();
        });
        exit->set_text("EXIT");
    }

    auto* o = parent->add_frame("options");
    o->bind(input);
    o->bind(window);
    o->bind(framebuffer);
    o->callback([&] {
        if(ESCAPE()) {
            audio_interface.start_source("click");
            parent->pop();
        }
    });

    auto* opts = o->add_element<ui_type::Body>("body");
    opts->align(ui::eCenter);
    opts->set_size(.35, .6);
    opts->set_color({ .luminance = 176 });

    parent->bind(&parent, "main");
    audio_interface.start_source(current_song);

    o->element("dadsasdsad");
    time.start(), seconds.start();
    while(not do_escape) {
        TAG_FRAME("menu loop")
        if(not parent.tick()) break;
        framebuffer.post_buffer();

        poll_fps();
        poll_sleep();
        wipe_screen();
    }

    if(begin_game) {
        float music_volume = 1.0;
        const float fade_rate = music_volume * 0.015;
        while(not audio_interface.fade_out(current_song, fade_rate) and not ESCAPE()) {
            poll_sleep();
        }

        const double exit_time = (music_volume > 0.1) ? 0.5 : (1.5 - music_volume);
        const auto exit_dur = api::ms_duration(exit_time * 400.0);
        std::this_thread::sleep_for(exit_dur);
        return 1;
    }

    framebuffer.get_active_buffer()->set_buffer_data(-1);
    framebuffer.post_buffer();
    return 0;
}

static void render_init(api::Framebuffer<ModeASCII>& fb, api::Coords bc) {
    fb.set_details(bc, 3);
    fb.initialize_buffers();
    render::initialize_buffer(fb);
    render::initialize_screen(bc * api::Coords{ 1, 2 });
}

void audio_init(audio::XAudioInterface& ai) {
    ai.register_source("click", audio::eCircularInstance);
    ai.register_source("error", audio::eCircularInstance);
    ai.register_source("thud", audio::eCircularInstance);
    ai.register_source("box", audio::eCircularInstance);

    ai.register_source("golfcup", audio::eSingleInstance);
    ai.register_source("trophyget", audio::eSingleInstance);
    ai.register_source("dooropen", audio::eSingleInstance);
    ai.register_source("doorclose", audio::eSingleInstance);

    ai.register_source("bosssong", audio::eLoopingInstance);
    ai.register_source("cavesong", audio::eLoopingInstance);
    ai.register_source("deathsong", audio::eLoopingInstance);
    ai.register_source("intersong", audio::eLoopingInstance);
    ai.register_source("junglesong", audio::eLoopingInstance);
    ai.register_source("mainsong", audio::eLoopingInstance);
    ai.register_source("menusong", audio::eLoopingInstance);
    ai.register_source("naturesong", audio::eLoopingInstance);
}

bool valid_distance(TPE_Vec3 v) {
    return not (v.x == TPE_INFINITY or v.y == TPE_INFINITY or v.z == TPE_INFINITY);
}

void buf_print(api::DefaultFramebuffer& buffer, TPE_Vec3 v) {
    buffer.write_line("X: %i, Y: %i, Z: %i      ", v.x, v.y, v.z);
}

void buf_print(api::DefaultFramebuffer& buffer, const char* str, TPE_Vec3 v) {
    buffer.write_line("%s: { X: %i, Y: %i, Z: %i }      ", str, v.x, v.y, v.z);
}