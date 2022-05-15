#include "algorithms/equals.h"

#include <map>

namespace tema {

// TODO: Optimize via some hashes in statements

using var_mapping = std::map<const variable*, const variable*>;

// Check if two variables are "equal", taking into account variables bound using "forall" statements.
bool var_equals(const var_mapping& bound_vars,
                const variable* a,
                const mcga::meta::one_of<expression, statement> auto* b) {
    if (!b->is_var()) {
        return false;
    }
    if (a == b->as_var().get()) {
        return true;
    }
    const auto it = bound_vars.find(a);
    return it != bound_vars.end() && it->second == b->as_var().get();
}

struct equals_visitor {
    const statement* b_stmt{nullptr};
    const expression* b_expr{nullptr};

    // TODO: This will contain like 1-2 variables at most, do a flat map / vector for it.
    std::map<const variable*, const variable*> bound_vars;

    explicit equals_visitor(const statement& b_stmt)
        : b_stmt(&b_stmt) {}

    explicit equals_visitor(const expression& b_expr)
        : b_expr(&b_expr) {}

    bool operator()(const statement::truth&) const {
        // As truth is a singleton, this would be the same pointer, covered below.
        return false;
    }
    bool operator()(const statement::contradiction&) const {
        // As contradiction is a singleton, this would be the same pointer, covered below.
        return false;
    }
    bool operator()(const statement::implies& a) {
        return b_stmt->is_implies() &&
               visit_recursive(*a.from, *b_stmt->as_implies().from) &&
               visit_recursive(*a.to, *b_stmt->as_implies().to);
    }
    bool operator()(const statement::equiv& a) {
        return b_stmt->is_equiv() &&
               visit_recursive(*a.left, *b_stmt->as_equiv().left) &&
               visit_recursive(*a.right, *b_stmt->as_equiv().right);
    }
    bool operator()(const statement::neg& a) {
        return b_stmt->is_neg() &&
               visit_recursive(*a.inner, *b_stmt->as_neg().inner);
    }
    bool operator()(const statement::conj& a) {
        if (!b_stmt->is_conj() || a.inner.size() != b_stmt->as_conj().inner.size()) {
            return false;
        }
        const auto& app_terms = b_stmt->as_conj().inner;
        // TODO: Use ranges to iterate both arrays at once.
        for (auto it = a.inner.begin(); it != a.inner.end(); it++) {
            if (!visit_recursive(**it, *app_terms[size_t(it - a.inner.begin())])) {
                return false;
            }
        }
        return true;
    }
    bool operator()(const statement::disj& a) {
        if (!b_stmt->is_disj() || a.inner.size() != b_stmt->as_disj().inner.size()) {
            return false;
        }
        const auto& app_terms = b_stmt->as_disj().inner;
        // TODO: Use ranges to iterate both arrays at once.
        for (auto it = a.inner.begin(); it != a.inner.end(); it++) {
            if (!visit_recursive(**it, *app_terms[size_t(it - a.inner.begin())])) {
                return false;
            }
        }
        return true;
    }
    bool operator()(const statement::forall& a) {
        if (!b_stmt->is_forall()) {
            return false;
        }
        if (bound_vars.contains(a.var.get())) {
            throw std::runtime_error("Invalid statement, forall twice with the same variable");
        }
        const auto [it, inserted] = bound_vars.emplace(a.var.get(), b_stmt->as_forall().var.get());
        const auto result = visit_recursive(*a.inner, *b_stmt->as_forall().inner);
        bound_vars.erase(it);
        return result;
    }
    bool operator()(const statement::var_stmt& a) const {
        return var_equals(bound_vars, a.var.get(), b_stmt);
    }
    bool operator()(const relationship& a) {
        return b_stmt->is_rel() &&
               a.type == b_stmt->as_rel().type &&
               visit_recursive(*a.left, *b_stmt->as_rel().left) &&
               visit_recursive(*a.right, *b_stmt->as_rel().right);
    }
    bool operator()(const variable_ptr& a) const {
        return var_equals(bound_vars, a.get(), b_expr);
    }
    bool operator()(const expression::binop& a) {
        return b_expr->is_binop() &&
               a.type == b_expr->as_binop().type &&
               visit_recursive(*a.left, *b_expr->as_binop().left) &&
               visit_recursive(*a.right, *b_expr->as_binop().right);
    }

    [[nodiscard]] bool visit_recursive(const statement& a, const statement& new_b) {
        if (&a == &new_b) {
            // Optimize the case when it's the same pointer
            return true;
        }
        auto old_b = std::exchange(b_stmt, &new_b);
        const auto result = a.template accept_r<bool>(*this);
        b_stmt = old_b;
        return result;
    }

    [[nodiscard]] bool visit_recursive(const expression& a, const expression& new_b) {
        if (&a == &new_b) {
            // Optimize the case when it's the same pointer
            return true;
        }
        auto old_b = std::exchange(b_expr, &new_b);
        const auto result = a.template accept_r<bool>(*this);
        b_expr = old_b;
        return result;
    }
};

bool equals(const expression& a, const expression& b) {
    if (&a == &b) {
        return true;
    }
    return a.accept_r<bool>(equals_visitor{b});
}

bool equals(const statement& a, const statement& b) {
    if (&a == &b) {
        return true;
    }
    return a.accept_r<bool>(equals_visitor{b});
}

}  // namespace tema
