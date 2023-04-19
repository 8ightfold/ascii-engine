#ifndef PROJECT3_TEST_UTILS_HPP
#define PROJECT3_TEST_UTILS_HPP

#include <cstddef>
#include <type_traits>

namespace parser {
    template <typename T>
    struct TypeWrapper { using type = T; };

    template <auto V>
    struct AutoWrapper { static constexpr auto value = V; };

    template <typename T>
    inline constexpr TypeWrapper<T> TW {};

    template <auto V>
    inline constexpr AutoWrapper<V> AW {};

    template <typename T, std::size_t N>
    using FixedArray = T[N];
}

#endif //PROJECT3_TEST_UTILS_HPP
