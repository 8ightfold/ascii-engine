#ifndef PROJECT3_CONFIG_HPP
#define PROJECT3_CONFIG_HPP

#undef COMPILER_CURR
#undef COMPILER_NAME
#undef XCAT
#undef CAT
#undef EXPAND
#undef EAT

#ifndef COMPILER_STRICT_CONFORMANCE
#  define COMPILER_STRICT_CONFORMANCE 1
#endif

#ifndef COMPILER_DEBUG
#  define COMPILER_DEBUG 0
#endif

// Possible values: 8, 16, 32
#ifndef COMPILER_UNICODE_VERSION
#  define COMPILER_UNICODE_VERSION 8
#endif

#define COMPILER_USTRINGIFY_VER(...) EXPAND(CAT(CAT(U, COMPILER_UNICODE_VERSION), STRINGIFY)(__VA_ARGS__))

#define XCAT(x, y) x ## y
#define CAT(x, y) XCAT(x, y)
#define EXPAND(...) __VA_ARGS__
#define EAT(...)

#define XSTRINGIFY(...) #__VA_ARGS__
#define STRINGIFY(...) XSTRINGIFY(__VA_ARGS__)
#define LSTRINGIFY(...) CAT(L, XSTRINGIFY(__VA_ARGS__))
#define RSTRINGIFY(...) CAT(R, XSTRINGIFY(__raw__(__VA_ARGS__)__raw__))
#define U8STRINGIFY(...) CAT(u8, XSTRINGIFY(__VA_ARGS__))
#define U16STRINGIFY(...) CAT(u, XSTRINGIFY(__VA_ARGS__))
#define U32STRINGIFY(...) CAT(U, XSTRINGIFY(__VA_ARGS__))
#define USTRINGIFY(...) COMPILER_USTRINGIFY_VER(__VA_ARGS__)

#define COMPILER_CPP97 199711
#define COMPILER_CPP98 COMPILER_CPP97
#define COMPILER_CPP11 201103
#define COMPILER_CPP14 201402
#define COMPILER_CPP17 201703
#define COMPILER_CPP20 202002
#define COMPILER_CPP23 202304           // Just a guess for now

#define VCOMPILER_UNKNOWN   0b0000000000
#define VCOMPILER_GNU       0b0100000000
#define VCOMPILER_LLVM      0b1000000000
#define VCOMPILER_GCC       0b0100000001
#define VCOMPILER_CLANG     0b1000000010
#define VCOMPILER_MSVC      0b0000000100
#define VCOMPILER_ICC       0b0000001000
#define VCOMPILER_ICX       0b1000010010
#define VCOMPILER_MINGW     0b0100100001
#define VCOMPILER_NVCPP     0b0001000000
#define VCOMPILER_ELLCC     0b1010000000

#define COMPILER_FUNCTION_CLASSIC 0

#if defined(__clang__)
#
#  if defined(__ELLCC__)
#    define COMPILER_ELLCC "ellcc"
#    define COMPILER_CURR VCOMPILER_ELLCC
#    define COMPILER_NAME COMPILER_ELLCC
#  elif defined(__INTEL_LLVM_COMPILER)
#    define COMPILER_ICX "icx"
#    define COMPILER_CLANG "clang"
#    define COMPILER_CURR VCOMPILER_ICX
#    define COMPILER_NAME COMPILER_ICX
#  else
#    define COMPILER_CLANG "clang"
#    define COMPILER_CURR VCOMPILER_CLANG
#    define COMPILER_NAME COMPILER_CLANG
#  endif
#
#  define COMPILER_LLVM "LLVM"
#
#elif defined(__INTEL_COMPILER)
#
#  define COMPILER_ICC "icc"
#  define COMPILER_GNU "GNU"
#  define COMPILER_CURR VCOMPILER_ICC
#  define COMPILER_NAME COMPILER_ICC
#
#elif defined(__GNUC__)
#
#  if defined(__MINGW32__)
#    define COMPILER_MINGW "mingw"
#    define COMPILER_GCC "gcc"
#    define COMPILER_CURR VCOMPILER_MINGW
#    define COMPILER_NAME COMPILER_MINGW
#  elif defined(__NVCOMPILER)
#    define COMPILER_NVCPP "nvc++"
#    define COMPILER_CURR VCOMPILER_NVCPP
#    define COMPILER_NAME COMPILER_NVCPP
#  else
#    define COMPILER_GCC "gcc"
#    define COMPILER_CURR VCOMPILER_GCC
#    define COMPILER_NAME COMPILER_GCC
#  endif
#
#  define COMPILER_GNU "GNU"
#
#elif defined(_MSC_VER) || defined(_MSVC_LANG)
#
#  define COMPILER_MSVC "msvc"
#  define COMPILER_CURR VCOMPILER_MSVC
#  define COMPILER_NAME COMPILER_MSVC
#
#else
#
#  define COMPILER_UNKNOWN "unknown"
#  define COMPILER_CURR VCOMPILER_UNKNOWN
#  define COMPILER_NAME COMPILER_UNKNOWN
#  if COMPILER_STRICT_CONFORMANCE
#    error Your compiler is currently unsupported!
#  endif
#
#endif

#if defined(COMPILER_MSVC) && defined(_MSVC_LANG)
#  define COMPILER_CPP_VERSION _MSVC_LANG
#elif defined(__cplusplus) && __cplusplus
#  define COMPILER_CPP_VERSION __cplusplus
#else
#  error This must be compiled with C++!
#endif

#define CPPVER(num) (COMPILER_CPP_VERSION >= CAT(COMPILER_CPP, num))

#if defined(COMPILER_GNU) || defined(COMPILER_LLVM)
#  define COMPILER_FUNCTION __PRETTY_FUNCTION__
#elif defined(COMPILER_MSVC)
#  define COMPILER_FUNCTION __FUNCSIG__
#else
#  define COMPILER_FUNCTION __func__
#  undef  COMPILER_FUNCTION_CLASSIC
#  define COMPILER_FUNCTION_CLASSIC 1
#endif

#if defined(COMPILER_GNU) || defined(COMPILER_LLVM)
#  define UNREACHABLE_I __builtin_unreachable()
#  define FORCE_INLINE inline __attribute__((always_inline))
#elif defined(COMPILER_MSVC)
#  define UNREACHABLE_I __assume(0)
#  define FORCE_INLINE __forceinline
#else
#  define UNREACHABLE_I void(0)
#  define FORCE_INLINE inline
#endif

#define UNREACHABLE(...) UNREACHABLE_I

#if defined(COMPILER_MINGW)
#  define CLRCALL __clrcall
#  define STDCALL __stdcall
#  define VECCALL
#elif defined(COMPILER_MSVC)
#  define CLRCALL __clrcall
#  define STDCALL __stdcall
#  define VECCALL __vectorcall
#else
#  define CDECL
#  define CLRCALL
#  define VECCALL
#  define FASTCALL
#endif

#if defined(COMPILER_LLVM) || defined(COMPILER_MSVC)
#  define RESTRICT __restrict
#elif defined(COMPILER_GNU)
#  define RESTRICT __restrict__
#else
#  define RESTRICT
#endif

#if CPPVER(23)
#  define ASSUME(...) [[assume(__VA_ARGS__)]]
#else
#  define ASSUME(...)
#endif

#if CPPVER(20)
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

#if CPPVER(17)
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

#if CPPVER(14)
#  ifndef FDEPRECATED
#    define FDEPRECATED(...) [[deprecated]]
#  endif
#  define DEPRECATED [[deprecated]]
#else
#  define DEPRECATED
#  define FDEPRECATED(...)
#endif

#if CPPVER(11)
#  define CARRIES_DEPENDENCY [[carries_dependency]]
#  define CNOEXCEPT const NOEXCEPT
#  define NOEXCEPT noexcept
#  define NORETURN [[noreturn]]
#else
#  define CARRIES_DEPENDENCY
#  define NOEXCEPT
#  define NORETURN
#endif

#if !CPPVER(11)
#  undef RSTRINGIFY
#  undef U8STRINGIFY
#  undef U16STRINGIFY
#  undef U32STRINGIFY
#  undef USTRINGIFY
#endif

#define COMPILER_FILE __FILE__
#define COMPILER_LINE __LINE__

#if COMPILER_DEBUG == 1
#  define DEBUG_ONLY(...) __VA_ARGS__
#else
#  define DEBUG_ONLY(...)
#endif

#endif //PROJECT3_CONFIG_HPP
