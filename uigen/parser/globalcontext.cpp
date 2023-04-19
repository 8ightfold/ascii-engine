#include "globalcontext.hpp"

namespace parser {
    TypeContext::TypeContext(std::string_view type_name) : _type_name(type_name) {
        _fundamental = is_fundamental(type_name);
    }

    TypeContext::_map_type* TypeContext::operator->() { return &_type_values; }
    std::string_view TypeContext::type() const { return _type_name; }
    const TypeContext::_map_type& TypeContext::values() const { return _type_values; }


    void GlobalContext::set_dirs(const fs::path& exe_dir, const fs::path& core_dir) {
        _exe_dir = exe_dir;
        _core_dir = core_dir;
    }

    fs::path GlobalContext::get_exe() const { return _exe_dir; }
    fs::path GlobalContext::get_core() const { return _core_dir; }

    void GlobalContext::verify_core() const {
        if(not fs::exists(_core_dir / "core.uxi")) {
            std::cerr << "No core file found at " << _core_dir << "." << std::endl;
            std::exit(-1);
        }
    }
}