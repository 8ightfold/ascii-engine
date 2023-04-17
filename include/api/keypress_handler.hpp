#ifndef PROJECT3_TEST_KEYPRESS_HANDLER_HPP
#define PROJECT3_TEST_KEYPRESS_HANDLER_HPP

#include <api/core.hpp>
#include <api/input.hpp>

namespace api {
    enum PressType {
        eKeystroke,
        eContinuous,
        eSingle
    };

    struct KeypressHandler {
    private:
        KeypressHandler(ObjectBinding<KeyboardInput>& k, int scancode, PressType mode)
                : _keyboard(k), _scancode(scancode), _mode(mode) {}

    public:
        NODISCARD bool operator()() CNOEXCEPT;
        void rebind(KeyboardInput& keyboard) NOEXCEPT;
        NODISCARD PressType get_type() CNOEXCEPT;
        NODISCARD int get_scancode() CNOEXCEPT;

    private:
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


#endif //PROJECT3_TEST_KEYPRESS_HANDLER_HPP
