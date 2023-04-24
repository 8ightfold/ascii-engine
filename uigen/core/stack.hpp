#ifndef PROJECT3_TEST_STACK_HPP
#define PROJECT3_TEST_STACK_HPP

#include <deque>
#include <iterator>
#include <utility>

namespace parser {
    template <typename T>
    struct ReverseRange {
        explicit ReverseRange(const T& val) : _val(val) {}

        auto begin() const { return _val.rbegin(); }
        auto end() const { return _val.rend(); }

    private:
        const T& _val;
    };

    template <typename T, typename Cont = std::deque<T>>
    struct Stack {
        using size_type = typename Cont::size_type;
        using value_t = typename Cont::value_type;
        using ref_t = value_t&;
        using cref_t = const value_t&;
        using move_t = value_t&&;

        Stack() : _underlying_data(Cont()) {}
        Stack(const Stack& stk) : _underlying_data(stk._underlying_data) {}
        Stack(Stack&& stk) noexcept : _underlying_data(std::move(stk._underlying_data)) {}
        template <typename Iter> Stack(Iter begin, Iter end) : _underlying_data(begin, end) {}
        ~Stack() = default;

        void push(cref_t val) { _underlying_data.push_back(val); }
        void push(move_t val) { _underlying_data.push_back(std::move(val)); }

        template <typename...TT>
        decltype(auto) emplace(TT&&...tt) { return _underlying_data.emplace_back(std::forward<TT>(tt)...); }

        void erase() { _underlying_data.erase(); }

        void pop() { _underlying_data.pop_back(); }
        ref_t top() { return _underlying_data.back(); }
        cref_t top() const { return _underlying_data.back(); }

        [[nodiscard]]
        size_type size() const noexcept { return _underlying_data.size(); }

        auto begin() const { return _underlying_data.begin(); }
        auto end() const { return _underlying_data.end(); }
        auto rbegin() const { return _underlying_data.rbegin(); }
        auto rend() const { return _underlying_data.rend(); }

        [[nodiscard]]
        bool empty() const noexcept { return _underlying_data.empty(); }

        [[nodiscard]]
        auto operator<=>(const Stack&) const = default;

    private:
        Cont _underlying_data;
    };

    template <typename Cont>
    Stack(Cont) -> Stack<typename Cont::value_type, Cont>;

    template <typename Iter>
    Stack(Iter, Iter) -> Stack<typename std::iterator_traits<Iter>::value_type>;
}

#endif //PROJECT3_TEST_STACK_HPP
