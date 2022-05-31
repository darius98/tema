#include "core/expression.h"

namespace tema {

bool expression::is_binop() const noexcept {
    return holds_alternative<binop>(data);
}
auto expression::as_binop() const -> const binop& {
    return get<binop>(data);
}

bool expression::is_call() const noexcept {
    return holds_alternative<call>(data);
}
auto expression::as_call() const -> const call& {
    return get<call>(data);
}

bool expression::is_var() const noexcept {
    return holds_alternative<variable_ptr>(data);
}
variable_ptr expression::as_var() const {
    return get<variable_ptr>(data);
}

expr_ptr var_expr(const variable_ptr& var) {
    return expression::make(var);
}

expr_ptr call(expr_ptr callee, std::vector<expr_ptr> params) {
    return expression::make(expression::call{std::move(callee), std::move(params)});
}

expr_ptr binop(expr_ptr left, binop_type type, expr_ptr right) {
    return expression::make(expression::binop{
            type,
            std::move(left),
            std::move(right),
    });
}

}  // namespace tema
