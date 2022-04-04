#pragma once

#include <type_traits>

#include "statement.h"

namespace tema {

template<class... L>
struct statement_visitor {
    using P = util::overload<L...>;

    P partial;

    explicit statement_visitor(L&&... l)
        : partial{std::forward<L>(l)...} {}

    void operator()(const statement::truth& expr) {
        if constexpr (std::is_invocable_v<P&, decltype(expr)>) {
            partial(expr);
        } else {
            static_assert(std::is_invocable_v<P&, decltype(*this), decltype(expr)>);
            partial(*this, expr);
        }
    }

    void operator()(const statement::contradiction& expr) {
        if constexpr (std::is_invocable_v<P&, decltype(expr)>) {
            partial(expr);
        } else {
            static_assert(std::is_invocable_v<P&, decltype(*this), decltype(expr)>);
            partial(*this, expr);
        }
    }

    void operator()(const variable_ptr& expr) {
        if constexpr (std::is_invocable_v<P&, decltype(expr)>) {
            partial(expr);
        } else {
            static_assert(std::is_invocable_v<P&, decltype(*this), decltype(expr)>);
            partial(*this, expr);
        }
    }

    void operator()(const statement::forall& expr) {
        if constexpr (std::is_invocable_v<P&, decltype(expr)>) {
            partial(expr);
        } else {
            static_assert(std::is_invocable_v<P&, decltype(*this), decltype(expr)>);
            partial(*this, expr);
        }
    }

    void operator()(const statement::exists& expr) {
        if constexpr (std::is_invocable_v<P&, decltype(expr)>) {
            partial(expr);
        } else {
            static_assert(std::is_invocable_v<P&, decltype(*this), decltype(expr)>);
            partial(*this, expr);
        }
    }

    void operator()(const statement::implies& expr) {
        if constexpr (std::is_invocable_v<P&, decltype(expr)>) {
            partial(expr);
        } else {
            static_assert(std::is_invocable_v<P&, decltype(*this), decltype(expr)>);
            partial(*this, expr);
        }
    }

    void operator()(const statement::equiv& expr) {
        if constexpr (std::is_invocable_v<P&, decltype(expr)>) {
            partial(expr);
        } else {
            static_assert(std::is_invocable_v<P&, decltype(*this), decltype(expr)>);
            partial(*this, expr);
        }
    }

    void operator()(const statement::neg& expr) {
        if constexpr (std::is_invocable_v<P&, decltype(expr)>) {
            partial(expr);
        } else {
            static_assert(std::is_invocable_v<P&, decltype(*this), decltype(expr)>);
            partial(*this, expr);
        }
    }

    void operator()(const statement::conj& expr) {
        if constexpr (std::is_invocable_v<P&, decltype(expr)>) {
            partial(expr);
        } else {
            static_assert(std::is_invocable_v<P&, decltype(*this), decltype(expr)>);
            partial(*this, expr);
        }
    }

    void operator()(const statement::disj& expr) {
        if constexpr (std::is_invocable_v<P&, decltype(expr)>) {
            partial(expr);
        } else {
            static_assert(std::is_invocable_v<P&, decltype(*this), decltype(expr)>);
            partial(*this, expr);
        }
    }
};
template<class... L>
statement_visitor(L...) -> statement_visitor<L...>;

}// namespace tema
