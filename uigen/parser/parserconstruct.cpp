#include "parserconstruct.hpp"

namespace parser {
    bool is_fundamental(std::string_view type) {
        return (std::find(fundamental_types.cbegin(), fundamental_types.cend(), type) != fundamental_types.cend());
    }


    CallbackConstruct::CallbackConstruct(std::string_view name) : _name(name) {}

    void CallbackConstruct::add_type(std::string_view name, const std::string& type) {
        _types.emplace_back(name, type);
    }

    void CallbackConstruct::add_type(std::string_view name, std::string_view type) {
        auto type_string = static_cast<std::string>(type);
        _types.emplace_back(name, std::move(type_string));
    }
}
