#ifndef PROJECT3_TEST_FRAMEBUFFER_HPP
#define PROJECT3_TEST_FRAMEBUFFER_HPP

#include <functional>
#include <memory>
#include <span>
#include <vector>

#include <api/core.hpp>

#define API_STRIDE(cd, rs) (cd.x + (cd.y * rs.x))
#define API_WRAP(value, max) do { value = (value >= max) ? 0 : (value); } while(0)

namespace api {
    template <typename T>
    struct Buffer {
        using _buffer_t = std::vector<T>;

        Buffer() : _console_res({ 0,0 }) { _init_buffer(); }
        Buffer(Coords res) : _console_res(res) { _init_buffer(); }
        Buffer(Coords res, T v) : _console_res(res) { _init_buffer(v); }

        void resize_buffer(Coords new_res) NOEXCEPT {
            this->_console_res = new_res;
            _realloc_buffer();
        }

        void set_buffer_data(const T& v) NOEXCEPT {
            std::fill(_buffer.begin(), _buffer.end(), v);
        }

        void set_value(Coords pos, const T& v) NOEXCEPT {
            _buffer[API_STRIDE(pos, _console_res)] = v;
        }

        T& operator[](Coords pos) NOEXCEPT {
            int off = API_STRIDE(pos, _console_res);
            if(off > _buffer.size()) {
                return _buffer[0];
            }
            return _buffer[off];
        }

        std::span<T> get_buffer_data() NOEXCEPT {
            return { _buffer.data(), _buffer.size() };
        }

        T* raw_data() NOEXCEPT {
            return _buffer.data();
        }

        NODISCARD T get_value(Coords pos) CNOEXCEPT {
            return _buffer[API_STRIDE(pos, _console_res)];
        }

        NODISCARD Coords get_coords() CNOEXCEPT {
            return _console_res;
        }

        NODISCARD int get_x() CNOEXCEPT {
            return _console_res.x;
        }

        NODISCARD int get_y() CNOEXCEPT {
            return _console_res.y;
        }

        NODISCARD int area() CNOEXCEPT {
            return _console_res.area();
        }


    private:
        void _init_buffer() NOEXCEPT {
            _buffer =_buffer_t(_console_res.area(), T{});
        }

        void _init_buffer(const T& v) NOEXCEPT {
            _buffer = _buffer_t(_console_res.area(), v);
        }

        void _realloc_buffer() NOEXCEPT {
            _buffer.resize(_console_res.area());
        }

    private:
        _buffer_t _buffer;
        Coords _console_res;
    };


    template <typename T>
    using WrappedBuffer = std::reference_wrapper<Buffer<T>>;


    template <typename T>
    struct Framebuffer {
        enum class State {
            eInvalid,
            eNormal,
            eDoResize,
        };

        using _ibuffer_t = Buffer<T>;
        using _wibuffer_t = WrappedBuffer<T>;

        Framebuffer(int count = 0) : _console_res({ 0,0 }), _buffer_count(count) {}
        Framebuffer(Coords coords, int count = 0) : _console_res(coords), _buffer_count(count) {}

        void set_details(Coords coords, int count) NOEXCEPT {
            if(not _running) {
                _console_res = coords;
                _buffer_count = count;
            }
        }

        bool initialize_buffers() NOEXCEPT {
            if(_running) UNLIKELY { return false; }

            if(_console_res.area() && _buffer_count) {
                _buffers = std::vector<_ibuffer_t>(_buffer_count, { _console_res });
                _drawing_buffer = _buffers.size() - 1;
                _cout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
                _running = true;
                return true;
            }
            else {
                debug_printf("initialize_buffers(): Framebuffer passed invalid arguments.");
                return false;
            }
        }

        void update_buffers(Coords coords) NOEXCEPT {
            if(_running && coords.area()) LIKELY {
                _console_res = coords;
                _buffer_state = State::eDoResize;
                _cout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
            }
            else UNLIKELY {
                debug_printf("update_buffers(Coords coords): Framebuffer passed invalid arguments.");
                _buffer_state = State::eInvalid;
            }
        }

        _ibuffer_t* operator->() NOEXCEPT {
            if(_running) LIKELY {
                return &_buffers[_selected_buffer];
            }
            else UNLIKELY {
                assert(!"operator->(): Framebuffers have not been initialized.");
                unreachable();
            }
        }

        _wibuffer_t get_active_buffer() NOEXCEPT {
            if(_running) LIKELY {
                return { _buffers[_selected_buffer] };
            }
            else UNLIKELY {
                assert(!"get_active_buffer(): Framebuffers have not been initialized.");
                unreachable();
            }
        }

        T* get_raw_buffer_data() NOEXCEPT {
            return get_active_buffer().get().raw_data();
        }

        State post_buffer() NOEXCEPT {
            auto buffer_data = _buffers[_selected_buffer].get_buffer_data();
            fwrite(buffer_data.data(), sizeof(T), buffer_data.size(), stdout);
            SetConsoleCursorPosition(_cout_handle, { 0,0 });

            if(_buffer_state == State::eDoResize) UNLIKELY {
                return _update_buffers();
            }

            return _buffer_state;
        }

        _wibuffer_t swap_buffers() NOEXCEPT {
            ++_selected_buffer, ++_drawing_buffer;
            API_WRAP(_selected_buffer, _buffers.size());
            API_WRAP(_drawing_buffer, _buffers.size());
            return _buffers[_selected_buffer];
        }

        void sync_buffers() NOEXCEPT {
            _buffers[_drawing_buffer] = _buffers[_selected_buffer];
        }

        NODISCARD Coords get_coords() CNOEXCEPT {
            return _console_res;
        }

        NODISCARD int get_buffer_count() CNOEXCEPT {
            return _buffer_count;
        }

    private:
        State _update_buffers() NOEXCEPT {
            for(_ibuffer_t& buf : _buffers) {
                buf.resize_buffer(_console_res);
            }

            _buffer_state = State::eNormal;
            return State::eDoResize;
        }

        std::vector<_ibuffer_t> _buffers;
        Coords _console_res;
        int _buffer_count = 0;
        HANDLE _cout_handle;

        int _selected_buffer = 0;
        int _drawing_buffer = 0;

        bool _running = false;
        State _buffer_state = State::eNormal;
    };

    template <typename T>
    using FramebufferBinding = ObjectBinding<Framebuffer<T>>;

    template <typename T>
    using BufferType = typename Buffer<T>::_buffer_t;
}

#endif //PROJECT3_TEST_FRAMEBUFFER_HPP
