#ifndef PROJECT3_TEST_CONFIG_CONDITIONALS_HPP
#define PROJECT3_TEST_CONFIG_CONDITIONALS_HPP

#define PP_BOOL(num) CAT(PP_BOOL, num)

#define PP_NEG(num) CAT(PP_NEG, PP_BOOL(num))
#define PP_NEG0 1
#define PP_NEG1 0

#define VALID_INT(num) VALID_INT_I(CAT(PP_VALID_INT, num), 0)
#define VALID_INT_I(_, N, ...) VALID_INT_II(_, N, __VA_ARGS__)
#define VALID_INT_II(_, N, ...) N

#define EMPTY(...) XEMPTY(__VA_OPT__(,) PP_FALSE, PP_TRUE)
#define XEMPTY(_, N, ...) N

#define PP_IF(cond, t) CAT(PP_IF, PP_BOOL(cond))(t)
#define PP_IFELSE(cond, t, f) PP_IF(cond, t) PP_IF(PP_NEG(cond), f)
#define PP_IF0(...)
#define PP_IF1(t) t

#define PP_ALTERNATE(option, ...) PP_ALTERNATE_I( PP_IFELSE(EMPTY(__VA_ARGS__), (option), (__VA_ARGS__)) )
#define PP_ALTERNATE_I(pack) PEXPAND(pack)

#endif //PROJECT3_TEST_CONFIG_CONDITIONALS_HPP
