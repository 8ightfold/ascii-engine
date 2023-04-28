#ifndef PROJECT3_TEST_CONFIG_COMPILER_HPP
#define PROJECT3_TEST_CONFIG_COMPILER_HPP

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

#include <config/cpp.hpp>

#if CPPVER_MOST(17) && defined(COMPILER_MSVC)
#  define not !
#  define and &&
#  define or ||
#endif

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

#ifdef COMPILER_MSVC
#  define COMPILER_UUID_I(value) uuid(STRINGIFY(value))
#  define COMPILER_UUID(value) __declspec(COMPILER_UUID_I(value))
#else
#  define COMPILER_UUID(...)
#endif

#if defined(COMPILER_GNU) || defined(COMPILER_LLVM)
#  define COMPILER_FUNCTION __PRETTY_FUNCTION__
#elif defined(COMPILER_MSVC)
#  define COMPILER_FUNCTION __FUNCSIG__
#else
#  define COMPILER_FUNCTION __func__
#  undef  COMPILER_FUNCTION_CLASSIC
#  define COMPILER_FUNCTION_CLASSIC 1
#endif

#define COMPILER_FILE __FILE__
#define COMPILER_LINE __LINE__
#define DEBUG_INFO COMPILER_FILE, COMPILER_FUNCTION, COMPILER_LINE

#if COMPILER_DEBUG == 1
#  define DEBUG_ONLY(...) __VA_ARGS__
#else
#  define DEBUG_ONLY(...)
#endif

#endif //PROJECT3_TEST_CONFIG_COMPILER_HPP
