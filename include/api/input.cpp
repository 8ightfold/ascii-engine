#include "input.hpp"

namespace api {
    bool KeyboardInput::get_key_pressed(int scancode) CNOEXCEPT {
        return (GetAsyncKeyState(scancode) & api::pressed);
    }

    bool KeyboardInput::get_key_held(int scancode) CNOEXCEPT {
        return (GetAsyncKeyState(scancode) & api::held);
    }

    bool KeyboardInput::get_key_active(int scancode) CNOEXCEPT {
        auto state = GetAsyncKeyState(scancode);
        return (state & api::pressed) || (state & api::held);
    }


    Coords MouseInput::get_screen_position() CNOEXCEPT {
        POINT p;
        GetCursorPos(&p);
        return { p.x, p.y };
    }

    void MouseInput::set_screen_position(Coords c) CNOEXCEPT {
        SetCursorPos(c.x, c.y);
    }


    CursorHider::CursorHider() {
        MAYBE_UNUSED const static int return_code = _register_restore();
        auto filepath = ResourceLocator::get_file("cursors/hidden.cur");
        HANDLE noCursorHandle = LoadCursorFromFileW(filepath.c_str());
        HCURSOR noCursor = CopyCursor(noCursorHandle);
        SetSystemCursor(noCursor, OCR_NORMAL);
    }

    CursorHider::~CursorHider() {
        _restore_cursor();
    }

    int CursorHider::_register_restore() {
        const int return_code = std::atexit(_restore_cursor);
        assertion(return_code == 0);
        return return_code;
    }

    void CursorHider::_restore_cursor() {
        auto filepath = ResourceLocator::get_file("cursors/dark.cur");
        HANDLE darkCursorHandle = LoadCursorFromFileW(filepath.c_str());
        HCURSOR darkCursor = CopyCursor(darkCursorHandle);
        SetSystemCursor(darkCursor, OCR_NORMAL);
    }
}
