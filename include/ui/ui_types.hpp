#ifndef PROJECT3_TEST_UI_TYPES_HPP
#define PROJECT3_TEST_UI_TYPES_HPP

#include <ui/ui_element.hpp>

namespace ui {
    UI_DECL UIBody : UI_ELEMENT {

    };

    UI_DECL UIType {
        using Element = ui::UIElement<Parent, ColorMode>;
        using Body = ui::UIBody<Parent, ColorMode>;
    };
}

#endif //PROJECT3_TEST_UI_TYPES_HPP
