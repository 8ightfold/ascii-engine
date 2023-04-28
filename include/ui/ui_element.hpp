#ifndef PROJECT3_TEST_UI_ELEMENT_HPP
#define PROJECT3_TEST_UI_ELEMENT_HPP

#include <concepts>
#include <type_traits>
#include <ui/core.hpp>

#define UI_DECL template <typename Parent, typename ColorMode = char> struct
#define UI_DEF template <typename Parent, typename ColorMode> struct
#define UI_ELEMENT ui::UIElement<Parent, ColorMode>

namespace ui {
    UI_DECL UIElement;

    template <typename Parent, typename ColorMode = char>
    using SafeUIElement = std::unique_ptr<UIElement<Parent, ColorMode>>;

    enum ElementAlignment {
        eCenterX    = 0b001,
        eCenterY    = 0b010,
        eCenter     = 0b011,
    };

    UI_DECL UIFrame {
        using ElementType = SafeUIElement<Parent, ColorMode>;
        using BufferType = api::Framebuffer<ColorMode>;

        explicit UIFrame(UIColor color = {}) : _color(color) {}
        explicit UIFrame(Parent& parent, UIColor color = {}) : _parent(parent), _color(color) {}
        explicit UIFrame(Parent* parent, UIColor color = {}) : _parent(*parent), _color(color) {}
        UIFrame(const UIFrame&) = delete;

        template <typename T, typename...UU>
        NODISCARD T& add_element(const std::string& name, UU&&...uu) {
            using _elem_type = UIElement<Parent, ColorMode>;
            static_assert(std::is_base_of_v<_elem_type, T>);
            assertion(not _locked);

            auto* instance = new T(std::forward<UU>(uu)...);
            _elements.emplace(name, instance);
            return *instance;
        }

        void propagate(Parent* parent) NOEXCEPT {
            assertion(parent, "Parent class was not initialized before propagation.");
            _parent = *parent;
            propagate();
        }

        void propagate() NOEXCEPT {
            assertion(_parent.active(), "Parent class was not bound before propagation.");
            for(auto& pair : _elements)  {
                pair.second->_propagate(*this);
            }
            _locked = true;
        }

        void compile() NOEXCEPT {
            if(not _locked) UNLIKELY propagate();
            for(auto& pair : _elements)  {
                pair.second->compile();
            }
            _compiled = true;
        }

        void render() NOEXCEPT {
            debug_assert(_compiled);
            for(auto& pair : _elements)  {
                pair.second->render();
            }
        }

        void bind(Parent& parent) NOEXCEPT { _parent = parent; }
        void bind(Parent* parent) NOEXCEPT { _parent = *parent; }
        void bind(api::Console& device) NOEXCEPT { _device = device; }
        void bind(BufferType& framebuffer) NOEXCEPT { _framebuffer = framebuffer; }
        void bind(api::Input& input) NOEXCEPT { _input = input; }

        NODISCARD BufferType& framebuffer() NOEXCEPT {
            debug_assert(has_framebuffer());
            return *_framebuffer;
        }

        NODISCARD bool has_parent() CNOEXCEPT { return _parent.active(); }
        NODISCARD bool has_device() CNOEXCEPT { return _device.active(); }
        NODISCARD bool has_framebuffer() CNOEXCEPT { return _framebuffer.active(); }
        NODISCARD bool has_input() CNOEXCEPT { return _input.active(); }
        NODISCARD bool blocking() CNOEXCEPT { return _blocking; }

        NODISCARD api::Coords screen_size() CNOEXCEPT {
            debug_assert(has_device());
            return _device->get_buffer_coords();
        }

    private:
        api::Map<std::string, ElementType> _elements;
        bool _blocking = true;
        UIColor _color = { 1, 0, 255 };

        bool _locked = false;
        bool _compiled = false;

        api::ObjectBinding<Parent> _parent;
        api::ObjectBinding<api::Console> _device;
        api::ObjectBinding<BufferType> _framebuffer;
        api::ObjectBinding<api::Input> _input;
    };


    UI_DEF UIElement {
        using BaseType = UIFrame<Parent, ColorMode>;
        using ElementType = SafeUIElement<Parent, ColorMode>;
        using InternalCallbackType = std::function<void()>;

        UIElement() = default;
        UIElement(const UIElement&) = delete;
        virtual ~UIElement() = default;

        void align(ElementAlignment align, std::same_as<ElementAlignment> auto...extra) {
            _alignment |= align;
            ((_alignment |= extra), ...);
        }

        void set_size(int x, int y) {
            _callback_registry.emplace_back([this, x, y] { this->_set_size(x, y); });
        }

        void set_color(std::uint8_t color) { _color = color; }

        void modify_coords(api::Coords c) {
            _screen_position.set_position(c);
        }

        virtual void compile() {
            for(auto& callback : _callback_registry) callback();
            _callback_registry.clear();
            _texture.resize(_screen_position.area(), _color);
        }

        virtual void render() {
            auto& fb = _base->framebuffer();
            box_copy(fb, _screen_position, _texture);
        }

        virtual bool is_static() {
            return _static;
        }

        void propagate() NOEXCEPT {
            assertion(_base.active());
            for(auto&& [key, element] : _elements) {
                element->_propagate(*_base);
            }
        }

        NODISCARD bool has_base() CNOEXCEPT { return _base.active(); }
        NODISCARD api::Coords position() CNOEXCEPT { return _screen_position.top_left; }

    private:
        static int _center_axis(int screen_dim, int element_dim) NOEXCEPT {
            int offset = (screen_dim / 2) - (element_dim / 2);
            return (offset < 0) ? 0 : offset;
        }

        void _set_size(int x, int y) NOEXCEPT {
            _screen_position.set_size(x, y);

            if(_alignment) {
                debug_assert(has_base() and _base->has_device());
                const api::Coords size = _screen_position.size();
                api::Coords new_pos = _screen_position.top_left;

                if(_alignment & eCenterX)
                    new_pos.y = _center_axis(_base->screen_size().y, size.y);

                if(_alignment & eCenterY)
                    new_pos.x = _center_axis(_base->screen_size().x, size.x);

                _screen_position.set_position(new_pos);
            }
        }

        void _propagate(BaseType& base) NOEXCEPT {
            _base = base;
            propagate();
        }

    protected:
        api::Map<std::string, ElementType> _elements;
        api::ObjectBinding<BaseType> _base;

        std::vector<InternalCallbackType> _callback_registry;
        Rect _screen_position = {};
        std::uint8_t _alignment = 0;
        std::uint8_t _color = 219;

        std::vector<ColorMode> _texture;
        bool _static = true;
        bool _visible = true;

        friend struct UIFrame<Parent, ColorMode>;
    };
}

#endif //PROJECT3_TEST_UI_ELEMENT_HPP
