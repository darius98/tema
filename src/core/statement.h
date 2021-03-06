#pragma once

#include <memory>
#include <vector>

#include <mcga/meta/cvref_same_as.hpp>
#include <mcga/meta/tpack.hpp>

#include "core/expression.h"
#include "core/relationship.h"
#include "core/variable.h"

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

    struct var_stmt {
        variable_ptr var;
    };

    using types = mcga::meta::tpack<truth, contradiction, implies, equiv, neg, conj, disj, forall, var_stmt, relationship>;

private:
    mcga::meta::variant_for<types> data;

    // Ensure that statements are only created through the make factory below
    struct private_tag {};

public:
    statement(private_tag, mcga::meta::one_of_pack<types> auto data)
        : data(std::move(data)) {}

    static statement_ptr make(mcga::meta::one_of_pack<types> auto data) {
        return std::make_shared<const statement>(private_tag{}, std::move(data));
    }

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
        return std::visit(std::forward<V>(visitor), data);
    }
};

using statement_ptr = statement::statement_ptr;

[[nodiscard]] statement_ptr truth();

[[nodiscard]] statement_ptr contradiction();

[[nodiscard]] statement_ptr implies(statement_ptr from, statement_ptr to);

[[nodiscard]] statement_ptr equiv(statement_ptr left, statement_ptr right);

[[nodiscard]] statement_ptr neg(statement_ptr stmt);

[[nodiscard]] statement_ptr conj(std::vector<statement_ptr> stmts);

[[nodiscard]] statement_ptr conj(mcga::meta::cvref_same_as<statement_ptr> auto&&... stmts) {
    return conj({std::forward<decltype(stmts)>(stmts)...});
}

[[nodiscard]] statement_ptr disj(std::vector<statement_ptr> stmts);

[[nodiscard]] statement_ptr disj(mcga::meta::cvref_same_as<statement_ptr> auto&&... stmts) {
    return disj({std::forward<decltype(stmts)>(stmts)...});
}

[[nodiscard]] statement_ptr forall(variable_ptr var, statement_ptr inner);

[[nodiscard]] statement_ptr var_stmt(variable_ptr var);

[[nodiscard]] statement_ptr rel_stmt(expr_ptr left, rel_type type, expr_ptr right);

}  // namespace tema
