#include "parsercore.hpp"

namespace parser {
    ParserCore::ParserCore(GlobalContext* global_ctx)
    : _includes(global_ctx->get_core()), _global_ctx(global_ctx) {}

    void ParserCore::run_parser() {
        const fs::path core_path = _global_ctx->get_core() / "core.uxi";
        auto& file = _includes.add_file("core.uxi");
        _stack_emplace(_includes, file);

        try {
            _parse_stack();
            _completed = true;
        }
        catch(UnbalancedExpressionTypes& uet) {
            std::cerr << "Invalid type close '" << uet.what() << "'" << std::endl;
            this->stacktrace();
        }
        catch(InvalidMetaValue& imv) {
            std::cerr << "Invalid meta expression '" << imv.what() << "'" << std::endl;
        }
        catch(InvalidExpression& ie) {
            std::cerr << "Unable to parse expression '" << ie.what() << "'" << std::endl;
        }
        catch(std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
        catch(...) {
            std::cerr << "Something has gone terribly wrong (caught non exception throw). Exiting." << std::endl;
            std::exit(-1);
        }
    }

    void ParserCore::stacktrace() {
        std::size_t max_depth = _global_ctx->get_stacktrace_depth();
        _completed = false;
        while(not _ctx_stack.empty() && max_depth) {
            print_top_frame();
            _ctx_stack.pop();
            --max_depth;
        }
    }

    void ParserCore::print_top_frame() {
        if(auto* ptr = _stack_top()) {
            ptr->print_stacktrace();
        }
    }

    void ParserCore::check_completeness() const {
        if(not _completed) {
            std::cerr << "Parsing failed. Exiting." << std::endl;
            std::exit(-1);
        }
    }


    void ParserCore::_parse_stack() {
        while(not _ctx_stack.empty()) {
            auto* current_ctx = _stack_top();

            while(not current_ctx->finished()) {
                current_ctx->next();
                DEBUG_ONLY(3, current_ctx->print_last();)
                current_ctx->parse();
            }

            _ctx_stack.pop();
        }
    }

    FileContext& ParserCore::_stack_emplace(FileTree& tree, File& file) {
        return _ctx_stack.emplace(_global_ctx, &tree, &file);
    }

    FileContext& ParserCore::_stack_emplace(FileTree* tree, File& file) {
        return _ctx_stack.emplace(_global_ctx, tree, &file);
    }

    FileContext* ParserCore::_stack_top() {
        if(_ctx_stack.empty()) return nullptr;
        else return std::addressof(_ctx_stack.top());
    }


    ParserCore create_parser(GlobalContext* global_ctx) {
        if(not global_ctx) {
            std::cerr << "GlobalContext has not been initialized." << std::endl;
            std::exit(-1);
        }

        return { global_ctx };
    }
}