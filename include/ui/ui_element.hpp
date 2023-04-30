#ifndef PROJECT3_TEST_UI_ELEMENT_HPP
#define PROJECT3_TEST_UI_ELEMENT_HPP

#include <concepts>
#include <type_traits>
#include <ui/core.hpp>

#define UI_DECL template <typename Parent, typename ColorMode = char> struct
#define UI_DEF template <typename Parent, typename ColorMode> struct
#define UI_ELEMENT ui::UIElement<Parent, ColorMode>

namespace ui {
    UI_DECL UIFrame;
    UI_DECL UIElement;

    template <typename Parent, typename ColorMode = char>
    using SafeUIFrame = std::unique_ptr<UIFrame<Parent, ColorMode>>;

    template <typename Parent, typename ColorMode = char>
    using SafeUIElement = std::unique_ptr<UIElement<Parent, ColorMode>>;

    enum ElementAlignment {
        eCenterX        = 0x1,         /// Center element vertically
        eCenterY        = 0x2,         /// Center element horizontally
        eCenter         = (0x1 | 0x2), /// Center element vertically and horizontally
        eTextLeft       = 0x4,
        eTextCenter     = 0x8,
        eTextRight      = 0x10,
        eTextSuspend    = 0x20,
        eJustify FDEPRECATED("Unimplemented") = 0,
    };

    using ElementAlignmentType = std::underlying_type_t<ElementAlignment>;

    UI_DECL UIFrameManager {
        using FrameType = ui::SafeUIFrame<Parent>;
        using RawFrameType = ui::UIFrame<Parent>;

        template <typename...TT>
        RawFrameType* add_frame(const std::string& name, TT&&...tt) NOEXCEPT {
            auto* instance = new RawFrameType(std::forward<TT>(tt)...);
            instance->_manager = *this;
            _frame_registry.emplace(name, instance);
            return instance;
        }

        RawFrameType* frame(const std::string& name) NOEXCEPT {
            debug_assert(_frame_registry.contains(name), "No frame named '" + name + "' registered.");
            return _frame_registry.at(name).get();
        }

        void bind(Parent* parent, const std::string& entry_point) {
            _parent = parent;
            _entry_point = entry_point;

            for(auto& frame_pair : _frame_registry) {
                frame_pair.second->propagate(_parent);
                frame_pair.second->compile();
            }

            push(_entry_point);
        }

        void push(const std::string& name) { if(_push_next.empty()) _push_next = name; }
        void pop() { _pop_top = true; }

        bool operator()() NOEXCEPT {
            _update_queues();
            if(_frame_queue.empty()) return false;
            _frame_queue.back()->tick();
            return true;
        }

    private:
        void _update_queues() {
            if(_pop_top) {
                if(not _frame_queue.empty()) {
                    _frame_queue.pop_back();
                    _frame_name_queue.pop_back();
                }
                _pop_top = false;
            }

            if(not _push_next.empty()) {
                auto* instance = _frame_registry.at(_push_next).get();
                _frame_queue.push_back(instance);
                _frame_name_queue.push_back(_push_next);
                _push_next = "";
            }
        }

    private:
        Parent* _parent = nullptr;
        api::Map<std::string, FrameType> _frame_registry;
        std::string _entry_point;

        std::deque<RawFrameType*> _frame_queue;
        std::deque<std::string> _frame_name_queue;

        std::string _push_next;
        bool _pop_top = false;
    };


    UI_DEF UIFrame {
        using ManagerType = UIFrameManager<Parent, ColorMode>;
        using ElementType = SafeUIElement<Parent, ColorMode>;
        using BufferType = api::Framebuffer<ColorMode>;

        explicit UIFrame(UIColor color = {}) : _color(color) {}
        explicit UIFrame(Parent& parent, UIColor color = {}) : _parent(parent), _color(color) {}
        explicit UIFrame(Parent* parent, UIColor color = {}) : _parent(*parent), _color(color) {}
        UIFrame(const UIFrame&) = delete;

        template <typename T, typename...UU>
        NODISCARD T* add_element(const std::string& name, UU&&...uu) {
            static_assert(std::is_base_of_v<UI_ELEMENT, T>);
            assertion(not _locked);

            auto* instance = new T(std::forward<UU>(uu)...);
            _elements.emplace(name, instance);
            return instance;
        }

        UI_ELEMENT* element(const std::string& name) {
            if(not _elements.contains(name)) FATAL("Unregistered element '" + name + "'");
            return _elements.at(name).get();
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
            debug_assert(has_framebuffer());
            if(not _locked) UNLIKELY propagate();
            for(auto& pair : _elements)  {
                pair.second->compile();
            }
            _compiled = true;
        }

        void tick() NOEXCEPT {
            debug_assert(_compiled);
            auto active_buffer = _framebuffer->get_active_buffer();

            if(_color.alpha == 255) {
                // TODO: Add color picker
                auto fill = _color.luminance;
                active_buffer->set_buffer_data(fill);
            }
            else if(not _texture.empty()) {
                active_buffer->set_buffer_texture(_texture);
            }

            for(auto& func : _key_intercepts) { func(); }

            for(auto& pair : _elements)  {
                pair.second->tick();
            }
        }

        void callback(auto func) NOEXCEPT {
            _key_intercepts.emplace_back(func);
        }

        void push(const std::string& name) NOEXCEPT { _manager->push(name); }
        void pop() NOEXCEPT { _manager->pop(); }

        void bind(Parent& parent) NOEXCEPT { _parent = parent; }
        void bind(Parent* parent) NOEXCEPT { _parent = *parent; }
        void bind(api::Console& device) NOEXCEPT { _device = device; }
        void bind(BufferType& framebuffer) NOEXCEPT { _framebuffer = framebuffer; }
        void bind(api::Input& input) NOEXCEPT { _input = input; }

        NODISCARD BufferType& framebuffer() NOEXCEPT {
            debug_assert(has_framebuffer());
            return *_framebuffer;
        }

        NODISCARD api::Console& device() NOEXCEPT {
            debug_assert(has_device());
            return *_device;
        }

        NODISCARD api::Input& input() NOEXCEPT {
            debug_assert(has_input());
            return *_input;
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
        UIColor _color = { 1, 255, 255 };
        std::vector<ColorMode> _texture;
        std::vector<std::function<void()>> _key_intercepts;

        bool _locked = false;
        bool _compiled = false;

        api::ObjectBinding<Parent> _parent;
        api::ObjectBinding<ManagerType> _manager;
        api::ObjectBinding<api::Console> _device;
        api::ObjectBinding<BufferType> _framebuffer;
        api::ObjectBinding<api::Input> _input;

        friend struct UIFrameManager<Parent, ColorMode>;
    };


    UI_DEF UIElement {
        using BaseType = UIFrame<Parent, ColorMode>;
        using ElementType = SafeUIElement<Parent, ColorMode>;
        using InternalCallbackType = std::function<void()>;

        UIElement() = default;
        UIElement(const UIElement&) = delete;
        virtual ~UIElement() = default;

        template <typename T, typename...UU>
        NODISCARD T* add_element(const std::string& name, UU&&...uu) {
            static_assert(std::is_base_of_v<UI_ELEMENT, T>);
            assertion(not _locked);

            auto* instance = new T(std::forward<UU>(uu)...);
            _elements.emplace(name, instance);
            return instance;
        }

        UI_ELEMENT* element(const std::string& name) {
            if(not _elements.contains(name)) FATAL("Unregistered element '" + name + "'");
            return _elements.at(name).get();
        }

        void propagate() NOEXCEPT {
            assertion(_base.active());
            for(auto&& [key, element] : _elements) {
                element->_propagate(*_base);
            }
            _locked = true;
        }

        virtual void compile() {
            _run_registry();
            _texture_resize();
            _set_compiled();
        }

        virtual void tick() {
            auto& fb = _base->framebuffer();
            box_copy(fb, _screen_position, _texture);
            _tick_children();
        }

        void align(ElementAlignment align, std::same_as<ElementAlignment> auto...extra) {
            _alignment |= align;
            ((_alignment |= extra), ...);
        }

        /**
         * @param x, y The size in buffer coordinates
         */
        virtual void set_size(int x, int y) {
            _push_callback([this, x, y] {
                this->_set_size(x, y);
            });
        }

        /**
         * @param x, y The relative size in the range [0, 1]
         */
        virtual void set_size(double x, double y) {
            _push_callback([this, x, y] {
                auto [dx, dy] = this->_base->framebuffer()->get_coords();
                this->_set_size(dx * x, dy * y);
            });
        }

        virtual void set_position(int x, int y) {
            _push_callback([this, x, y] {
                this->_set_position(x, y);
            });
        }

        virtual void set_position(double x, double y) {
            _push_callback([this, x, y] {
                auto [dx, dy] = this->_base->framebuffer()->get_coords();
                auto nx = (x + 1.0) / 2.0;
                auto ny = (-y + 1.0) / 2.0;
                this->_set_position(dx * nx, dy * ny);
            });
        }

        void set_color(UIColor color) {
            _color = color;
        }

        void push(const std::string& name) { _base->push(name); }
        void pop() { _base->pop(); }

        virtual bool is_static() { return true; }
        virtual bool is_visible() { return true; }

        NODISCARD api::Coords mouse_position() NOEXCEPT {
            auto& input = _base->input();
            auto& device = _base->device();
            auto& buffer = _base->framebuffer();
            const api::dVec2 error_correct = { .x = 1.0085, .y = 1.009 };

            api::dVec2 screen_coords = device.get_screen_coords() - 1;
            api::dVec2 true_screen_position = input.get_screen_position() - 1;
            api::dVec2 screen_scale = true_screen_position / screen_coords;
            api::dVec2 screen_pos = api::dVec2(buffer.get_coords()) * screen_scale * error_correct;

            api::Coords return_value { };
            return_value = screen_pos;
            return return_value;
        }

        NODISCARD bool has_base() CNOEXCEPT { return _base.active(); }
        NODISCARD api::Coords position() CNOEXCEPT { return _screen_position.top_left; }

    protected:
        static int _center_axis(int screen_dim, int element_dim) NOEXCEPT {
            int offset = (screen_dim / 2) - (element_dim / 2);
            return (offset < 0) ? 0 : offset;
        }

        void _propagate(BaseType& base) NOEXCEPT {
            _base = base;
            propagate();
        }

        void _set_size(int x, int y) NOEXCEPT {
            _screen_position.set_size(x, y);

            if(_alignment & eCenter) {
                const api::Coords size = _screen_position.size();
                api::Coords new_pos = _screen_position.top_left;
                auto [bx, by] = _base_size();

                if(_alignment & eCenterX)
                    new_pos.y = _center_axis(by, size.y);

                if(_alignment & eCenterY)
                    new_pos.x = _center_axis(bx, size.x);

                _screen_position.set_position(new_pos);
            }
            this->_print_position("Size");
        }

        void _set_position(int x, int y) {
            if(_alignment & eCenter) {
                const api::Coords size = _screen_position.size();
                auto [bx, by] = _base_size();

                if(_alignment & eCenterX) y = _center_axis(by, size.y);
                if(_alignment & eCenterY) x = _center_axis(bx, size.x);
            }

            _screen_position.set_position(x, y);
            this->_print_position("Position");
        }

        void _print_position(const std::string& str) {
            auto [tx, ty] = this->_screen_position.top_left;
            auto [bx, by] = this->_screen_position.bottom_right;
            _base->framebuffer().write_line("%s: { %i, %i }, { %i, %i }", str.c_str(), tx, ty, bx, by);
        }

        api::Coords _base_size() NOEXCEPT {
            debug_assert(has_base() and _base->has_device());
            return _base->screen_size();
        }

        void _set_compiled() NOEXCEPT {
            for(auto&& [key, element] : _elements) {
                element->compile();
            }
            _compiled = true;
        }

        void _run_registry() NOEXCEPT {
            for(auto& callback : _callback_registry) callback();
            _callback_registry.clear();
        }

        template <typename F>
        void _push_callback(F&& func) NOEXCEPT {
            _callback_registry.emplace_back(std::forward<F>(func));
        }

        void _texture_resize() NOEXCEPT {
            _texture.resize(_screen_position.area(), _color.luminance);
        }

        void _texture_fill() NOEXCEPT {
            std::fill(_texture.begin(), _texture.end(), _color.luminance);
        }

        void _tick_children() {
            for(auto& pair : _elements)  {
                pair.second->tick();
            }
        }

    protected:
        api::Map<std::string, ElementType> _elements;
        api::ObjectBinding<BaseType> _base;

        std::vector<InternalCallbackType> _callback_registry;
        Rect _screen_position = {};
        ElementAlignmentType _alignment = 0;
        UIColor _color = {};
        std::vector<ColorMode> _texture;

        bool _locked = false;
        bool _compiled = false;

        friend struct UIFrame<Parent, ColorMode>;
    };
}

#endif //PROJECT3_TEST_UI_ELEMENT_HPP
