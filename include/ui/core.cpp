#include "core.hpp"

namespace ui {
    void Rect::set_position(int x, int y) {
        zero_orient();
        api::Coords offset { x, y };
        top_left += offset;
        bottom_right += offset;
    }

    void Rect::set_size(int x, int y) {
        bottom_right = api::Coords { x, y } + top_left;
    }

    void Rect::set_position(api::Coords c) { set_position(c.x, c.y); }
    void Rect::set_size(api::Coords c) { set_size(c.x, c.y); }

    void Rect::zero_orient() {
        api::Coords offset = top_left;
        top_left -= offset;
        bottom_right -= offset;
    }

    api::Coords Rect::size() CNOEXCEPT {
        return bottom_right - top_left;
    }

    int Rect::area() CNOEXCEPT {
        api::Coords isize = size();
        return isize.x * isize.y;
    }
}
