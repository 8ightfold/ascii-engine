#ifndef PROJECT3_TEST_API_CORE_HPP
#define PROJECT3_TEST_API_CORE_HPP

#include <cstdio>
#include <climits>
#include <type_traits>
#include <utility>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <api/winapi.hpp>
#include <api/detail/vec.hpp>
#include <api/detail/object_binding.hpp>
#include <api/detail/circular_queue.hpp>

#if !CPPVER(20)
#  error Must compile with C++20 (this project uses features such as __VA_OPT__ and std::span)!
#endif

#define assertion(cond, ...) \
do { api::assertion_impl(DEBUG_INFO, bool(cond), PP_ALTERNATE("Assertion '" #cond "' failed.", __VA_ARGS__)); } while(0)

#define debug_assert(cond, ...) DEBUG_ONLY( assertion(cond __VA_OPT__(,) __VA_ARGS__) )
#define debug_printf(fmt, ...) do { DEBUG_ONLY( std::printf(fmt __VA_OPT__(,) __VA_ARGS__); ) } while(0)

#define FATAL(err) api::fatal_error(DEBUG_INFO, err)
#define WIN_PRESSED(sc) [](int scancode) -> bool { \
return ((scancode) & 0x0001) || ((scancode) & 0x8000); }(GetAsyncKeyState(sc))

namespace api {
    struct Coords {
        NODISCARD operator POINT() CNOEXCEPT;
        NODISCARD operator COORD() CNOEXCEPT;
        NODISCARD operator iVec2() CNOEXCEPT;
        NODISCARD operator dVec2() CNOEXCEPT;
        Coords& operator=(POINT p) NOEXCEPT;
        Coords& operator=(COORD p) NOEXCEPT;
        NODISCARD int area() CNOEXCEPT;
        NODISCARD Coords operator+(int i) CNOEXCEPT;
        NODISCARD Coords operator+(Coords c) CNOEXCEPT;
        NODISCARD Coords operator-(Coords c) CNOEXCEPT;
        NODISCARD Coords operator*(Coords c) CNOEXCEPT;
        NODISCARD Coords operator/(int i) CNOEXCEPT;
        Coords& operator+=(Coords c) NOEXCEPT;
        Coords& operator-=(Coords c) NOEXCEPT;
        Coords& operator/=(int i) NOEXCEPT;
        Coords& operator/=(double d) NOEXCEPT;
        Coords& operator/=(Coords c) NOEXCEPT;
        Coords& operator/=(dVec2 d) NOEXCEPT;

        int x, y;
    };


    template <typename K, typename V, typename...EE>
    using Map = std::unordered_map<K, V, EE...>;

    template <typename K, typename...EE>
    using Set = std::unordered_set<K, EE...>;

    template <typename K, typename V, typename...EE>
    using OrderedMap = std::map<K, V, EE...>;

    template <typename K, typename...EE>
    using OrderedSet = std::set<K, EE...>;


    template <typename T>
    inline constexpr std::size_t bitsof = sizeof(T) * CHAR_BIT;

    enum toggle : bool {
        off, on
    };

    void assertion_impl(const char* filename, const char* func, int line, bool condition, const std::string& err);
    NORETURN void fatal_error(const char* filename, const char* func, int line, const std::string& err);
    void handle_last_error(const char* filename, int line, LPTSTR lpszFunction, bool exit_on_error = true);
    NORETURN void waiting_exit(int code);
    NORETURN FORCE_INLINE void unreachable() { UNREACHABLE(); }
}

#define PROJECT3_TEST_API_CORE_HPP_I
#endif //PROJECT3_TEST_API_CORE_HPP
