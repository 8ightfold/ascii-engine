#ifndef PROJECT3_TEST_API_CORE_HPP
#define PROJECT3_TEST_API_CORE_HPP

#include <cassert>
#include <cstdio>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>

#include <api/winapi.hpp>
#include <config.hpp>


#define debug_assert(cond, message) DEBUG_ONLY( assert(cond && message) )
#define debug_printf(fmt, ...) DEBUG_ONLY( std::printf(fmt __VA_OPT__(,) __VA_ARGS__) )

namespace api {
    template <typename T, std::size_t N> struct Vec;

    template <typename T>
    struct Vec<T, 1> {
        T x;
    };

    template <typename T>
    struct Vec<T, 2> {
        T x, y;

        NODISCARD Vec<T, 2> operator+(const Vec<T, 2>& d) NOEXCEPT {
            return { x + d.x, y + d.y };
        }

        NODISCARD Vec<T, 2> operator+(T t) NOEXCEPT {
            return { x + t, y + t };
        }

        NODISCARD Vec<T, 2> operator/(const Vec<T, 2>& d) NOEXCEPT {
            return { x / d.x, y / d.y };
        }
    };

    template <typename T>
    struct Vec<T, 3> {
        T x, y, z;
    };

    template <typename T>
    struct Vec<T, 4> {
        T x, y, z, w;
    };

    using fVec1 = Vec<float, 1>;
    using fVec2 = Vec<float, 2>;
    using fVec3 = Vec<float, 3>;
    using fVec4 = Vec<float, 4>;

    using dVec1 = Vec<double, 1>;
    using dVec2 = Vec<double, 2>;
    using dVec3 = Vec<double, 3>;
    using dVec4 = Vec<double, 4>;

    using iVec1 = Vec<int, 1>;
    using iVec2 = Vec<int, 2>;
    using iVec3 = Vec<int, 3>;
    using iVec4 = Vec<int, 4>;

    struct Coords {
        NODISCARD operator POINT() CNOEXCEPT;
        NODISCARD operator COORD() CNOEXCEPT;
        NODISCARD operator iVec2() CNOEXCEPT;
        NODISCARD operator dVec2() CNOEXCEPT;
        Coords& operator=(POINT p) NOEXCEPT;
        Coords& operator=(COORD p) NOEXCEPT;
        NODISCARD int area() CNOEXCEPT;
        Coords operator+(int i) CNOEXCEPT;
        Coords& operator/=(int i) NOEXCEPT;
        Coords& operator/=(double d) NOEXCEPT;
        Coords& operator/=(Coords c) NOEXCEPT;
        Coords& operator/=(dVec2 d) NOEXCEPT;

        int x, y;
    };

    enum toggle : bool {
        off, on
    };

    template <typename T>
    struct ObjectBinding {
        ObjectBinding() = default;
        ObjectBinding(T& data) : _data(std::addressof(data)) {}

        ObjectBinding& operator=(T& data) { _data = std::addressof(data); return *this; }

        T* operator->() NOEXCEPT { return _data; }
        const T* operator->() CNOEXCEPT { return _data; }
        NODISCARD bool active() CNOEXCEPT { return _data; }

    private:
        T* _data = nullptr;
    };

    template <typename T>
    struct CircularQueue {
        explicit CircularQueue(std::size_t size) : _size(size) {
            _data = new T[size] {};
        }

        CircularQueue(const CircularQueue&) = delete;
        CircularQueue(CircularQueue&&) = delete;

        ~CircularQueue() {
            delete[] _data;
        }

        void apply(auto&& functor) NOEXCEPT {
            auto data_span = data();
            for(auto& t : data_span)
                functor(t);
        }

        T& next() NOEXCEPT {
            if(_location == _size)
                _location = 0;

            T& t = _data[_location];
            ++_location;
            return t;
        }

        NODISCARD std::span<T> data() CNOEXCEPT {
            return { _data, _size };
        }

    private:
        std::size_t _size;
        std::size_t _location = 0;
        T* _data = nullptr;
    };

    void throw_last_error(const char* filename, int line, LPTSTR lpszFunction, bool exit_on_error = true);
    NORETURN FORCE_INLINE void unreachable() { UNREACHABLE(); }
}

#define PROJECT3_TEST_API_CORE_HPP_I
#endif //PROJECT3_TEST_API_CORE_HPP
