#ifndef PROJECT3_TEST_FRAMEBUFFER_HPP
#define PROJECT3_TEST_FRAMEBUFFER_HPP

#include <algorithm>
#include <cstdarg>
#include <memory>
#include <span>
#include <vector>

#include <api/core.hpp>

#define API_STRIDE(cd, rs) (cd.x + (cd.y * rs.x))
#define API_WRAP(value, max) do { value = (value >= max) ? 0 : (value); } while(0)

namespace api {
    template <typename T = ModeASCII>
    struct Buffer {
        using _buffer_t = std::vector<T>;

        Buffer() : _console_res({ 0,0 }) { _init_buffer(); }
        Buffer(Coords res) : _console_res(res) { _init_buffer(); }
        Buffer(Coords res, T v) : _console_res(res) { _init_buffer(v); }

        void clear() NOEXCEPT {
            set_buffer_data(255);
        }

        void set_buffer_data(const T& v) NOEXCEPT {
            std::fill(_buffer.begin(), _buffer.end(), v);
        }

        void set_buffer_texture(const _buffer_t& tex) {
            debug_assert(tex.size() == _buffer.size());
            std::copy(tex.cbegin(), tex.cend(), _buffer.begin());
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
        void resize_buffer(Coords new_res) NOEXCEPT {
            this->_console_res = new_res;
            _realloc_buffer();
        }

        T* get_line(int line) NOEXCEPT {
            debug_assert(line < _console_res.y);
            return _buffer.data() + (line * _console_res.x);
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

        _buffer_t _capture_buffer() CNOEXCEPT {
            return _buffer;
        }

    private:
        _buffer_t _buffer;
        Coords _console_res;

        template <typename U>
        friend struct Framebuffer;
    };


    template <typename T = ModeASCII>
    using WrappedBuffer = ObjectBinding<Buffer<T>>;


    template <typename T = ModeASCII>
    struct Framebuffer {
        enum class State {
            eInvalid,
            eNormal,
            eDoResize,
        };

        using _ibuffer_t = Buffer<T>;
        using _wibuffer_t = WrappedBuffer<T>;
        using _ibuffer_underlying_t = typename _ibuffer_t::_buffer_t;

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
                FATAL("Framebuffers have not been initialized.");
            }
        }

        _wibuffer_t get_active_buffer() NOEXCEPT {
            if(_running) LIKELY {
                return { _buffers[_selected_buffer] };
            }
            else UNLIKELY {
                FATAL("Framebuffers have not been initialized.");
            }
        }

        T* get_raw_buffer_data() NOEXCEPT {
            return get_active_buffer()->raw_data();
        }

        State post_buffer() NOEXCEPT {
            BEG_FRAME("buffer writing")
            auto buffer_data = _buffers[_selected_buffer].get_buffer_data();
            DWORD written_characters;
            WriteConsoleOutputCharacterA(_cout_handle, buffer_data.data(), buffer_data.size(), { 0,0 }, &written_characters);
            END_FRAME("buffer writing")

            if(_buffer_state == State::eDoResize) UNLIKELY {
                return _update_buffers();
            }

            _written_lines = 0;
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

        int write_line(const std::string& format, auto...vv) NOEXCEPT {
            if constexpr(std::is_same_v<T, char>) {
                if(_written_lines >= get_active_buffer()->get_y()) _written_lines = 0;

                char* string = get_active_buffer()->get_line(_written_lines);
                const char* raw_format = format.c_str();

                ++_written_lines;
                return _bufprintf(string, raw_format, vv...);
            }

            return 0;
        }

        /**
         * Returns the data from the last posted buffer.
         * Good for extracting static frames.
         */
        _ibuffer_underlying_t buffer_snapshot() CNOEXCEPT {
            return _buffers[_drawing_buffer]._capture_buffer();
        }

        NODISCARD Coords get_coords() CNOEXCEPT {
            return _console_res;
        }

        NODISCARD int get_buffer_count() CNOEXCEPT {
            return _buffer_count;
        }

        NODISCARD dVec2 get_screen_coords(api::Coords screen_coords) CNOEXCEPT {
            return api::dVec2(get_coords()) / api::dVec2(screen_coords + 1);
        }

    private:
        static int _bufprintf(char* string, const char* format, ...) NOEXCEPT {
            va_list ls;
            int ret;

            va_start(ls, format);
            ret = vsprintf(string, format, ls);
            va_end(ls);

            return ret;
        }

        State _update_buffers() NOEXCEPT {
            for(_ibuffer_t& buf : _buffers) {
                buf.resize_buffer(_console_res);
            }

            _buffer_state = State::eNormal;
            return State::eDoResize;
        }

    private:
        std::vector<_ibuffer_t> _buffers;
        Coords _console_res;
        int _buffer_count = 0;
        HANDLE _cout_handle;

        int _selected_buffer = 0;
        int _drawing_buffer = 0;

        bool _running = false;
        State _buffer_state = State::eNormal;
        std::size_t _written_lines = 0;
    };

    template <typename T = ModeASCII>
    using FramebufferBinding = ObjectBinding<Framebuffer<T>>;

    template <typename T = ModeASCII>
    using BufferType = typename Buffer<T>::_buffer_t;

    using DefaultFramebuffer = Framebuffer<>;
}

#endif //PROJECT3_TEST_FRAMEBUFFER_HPP
