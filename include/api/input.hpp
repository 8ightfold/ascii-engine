#ifndef PROJECT3_TEST_INPUT_HPP
#define PROJECT3_TEST_INPUT_HPP

#include <api/core.hpp>
#include <api/console.hpp>
#include <api/resource_locator.hpp>

namespace api {
    inline constexpr unsigned short pressed  = 1;
    inline constexpr unsigned short held     = 1 << (bitsof<SHORT> - 1);

    struct KeyboardInput {
        KeyboardInput() = default;

        /// Checks if key was pressed since last check (good for text)
        NODISCARD bool get_key_pressed(int scancode) CNOEXCEPT;

        /// Checks if key is currently down (good for continuous input)
        NODISCARD bool get_key_held(int scancode) CNOEXCEPT;

        /// Checks both
        NODISCARD bool get_key_active(int scancode) CNOEXCEPT;
    };

    struct MouseInput {
        MouseInput() = default;
        NODISCARD Coords get_screen_position() CNOEXCEPT;
        void set_screen_position(Coords c) CNOEXCEPT;
    };

    struct Input : KeyboardInput, MouseInput {};

    struct CursorHider {
        CursorHider();
        ~CursorHider();

    private:
        static int _register_restore();
        static void _restore_cursor();
    };
}

#endif //PROJECT3_TEST_INPUT_HPP
