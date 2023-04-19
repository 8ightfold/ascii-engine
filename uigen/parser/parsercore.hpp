#ifndef PROJECT3_TEST_PARSERCORE_HPP
#define PROJECT3_TEST_PARSERCORE_HPP

#include <iostream>

#include <parser/globalcontext.hpp>
#include <parser/filecontext.hpp>

namespace parser {
    struct ParserCore {
        ParserCore(GlobalContext* global_ctx);  // NOLINT
        void run_parser();

        void stacktrace(int max_depth = 1);
        void print_top_frame();
        void check_completeness() const;

    private:
        void _parse_stack();
        FileContext& _stack_emplace(FileTree& tree, File& file);
        FileContext& _stack_emplace(FileTree* tree, File& file);
        FileContext* _stack_top();

    private:
        GlobalContext* _global_ctx = nullptr;
        FileTree _includes;
        Stack<FileContext> _ctx_stack;
        bool _completed = false;
    };

    ParserCore create_parser(GlobalContext* global_ctx);
}

#endif //PROJECT3_TEST_PARSERCORE_HPP
