#include "core/statement.h"

namespace tema {

bool statement::is_truth() const noexcept {
    return holds_alternative<truth>(data);
}
auto statement::as_truth() const -> const truth& {
    return get<truth>(data);
}

bool statement::is_contradiction() const noexcept {
    return holds_alternative<contradiction>(data);
}
auto statement::as_contradiction() const -> const contradiction& {
    return get<contradiction>(data);
}

bool statement::is_implies() const noexcept {
    return holds_alternative<implies>(data);
}
auto statement::as_implies() const -> const implies& {
    return get<implies>(data);
}

bool statement::is_equiv() const noexcept {
    return holds_alternative<equiv>(data);
}
auto statement::as_equiv() const -> const equiv& {
    return get<equiv>(data);
}

bool statement::is_neg() const noexcept {
    return holds_alternative<neg>(data);
}
auto statement::as_neg() const -> const neg& {
    return get<neg>(data);
}

bool statement::is_conj() const noexcept {
    return holds_alternative<conj>(data);
}
auto statement::as_conj() const -> const conj& {
    return get<conj>(data);
}

bool statement::is_disj() const noexcept {
    return holds_alternative<disj>(data);
}
auto statement::as_disj() const -> const disj& {
    return get<disj>(data);
}

bool statement::is_forall() const noexcept {
    return holds_alternative<forall>(data);
}
auto statement::as_forall() const -> const forall& {
    return get<forall>(data);
}

bool statement::is_var() const noexcept {
    return holds_alternative<var_stmt>(data);
}
variable_ptr statement::as_var() const {
    return get<var_stmt>(data).var;
}

bool statement::is_rel() const noexcept {
    return holds_alternative<relationship>(data);
}
const relationship& statement::as_rel() const {
    return get<relationship>(data);
}

statement_ptr truth() {
    static statement_ptr universal_truth = statement::make(statement::truth{});
    return universal_truth;
}

statement_ptr contradiction() {
    static statement_ptr universal_contradiction = statement::make(statement::contradiction{});
    return universal_contradiction;
}

statement_ptr implies(statement_ptr from, statement_ptr to) {
    return statement::make(statement::implies{std::move(from), std::move(to)});
}

statement_ptr equiv(statement_ptr left, statement_ptr right) {
    return statement::make(statement::equiv{std::move(left), std::move(right)});
}

statement_ptr neg(statement_ptr stmt) {
    return statement::make(statement::neg{std::move(stmt)});
}

statement_ptr conj(std::vector<statement_ptr> stmts) {
    return statement::make(statement::conj{std::move(stmts)});
}

statement_ptr disj(std::vector<statement_ptr> stmts) {
    return statement::make(statement::disj{std::move(stmts)});
}

statement_ptr forall(variable_ptr var, statement_ptr inner) {
    return statement::make(statement::forall{std::move(var), std::move(inner)});
}

statement_ptr var_stmt(variable_ptr var) {
    return statement::make(statement::var_stmt{std::move(var)});
}

statement_ptr rel_stmt(expr_ptr left, rel_type type, expr_ptr right) {
    return statement::make(relationship{type, std::move(left), std::move(right)});
}

}  // namespace tema
