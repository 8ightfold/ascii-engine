#ifndef PROJECT3_TEST_UI_TYPES_HPP
#define PROJECT3_TEST_UI_TYPES_HPP

#include <ui/ui_element.hpp>

#define UI_ELEMENT_T(type) typename UI_ELEMENT::type
#define UI_CUSTOM(name) name <Parent, ColorMode>
#define UI_CUSTOM_T(name, type) typename UI_CUSTOM(name)::type

namespace ui {
    UI_DECL UIBody : UI_CUSTOM(UIElement) {};

    UI_DECL UIButton : UI_ELEMENT {
        using ThisType = UI_CUSTOM(UIButton);

        void compile() override {
            this->_run_registry();
            this->_texture_resize();

            if(not _on_hover) {
                _on_hover = [](ThisType* elem) {
                    elem->_default_hover();
                };
            }

            if(not _on_click) {
                _on_click = [](ThisType* elem) {
                    elem->_base->framebuffer().write_line("Clicked!");
                };
            }

            _cached_color = this->_color;
            this->_set_compiled();
        }

        void tick() NOEXCEPT override {
            bool modified = false;

            auto& fb = this->_base->framebuffer();
            box_copy(fb, this->_screen_position, this->_texture);
            box_copy(fb, this->_text_position, this->_text);

            _check_hover_click();
            if(_last_hover != _hovered) {
                modified = true;
                _last_hover = _hovered;
            }

            if(_hovered and modified) _on_hover(this);
            else if (modified) {
                this->_color = _cached_color;
                this->_texture_fill();
            }

            if(_clicked) _on_click(this);
            this->_tick_children();
        }

        void set_size(int x, int y) override {
            assertion(not this->_locked);
            this->_push_callback([this, x, y] {
                this->_set_size(x, y);
                _set_text_size();
            });
        }

        void set_size(double x, double y) override {
            this->_push_callback([this, x, y] {
                auto [dx, dy] = this->_base->framebuffer()->get_coords();
                this->_set_size(dx * x, dy * y);
                _set_text_size();
            });
        }

        void set_position(int x, int y) override {
            this->_push_callback([this, x, y] {
                this->_set_position(x, y);
                _set_text_size();
            });
        }

        void set_position(double x, double y) override {
            this->_push_callback([this, x, y] {
                auto [dx, dy] = this->_base->framebuffer()->get_coords();
                auto nx = (x + 1.0) / 2.0;
                auto ny = (-y + 1.0) / 2.0;
                this->_set_position(dx * nx, dy * ny);
                _set_text_size();
            });
        }

        void set_scale(int s) {
            _text_size_in = (s < 0) ? 0 : s;
        }

        void on_click(auto func) NOEXCEPT {
            _on_click = func;
        }

        void set_text(std::string text) {
            assertion(not this->_locked);
            this->_push_callback([this, text] {
                _set_text_size();
                _set_text_data(text);
            });
        }

    protected:
        void _set_text_data(const std::string& text) NOEXCEPT {
            auto strings = split_string(text, '\n');
            auto [x, y] = this->_text_position.size();
            if(not (x and y)) return;

            if(strings.front().empty()) strings.pop_front();
            if(strings.back().empty()) strings.pop_back();

            std::size_t largest_string = 0;
            for(const auto& string : strings) {
                const std::size_t size = string.size();
                largest_string = (size > largest_string) ? size : largest_string;
            }

            largest_string = (largest_string > x) ? x : largest_string;
            ColorMode* data = _text.data();

            for(int idx = 0; idx < strings.size(); ++idx) {
                data += x;
                _scale_string(strings[idx], largest_string);
                debug_assert(strings[idx].size() == largest_string, "string '" + strings[idx] + "' resize failed.");
                std::copy(strings[idx].begin(), strings[idx].end(), data);
            }
        }

        void _set_text_size() NOEXCEPT {
            _text_position = this->_screen_position;
            _text_position.shrink(_text_size_in);
            _text.resize(_text_position.area(), 255);
        }

        void _check_hover_click() NOEXCEPT {
            static api::KeypressHandlerFactory khf(this->_base->input());
            static api::KeypressHandler CLICKED = khf(VK_LBUTTON, api::eSingle);

            auto& input = this->_base->input();
            api::Coords screen_pos = this->mouse_position();

            _hovered = this->_screen_position.intersects(screen_pos);
            _clicked = _hovered and CLICKED();
        }

        void _default_hover() NOEXCEPT {
            _cached_color = this->_color;
            this->set_color({ .luminance = 219 });
            this->_texture_fill();
        }

    private:
        void remove_spaces(std::string& str , std::size_t string_length) {
            std::size_t leading = str.find_first_not_of(' ');
            std::size_t trailing = str.find_last_not_of(' ');
            trailing = (trailing != std::string::npos) ? trailing + 1 : str.size();

            str = str.substr(leading, trailing - leading);
            if(str.size() > string_length) str.resize(string_length);
        }

        void _scale_string(std::string& str, std::size_t string_length) {
            if(this->_alignment & eTextLeft) {
                this->_base->framebuffer().write_line("left");
                str.resize(string_length, ' ');
            }
            else if(this->_alignment & eTextCenter) {
                this->_base->framebuffer().write_line("center");
                remove_spaces(str, string_length);
                std::size_t scale = (string_length - str.size()) / 2;
                std::string scale_by(scale, ' ');
                str = scale_by + str + scale_by;
                str.resize(string_length, ' ');
            }
            else if(this->_alignment & eTextRight) {
                this->_base->framebuffer().write_line("right");
                remove_spaces(str, string_length);
                std::size_t scale = string_length - str.size();
                str = std::string(scale, ' ') + str;
            }
        }

    protected:
        std::function<void(ThisType*)> _on_hover;
        std::function<void(ThisType*)> _on_click;
        UIColor _cached_color;

        int _text_size_in = 1;
        Rect _text_position = {};
        std::vector<ColorMode> _text;

        bool _hovered = false;
        bool _last_hover = false;
        bool _clicked = false;
    };

    UI_DECL UIType {
        using Element = ui::UIElement<Parent, ColorMode>;
        using Body = ui::UIBody<Parent, ColorMode>;
        using Button = ui::UIButton<Parent, ColorMode>;
    };
}

#endif //PROJECT3_TEST_UI_TYPES_HPP
