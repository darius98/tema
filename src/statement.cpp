#include "statement.h"

namespace tema {

bool statement::is_truth() const noexcept {
    return holds_alternative<truth>(data);
}
const statement::truth& statement::as_truth() const {
    return get<truth>(data);
}

bool statement::is_contradiction() const noexcept {
    return holds_alternative<contradiction>(data);
}
const statement::contradiction& statement::as_contradiction() const {
    return get<contradiction>(data);
}

bool statement::is_implies() const noexcept {
    return holds_alternative<implies>(data);
}
const statement::implies& statement::as_implies() const {
    return get<implies>(data);
}

bool statement::is_equiv() const noexcept {
    return holds_alternative<equiv>(data);
}
const statement::equiv& statement::as_equiv() const {
    return get<equiv>(data);
}

bool statement::is_neg() const noexcept {
    return holds_alternative<neg>(data);
}
const statement::neg& statement::as_neg() const {
    return get<neg>(data);
}

bool statement::is_conj() const noexcept {
    return holds_alternative<conj>(data);
}
const statement::conj& statement::as_conj() const {
    return get<conj>(data);
}

bool statement::is_disj() const noexcept {
    return holds_alternative<disj>(data);
}
const statement::disj& statement::as_disj() const {
    return get<disj>(data);
}

bool statement::is_forall() const noexcept {
    return holds_alternative<forall>(data);
}

const statement::forall& statement::as_forall() const {
    return get<forall>(data);
}

bool statement::is_var() const noexcept {
    return holds_alternative<variable_ptr>(data);
}
variable_ptr statement::as_var() const {
    return get<variable_ptr>(data);
}

bool statement::is_rel() const noexcept {
    return holds_alternative<relationship>(data);
}

const relationship& statement::as_rel() const {
    return get<relationship>(data);
}

statement_ptr truth() {
    static statement_ptr universal_truth = std::make_shared<const statement>(statement::private_tag{}, statement::truth{});
    return universal_truth;
}

statement_ptr contradiction() {
    static statement_ptr universal_contradiction = std::make_shared<const statement>(statement::private_tag{}, statement::contradiction{});
    return universal_contradiction;
}

statement_ptr implies(statement_ptr from, statement_ptr to) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::implies{std::move(from), std::move(to)});
}

statement_ptr equiv(statement_ptr left, statement_ptr right) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::equiv{std::move(left), std::move(right)});
}

statement_ptr neg(statement_ptr stmt) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::neg{std::move(stmt)});
}

statement_ptr conj(std::vector<statement_ptr> stmts) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::conj{std::move(stmts)});
}

statement_ptr disj(std::vector<statement_ptr> stmts) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::disj{std::move(stmts)});
}

statement_ptr forall(variable_ptr var, statement_ptr inner) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::forall{std::move(var), std::move(inner)});
}

statement_ptr var_stmt(variable_ptr var) {
    return std::make_shared<const statement>(statement::private_tag{}, std::move(var));
}

statement_ptr rel_stmt(relationship rel) {
    return std::make_shared<const statement>(statement::private_tag{}, std::move(rel));
}

statement_ptr rel_stmt(expr_ptr left, rel_type type, expr_ptr right) {
    return rel_stmt({
            .type = type,
            .left = std::move(left),
            .right = std::move(right),
    });
}

}// namespace tema
