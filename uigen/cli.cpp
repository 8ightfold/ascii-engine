#include "cli.hpp"

struct ArgRepresentation {
    explicit ArgRepresentation(char* argv[]) : _argv(argv) {
        _exe_path = *_argv;
        ++_argv;
    }

    std::string_view next() {
        if(not *_argv) {
            std::cerr << "Argument parser attempted to read invalid memory. Exiting." << std::endl;
            std::exit(-1);
        }

        const std::size_t len = std::strlen(*_argv);
        std::string_view arg { *_argv, len };
        ++_argv;
        return arg;
    }

    [[nodiscard]] fs::path get_exe_path() const {
        return { _exe_path };
    }

    operator bool() const /* NOLINT */ {
        return *_argv;
    }

private:
    const char* _exe_path = nullptr;
    char** _argv = nullptr;
};

static void print_help() {
    const char help_text[] =
            "Usage:\n"
            "-b, --base <path>      | Path that will be used as core directory.\n"
            "-d, --debug <level>    | Choose level of debug info from 0-3.\n"
            "-h, --help             | Print this help text.\n"
            "-o, --output           | Name of the output file.\n"
            "-p, --package <name>   | Name that will be used for package.\n"
            "-s, --strace <depth>   | Increase the maximum stacktrace depth.\n"
            "-t, --time             | Measure and print compilation time.\n"
            "-v, --version          | Print UIgen version info.";

    std::cout << help_text << std::endl;
}

static void print_version() {
    std::cout << "Using UIgen version " << UIGEN_VERSION << ".\n";
    std::cout << "Created by Eightfold, 2023." << std::endl;
}

static bool check_arg(std::string_view arg, std::string_view long_arg) {
    std::string_view short_arg = long_arg.substr(1, 2);
    return (arg.starts_with(short_arg) or arg == long_arg);
}

static bool shorten_arg(std::string_view& arg, std::string_view long_arg) {
    std::string_view short_arg = long_arg.substr(1, 2);

    if(arg.starts_with(short_arg) and arg.size() > 2) {
        arg.remove_prefix(2);
        return true;
    }

    return false;
}

parser::GlobalContext* parse_args(int /* argc */, char* argv[]) {
    auto* global_ctx = new parser::GlobalContext();
    ArgRepresentation arg_span { argv };

    fs::path exe_path = arg_span.get_exe_path();
    fs::path base_path;
    std::string_view package_name;

    bool reading_base_path = false;
    bool reading_debug_level = false;
    bool reading_output_filename = false;
    bool reading_stacktrace_depth = false;
    bool reading_package_name = false;

    while(arg_span) {
        auto arg = arg_span.next();

        if(reading_base_path) {
            base_path = arg;
            reading_base_path = false;
            continue;
        }
        else if(reading_debug_level) {
            global_ctx->set_debug_level(arg);
            reading_debug_level = false;
            continue;
        }
        else if(reading_output_filename) {
            global_ctx->set_output_filename(arg);
            reading_output_filename = false;
            continue;
        }
        else if(reading_stacktrace_depth) {
            global_ctx->set_stacktrace_depth(arg);
            reading_stacktrace_depth = false;
            continue;
        }
        else if(reading_package_name) {
            package_name = arg;
            reading_package_name = false;
            continue;
        }

        if(check_arg(arg, "--base")) {
            if(shorten_arg(arg, "--base")) base_path = arg;
            else reading_base_path = true;
        }
        else if(check_arg(arg, "--debug")) {
            if(shorten_arg(arg, "--debug")) {
                global_ctx->set_debug_level(arg);
            }
            else reading_debug_level = true;
        }
        else if(check_arg(arg, "--help")) {
            print_help();
            std::exit(0);
        }
        else if(check_arg(arg, "--output")) {
            if(shorten_arg(arg, "--output")) global_ctx->set_output_filename(arg);
            else reading_output_filename = true;
        }
        else if(check_arg(arg, "--package")) {
            if(shorten_arg(arg, "--package")) package_name = arg;
            else reading_package_name = true;
        }
        else if(check_arg(arg, "--strace")) {
            if(shorten_arg(arg, "--strace")) global_ctx->set_stacktrace_depth(arg);
            else reading_stacktrace_depth = true;
        }
        else if(check_arg(arg, "--time")) {
            global_ctx->enable_timer();
        }
        else if(check_arg(arg, "--version")) {
            print_version();
            std::exit(0);
        }
        else {
            std::cerr << "Invalid argument '" << arg << "'." << std::endl;
            std::exit(-1);
        }
    }

    global_ctx->set_dirs(exe_path, base_path);
    global_ctx->set_package_name(package_name);
    global_ctx->verify_core();

    return global_ctx;
}