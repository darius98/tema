#include "algorithms/match.h"

#include "algorithms/equals.h"

#include <set>
#include <utility>

namespace tema {

// TODO: Should this use an "equivalence" algorithm with some inlined logic
//  instead of "equals"? For example, it could do simple transformations
//  e.g. is_provably_equivalent(a<->b, b<->a)
//  or is_provably_equivalent(a&b, b&a)
//  or is_provably_equivalent(a+(b+c), (a+b)+c).

using var_mapping = std::map<const variable*, const variable*>;

[[nodiscard]] bool visit_recursive(auto& visitor,
                                   const util::one_of<statement, expression> auto* a,
                                   util::one_of<statement_ptr, expr_ptr> auto new_app_node) {
    auto old_app_node = std::move(visitor.app_node);
    visitor.app_node = std::move(new_app_node);
    const auto sub_stmt_matches = a->template accept_r<bool>(visitor);
    visitor.app_node = std::move(old_app_node);
    return sub_stmt_matches;
}

struct match_expression_visitor {
    const var_mapping* bound_vars;
    match_result* result;
    expr_ptr app_node;

    match_expression_visitor(const var_mapping* bound_vars, match_result* result, expr_ptr app_node)
        : bound_vars(bound_vars), result(result), app_node(std::move(app_node)) {}

    bool operator()(const variable_ptr& var) const {
        const auto bound_var_it = bound_vars->find(var.get());
        if (bound_var_it != bound_vars->end()) {
            return app_node->is_var() && app_node->as_var().get() == bound_var_it->second;
        }
        const auto it = result->expr_replacements.find(var);
        if (it == result->expr_replacements.end()) {
            result->expr_replacements.emplace(var, app_node);
            return true;
        }
        return equals(it->second.get(), app_node.get());
    }
    bool operator()(const expression::binop& expr) {
        return app_node->is_binop() &&
               expr.type == app_node->as_binop().type &&
               visit_recursive(*this, expr.left.get(), app_node->as_binop().left) &&
               visit_recursive(*this, expr.right.get(), app_node->as_binop().right);
    }
};

struct match_statement_visitor {
    statement_ptr app_node;
    match_result result;

    // TODO: This will contain like 1-2 variables at most, do a flat map / vector for it.
    var_mapping bound_vars;// For forall

    explicit match_statement_visitor(statement_ptr app_node)
        : app_node(std::move(app_node)) {}

    bool operator()(const statement::truth&) const {
        return app_node->is_truth();
    }
    bool operator()(const statement::contradiction&) const {
        return app_node->is_contradiction();
    }
    bool operator()(const statement::implies& expr) {
        return app_node->is_implies() &&
               visit_recursive(*this, expr.from.get(), app_node->as_implies().from) &&
               visit_recursive(*this, expr.to.get(), app_node->as_implies().to);
    }
    bool operator()(const statement::equiv& expr) {
        return app_node->is_equiv() &&
               visit_recursive(*this, expr.left.get(), app_node->as_equiv().left) &&
               visit_recursive(*this, expr.right.get(), app_node->as_equiv().right);
    }
    bool operator()(const statement::neg& expr) {
        return app_node->is_neg() &&
               visit_recursive(*this, expr.inner.get(), app_node->as_neg().inner);
    }
    bool operator()(const statement::conj& expr) {
        if (!app_node->is_conj() || expr.inner.size() != app_node->as_conj().inner.size()) {
            return false;
        }
        const auto& app_terms = app_node->as_conj().inner;
        for (auto it = expr.inner.begin(); it != expr.inner.end(); it++) {
            if (!visit_recursive(*this, it->get(), app_terms[it - expr.inner.begin()])) {
                return false;
            }
        }
        return true;
    }
    bool operator()(const statement::disj& expr) {
        if (!app_node->is_disj() || expr.inner.size() != app_node->as_disj().inner.size()) {
            return false;
        }
        const auto& app_terms = app_node->as_disj().inner;
        for (auto it = expr.inner.begin(); it != expr.inner.end(); it++) {
            if (!visit_recursive(*this, it->get(), app_terms[it - expr.inner.begin()])) {
                return false;
            }
        }
        return true;
    }
    bool operator()(const statement::forall& expr) {
        if (!app_node->is_forall()) {
            // TODO: Are we sure? There may be other cases when we can still match.
            return false;
        }
        if (bound_vars.contains(expr.var.get())) {
            throw std::runtime_error("Invalid statement, forall twice with the same variable");
        }
        const auto [it, inserted] = bound_vars.emplace(expr.var.get(), app_node->as_forall().var.get());
        const auto inner_matches = visit_recursive(*this, expr.inner.get(), app_node->as_forall().inner);
        bound_vars.erase(it);
        return inner_matches;
    }
    bool operator()(const variable_ptr& var) {
        const auto bound_var_it = bound_vars.find(var.get());
        if (bound_var_it != bound_vars.end()) {
            return app_node->is_var() && app_node->as_var().get() == bound_var_it->second;
        }
        const auto it = result.stmt_replacements.find(var);
        if (it == result.stmt_replacements.end()) {
            result.stmt_replacements.emplace(var, app_node);
            return true;
        }
        return equals(it->second.get(), app_node.get());
    }
    bool operator()(const relationship& rel) {
        if (!app_node->is_rel() || app_node->as_rel().type != rel.type) {
            return false;
        }
        if (!rel.left->accept_r<bool>(match_expression_visitor{&bound_vars, &result, app_node->as_rel().left})) {
            return false;
        }
        if (!rel.right->accept_r<bool>(match_expression_visitor{&bound_vars, &result, app_node->as_rel().right})) {
            return false;
        }
        return true;
    }
};

std::optional<match_result> match(const expression* law, expr_ptr application) {
    var_mapping bound_vars;
    match_result result;
    match_expression_visitor visitor(&bound_vars, &result, std::move(application));
    if (!law->accept_r<bool>(visitor)) {
        // TODO: Test this when it becomes possible for two expressions not to be equal, lol.
        return std::nullopt;
    }
    return result;
}

std::optional<match_result> match(const statement* law, statement_ptr application) {
    match_statement_visitor visitor(std::move(application));
    if (!law->accept_r<bool>(visitor)) {
        return std::nullopt;
    }
    return std::move(visitor.result);
}

}// namespace tema
