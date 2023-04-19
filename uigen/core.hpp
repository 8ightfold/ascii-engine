#ifndef PROJECT3_TEST_UI_PARSER_CORE_HPP
#define PROJECT3_TEST_UI_PARSER_CORE_HPP

/*
    ctre is NOT made by me
    It was created by Hana Dusíková
    You can find the repo here: https://github.com/hanickadot/compile-time-regular-expressions
 */

#define XCAT(x, y) x ## y
#define CAT(x, y) XCAT(x, y)
#define XSTRINGIFY(...) #__VA_ARGS__
#define STRINGIFY(...) XSTRINGIFY(__VA_ARGS__)
#define LSTRINGIFY(...) CAT(L, XSTRINGIFY(__VA_ARGS__))
#define RSTRINGIFY(...) CAT(R, XSTRINGIFY(__r(__VA_ARGS__)__r))

#define main(...) main(int argc, char* argv[])
#define INIT_GCTX(...) parse_args(argc, argv)

#define BIND_REGEX(expr) parser::RegexBinder<expr, expr>

#include <core/fileloader.hpp>
#include <core/utils.hpp>
#include <core/regex.hpp>
#include <core/stack.hpp>

#endif //PROJECT3_TEST_UI_PARSER_CORE_HPP
