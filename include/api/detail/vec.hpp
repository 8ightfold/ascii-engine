#ifndef PROJECT3_TEST_VEC_HPP
#define PROJECT3_TEST_VEC_HPP

#include <config.hpp>

namespace api {
    template <typename T, std::size_t N> struct Vec;

    template <typename T>
    struct Vec<T, 1> {
        T x;
    };

    template <typename T>
    struct Vec<T, 2> {
        T x, y;

        template <typename U = T>
        NODISCARD Vec<T, 2> operator+(const Vec<U, 2>& d) NOEXCEPT {
            return { T(x + d.x), T(y + d.y) };
        }

        template <typename U = T>
        NODISCARD Vec<T, 2> operator+(U t) NOEXCEPT {
            return { T(x + t), T(y + t) };
        }

        template <typename U = T>
        NODISCARD Vec<T, 2> operator-(const Vec<U, 2>& d) NOEXCEPT {
            return { T(x - d.x), T(y - d.y) };
        }

        template <typename U = T>
        NODISCARD Vec<T, 2> operator-(U t) NOEXCEPT {
            return { T(x - t), T(y - t) };
        }

        template <typename U = T>
        NODISCARD Vec<T, 2> operator*(const Vec<U, 2>& d) NOEXCEPT {
            return { T(x * d.x), T(y * d.y) };
        }

        template <typename U = T>
        NODISCARD Vec<T, 2> operator*(U t) NOEXCEPT {
            return { T(x * t), T(y * t) };
        }

        template <typename U = T>
        NODISCARD Vec<T, 2> operator/(const Vec<U, 2>& d) NOEXCEPT {
            return { T(x / d.x), T(y / d.y) };
        }

        template <typename U = T>
        NODISCARD Vec<T, 2> operator/(U t) NOEXCEPT {
            return { T(x / t), T(y / t) };
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
}

#endif //PROJECT3_TEST_VEC_HPP
