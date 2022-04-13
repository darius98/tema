#include "match.h"

#include "algorithms/equals.h"

namespace tema {

struct match_visitor {
    statement_ptr app_node;
    std::map<variable_ptr, statement_ptr> replacements;

    bool operator()(const statement::truth&) const {
        return app_node->is_truth();
    }
    bool operator()(const statement::contradiction&) const {
        return app_node->is_contradiction();
    }
    bool operator()(const variable_ptr& var) {
        const auto it = replacements.find(var);
        if (it == replacements.end()) {
            replacements.emplace(var, app_node);
            return true;
        }
        return equals(it->second.get(), app_node.get());
    }
    bool operator()(const statement::implies& expr) {
        return app_node->is_implies() &&
               visit_recursive(expr.from.get(), app_node->as_implies().from) &&
               visit_recursive(expr.to.get(), app_node->as_implies().to);
    }
    bool operator()(const statement::equiv& expr) {
        return app_node->is_equiv() &&
               visit_recursive(expr.left.get(), app_node->as_equiv().left) &&
               visit_recursive(expr.right.get(), app_node->as_equiv().right);
    }
    bool operator()(const statement::neg& expr) {
        return app_node->is_neg() &&
               visit_recursive(expr.inner.get(), app_node->as_neg().inner);
    }
    bool operator()(const statement::conj& expr) {
        if (!app_node->is_conj() || expr.inner.size() != app_node->as_conj().inner.size()) {
            return false;
        }
        const auto& app_terms = app_node->as_conj().inner;
        for (auto it = expr.inner.begin(); it != expr.inner.end(); it++) {
            if (!visit_recursive(it->get(), app_terms[it - expr.inner.begin()])) {
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
            if (!visit_recursive(it->get(), app_terms[it - expr.inner.begin()])) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool visit_recursive(const statement* a, statement_ptr new_app_node) {
        auto old_app_node = app_node;
        app_node = std::move(new_app_node);
        const auto result = a->accept_r<bool>(*this);
        app_node = old_app_node;
        return result;
    }
};

std::optional<match_result> match(const statement* law, const statement* application) {
    match_visitor visitor{.app_node = application->shared_from_this()};
    if (!law->accept_r<bool>(visitor)) {
        return std::nullopt;
    }
    return std::move(visitor.replacements);
}

}// namespace tema
