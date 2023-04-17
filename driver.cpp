#include <api/console.hpp>
#include <api/framebuffer.hpp>
#include <api/keypress_handler.hpp>
#include <api/timer.hpp>

#include <ui/strided_memcpy.hpp>
#include <render/core.hpp>
#include <audio/audiointerface.hpp>

#include <cstdio>
#include <cstdarg>

static void print_controls();
static void print_chars();
static int esprintf(char* string, const char* format, ...);

int main(int argc, char* argv[]) {
    api::Console::initialize();
    auto audio_interface = audio::XAudioInterface::create_interface();

    api::Console window({
            .fullscreen = api::on,
            .font_size = { 16,16 },
            .console_title = "Morpheus, Dorpheus, Orpheus, Go eat some walruses"
    });

    api::Framebuffer<char> framebuffer;
    framebuffer.set_details(window.get_buffer_coords(), 2);
    framebuffer.initialize_buffers();

    api::KeyboardInput keyboard;
    api::KeypressHandlerFactory khf(keyboard);
    api::MouseInput mouse;
    api::Timer timer, seconds;
    constexpr double framerate_ms = 1000.0 / 120.0;

    audio_interface.register_source("wind", audio::eLoopingInstance);
    audio_interface.register_source("walking");
    audio_interface.register_source("boing", audio::eCircularInstance);

    const api::KeypressHandler ESCAPE = khf(VK_ESCAPE, api::eSingle);
    const api::KeypressHandler RESET = khf('R', api::eSingle);

    DEBUG_ONLY( print_chars(); )
    std::cout << "Press 'Return' to continue...";
    while(not keyboard.get_key_pressed(VK_RETURN)) {
        if(ESCAPE()) std::exit(0);
    }
    framebuffer.swap_buffers();

    audio_interface.start_source("wind");
    audio_interface.start_source("walking");

    auto screen_coords = (api::dVec2)(window.get_screen_coords() + 1) / (api::dVec2)framebuffer.get_coords();

    api::BufferType<char> buffer;
    buffer.resize(framebuffer->area());

    unsigned char I = 0;
    unsigned int frames = 0;
    double frame_time = framerate_ms;
    auto dur = api::ms_duration(framerate_ms);

    timer.start(); seconds.start();
    while(not ESCAPE()) {
        auto active_buffer = framebuffer.get_active_buffer();
        window.reset_keystate();

        active_buffer->set_buffer_data(I);
        esprintf(active_buffer->raw_data(), "count: %i; seconds: %f", I, (seconds.elapsed_ms() / 1000.0));
        framebuffer.post_buffer();

        if(I == 6) I = 11;
        else if(I == 12) I = 14;
        else ++I;

        ++frames;

        timer.stop();
        dur = api::ms_duration(framerate_ms - timer.elapsed_ms());
        std::this_thread::sleep_for(dur);

        if(RESET()) {
            audio_interface.stop_source("walking");
            audio_interface.start_source("boing");
            I = 0;
            frames = 0;
            seconds.restart();
        }

        timer.restart();
    }
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