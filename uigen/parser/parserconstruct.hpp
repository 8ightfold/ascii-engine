#ifndef PROJECT3_TEST_PARSERCONSTRUCT_HPP
#define PROJECT3_TEST_PARSERCONSTRUCT_HPP

#include <algorithm>
#include <array>
#include <core.hpp>

namespace parser {
    using namespace std::literals;
    inline constexpr std::array fundamental_types {
            "alias"sv, "body"sv, "button"sv, "class"sv, "title"sv
    };

    constexpr bool is_fundamental(std::string_view type) {
        return (std::find(fundamental_types.cbegin(), fundamental_types.cend(), type) != fundamental_types.cend());
    }

    enum struct ItemType {
        eValue, eArray, eFunction, eReplacement
    };

    enum struct ColorType {
        eDefault, // Whichever the engine default is (ASCII for now)
        eASCII, eRGB
    };

    struct CallbackConstruct {
    private:
        std::string_view _name;
        std::deque<std::string_view> _types;
    };

    struct ItemConstruct {
    private:
        ItemType _type;
        std::string_view name;
        std::deque<std::string_view> _values;
    };

    struct ClassConstruct {
    private:
        std::size_t _type_offset; // Index to registered type
        std::deque<ItemConstruct> _class_items;
        std::deque<ClassConstruct> subclasses;
    };

    struct AliasConstruct {
    private:
    };
}

#endif //PROJECT3_TEST_PARSERCONSTRUCT_HPP
