#include "input.hpp"

namespace api {



    CursorHider::CursorHider() {
        auto filepath = ResourceLocator::get_file("cursors/hidden.cur");
        HANDLE noCursorHandle = LoadCursorFromFileW(filepath.c_str());
        HCURSOR noCursor = CopyCursor(noCursorHandle);
        SetSystemCursor(noCursor, OCR_NORMAL);
    }

    CursorHider::~CursorHider() {
        auto filepath = ResourceLocator::get_file("cursors/dark.cur");
        HANDLE darkCursorHandle = LoadCursorFromFileW(filepath.c_str());
        HCURSOR darkCursor = CopyCursor(darkCursorHandle);
        SetSystemCursor(darkCursor, OCR_NORMAL);
    }
}
