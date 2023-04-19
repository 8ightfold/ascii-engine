#ifndef PROJECT3_TEST_FILECONTEXT_HPP
#define PROJECT3_TEST_FILECONTEXT_HPP

#include <array>
#include <utility>
#include <parser/globalcontext.hpp>

namespace parser {
    enum struct ExpressionType {
        eInvalid, eMeta, eNode, eSkip,
    };

    struct InvalidExpression : std::exception {
        [[nodiscard]] const char* what() const noexcept override { return _data.c_str(); }
        explicit InvalidExpression(std::string_view sv) : _data(sv) {}

    protected:
        std::string _data;
    };

    struct UnbalancedExpressionTypes : InvalidExpression {
        [[nodiscard]] const char* what() const noexcept override { return _data.c_str(); }
        explicit UnbalancedExpressionTypes(std::string_view sv) : InvalidExpression(sv) {}
    };


    struct FileContext {
        struct FileContextParser {
            explicit FileContextParser(FileContext* ctx) : _ctx(ctx) {}

            std::string_view parse(auto expr) {
                auto&& [_, meta, node, comment, invalid] = expr;
                if(comment) {
                    set_type(ExpressionType::eSkip);
                    return comment;
                }
                else if(meta) {
                    set_type(ExpressionType::eMeta);
                    return meta;
                }
                else if(node) {
                    set_type(ExpressionType::eNode);
                    return node;
                }
                else {
                    set_type(ExpressionType::eInvalid);
                    if(invalid) return invalid;
                    else return "";
                }
            }

            void set_type(ExpressionType type) {
                _ctx->_last_type = type;
            }

        private:
            FileContext* _ctx = nullptr;
        };

        FileContext(GlobalContext* global_ctx, FileTree* tree_ctx, File* file_ctx);
        ~FileContext() = default;

        void next();
        void parse();

        [[nodiscard]] bool finished() const;

        void print_last(std::ostream& os = std::cout) const;
        void print_stacktrace(std::ostream& os = std::cerr) const;

    private:
        void _parse_node();
        bool _set_top_level();
        TypeContext _pop_stack();
        [[nodiscard]] bool _compare_top_type(std::string_view type) const;

    private:
        GlobalContext* _global_ctx = nullptr;
        FileTree* _tree = nullptr;
        File* _current_file = nullptr;

        SafeRegexExpr _expr = nullptr;
        FileContextParser _parser_callbacks;
        std::string_view _last_value;
        ExpressionType _last_type = ExpressionType::eInvalid;

        Stack<TypeContext> _type_stack;
        bool _finished = false;
        bool _top_level = true;

        friend struct FileContextParser;
    };
}

#endif //PROJECT3_TEST_FILECONTEXT_HPP
