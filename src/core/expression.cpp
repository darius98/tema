#include "core/expression.h"

namespace tema {

bool expression::is_binop() const noexcept {
    return holds_alternative<binop>(data);
}
const expression::binop& expression::as_binop() const {
    return get<binop>(data);
}

bool expression::is_var() const noexcept {
    return holds_alternative<variable_ptr>(data);
}
variable_ptr expression::as_var() const {
    return get<variable_ptr>(data);
}

expr_ptr var_expr(variable_ptr var) {
    return std::make_shared<const expression>(expression::private_tag{}, std::move(var));
}

expr_ptr binop(expression::binop content) {
    return std::make_shared<const expression>(expression::private_tag{}, std::move(content));
}

expr_ptr binop(expr_ptr left, binop_type type, expr_ptr right) {
    return binop(expression::binop{type, std::move(left), std::move(right)});
}

}// namespace tema
