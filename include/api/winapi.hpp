#ifndef PROJECT3_TEST_WINAPI_HPP
#define PROJECT3_TEST_WINAPI_HPP

#if !defined(PROJECT3_TEST_API_CORE_HPP) || defined(PROJECT3_TEST_API_CORE_HPP_I)
#  error Must be included in and only in <api/core.hpp>!
#elif defined(PROJECT3_CONFIG_HPP)
#  error Must be included before <config.hpp>!
#endif

#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

#define NOMINMAX 1
#define OEMRESOURCE
#include <Windows.h>

#if defined(TRACY_ENABLE) && COMPILER_DEBUG
#  include <tracy/Tracy.hpp>
#  define TAG_FRAME(name) FrameMarkNamed(name);
#  define BEG_FRAME(name) FrameMarkStart(name);
#  define END_FRAME(name) FrameMarkEnd(name);
#else
#  define TAG_FRAME(...)
#  define BEG_FRAME(...)
#  define END_FRAME(...)
#endif

#define API_INVOKE_WINAPI(fn, err, ...)                                                                     \
[] <typename...internal_TT> (internal_TT&&...internal_tt) {                                                 \
    return                                                                                                  \
    [&] <typename internal_R, typename...internal_Args>                                                     \
    (internal_R(__stdcall*)(internal_Args...)) {                                                            \
        static_assert(sizeof...(internal_TT) == sizeof...(internal_Args),                                   \
            "incorrect amount of arguments for " #fn ".");                                                  \
        static_assert((std::is_convertible_v<internal_TT, internal_Args> && ...),                           \
            "invalid arguments for " #fn ".");                                                              \
        auto tmp = fn(std::forward<internal_TT>(internal_tt)...);                                           \
        if(tmp == err) [[unlikely]] {                                                                       \
            std::string str { #fn };                                                                        \
            api::handle_last_error(__FILE__, __LINE__,                                                      \
                reinterpret_cast<LPTSTR>(str.data()) __VA_OPT__(,) __VA_ARGS__);                            \
        }                                                                                                   \
        return tmp;                                                                                         \
    } (fn);                                                                                                 \
}

#define $invoke_winapi(fn, err, ...) API_INVOKE_WINAPI(fn, err __VA_OPT__(,) __VA_ARGS__)

template <typename T>
T typed_alloc(std::size_t sz) noexcept {
    auto* ptr = (T)malloc(sz);
    return std::launder(ptr);
}

#endif //PROJECT3_TEST_WINAPI_HPP
