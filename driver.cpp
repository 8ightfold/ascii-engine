#include <api/console.hpp>
#include <api/framebuffer.hpp>
#include <api/input.hpp>
#include <api/timer.hpp>

#include <ui/strided_memcpy.hpp>
#include <engine/core.hpp>

#include <cstdio>
#include <cstdarg>

#define CHAIN_HEAD() if(false) {}

static void print_controls();
static void print_chars();
static int esprintf(char* string, const char* format, ...);

namespace api {
    enum PressType {
        eKeystroke,
        eContinuous,
        eSingle
    };

    struct KeypressHandler {
        NODISCARD bool operator()() CNOEXCEPT {
            switch(_mode) {
                case PressType::eKeystroke: {
                    return _keyboard->get_key_pressed(_scancode);
                }
                case PressType::eContinuous: {
                    return _keyboard->get_key_active(_scancode);
                }
                case PressType::eSingle: {
                    bool bpressed = _keyboard->get_key_pressed(_scancode);
                    if(not _pressed && bpressed) {
                        _pressed = true;
                        return true;
                    }
                    else if(_pressed && not bpressed) {
                        _pressed = false;
                    }

                    return false;
                }
            }
            return false;
        }

        void rebind(KeyboardInput& keyboard) NOEXCEPT {
            _keyboard = keyboard;
            _pressed = false;
        }

        NODISCARD PressType get_type() CNOEXCEPT {
            return _mode;
        }

        NODISCARD int get_scancode() CNOEXCEPT {
            return _scancode;
        }

    private:
        KeypressHandler(ObjectBinding<KeyboardInput>& k, int scancode, PressType mode)
        : _keyboard(k), _scancode(scancode), _mode(mode) {}

        ObjectBinding<KeyboardInput> _keyboard;
        int _scancode;
        PressType _mode;
        mutable bool _pressed = false;

        friend struct KeypressHandlerFactory;
    };

    struct KeypressHandlerFactory {
        KeypressHandlerFactory(KeyboardInput& k) : _keyboard(k) {}

        NODISCARD KeypressHandler operator()(int scancode, PressType mode = PressType::eSingle) NOEXCEPT {
            return { _keyboard, scancode, mode };
        }

    private:
        ObjectBinding<KeyboardInput> _keyboard;
    };
}

int main(int argc, char* argv[]) {
    std::printf("Current dir: %s\n", argv[0]);
    api::Console::initialize();

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
    api::Timer timer;
    constexpr double framerate_ms = 1000.0 / 30.0;

    const api::KeypressHandler ESCAPE = khf(VK_ESCAPE, api::eSingle);
    const api::KeypressHandler SAVE_BUFFER = khf('S', api::eSingle);
    const api::KeypressHandler SWAP_BUFFERS = khf('E', api::eSingle);
    const api::KeypressHandler TOGGLE_COORDS = khf('C', api::eSingle);
    const api::KeypressHandler CLEAR_BUFFER = khf('R', api::eSingle);
    const api::KeypressHandler FILL_BUFFER = khf('X', api::eSingle);
    const api::KeypressHandler TOGGLE_POLLING = khf('P', api::eSingle);

    const api::KeypressHandler SAVE_MESSAGE = khf('S', api::eContinuous);
    const api::KeypressHandler DRAW = khf(VK_LBUTTON, api::eContinuous);
    const api::KeypressHandler SCREEN_COORDS = khf('V', api::eContinuous);
    const api::KeypressHandler BUFFER_COORDS = khf('B', api::eContinuous);

    DEBUG_ONLY( print_chars(); )
    print_controls();
    while(not keyboard.get_key_pressed(VK_RETURN)) {
        if(ESCAPE()) std::exit(0);
    }
    framebuffer.swap_buffers();

    auto screen_coords = (api::dVec2)(window.get_screen_coords() + 1) / (api::dVec2)framebuffer.get_coords();
    bool display_coords = true;
    bool do_constant_polling = false;
    api::Coords disp_coords {};

    api::BufferType<char> buffer;
    buffer.resize(framebuffer->area());

    while(not ESCAPE()) {
        auto active_buffer = framebuffer.get_active_buffer();
        window.reset_keystate();
        timer.restart();

        if(TOGGLE_COORDS()) {
            display_coords = !display_coords;
        }

        if(CLEAR_BUFFER()) {
            active_buffer.get().set_buffer_data(255);
        }

        if(FILL_BUFFER()) {
            active_buffer.get().set_buffer_data(176);
        }

        if(DRAW()) {
            api::Coords pos = mouse.get_screen_position();
            pos /= screen_coords;
            active_buffer.get()[pos] = 219;
            disp_coords = pos;
        }
        else disp_coords = mouse.get_screen_position();

        if(SWAP_BUFFERS()) {
            std::putchar('\a');
            active_buffer = framebuffer.swap_buffers();
            framebuffer.sync_buffers();
        }
        framebuffer.post_buffer();

        if(SAVE_BUFFER()) {
            framebuffer.sync_buffers();
        }

        CHAIN_HEAD()
        else if(SAVE_MESSAGE()) {
            window.reset_keystate();
            api::Coords pos = framebuffer.get_coords();
            printf("SAVED...    ");
        }
        else if(BUFFER_COORDS()) {
            window.reset_keystate();
            api::Coords pos = framebuffer.get_coords();
            printf("BUFFER: { %i, %i }   ", pos.x, pos.y);
        }
        else if(SCREEN_COORDS()) {
            window.reset_keystate();
            api::Coords pos = window.get_screen_coords();
            printf("SCREEN: { %i, %i }   ", pos.x, pos.y);
        }
        else if(display_coords) {
            window.reset_keystate();
            printf("COORDS: { %i, %i }   ", disp_coords.x, disp_coords.y);
        }

        if(TOGGLE_POLLING()) {
            do_constant_polling = !do_constant_polling;
        }

        if(do_constant_polling) {
            while(framerate_ms > timer.elapsed_ms()) {
                if(DRAW()) {
                    api::Coords pos = mouse.get_screen_position();
                    pos /= screen_coords;
                    active_buffer.get()[pos] = 219;
                }
            }
            window.reset_keystate();
        }
        else {
            timer.stop();
            auto dur = api::ms_duration(framerate_ms - timer.elapsed_ms());
            std::this_thread::sleep_for(dur);
        }
    }
}


static void print_controls() {
    std::cout << "CONTROLS:\n";
    std::cout << "Save frame: 'S'\n";
    std::cout << "Load frame: 'E'\n";
    std::cout << "Reset frame: 'R'\n";
    std::cout << "Toggle coords: 'C'\n";
    std::cout << "Display screen coords: 'V'\n";
    std::cout << "Display buffer coords: 'B'\n";
    std::cout << "Fill buffer: 'X'\n";
    std::cout << "Press return to begin..." << std::endl;
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