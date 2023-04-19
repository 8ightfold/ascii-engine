#ifndef PROJECT3_TEST_UI_CORE_HPP
#define PROJECT3_TEST_UI_CORE_HPP

#include <api/core.hpp>
#include <api/framebuffer.hpp>
#include <ui/strided_memcpy.hpp>

namespace ui {
    struct Rect {
        api::Coords top_left;
        api::Coords bottom_right;
    };


}

#endif //PROJECT3_TEST_UI_CORE_HPP
