#include "apply_vars.h"

namespace tema {

struct apply_vars_visitor {
    const std::map<variable_ptr, statement_ptr>& replacements;
    std::set<variable_ptr> unmatched_vars;

    explicit apply_vars_visitor(const std::map<variable_ptr, statement_ptr>& replacements)
        : replacements(replacements) {}

    statement_ptr operator()(const statement::truth&) const {
        return nullptr;
    }
    statement_ptr operator()(const statement::contradiction&) const {
        return nullptr;
    }
    statement_ptr operator()(const variable_ptr& var) {
        const auto it = replacements.find(var);
        if (it == replacements.end()) {
            unmatched_vars.insert(var);
            return nullptr;
        }
        return it->second;
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
                new_children[it - expr.inner.begin()] = new_child;
            }
        }
        if (new_children.empty()) {
            return nullptr;
        }
        for (auto it = new_children.begin(); it != new_children.end(); it++) {
            if (*it == nullptr) {
                *it = expr.inner[it - new_children.begin()];
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
                new_children[it - expr.inner.begin()] = new_child;
            }
        }
        if (new_children.empty()) {
            return nullptr;
        }
        for (auto it = new_children.begin(); it != new_children.end(); it++) {
            if (*it == nullptr) {
                *it = expr.inner[it - new_children.begin()];
            }
        }
        return disj(std::move(new_children));
    }
};

apply_vars_result apply_vars(const statement* law, const std::map<variable_ptr, statement_ptr>& replacements) {
    apply_vars_visitor visitor{replacements};
    apply_vars_result result;
    result.stmt = law->accept_r<statement_ptr>(visitor);
    result.unmatched_vars = std::move(visitor.unmatched_vars);
    if (result.stmt == nullptr) {
        result.stmt = law->shared_from_this();
    }
    return result;
}

}// namespace tema
