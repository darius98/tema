#pragma once

#include <memory>
#include <vector>

#include "core/expression.h"
#include "core/relationship.h"
#include "core/variable.h"
#include "util/cvref_same_as.h"
#include "util/tpack.h"

namespace tema {

struct statement {
    using statement_ptr = std::shared_ptr<const statement>;

    struct truth {};

    struct contradiction {};

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

    struct forall {
        variable_ptr var;
        statement_ptr inner;
    };

    using types = util::tpack<truth, contradiction, implies, equiv, neg, conj, disj, forall, variable_ptr, relationship>;

private:
    util::variant_for<types> data;

    // Ensure that statements are only created through the friend factories below
    struct private_tag {};

public:
    statement(private_tag, util::one_of_pack<types> auto t)
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

    [[nodiscard]] bool is_forall() const noexcept;
    [[nodiscard]] const forall& as_forall() const;

    [[nodiscard]] bool is_var() const noexcept;
    [[nodiscard]] variable_ptr as_var() const;

    [[nodiscard]] bool is_rel() const noexcept;
    [[nodiscard]] const relationship& as_rel() const;

    template<class V>
    void accept(V&& visitor) const {
        std::visit(std::forward<V>(visitor), data);
    }

    template<class R, class V>
    R accept_r(V&& visitor) const {
        return std::visit<R>(std::forward<V>(visitor), data);
    }

    friend statement_ptr truth();
    friend statement_ptr contradiction();
    friend statement_ptr implies(statement_ptr from, statement_ptr to);
    friend statement_ptr equiv(statement_ptr left, statement_ptr right);
    friend statement_ptr neg(statement_ptr stmt);
    friend statement_ptr conj(std::vector<statement_ptr> stmts);
    friend statement_ptr disj(std::vector<statement_ptr> stmts);
    friend statement_ptr forall(variable_ptr var, statement_ptr inner);
    friend statement_ptr var_stmt(variable_ptr var);
    friend statement_ptr rel_stmt(relationship rel);
};

using statement_ptr = statement::statement_ptr;

[[nodiscard]] statement_ptr truth();

[[nodiscard]] statement_ptr contradiction();

[[nodiscard]] statement_ptr implies(statement_ptr from, statement_ptr to);

[[nodiscard]] statement_ptr equiv(statement_ptr left, statement_ptr right);

[[nodiscard]] statement_ptr neg(statement_ptr stmt);

[[nodiscard]] statement_ptr conj(std::vector<statement_ptr> stmts);

[[nodiscard]] statement_ptr conj(util::cvref_same_as<statement_ptr> auto&&... stmts) {
    return conj({std::forward<decltype(stmts)>(stmts)...});
}

[[nodiscard]] statement_ptr disj(std::vector<statement_ptr> stmts);

[[nodiscard]] statement_ptr disj(util::cvref_same_as<statement_ptr> auto&&... stmts) {
    return disj({std::forward<decltype(stmts)>(stmts)...});
}

[[nodiscard]] statement_ptr forall(variable_ptr var, statement_ptr inner);

[[nodiscard]] statement_ptr var_stmt(variable_ptr var);

[[nodiscard]] statement_ptr rel_stmt(relationship rel);
[[nodiscard]] statement_ptr rel_stmt(expr_ptr left, rel_type type, expr_ptr right);

}// namespace tema
