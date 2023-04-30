#ifndef PROJECT3_TEST_CONSOLE_HPP
#define PROJECT3_TEST_CONSOLE_HPP

#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#include <api/core.hpp>

namespace api {
    struct ConsoleOptions {
        toggle fullscreen = off;
        toggle visible_cursor = off;
        Coords console_size = { 0, 0 };
        Coords font_size = { 4, 4 };
        std::string console_title = "console";
        std::wstring font = L"Terminal";
    };

    struct Console {
        explicit Console(const ConsoleOptions& opt = {});

        Console(const Console&) = delete;
        Console(Console&&) = delete;

        static void initialize() NOEXCEPT;

        void set_all(const ConsoleOptions& opt = {}) NOEXCEPT;
        void update_buffer() NOEXCEPT;
        void set_keystate(api::Coords pos) NOEXCEPT;
        DEPRECATED void reset_keystate() NOEXCEPT;

        void set_cursor_state(toggle state) NOEXCEPT;
        void set_scrollbar_state(toggle state) NOEXCEPT;
        void set_fullscreen_state(toggle state) NOEXCEPT;
        void set_console_font(const std::wstring& font_name) NOEXCEPT;
        void disable_selection() CNOEXCEPT;

        NODISCARD Coords get_screen_coords() CNOEXCEPT;
        NODISCARD Coords get_buffer_coords() CNOEXCEPT;
        NODISCARD Coords get_font_resolution() CNOEXCEPT;

    private:
        NODISCARD static HWND _get_console_window() NOEXCEPT;
        static Coords _get_console_size(const ConsoleOptions& opt) NOEXCEPT;
        static Coords _get_small_rect_resolution(const SMALL_RECT& rect) NOEXCEPT;
        HANDLE _allocate_buffer() NOEXCEPT;

    private:
        HWND _console_handle;
        HANDLE _console_buffer_handle;
        HANDLE _cin_handle;
        HANDLE _cout_handle;

        Coords _console_size;               // Physical size of the console
        Coords _font_res;                   // Scale of the font
        Coords _console_res;                // Character resolution

        toggle _cursor_state;
        toggle _scrollbar_state;
        toggle _fullscreen_state;

        std::string _console_title;         // Name of the console
        std::wstring _font_name;            // Font value
    };
}

#endif //PROJECT3_TEST_CONSOLE_HPP
