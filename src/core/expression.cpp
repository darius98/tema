#include "core/expression.h"

#include "export.h"

namespace tema {

TEMA_EXPORT bool expression::is_binop() const noexcept {
    return holds_alternative<binop>(data);
}
TEMA_EXPORT const expression::binop& expression::as_binop() const {
    return get<binop>(data);
}

TEMA_EXPORT bool expression::is_var() const noexcept {
    return holds_alternative<variable_ptr>(data);
}
TEMA_EXPORT variable_ptr expression::as_var() const {
    return get<variable_ptr>(data);
}

TEMA_EXPORT expr_ptr var_expr(const variable_ptr& var) {
    return std::make_shared<const expression>(expression::private_tag{}, var);
}

TEMA_EXPORT expr_ptr binop(expr_ptr left, binop_type type, expr_ptr right) {
    return std::make_shared<const expression>(
            expression::private_tag{},
            expression::binop{
                    type,
                    std::move(left),
                    std::move(right),
            });
}

}  // namespace tema
