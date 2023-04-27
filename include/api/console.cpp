#include "console.hpp"

#define WAIT_FOREVER() do { while(volatile bool b = true) {} } while(0)

namespace api {
    Console::Console(const ConsoleOptions& opt):
    //_console_handle(_get_console_window()),
    //_cin_handle(GetStdHandle(STD_INPUT_HANDLE)), _cout_handle(GetStdHandle(STD_OUTPUT_HANDLE)),
    _console_size(_get_console_size(opt)), _font_res(opt.font_size),
    _console_res({ _console_size.x / _font_res.x, _console_size.y / _font_res.y }),
    _console_title(opt.console_title), _font_name(opt.font) {
        set_all(opt);
    }

    void Console::initialize() noexcept {
        if(GetConsoleWindow() == NULL) {
            $invoke_winapi(FreeConsole, 0)();
            if(not AllocConsole()) {
                FATAL("AllocConsole() failed.");
            }
        }
    }

    void Console::set_all(const ConsoleOptions& opt) noexcept {
        _console_buffer_handle = _allocate_buffer();
        SetConsoleTitle(_console_title.c_str());

        disable_selection();
        set_fullscreen_state(opt.fullscreen);
        set_cursor_state(off);
        set_scrollbar_state(off);
        set_console_font(_font_name);

        update_buffer();
        _console_res.y = _console_res.y + 1;

        COORD screen_buffer_size = _console_res;
        $invoke_winapi(SetConsoleScreenBufferSize, 0)(_console_buffer_handle, screen_buffer_size);
    }

    void Console::update_buffer() NOEXCEPT {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(_cout_handle, &csbi);
        _console_res = _get_small_rect_resolution(csbi.srWindow);
    }

    void Console::set_keystate(api::Coords pos) NOEXCEPT {
        SetConsoleCursorPosition(_cout_handle, { (SHORT)pos.x, (SHORT)pos.y });
    }

    void Console::reset_keystate() NOEXCEPT {
        SetConsoleCursorPosition(_cout_handle, { 0,0 });
    }

    void Console::set_cursor_state(toggle state) NOEXCEPT {
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = state;
        $invoke_winapi(SetConsoleCursorInfo, 0)(_cout_handle, &info);

        _cursor_state = state;
    }

    void Console::set_scrollbar_state(toggle state) NOEXCEPT {
        $invoke_winapi(ShowScrollBar, 0)(_console_handle, SB_BOTH, state);
        _scrollbar_state = state;
    }

    void Console::set_fullscreen_state(toggle state) NOEXCEPT {
        COORD screen_size;
        if(state == on) {
            if(not SetConsoleDisplayMode(_cout_handle, CONSOLE_FULLSCREEN_MODE, &screen_size)) {
                SetWindowLong(_console_handle, GWL_STYLE,
                              GetWindowLong(_console_handle, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
                SendMessage(_console_handle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
            }
        }

        _fullscreen_state = state;
    }

    void Console::set_console_font(const std::wstring& font_name) NOEXCEPT {
        CONSOLE_FONT_INFOEX cf {
                .cbSize = sizeof(CONSOLE_FONT_INFOEX),
                .dwFontSize = _font_res,
        };
        std::copy(font_name.begin(), font_name.end(), cf.FaceName);

        $invoke_winapi(SetCurrentConsoleFontEx, 0)(_console_buffer_handle, 0, &cf);
        _font_name = font_name;
    }

    void Console::disable_selection() CNOEXCEPT {
        DWORD prev_mode;
        GetConsoleMode(_console_buffer_handle, &prev_mode);
        $invoke_winapi(SetConsoleMode, 0)(
                _cin_handle, ENABLE_EXTENDED_FLAGS |
                             (prev_mode & ~ENABLE_QUICK_EDIT_MODE));
    }

    Coords Console::get_screen_coords() const noexcept {
        return _console_size;
    }

    Coords Console::get_buffer_coords() CNOEXCEPT {
        return _console_res;
    }

    Coords Console::get_font_resolution() CNOEXCEPT {
        return _font_res;
    }

    HANDLE Console::_allocate_buffer() NOEXCEPT {
        _console_buffer_handle =
                $invoke_winapi(CreateConsoleScreenBuffer, INVALID_HANDLE_VALUE)(
                    GENERIC_READ | GENERIC_WRITE, 0,
                    nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
        //SetConsoleActiveScreenBuffer(_console_buffer_handle);
        _console_handle = GetConsoleWindow();
        _cin_handle  = GetStdHandle(STD_INPUT_HANDLE);
        _cout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

        return _console_buffer_handle;
    }

    HWND Console::_get_console_window() NOEXCEPT {
        HWND console = GetConsoleWindow();
        if(not console) {
            FATAL("Console has not been initialized.");
        }

        return console;
    }

    Coords Console::_get_console_size(const ConsoleOptions& opt) NOEXCEPT {
        if(opt.console_size.x == 0 || opt.console_size.y == 0) {
            return device_resolution();
        }
        else return opt.console_size;
    }

    Coords Console::_get_small_rect_resolution(const SMALL_RECT& rect) NOEXCEPT {
        return {
                .x = rect.Right - rect.Left + 1,
                .y = rect.Bottom - rect.Top + 1,
        };
    }
}
