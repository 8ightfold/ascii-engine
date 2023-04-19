#include <core.hpp>
#include <parser/parsercore.hpp>

static parser::GlobalContext* parse_args(int argc, char* argv[]) {
    if(not argv[1]) {
        std::cerr << "No input path specified." << std::endl;
        std::exit(-1);
    }

    auto* global_ctx = new parser::GlobalContext();

    fs::path exe_path = argv[0];
    fs::path base_path = argv[1];

    global_ctx->set_dirs(exe_path, base_path);
    global_ctx->verify_core();

    return global_ctx;
}

int main() {
    auto* global_ctx = INIT_GCTX();
    auto parse = parser::create_parser(global_ctx);

    parse.run_parser();
    parse.check_completeness();
}