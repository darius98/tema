#include "core/scope.h"

#include "export.h"

namespace tema {

TEMA_EXPORT var_not_found::var_not_found()
    : std::runtime_error{"variable not found"} {}

TEMA_EXPORT scope::scope(const scope* parent)
    : parent_(parent) {}

TEMA_EXPORT bool scope::has_parent() const {
    return parent_ != nullptr;
}

TEMA_EXPORT const scope* scope::parent() const {
    return parent_;
}

TEMA_EXPORT std::set<variable_ptr> scope::own_vars() const {
    std::set<variable_ptr> vars;
    std::transform(vars_by_symbol.begin(), vars_by_symbol.end(), std::inserter(vars, vars.end()), [](const auto& p) {
        return p.second;
    });
    return vars;
}

TEMA_EXPORT bool scope::has_own_var(symbol_view sym) const {
    return vars_by_symbol.find(sym) != vars_by_symbol.end();
}

TEMA_EXPORT bool scope::has_var(symbol_view sym) const {
    const scope* current = this;
    while (current != nullptr) {
        if (current->has_own_var(sym)) {
            return true;
        }
        current = current->parent_;
    }
    return false;
}

TEMA_EXPORT bool scope::has_own_var(const variable_ptr& var) const {
    const auto it = vars_by_symbol.find(var->name);
    return it != vars_by_symbol.end() && it->second == var;
}

TEMA_EXPORT bool scope::has_var(const variable_ptr& var) const {
    const scope* current = this;
    while (current != nullptr) {
        if (current->has_own_var(var)) {
            return true;
        }
        current = current->parent_;
    }
    return false;
}

TEMA_EXPORT void scope::add_var(variable_ptr var) {
    // TODO: Maybe check that it doesn't exist already? Not sure, this is a
    //  low level API atm, will see how usage evolves.
    vars_by_symbol.emplace(var->name, var);
}

TEMA_EXPORT variable_ptr scope::get_own_var(symbol_view sym) const {
    const auto it = vars_by_symbol.find(sym);
    if (it == vars_by_symbol.end()) {
        // TODO: Maybe return nullptr instead? More efficient, but would set a
        //  dangerous precedent for error handling, not sure yet.
        throw var_not_found{};
    }
    return it->second;
}

TEMA_EXPORT variable_ptr scope::get_var(symbol_view sym) const {
    const scope* current = this;
    while (current != nullptr) {
        const auto it = current->vars_by_symbol.find(sym);
        if (it != current->vars_by_symbol.end()) {
            return it->second;
        }
        current = current->parent_;
    }
    // TODO: same as in get_own_var
    throw var_not_found{};
}

TEMA_EXPORT const std::vector<statement_ptr>& scope::own_statements() const {
    return statements;
}

TEMA_EXPORT void scope::add_statement(statement_ptr stmt) {
    statements.push_back(std::move(stmt));
}

}  // namespace tema
