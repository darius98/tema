#pragma once

#include <memory>
#include <vector>

#include "util/overload.h"
#include "util/pack.h"
#include "variable.h"

namespace tema {

struct statement {
    using statement_ptr = std::shared_ptr<const statement>;

    struct truth {};

    struct contradiction {};

    struct forall {
        variable_ptr var;
        statement_ptr inner;
    };

    struct exists {
        variable_ptr var;
        statement_ptr inner;
    };

    struct implies {
        statement_ptr from;
        statement_ptr to;
    };

    struct equiv {
        statement_ptr left;
        statement_ptr right;
    };

    struct neg {
        statement_ptr inner;
    };

    struct conj {
        std::vector<statement_ptr> inner;
    };

    struct disj {
        std::vector<statement_ptr> inner;
    };

    using types = util::pack<truth, contradiction, forall, exists, implies, equiv, neg, conj, disj, variable_ptr>;

private:
    util::variant_for_pack<types> data;

    // Ensure that statements are only created through the friend factories below
    struct private_tag {};

public:
    template<class T>
    requires util::pack_contains<types, T>
    explicit statement(private_tag, T t)
        : data(std::move(t)) {}

    statement(const statement&) = delete;
    statement& operator=(const statement&) = delete;
    statement(statement&&) = delete;
    statement& operator=(statement&&) = delete;
    ~statement() = default;

    [[nodiscard]] bool is_truth() const noexcept;
    [[nodiscard]] const truth& as_truth() const;

    [[nodiscard]] bool is_contradiction() const noexcept;
    [[nodiscard]] const contradiction& as_contradiction() const;

    [[nodiscard]] bool is_var() const noexcept;
    [[nodiscard]] variable_ptr as_var() const;

    [[nodiscard]] bool is_forall() const noexcept;
    [[nodiscard]] const forall& as_forall() const;

    [[nodiscard]] bool is_exists() const noexcept;
    [[nodiscard]] const exists& as_exists() const;

    [[nodiscard]] bool is_implies() const noexcept;
    [[nodiscard]] const implies& as_implies() const;

    [[nodiscard]] bool is_equiv() const noexcept;
    [[nodiscard]] const equiv& as_equiv() const;

    [[nodiscard]] bool is_neg() const noexcept;
    [[nodiscard]] const neg& as_neg() const;

    [[nodiscard]] bool is_conj() const noexcept;
    [[nodiscard]] const conj& as_conj() const;

    [[nodiscard]] bool is_disj() const noexcept;
    [[nodiscard]] const disj& as_disj() const;

    template<class V>
    void accept(V&& visitor) const {
        std::visit(std::forward<V>(visitor), data);
    }

    friend statement_ptr truth();
    friend statement_ptr contradiction();
    friend statement_ptr var_stmt(variable_ptr var);
    friend statement_ptr forall(variable_ptr var, statement_ptr stmt);
    friend statement_ptr exists(variable_ptr var, statement_ptr stmt);
    friend statement_ptr implies(statement_ptr from, statement_ptr to);
    friend statement_ptr equiv(statement_ptr left, statement_ptr right);
    friend statement_ptr neg(statement_ptr stmt);
    friend statement_ptr conj(std::vector<statement_ptr> stmts);
    friend statement_ptr disj(std::vector<statement_ptr> stmts);
};

using statement_ptr = statement::statement_ptr;

statement_ptr truth();

statement_ptr contradiction();

statement_ptr var_stmt(variable_ptr var);

statement_ptr forall(variable_ptr var, statement_ptr stmt);

statement_ptr exists(variable_ptr var, statement_ptr stmt);

statement_ptr implies(statement_ptr from, statement_ptr to);

statement_ptr equiv(statement_ptr left, statement_ptr right);

statement_ptr neg(statement_ptr stmt);

statement_ptr conj(std::vector<statement_ptr> stmts);

template<class... T>
requires(std::is_same_v<T, statement_ptr>&&...) auto conj(T... stmts) {
    return conj({std::move(stmts)...});
}

statement_ptr disj(std::vector<statement_ptr> stmts);

template<class... T>
requires(std::is_same_v<T, statement_ptr>&&...) auto disj(T... stmts) {
    return disj({std::move(stmts)...});
}

}// namespace tema
