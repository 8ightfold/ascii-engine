#ifndef PROJECT3_TEST_GLOBALCONTEXT_HPP
#define PROJECT3_TEST_GLOBALCONTEXT_HPP

#include <core.hpp>
#include <parser/parserconstruct.hpp>
#include <parser/filesystem.hpp>

namespace parser {
    struct TypeContext {
        using _map_type = std::unordered_map<std::string_view, std::string_view>;
        explicit TypeContext(std::string_view type_name);

        _map_type* operator->();
        [[nodiscard]] std::string_view type() const;
        [[nodiscard]] const _map_type& values() const;

    private:
        std::string_view _type_name;
        _map_type _type_values;
        bool _fundamental = false;
    };


    struct GlobalContext {
        GlobalContext() = default;

        void set_dirs(const fs::path& exe_dir, const fs::path& core_dir);

        [[nodiscard]] fs::path get_exe() const;
        [[nodiscard]] fs::path get_core() const;
        void verify_core() const;

    private:
        fs::path _exe_dir;
        fs::path _core_dir;

        // Config stuff
        ColorType _color_type = ColorType::eDefault;
        bool _set_color_type = false;

        // TODO: Add actual type
        std::vector<int> _registered_types;
    };
}

#endif //PROJECT3_TEST_GLOBALCONTEXT_HPP
