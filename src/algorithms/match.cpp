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

struct match_visitor {
    statement_ptr app_stmt;
    expr_ptr app_expr;
    match_result result;

    // TODO: This will contain like 1-2 variables at most, do a flat map / vector for it.
    var_mapping bound_vars;  // For forall

    explicit match_visitor(statement_ptr app_stmt)
        : app_stmt(std::move(app_stmt)) {}

    explicit match_visitor(expr_ptr app_expr)
        : app_expr(std::move(app_expr)) {}

    bool operator()(const statement::truth&) const {
        return app_stmt->is_truth();
    }
    bool operator()(const statement::contradiction&) const {
        return app_stmt->is_contradiction();
    }
    bool operator()(const statement::implies& expr) {
        return app_stmt->is_implies() &&
               visit_recursive(*expr.from, app_stmt->as_implies().from) &&
               visit_recursive(*expr.to, app_stmt->as_implies().to);
    }
    bool operator()(const statement::equiv& expr) {
        return app_stmt->is_equiv() &&
               visit_recursive(*expr.left, app_stmt->as_equiv().left) &&
               visit_recursive(*expr.right, app_stmt->as_equiv().right);
    }
    bool operator()(const statement::neg& expr) {
        return app_stmt->is_neg() &&
               visit_recursive(*expr.inner, app_stmt->as_neg().inner);
    }
    bool operator()(const statement::conj& expr) {
        if (!app_stmt->is_conj() || expr.inner.size() != app_stmt->as_conj().inner.size()) {
            return false;
        }
        const auto& app_terms = app_stmt->as_conj().inner;
        // TODO: Use ranges to iterate both arrays at once.
        for (auto it = expr.inner.begin(); it != expr.inner.end(); it++) {
            if (!visit_recursive(**it, app_terms[static_cast<std::size_t>(it - expr.inner.begin())])) {
                return false;
            }
        }
        return true;
    }
    bool operator()(const statement::disj& expr) {
        if (!app_stmt->is_disj() || expr.inner.size() != app_stmt->as_disj().inner.size()) {
            return false;
        }
        const auto& app_terms = app_stmt->as_disj().inner;
        // TODO: Use ranges to iterate both arrays at once.
        for (auto it = expr.inner.begin(); it != expr.inner.end(); it++) {
            if (!visit_recursive(**it, app_terms[static_cast<std::size_t>(it - expr.inner.begin())])) {
                return false;
            }
        }
        return true;
    }
    bool operator()(const statement::forall& expr) {
        if (!app_stmt->is_forall()) {
            // TODO: Are we sure? There may be other cases when we can still match.
            return false;
        }
        if (bound_vars.contains(expr.var.get())) {
            throw std::runtime_error("Invalid statement, forall twice with the same variable");
        }
        const auto [it, inserted] = bound_vars.emplace(expr.var.get(), app_stmt->as_forall().var.get());
        const auto inner_matches = visit_recursive(*expr.inner, app_stmt->as_forall().inner);
        bound_vars.erase(it);
        return inner_matches;
    }
    bool operator()(const statement::var_stmt& var) {
        const auto bound_var_it = bound_vars.find(var.var.get());
        if (bound_var_it != bound_vars.end()) {
            return app_stmt->is_var() && app_stmt->as_var().get() == bound_var_it->second;
        }
        const auto it = result.stmt_replacements.find(var.var);
        if (it == result.stmt_replacements.end()) {
            result.stmt_replacements.emplace(var.var, app_stmt);
            return true;
        }
        return equals(*it->second, *app_stmt);
    }
    bool operator()(const relationship& rel) {
        if (!app_stmt->is_rel() || app_stmt->as_rel().type != rel.type) {
            return false;
        }
        if (!visit_recursive(*rel.left, app_stmt->as_rel().left)) {
            return false;
        }
        if (!visit_recursive(*rel.right, app_stmt->as_rel().right)) {
            return false;
        }
        return true;
    }
    bool operator()(const variable_ptr& var) {
        const auto bound_var_it = bound_vars.find(var.get());
        if (bound_var_it != bound_vars.end()) {
            return app_expr->is_var() && app_expr->as_var().get() == bound_var_it->second;
        }
        const auto it = result.expr_replacements.find(var);
        if (it == result.expr_replacements.end()) {
            result.expr_replacements.emplace(var, app_expr);
            return true;
        }
        return equals(*it->second, *app_expr);
    }
    bool operator()(const expression::binop& expr) {
        return app_expr->is_binop() &&
               expr.type == app_expr->as_binop().type &&
               visit_recursive(*expr.left, app_expr->as_binop().left) &&
               visit_recursive(*expr.right, app_expr->as_binop().right);
    }
    bool operator()(const expression::call& expr) {
        if (!app_expr->is_call() ||
            expr.params.size() != app_expr->as_call().params.size() ||
            !visit_recursive(*expr.callee, app_expr->as_call().callee)) {
            return false;
        }
        const auto& app_params = app_expr->as_call().params;
        // TODO: Use ranges to iterate both arrays at once.
        for (auto it = expr.params.begin(); it != expr.params.end(); it++) {
            if (!visit_recursive(**it, app_params[static_cast<std::size_t>(it - expr.params.begin())])) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool visit_recursive(const statement& a, statement_ptr new_app_stmt) {
        return visit_recursive(a, app_stmt, std::move(new_app_stmt));
    }

    [[nodiscard]] bool visit_recursive(const expression& a, expr_ptr new_app_expr) {
        return visit_recursive(a, app_expr, std::move(new_app_expr));
    }

    [[nodiscard]] bool visit_recursive(const mcga::meta::one_of<statement, expression> auto& a,
                                       mcga::meta::one_of<statement_ptr, expr_ptr> auto& app_node,
                                       mcga::meta::one_of<statement_ptr, expr_ptr> auto new_app_node) {
        auto old_app_node = std::move(app_node);
        app_node = std::move(new_app_node);
        const auto sub_result = a.template accept_r<bool>(*this);
        app_node = std::move(old_app_node);
        return sub_result;
    }
};

std::optional<match_result> match(const expression& law, expr_ptr application) {
    match_visitor visitor(std::move(application));
    if (!law.accept_r<bool>(visitor)) {
        return std::nullopt;
    }
    return std::move(visitor.result);
}

std::optional<match_result> match(const statement& law, statement_ptr application) {
    match_visitor visitor(std::move(application));
    if (!law.accept_r<bool>(visitor)) {
        return std::nullopt;
    }
    return std::move(visitor.result);
}

}  // namespace tema
