#include <core.hpp>
#include <parser/parsercore.hpp>

/*  UXI Documentation:
 *  GENERAL INFO:
 *  When generating code, you must provide a package name. This will be used by the generated
 *  class, and will be how you bind it to your renderer, as well as how context pointers will be created.
 *  While it is only necessary to bind the parent class to the generated one,
 *  some intrinsics (such as device contexts) will not work as intended. You can #enable features like this.
 *
 *  ENTRY:
 *      Defines the first context pointer which will be pushed.
 *      Must be defined, as it will be used when calling UIinstance.
 *
 *  MODULES:
 *      Modules are defined in a strict hierarchical structure. They are all referenced
 *      from the context of the core directory. If your file was at the location
 *      "/folder/subfolder/file.uxi", this would become "folder::subfolder::file".
 *      Class instances can then be accessed using periods.
 *      If you have a class "class_type" in the previously listed file,
 *      you would access it by doing "folder::subfolder::file.class_type".
 *  CALLBACKS:
 *      Callbacks are useful for passing information back to the parent class.
 *      They are registered with C++ types, and are therefore bound by C++ rules.
 *      These can be called by doing "::callback_name(...)".
 *      Here are some examples:
 *
 *      This will use the default type (UI*):
 *          #callback foo(ctx)
 *      This will set a type in the class (_callback_bar0):
 *          #callback bar(value: int)
 *      A more complex example (first is UI*, second is _callback_qux1):
 *          #callback qux(first, second: void(*)())
 *
 *  INTRINSICS:
 *      .device
 *      .parent
 *      push(ctx)
 *      pop()
 */

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
    global_ctx->set_package_name("example");

    return global_ctx;
}

int main() {
    auto* global_ctx = INIT_GCTX();
    auto parse = parser::create_parser(global_ctx);

    parse.run_parser();
    parse.check_completeness();
}