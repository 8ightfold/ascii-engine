#ifndef PROJECT3_TEST_PARSERCONSTRUCT_HPP
#define PROJECT3_TEST_PARSERCONSTRUCT_HPP

#include <algorithm>
#include <array>
#include <unordered_map>
#include <core.hpp>

using namespace std::literals;

namespace parser {
    template <typename T>
    using UnorderedMap = std::unordered_map<std::string_view, T>;

    inline constexpr std::array fundamental_types {
            "alias"sv, "body"sv, "button"sv, "class"sv, "title"sv
    };

    bool is_fundamental(std::string_view type);

    enum struct ItemType {
        /// Normal single value type (eg. on, 1.0, etc.)
        eValue,
        /// Bracketed type, can hold multiple values (eg. [1.0, 0.0])
        eArray,
        /// "Executed" code that can invoke callbacks, push context frames, etc
        eFunction,
        /// Double square bracketed type, used for replacement in aliases
        eReplacement
    };

    /**
     * Defines the color mode. It must match with the mode that is used by the engine.
     * The color mode will determine the format used for the 'color' item.
     * ASCII will set the format to [luminance, alpha].
     * RGBA will set the format to [red, green, blue, alpha].
     * All values are in the range [0, 1].
     */
    enum struct ColorType {
        /// Whichever the engine default is (ASCII for now)
        eDefault,
        /// Renders in B&W ASCII, uses text for luminance
        eASCII,
        /// Renders using block text in color (unimplemented)
        eRGBA,
    };

    enum struct ExpressionType {
        eInvalid, eMeta, eNode, eSkip,
    };


    /**
     * Used as a base for all "expression exceptions". Can be used as a catch all.
     */
    struct InvalidExpression : std::exception {
        [[nodiscard]] const char* what() const noexcept override { return _data.c_str(); }
        explicit InvalidExpression(std::string_view sv) : _data(sv) {}
    protected:
        std::string _data;
    };

    /**
     * Thrown when GlobalContext detects there is an invalid or empty meta expression.
     * The former will generally be caused by a parsing error.
     *
     * @see InvalidExpression
     */
    struct InvalidMetaValue : InvalidExpression {
        [[nodiscard]] const char* what() const noexcept override { return _data.c_str(); }
        explicit InvalidMetaValue(std::string_view sv) : InvalidExpression(sv) {}
    };

    /**
     * Thrown when the FileContext parser detects there is a mismatch between
     * pushed and popped types. Throws at the point of failure.
     *
     * @see InvalidExpression
     */
    struct UnbalancedExpressionTypes : InvalidExpression {
        [[nodiscard]] const char* what() const noexcept override { return _data.c_str(); }
        explicit UnbalancedExpressionTypes(std::string_view sv) : InvalidExpression(sv) {}
    };


    struct CallbackConstruct {
        /// First is the name, second is the actual type
        using _type_rep_t = std::pair<std::string_view, std::string>;

        explicit CallbackConstruct(std::string_view name);
        void add_type(std::string_view name, const std::string& type);
        void add_type(std::string_view name, std::string_view type);

    private:
        std::string_view _name;
        std::deque<_type_rep_t> _types;
    };

    struct ItemConstruct {
    private:
        std::string_view _name;
        std::deque<std::string_view> _values;
        ItemType _type;
    };

    struct ClassConstruct {
    private:
        std::deque<ItemConstruct> _class_items;
        std::deque<ClassConstruct> subclasses;
    };

    struct AliasConstruct {
    private:
    };

    struct ModuleConstruct {
        ModuleConstruct() = default;

    private:
        UnorderedMap<ClassConstruct> _classes;
        UnorderedMap<AliasConstruct> _aliases;
    };
}

#endif //PROJECT3_TEST_PARSERCONSTRUCT_HPP
