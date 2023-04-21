#ifndef PROJECT3_TEST_GLOBALCONTEXT_HPP
#define PROJECT3_TEST_GLOBALCONTEXT_HPP

#include <chrono>
#include <core.hpp>
#include <parser/parserconstruct.hpp>
#include <parser/filesystem.hpp>

namespace parser {
    using steady_clock = std::chrono::steady_clock;
    using time_point = std::chrono::steady_clock::time_point;

    struct TypeContext {
        using _map_type = UnorderedMap<std::string_view>;
        explicit TypeContext(std::string_view type_name);

        _map_type* operator->();
        [[nodiscard]] std::string_view type() const;
        [[nodiscard]] const _map_type& values() const;

    private:
        std::string_view _type_name;
        _map_type _type_values;
        bool _fundamental = false;
    };

    /**
     * GlobalContext is passed to ParserCore and CodegenCore. It is the main method
     * for passing information between core objects and their subobjects.
     * Global source for config settings, type registries, etc.
     */
    struct GlobalContext {
        GlobalContext() = default;

        /**
         * Sets the paths needed for code generation.
         * @param exe_dir Path to the running executable.
         * @param core_dir Path to the main UI folder. This MUST contain the 'core.uxi' file.
         */
        void set_dirs(const fs::path& exe_dir, const fs::path& core_dir);

        void set_package_name(std::string_view name);
        void set_debug_level(std::string_view value_str);
        void set_output_filename(std::string_view name);
        void set_stacktrace_depth(std::string_view depth_str);

        void enable_timer();
        void start_timer();
        void stop_timer();

        void parse_meta(std::string_view meta);

        /**
         * Checks if the main UI folder contains the 'core.uxi' file.
         */
        void verify_core() const;

        bool check_debug_level(std::size_t value) const;

        /*
         * Standard getter functions. Pretty self-explanatory.
         */
        [[nodiscard]] fs::path get_exe() const;
        [[nodiscard]] fs::path get_core() const;
        [[nodiscard]] std::string_view get_package_name() const;
        [[nodiscard]] std::size_t get_stacktrace_depth() const;

    private:
        void _parse_directive(std::string_view directive, std::string_view value);
        void _parse_callback(std::string_view callback);
        void _parse_enable(std::string_view to_enable);

    private:
        fs::path _exe_dir;
        fs::path _core_dir;
        std::size_t _debug_level = 0;
        std::string_view _output_filename;
        std::string_view _package_name;
        std::size_t _stacktrace_depth = 1;
        bool _do_timing = false;
        time_point _time_start;
        std::unordered_map<std::string, ModuleConstruct> _modules;

        ColorType _color_type = ColorType::eDefault;
        std::string_view _entry_point;

        UnorderedMap<CallbackConstruct> _callbacks;

        bool _enabled_device = false;
        bool _enabled_intluminance = false;
    };
}

#endif //PROJECT3_TEST_GLOBALCONTEXT_HPP
