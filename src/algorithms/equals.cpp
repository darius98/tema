#include "equals.h"

#include <map>

namespace tema {

// TODO: Should this return true even if some simple transformations such as commutativity are needed?
//  e.g. equals(a<->b, b<->a) or equals(a&b, b&a)

// TODO: Optimize via some hashes in statements

using var_mapping = std::map<const variable*, const variable*>;

// Check if two variables are "equal", taking into account variables bound using "forall" statements.
bool var_equals(const var_mapping* bound_vars,
                const variable* a,
                const util::one_of_types<expression, statement> auto* b) {
    if (!b->is_var()) {
        return false;
    }
    if (a == b->as_var().get()) {
        return true;
    }
    const auto it = bound_vars->find(a);
    return it != bound_vars->end() && it->second == b->as_var().get();
}

struct equals_expression_visitor {
    const var_mapping* bound_vars;
    const expression* b;

    equals_expression_visitor(const var_mapping* bound_vars, const expression* b)
        : bound_vars(bound_vars), b(b) {}

    bool operator()(const variable_ptr& a) const {
        return var_equals(bound_vars, a.get(), b);
    }
};

struct equals_statement_visitor {
    const statement* b;

    // TODO: This will contain like 1-2 variables at most, do a flat map / vector for it.
    std::map<const variable*, const variable*> bound_vars;

    explicit equals_statement_visitor(const statement* b)
        : b(b) {}

    bool operator()(const statement::truth&) const {
        // As truth is a singleton, this would be the same pointer, covered below.
        return false;
    }
    bool operator()(const statement::contradiction&) const {
        // As contradiction is a singleton, this would be the same pointer, covered below.
        return false;
    }
    bool operator()(const statement::implies& a) {
        return b->is_implies() &&
               visit_recursive(a.from.get(), b->as_implies().from.get()) &&
               visit_recursive(a.to.get(), b->as_implies().to.get());
    }
    bool operator()(const statement::equiv& a) {
        return b->is_equiv() &&
               visit_recursive(a.left.get(), b->as_equiv().left.get()) &&
               visit_recursive(a.right.get(), b->as_equiv().right.get());
    }
    bool operator()(const statement::neg& a) {
        return b->is_neg() &&
               visit_recursive(a.inner.get(), b->as_neg().inner.get());
    }
    bool operator()(const statement::conj& a) {
        if (!b->is_conj() || a.inner.size() != b->as_conj().inner.size()) {
            return false;
        }
        const auto& app_terms = b->as_conj().inner;
        for (auto it = a.inner.begin(); it != a.inner.end(); it++) {
            if (!visit_recursive(it->get(), app_terms[it - a.inner.begin()].get())) {
                return false;
            }
        }
        return true;
    }
    bool operator()(const statement::disj& a) {
        if (!b->is_disj() || a.inner.size() != b->as_disj().inner.size()) {
            return false;
        }
        const auto& app_terms = b->as_disj().inner;
        for (auto it = a.inner.begin(); it != a.inner.end(); it++) {
            if (!visit_recursive(it->get(), app_terms[it - a.inner.begin()].get())) {
                return false;
            }
        }
        return true;
    }
    bool operator()(const statement::forall& a) {
        if (!b->is_forall()) {
            return false;
        }
        if (bound_vars.contains(a.var.get())) {
            throw std::runtime_error("Invalid statement, forall twice with the same variable");
        }
        const auto [it, inserted] = bound_vars.emplace(a.var.get(), b->as_forall().var.get());
        const auto result = visit_recursive(a.inner.get(), b->as_forall().inner.get());
        bound_vars.erase(it);
        return result;
    }

    bool visit_recursive(const statement* a, const statement* new_b) {
        if (a == new_b) {
            // Optimize the case when it's the same pointer
            return true;
        }
        auto old_b = b;
        b = new_b;
        const auto result = a->accept_r<bool>(*this);
        b = old_b;
        return result;
    }

    bool operator()(const variable_ptr& a) const {
        return var_equals(&bound_vars, a.get(), b);
    }
    bool operator()(const relationship& a) const {
        return b->is_rel() &&
               a.type == b->as_rel().type &&
               a.left->accept_r<bool>(equals_expression_visitor{&bound_vars, b->as_rel().left.get()}) &&
               a.right->accept_r<bool>(equals_expression_visitor{&bound_vars, b->as_rel().right.get()});
    }
};

bool equals(const statement* a, const statement* b) {
    if (a == b) {
        return true;
    }
    return a->accept_r<bool>(equals_statement_visitor{b});
}

}// namespace tema
