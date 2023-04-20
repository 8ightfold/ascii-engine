#ifndef PROJECT3_TEST_GLOBALCONTEXT_HPP
#define PROJECT3_TEST_GLOBALCONTEXT_HPP

#include <core.hpp>
#include <parser/parserconstruct.hpp>
#include <parser/filesystem.hpp>

namespace parser {
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

        void parse_meta(std::string_view meta);

        /**
         * Checks if the main UI folder contains the 'core.uxi' file.
         */
        void verify_core() const;

        /**
         * Checks if the color mode has been explicitly defined.
         * This will allow us to throw an error if it is declared multiple times.
         */
        [[nodiscard]] bool defined_colormode() const;

        /*
         * Standard getter functions. Self explanatory.
         */
        [[nodiscard]] fs::path get_exe() const;
        [[nodiscard]] fs::path get_core() const;
        [[nodiscard]] std::string_view get_package_name() const;

    private:
        void _parse_directive(std::string_view directive, std::string_view value);
        void _parse_callback(std::string_view callback);
        void _parse_enable(std::string_view to_enable);

    private:
        fs::path _exe_dir;
        fs::path _core_dir;
        std::string_view _package_name;
        std::unordered_map<std::string, ModuleConstruct> _modules;

        ColorType _color_type = ColorType::eDefault;
        std::string_view _entry_point;

        UnorderedMap<CallbackConstruct> _callbacks;

        bool _enabled_device = false;
        bool _enabled_intluminance = false;
    };
}

#endif //PROJECT3_TEST_GLOBALCONTEXT_HPP
