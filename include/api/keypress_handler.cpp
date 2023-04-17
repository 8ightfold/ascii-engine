#include "keypress_handler.hpp"

namespace api {
    NODISCARD bool KeypressHandler::operator()() CNOEXCEPT {
        switch(_mode) {
            case PressType::eKeystroke: {
                return _keyboard->get_key_pressed(_scancode);
            }
            case PressType::eContinuous: {
                return _keyboard->get_key_active(_scancode);
            }
            case PressType::eSingle: {
                bool bpressed = _keyboard->get_key_pressed(_scancode);
                bool bheld = _keyboard->get_key_active(_scancode);
                if(not _pressed && bpressed) {
                    _pressed = true;
                    return true;
                }
                else if(_pressed && not bheld) {
                    _pressed = false;
                }

                return false;
            }
        }
        return false;
    }

    void KeypressHandler::rebind(KeyboardInput& keyboard) NOEXCEPT {
        _keyboard = keyboard;
        _pressed = false;
    }

    NODISCARD PressType KeypressHandler::get_type() CNOEXCEPT {
        return _mode;
    }

    NODISCARD int KeypressHandler::get_scancode() CNOEXCEPT {
        return _scancode;
    }
}
