#include "filecontext.hpp"

#define TOP_LEVEL_REGXP R"r((?:\s*(#\w+\s+.*?)\s*\n)|(?:\s*(<.*?>)\s*)|(\/\/[^\n\r]*|\/\*.*\*\/)|([^\n\r]+))r"
#define NODE_REGXP R"r(<\s*([\w.:]+)\s*>|<\s*(?:([\w.:]*)\s+)?([\w.:]+=.*?)\s*>|<\s*\\([\w.:]+)\s*>)r"
#define NODE_VALUE_REGEXP R"r(([\w.:]+)=(\w+|"[^"]*"|\[(?:\s*[\w.:]+\s*,?)+\]|\{(?:\s*[^,]+\s*,?)+\}))r"

namespace parser {
    static void extract_values(FileContext::TypeContext& top, std::string_view values) {
        constexpr auto range = ctre::range<NODE_VALUE_REGEXP>;
        auto matched = range(values);

        for(auto [_, name, value] : matched) {
            top->emplace(name, value);
            std::cout << "  pushed '" << name << "': " << value << std::endl;
        }
    }

    static std::string_view pick_type_decl(auto& type, auto& start) {
        if(type.size()) return type.to_view();
        else if(start.size()) return start.to_view();
        else return ""sv;
    }
}

namespace parser {
    FileContext::FileContext(GlobalContext* global_ctx, FileTree* tree_ctx, File* file_ctx)
    : _global_ctx(global_ctx), _tree(tree_ctx), _current_file(file_ctx), _parser_callbacks(this) {
        constexpr BIND_REGEX(TOP_LEVEL_REGXP) bind;
        _expr = bind(_parser_callbacks);
        _expr->read_file(_current_file->get_view());
    }

    bool FileContext::finished() const { return _expr->finished(); }
    void FileContext::next() { _last_value = _expr->next(); }

    void FileContext::parse() {
        switch(_last_type) {
            case ExpressionType::eMeta: {
                break;
            }
            case ExpressionType::eNode: {
                _parse_node();
                break;
            }
            case ExpressionType::eInvalid: {
                std::cout << "UNKNOWN" << std::endl;
                throw InvalidExpression(_last_value);
            }
            default: return;
        }
    }

    void FileContext::_parse_node() {
        constexpr auto matcher = ctre::match<NODE_REGXP>;
        auto&& [_, type, start, values, end] = matcher(_last_value);
        std::string_view type_decl = pick_type_decl(type, start);

        if(not type_decl.empty()) {
            _type_stack.emplace(type_decl);
            std::cout << " added '" << type_decl << "'" << std::endl;
        }

        _set_top_level();

        if(values) {
            auto& top = _type_stack.top();
            extract_values(top, values);
        }
        else if(end) {
            if(_compare_top_type(end)) {
                std::cout << "  popped '" << (std::string_view)end << "'" << std::endl;
                auto last_ctx = _pop_stack();
                for(auto&& [name, value] : last_ctx.values()) {
                    std::cout << '\t' << name << ": " << value << std::endl;
                }
            }
            else throw UnbalancedExpressionTypes(_last_value);
        }
        else if(not (type or start)) {
            throw InvalidExpression(_last_value);
        }

        _set_top_level();

        if(_top_level) std::cout << std::endl;
    }

    bool FileContext::_set_top_level() {
        return (_top_level = _type_stack.empty());
    }

    FileContext::TypeContext FileContext::_pop_stack() {
        if(not _type_stack.empty()) {
            auto tctx = std::move(_type_stack.top());
            _type_stack.pop();
            return tctx;
        }

        throw std::runtime_error("Called pop() with stack.");
    }

    bool FileContext::_compare_top_type(std::string_view type) const {
        if(not _type_stack.empty()) {
            return (_type_stack.top().type() == type);
        }

        throw std::runtime_error("Called top() with empty stack.");
    }

    void FileContext::print_last(std::ostream& os) const {
        switch(_last_type) {
            case ExpressionType::eMeta: {
                os << "META: " << _last_value << std::endl;
                break;
            }
            case ExpressionType::eNode: {
                os << "NODE: " << _last_value << std::endl;
                break;
            }
            case ExpressionType::eInvalid: {
                os << "UNKNOWN" << std::endl;
            }
            default: return;
        }
    }

    void FileContext::print_stacktrace(std::ostream& os) const {
        std::size_t idx = 0;
        os << "STACK FOR " << _current_file->get_filename() << ":\n";

        for(auto&& tctx : ReverseRange(_type_stack)) {
            os << idx++ << ": " << tctx.type() << std::endl;
            for(auto&& [name, value] : tctx.values()) {
                os << "  > '" << name << "': " << value << std::endl;
            }
        }
    }
}
