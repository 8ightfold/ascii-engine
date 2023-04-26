#ifndef PROJECT3_TEST_INPUT_HPP
#define PROJECT3_TEST_INPUT_HPP

#include <api/core.hpp>
#include <api/console.hpp>

namespace api {
    inline constexpr unsigned short pressed  = 1;
    inline constexpr unsigned short held     = 1 << (bitsof<SHORT> - 1);

    struct KeyboardInput {
        KeyboardInput() = default;

        // Checks if key was pressed since last check (good for text)
        NODISCARD bool get_key_pressed(int scancode) CNOEXCEPT {
            return (GetAsyncKeyState(scancode) & api::pressed);
        }

        // Checks if key is currently down (good for continuous input)
        NODISCARD bool get_key_held(int scancode) CNOEXCEPT {
            return (GetAsyncKeyState(scancode) & api::held);
        }

        // Checks both
        NODISCARD bool get_key_active(int scancode) CNOEXCEPT {
            auto state = GetAsyncKeyState(scancode);
            return (state & api::pressed) || (state & api::held);
        }
    };

    struct MouseInput {
        MouseInput() = default;

        NODISCARD Coords get_screen_position() CNOEXCEPT {
            POINT p;
            GetCursorPos(&p);
            return { p.x, p.y };
        }

        void set_screen_position(Coords c) CNOEXCEPT {
            SetCursorPos(c.x, c.y);
        }
    };
}

#endif //PROJECT3_TEST_INPUT_HPP
