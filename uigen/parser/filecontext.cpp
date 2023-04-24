#include "filecontext.hpp"

#define TOP_LEVEL_REGXP R"r((?:\s*(#\w+\s+.*?)\s*\n)|(?:\s*(<.*?>)\s*)|(\/\/[^\n\r]*|\/\*.*\*\/)|([^\n\r]+))r"
#define NODE_REGXP R"r(<\s*([\w.:]+)\s*>|<\s*(?:([\w.:]*)\s+)?([\w.:]+=.*?)\s*>|<\s*\\([\w.:]+)\s*>)r"
#define NODE_VALUE_REGEXP R"r(([\w.:]+)=(\w+|"[^"]*"|\[(?:\s*[\w.:]+\s*,?)+\]|\{(?:\s*[^,]+\s*,?)+\}))r"

namespace parser {
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
    void FileContext::next() { if(not _lock_next) _last_value = _expr->next(); }

    void FileContext::parse() {
        switch(_last_type) {
            case ExpressionType::eMeta: {
                _global_ctx->parse_meta(_last_value);
                break;
            }
            case ExpressionType::eNode: {
                _parse_node();
                break;
            }
            case ExpressionType::eInvalid: {
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
            if(is_fundamental(type_decl)) {
                _type_stack.emplace(type_decl);
                if(type_decl == "alias") _lock_alias = true;

                if(DEBUG_LEVEL(1)) {
                    if(not _lock_alias) _debug_type_stack.emplace(type_decl);
                }
            }
            else {
                // TODO: Check for non fundamental types
            }
        }

        _set_debug_top_level();
        if(values) {

            if(DEBUG_LEVEL(1) and not _lock_alias) {
                _extract_debug_values(values);
            }
        }
        else if(end) {

            if(DEBUG_LEVEL(1) and not _lock_alias) {
                if(_compare_top_debug_type(end)) {
                    _print_items();
                }
                else throw UnbalancedExpressionTypes(_last_value);
            }
            else if(DEBUG_LEVEL(1) and end == "alias") {
                _lock_alias = false;
            }
        }
        else if(not (type or start)) {
            throw InvalidExpression(_last_value);
        }

        _set_debug_top_level();
        if(DEBUG_LEVEL(2) and _top_level) std::cout << std::endl;
    }

    bool FileContext::_set_debug_top_level() {
        return (_top_level = _debug_type_stack.empty());
    }

    TypeContext FileContext::_pop_debug_stack() {
        if(not _debug_type_stack.empty()) {
            auto tctx = std::move(_debug_type_stack.top());
            _debug_type_stack.pop();
            return tctx;
        }

        throw std::runtime_error("Called pop() with stack.");
    }

    void FileContext::_extract_debug_values(std::string_view values) {
        constexpr auto range = ctre::range<NODE_VALUE_REGEXP>;
        auto matched = range(values);
        auto& top = _debug_type_stack.top();

        for(auto [_, name, value] : matched) {
            top->emplace(name, value);
            DEBUG_ONLY(2, std::cout << "  pushed '" << name << "': " << value << std::endl;)
        }
    }

    bool FileContext::_compare_top_debug_type(std::string_view type) const {
        if(not _debug_type_stack.empty()) {
            return (_debug_type_stack.top().type() == type);
        }

        throw std::runtime_error("Called top() with empty stack.");
    }


    void FileContext::_print_items() {
        auto last_ctx = _pop_debug_stack();
        DEBUG_ONLY(2,
            for(auto&& [name, value] : last_ctx.values()) {
                std::cout << '\t' << name << ": " << value << std::endl;
            };
        )
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

        for(auto&& tctx : ReverseRange(_debug_type_stack)) {
            os << idx++ << ": " << tctx.type() << std::endl;
            for(auto&& [name, value] : tctx.values()) {
                os << "  > '" << name << "': " << value << std::endl;
            }
        }
    }
}
