#include "expression.h"

namespace tema {

bool expression::is_var() const noexcept {
    return std::holds_alternative<variable_ptr>(data);
}
variable_ptr expression::as_var() const {
    return std::get<variable_ptr>(data);
}

expr_ptr var_expr(variable_ptr var) {
    return std::make_shared<const expression>(expression::private_tag{}, std::move(var));
}

}// namespace tema
