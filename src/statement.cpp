#include "statement.h"

namespace tema {

bool statement::is_truth() const noexcept {
    return std::holds_alternative<truth>(data);
}
const statement::truth& statement::as_truth() const {
    return std::get<truth>(data);
}

bool statement::is_contradiction() const noexcept {
    return std::holds_alternative<contradiction>(data);
}
const statement::contradiction& statement::as_contradiction() const {
    return std::get<contradiction>(data);
}

bool statement::is_var() const noexcept {
    return std::holds_alternative<variable_ptr>(data);
}
variable_ptr statement::as_var() const {
    return std::get<variable_ptr>(data);
}

bool statement::is_forall() const noexcept {
    return std::holds_alternative<forall>(data);
}
const statement::forall& statement::as_forall() const {
    return std::get<forall>(data);
}

bool statement::is_exists() const noexcept {
    return std::holds_alternative<exists>(data);
}
const statement::exists& statement::as_exists() const {
    return std::get<exists>(data);
}

bool statement::is_implies() const noexcept {
    return std::holds_alternative<implies>(data);
}
const statement::implies& statement::as_implies() const {
    return std::get<implies>(data);
}

bool statement::is_equiv() const noexcept {
    return std::holds_alternative<equiv>(data);
}
const statement::equiv& statement::as_equiv() const {
    return std::get<equiv>(data);
}

bool statement::is_neg() const noexcept {
    return std::holds_alternative<neg>(data);
}
const statement::neg& statement::as_neg() const {
    return std::get<neg>(data);
}

bool statement::is_conj() const noexcept {
    return std::holds_alternative<conj>(data);
}
const statement::conj& statement::as_conj() const {
    return std::get<conj>(data);
}

bool statement::is_disj() const noexcept {
    return std::holds_alternative<disj>(data);
}
const statement::disj& statement::as_disj() const {
    return std::get<disj>(data);
}

statement_ptr truth() {
    static statement_ptr universal_truth = std::make_shared<const statement>(statement::private_tag{}, statement::truth{});
    return universal_truth;
}

statement_ptr contradiction() {
    static statement_ptr universal_contradiction = std::make_shared<const statement>(statement::private_tag{}, statement::contradiction{});
    return universal_contradiction;
}

statement_ptr var_stmt(variable_ptr var) {
    return std::make_shared<const statement>(statement::private_tag{}, var);
}

statement_ptr forall(variable_ptr var, statement_ptr stmt) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::forall{std::move(var), std::move(stmt)});
}

statement_ptr exists(variable_ptr var, statement_ptr stmt) {
    return std::make_shared<const statement>(statement::private_tag{}, statement::exists{std::move(var), std::move(stmt)});
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

}// namespace tema
