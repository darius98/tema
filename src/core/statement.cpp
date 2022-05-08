#include "core/statement.h"

#include "util/export.h"

namespace tema {

TEMA_EXPORT bool statement::is_truth() const noexcept {
    return holds_alternative<truth>(data);
}
TEMA_EXPORT const statement::truth& statement::as_truth() const {
    return get<truth>(data);
}

TEMA_EXPORT bool statement::is_contradiction() const noexcept {
    return holds_alternative<contradiction>(data);
}
TEMA_EXPORT const statement::contradiction& statement::as_contradiction() const {
    return get<contradiction>(data);
}

TEMA_EXPORT bool statement::is_implies() const noexcept {
    return holds_alternative<implies>(data);
}
TEMA_EXPORT const statement::implies& statement::as_implies() const {
    return get<implies>(data);
}

TEMA_EXPORT bool statement::is_equiv() const noexcept {
    return holds_alternative<equiv>(data);
}
TEMA_EXPORT const statement::equiv& statement::as_equiv() const {
    return get<equiv>(data);
}

TEMA_EXPORT bool statement::is_neg() const noexcept {
    return holds_alternative<neg>(data);
}
TEMA_EXPORT const statement::neg& statement::as_neg() const {
    return get<neg>(data);
}

TEMA_EXPORT bool statement::is_conj() const noexcept {
    return holds_alternative<conj>(data);
}
TEMA_EXPORT const statement::conj& statement::as_conj() const {
    return get<conj>(data);
}

TEMA_EXPORT bool statement::is_disj() const noexcept {
    return holds_alternative<disj>(data);
}
TEMA_EXPORT const statement::disj& statement::as_disj() const {
    return get<disj>(data);
}

TEMA_EXPORT bool statement::is_forall() const noexcept {
    return holds_alternative<forall>(data);
}
TEMA_EXPORT const statement::forall& statement::as_forall() const {
    return get<forall>(data);
}

TEMA_EXPORT bool statement::is_var() const noexcept {
    return holds_alternative<variable_ptr>(data);
}
TEMA_EXPORT variable_ptr statement::as_var() const {
    return get<variable_ptr>(data);
}

TEMA_EXPORT bool statement::is_rel() const noexcept {
    return holds_alternative<relationship>(data);
}
TEMA_EXPORT const relationship& statement::as_rel() const {
    return get<relationship>(data);
}

TEMA_EXPORT statement_ptr truth() {
    static statement_ptr universal_truth = std::make_shared<const statement>(statement::private_tag{}, statement::truth{});
    return universal_truth;
}

TEMA_EXPORT statement_ptr contradiction() {
    static statement_ptr universal_contradiction = std::make_shared<const statement>(statement::private_tag{}, statement::contradiction{});
    return universal_contradiction;
}

TEMA_EXPORT statement_ptr implies(statement_ptr from, statement_ptr to) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::implies{std::move(from), std::move(to)});
}

TEMA_EXPORT statement_ptr equiv(statement_ptr left, statement_ptr right) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::equiv{std::move(left), std::move(right)});
}

TEMA_EXPORT statement_ptr neg(statement_ptr stmt) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::neg{std::move(stmt)});
}

TEMA_EXPORT statement_ptr conj(std::vector<statement_ptr> stmts) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::conj{std::move(stmts)});
}

TEMA_EXPORT statement_ptr disj(std::vector<statement_ptr> stmts) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::disj{std::move(stmts)});
}

TEMA_EXPORT statement_ptr forall(variable_ptr var, statement_ptr inner) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::forall{std::move(var), std::move(inner)});
}

TEMA_EXPORT statement_ptr var_stmt(variable_ptr var) {
    return std::make_shared<const statement>(statement::private_tag{}, std::move(var));
}

TEMA_EXPORT statement_ptr rel_stmt(relationship rel) {
    return std::make_shared<const statement>(statement::private_tag{}, std::move(rel));
}

TEMA_EXPORT statement_ptr rel_stmt(expr_ptr left, rel_type type, expr_ptr right) {
    return rel_stmt({
            .type = type,
            .left = std::move(left),
            .right = std::move(right),
    });
}

}  // namespace tema
