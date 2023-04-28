#ifndef PROJECT3_CONFIG_HPP
#define PROJECT3_CONFIG_HPP

#ifndef COMPILER_STRICT_CONFORMANCE
#  define COMPILER_STRICT_CONFORMANCE 1
#endif

#ifndef COMPILER_DEBUG
#  define COMPILER_DEBUG 0
#endif

#ifndef COMPILER_UNICODE_VERSION
/// Possible values: 8, 16, 32
#  define COMPILER_UNICODE_VERSION 8
#endif

#ifndef COMPILER_RESCAN_COUNT
/// Make sure you have a beefy pc for large values
#  define COMPILER_RESCAN_COUNT 5
#endif

#include <config/undefs.hpp>

#define COMPILER_CPP97 199711
#define COMPILER_CPP98 COMPILER_CPP97
#define COMPILER_CPP11 201103
#define COMPILER_CPP14 201402
#define COMPILER_CPP17 201703
#define COMPILER_CPP20 202002
#define COMPILER_CPP23 202304           /// Just a guess for now

#define CPPVER97_NEXT 11
#define CPPVER98_NEXT 11
#define CPPVER11_NEXT 14
#define CPPVER14_NEXT 17
#define CPPVER17_NEXT 20
#define CPPVER20_NEXT 23
#define CPPVER23_NEXT 26

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

#define COMPILER_VERSION_COUNT 6
#define COMPILER_MIN_VERSION 97
#define COMPILER_MAX_VERSION 23
#define VCOMPILER_BIT_COUNT 10

#define COMPILER_FUNCTION_CLASSIC 0

#define COMPILER_NUMERAL_MAX 100
#define COMPILER_ARITHMETIC_MAX 255
#define COMPILER_RESCAN_MAX 5

#include <config/values.hpp>
#include <config/functions.hpp>
#include <config/compiler.hpp>
#include <config/conditionals.hpp>

#define CHAIN_HEAD() if(false) {}
#define CHAIN_LINK(...) else if(__VA_ARGS__)
#define CHAIN_TAIL() else {}
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#endif //PROJECT3_CONFIG_HPP
