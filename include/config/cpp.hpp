#ifndef PROJECT3_TEST_CONFIG_CPP_HPP
#define PROJECT3_TEST_CONFIG_CPP_HPP

#define CPPVER_NEXT(num) TRICAT(CPPVER, num, _NEXT)
#define CPPVER_LEAST(num) (COMPILER_CPP_VERSION >= CAT(COMPILER_CPP, num))
#define CPPVER_MOST(num) (COMPILER_CPP_VERSION < CAT(COMPILER_CPP, CPPVER_NEXT(num)))
#define CPPVER_MATCH(num) (CPPVER_LEAST(num) && CPPVER_MOST(num))

#ifndef CPPVER
#  define CPPVER(num) CPPVER_LEAST(num)
#endif

#if CPPVER_LEAST(23)
#  define ASSUME(...) [[assume(__VA_ARGS__)]]
#else
#  define ASSUME(...)
#endif

#if CPPVER_LEAST(20)
#  define CONSTEVAL consteval
#  define FDEPRECATED(...) [[deprecated __VA_OPT__((__VA_ARGS__))]]
#  define FNODISCARD(...) [[nodiscard __VA_OPT__((__VA_ARGS__))]]
#  define LIKELY [[likely]]
#  define NO_UNIQUE_ADDRESS [[no_unique_address]]
#  define UNLIKELY [[unlikely]]
#else
#  define CONSTEVAL constexpr
#  define LIKELY
#  define NO_UNIQUE_ADDRESS
#  define UNLIKELY
#endif

#if CPPVER_LEAST(17)
#  ifndef FNODISCARD
#    define FNODISCARD(...) [[nodiscard]]
#  endif
#  define FALLTHROUGH [[fallthrough]]
#  define GLOBAL inline constexpr
#  define MAYBE_UNUSED [[maybe_unused]]
#  define NODISCARD [[nodiscard]]
#else
#  define FALLTHROUGH
#  define FNODISCARD(...)
#  define GLOBAL
#  define MAYBE_UNUSED
#  define NODISCARD
#endif

#if CPPVER_LEAST(14)
#  ifndef FDEPRECATED
#    define FDEPRECATED(...) [[deprecated]]
#  endif
#  define DEPRECATED [[deprecated]]
#else
#  define DEPRECATED
#  define FDEPRECATED(...)
#endif

#if CPPVER_LEAST(11)
#  define CARRIES_DEPENDENCY [[carries_dependency]]
#  define CNOEXCEPT const NOEXCEPT
#  define NOEXCEPT noexcept
#  define NORETURN [[noreturn]]
#else
#  define CARRIES_DEPENDENCY
#  define NOEXCEPT
#  define NORETURN
#endif

#if !CPPVER_LEAST(11)
#  undef RSTRINGIFY
#  undef U8STRINGIFY
#  undef U16STRINGIFY
#  undef U32STRINGIFY
#  undef USTRINGIFY

#  define U8STRINGIFY LSTRINGIFY
#  define U16STRINGIFY LSTRINGIFY
#  define U32STRINGIFY LSTRINGIFY
#  define USTRINGIFY LSTRINGIFY
#endif

#ifdef __cplusplus
#  define register
#  define PURE = 0
#endif

#endif //PROJECT3_TEST_CONFIG_CPP_HPP
