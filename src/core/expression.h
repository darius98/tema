#pragma once

#include <memory>
#include <vector>

#include <mcga/meta/tpack.hpp>

#include "core/variable.h"

namespace tema {

enum class binop_type {
    set_union = 0,
    set_intersection = 1,
    set_difference = 2,
    set_sym_difference = 3,
};

struct expression {
    using expr_ptr = std::shared_ptr<const expression>;

    struct binop {
        binop_type type;
        expr_ptr left;
        expr_ptr right;
    };

    struct call {
        expr_ptr callee;
        std::vector<expr_ptr> params;
    };

    using types = mcga::meta::tpack<binop, call, variable_ptr>;

private:
    mcga::meta::variant_for<types> data;

    // Ensure that expressions are only created through the friend factories below
    struct private_tag {};

public:
    expression(private_tag, mcga::meta::one_of_pack<types> auto data)
        : data(std::move(data)) {}

    static expr_ptr make(mcga::meta::one_of_pack<types> auto data) {
        return std::make_shared<const expression>(private_tag{}, std::move(data));
    }

    expression(const expression&) = delete;
    expression& operator=(const expression&) = delete;
    expression(expression&&) = delete;
    expression& operator=(expression&&) = delete;
    ~expression() = default;

    [[nodiscard]] bool is_binop() const noexcept;
    [[nodiscard]] const binop& as_binop() const;

    [[nodiscard]] bool is_call() const noexcept;
    [[nodiscard]] const call& as_call() const;

    [[nodiscard]] bool is_var() const noexcept;
    [[nodiscard]] variable_ptr as_var() const;

    template<class V>
    void accept(V&& visitor) const {
        std::visit(std::forward<V>(visitor), data);
    }

    template<class R, class V>
    R accept_r(V&& visitor) const {
        return std::visit(std::forward<V>(visitor), data);
    }
};

using expr_ptr = expression::expr_ptr;

[[nodiscard]] expr_ptr var_expr(const variable_ptr& var);
[[nodiscard]] expr_ptr call(expr_ptr callee, std::vector<expr_ptr> params);
[[nodiscard]] expr_ptr binop(expr_ptr left, binop_type type, expr_ptr right);

}  // namespace tema
