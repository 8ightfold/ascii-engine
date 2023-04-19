#ifndef PROJECT3_TEST_REGEX_HPP
#define PROJECT3_TEST_REGEX_HPP

#include <memory>
#include <string_view>
#include <utility>

#include <core/ctre.hpp>
#include <core/utils.hpp>

namespace parser {
    template <char...CC>
    struct CharPack {
        static constexpr std::size_t size = sizeof...(CC);
        static constexpr const char data[size] { CC... };
        static constexpr std::string_view view() noexcept { return { data, size }; }
        constexpr char operator[](std::size_t idx) const noexcept { return data[idx]; }
    };

    template <std::size_t N>
    struct FixedString {
        constexpr FixedString(FixedArray<const char, N>& idata) { std::copy(idata, idata + N, data); } // NOLINT
        constexpr char operator[](std::size_t idx) const noexcept { return data[idx]; }
        FixedArray<char, N> data = {};
        static constexpr std::size_t size = N;
    };

    template <FixedString S, std::size_t...NN>
    constexpr auto make_char_pack(std::index_sequence<NN...>) {
        return CharPack<S[NN]...>{};
    }

    template <FixedString S, typename T = decltype(make_char_pack<S>(std::make_index_sequence<S.size>{}))>
    using StaticString = std::remove_cvref_t<T>;

    struct IRegexExpr {
        virtual ~IRegexExpr() = default;
        virtual IRegexExpr* read_file(std::string_view sv) = 0;
        virtual std::string_view next() = 0;
        [[nodiscard]] virtual bool finished() const = 0;
        [[nodiscard]] virtual std::string_view expression() const = 0;
    };

    using SafeRegexExpr = std::unique_ptr<IRegexExpr>;

    template <CTRE_REGEX_INPUT_TYPE Input, typename Fn>
    struct RegexExpr final : IRegexExpr {
        explicit RegexExpr(Fn* pfunctor) : _functor(pfunctor) {}
        RegexExpr(Fn* pfunctor, std::string_view expr) : _functor(pfunctor), _expression(expr) {}
        RegexExpr(AutoWrapper<Input>, Fn* pfunctor, std::string_view expr) : _functor(pfunctor), _expression(expr) {}
        ~RegexExpr() override { delete _captures; }

        IRegexExpr* read_file(std::string_view sv) override {
            delete _captures;
            _captures = new capture_type(_match_value(sv));
            _iterator = _captures->begin();
            return this;
        }

        std::string_view next() override {
            if(not finished()) return _functor->parse(*_iterator++);
            else return "";
        }

        [[nodiscard]]
        bool finished() const override { return (_iterator == _captures->end()); }

        [[nodiscard]]
        std::string_view expression() const override { return _expression; }

    private:
        static constexpr auto _match_value = ctre::range<Input>;
        using capture_type = decltype(_match_value(std::declval<std::string_view&>()));
        using iter_type = decltype(std::declval<capture_type&>().begin());

        capture_type* _captures = nullptr;
        iter_type _iterator;
        Fn* _functor;
        std::string_view _expression;
    };

    template <CTRE_REGEX_INPUT_TYPE Input, FixedString Expr = FixedString("")>
    struct RegexBinder {
        template <typename Fn>
        SafeRegexExpr operator()(Fn& functor) const {
            using str_t = StaticString<Expr>;
            Fn* pfunc = std::addressof(functor);
            auto* pregex = new RegexExpr<Input, Fn>(pfunc, str_t::view());
            return SafeRegexExpr(pregex);
        }
    };
}

#endif //PROJECT3_TEST_REGEX_HPP
