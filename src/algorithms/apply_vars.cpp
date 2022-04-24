#include "algorithms/apply_vars.h"

namespace tema {

struct apply_vars_expression_visitor {
    const match_result& replacements;
    const std::set<const variable*>& bound_vars;
    std::set<variable_ptr>& unmatched_vars;

    apply_vars_expression_visitor(const match_result& replacements,
                                  const std::set<const variable*>& bound_vars,
                                  std::set<variable_ptr>& unmatched_vars)
        : replacements(replacements), bound_vars(bound_vars), unmatched_vars(unmatched_vars) {}

    expr_ptr operator()(const variable_ptr& var) {
        const auto it = replacements.expr_replacements.find(var);
        if (it == replacements.expr_replacements.end()) {
            if (!bound_vars.contains(var.get())) {
                unmatched_vars.insert(var);
            }
            return nullptr;
        }
        return it->second;
    }

    expr_ptr operator()(const expression::binop& op) {
        auto left = op.left->accept_r<expr_ptr>(*this);
        auto right = op.right->accept_r<expr_ptr>(*this);
        if (left == nullptr && right == nullptr) {
            return nullptr;
        }
        left = left ? left : op.left;
        right = right ? right : op.right;
        return binop(std::move(left), op.type, std::move(right));
    }
};

struct apply_vars_statement_visitor {
    const match_result& replacements;
    std::set<variable_ptr> unmatched_vars;
    std::set<const variable*> bound_vars;  // These are not unmatched because they are bound by a forall/exists statement.

    explicit apply_vars_statement_visitor(const match_result& replacements)
        : replacements(replacements) {}

    statement_ptr operator()(const statement::truth&) const {
        return nullptr;
    }
    statement_ptr operator()(const statement::contradiction&) const {
        return nullptr;
    }
    statement_ptr operator()(const statement::implies& expr) {
        auto from = expr.from->accept_r<statement_ptr>(*this);
        auto to = expr.to->accept_r<statement_ptr>(*this);
        if (from == nullptr && to == nullptr) {
            return nullptr;
        }
        from = from ? from : expr.from;
        to = to ? to : expr.to;
        return implies(std::move(from), std::move(to));
    }
    statement_ptr operator()(const statement::equiv& expr) {
        auto left = expr.left->accept_r<statement_ptr>(*this);
        auto right = expr.right->accept_r<statement_ptr>(*this);
        if (left == nullptr && right == nullptr) {
            return nullptr;
        }
        left = left ? left : expr.left;
        right = right ? right : expr.right;
        return equiv(std::move(left), std::move(right));
    }
    statement_ptr operator()(const statement::neg& expr) {
        auto inner = expr.inner->accept_r<statement_ptr>(*this);
        return inner ? neg(std::move(inner)) : nullptr;
    }
    statement_ptr operator()(const statement::conj& expr) {
        std::vector<statement_ptr> new_children;
        for (auto it = expr.inner.begin(); it != expr.inner.end(); it++) {
            auto new_child = (*it)->accept_r<statement_ptr>(*this);
            if (new_child != nullptr) {
                new_children.resize(expr.inner.size());
                // TODO: Figure out how to do this with ranges without a conversion
                new_children[size_t(it - expr.inner.begin())] = new_child;
            }
        }
        if (new_children.empty()) {
            return nullptr;
        }
        for (auto it = new_children.begin(); it != new_children.end(); it++) {
            if (*it == nullptr) {
                // TODO: Figure out how to do this with ranges without a conversion
                *it = expr.inner[size_t(it - new_children.begin())];
            }
        }
        return conj(std::move(new_children));
    }
    statement_ptr operator()(const statement::disj& expr) {
        std::vector<statement_ptr> new_children;
        for (auto it = expr.inner.begin(); it != expr.inner.end(); it++) {
            auto new_child = (*it)->accept_r<statement_ptr>(*this);
            if (new_child != nullptr) {
                new_children.resize(expr.inner.size());
                // TODO: Figure out how to do this with ranges without a conversion
                new_children[size_t(it - expr.inner.begin())] = new_child;
            }
        }
        if (new_children.empty()) {
            return nullptr;
        }
        for (auto it = new_children.begin(); it != new_children.end(); it++) {
            if (*it == nullptr) {
                // TODO: Figure out how to do this with ranges without a conversion
                *it = expr.inner[size_t(it - new_children.begin())];
            }
        }
        return disj(std::move(new_children));
    }
    statement_ptr operator()(const statement::forall& expr) {
        if (bound_vars.contains(expr.var.get())) {
            throw std::runtime_error("Invalid statement, forall twice with the same variable");
        }
        bound_vars.insert(expr.var.get());
        auto inner = expr.inner->accept_r<statement_ptr>(*this);
        bound_vars.erase(expr.var.get());
        if (replacements.stmt_replacements.contains(expr.var) || replacements.expr_replacements.contains(expr.var)) {
            // If we replaced this variable with something, there is no longer a need for this forall.
            return inner;
        }
        if (inner == nullptr) {
            return nullptr;
        }
        return forall(expr.var, inner);
    }
    statement_ptr operator()(const variable_ptr& var) {
        const auto it = replacements.stmt_replacements.find(var);
        if (it == replacements.stmt_replacements.end()) {
            if (!bound_vars.contains(var.get())) {
                unmatched_vars.insert(var);
            }
            return nullptr;
        }
        return it->second;
    }
    statement_ptr operator()(const relationship& rel) {
        auto left = rel.left->accept_r<expr_ptr>(apply_vars_expression_visitor{replacements, bound_vars, unmatched_vars});
        auto right = rel.right->accept_r<expr_ptr>(apply_vars_expression_visitor{replacements, bound_vars, unmatched_vars});
        if (left == nullptr && right == nullptr) {
            return nullptr;
        }
        left = left ? left : rel.left;
        right = right ? right : rel.right;
        return rel_stmt(std::move(left), rel.type, std::move(right));
    }
};

apply_vars_result apply_vars(const statement_ptr& law, const match_result& replacements) {
    apply_vars_statement_visitor visitor{replacements};
    apply_vars_result result;
    result.stmt = law->accept_r<statement_ptr>(visitor);
    result.unmatched_vars = std::move(visitor.unmatched_vars);
    if (result.stmt == nullptr) {
        result.stmt = law;
    }
    return result;
}

}  // namespace tema
