#include "scope.h"

namespace tema {

var_not_found::var_not_found(): std::runtime_error{"variable not found"} {}

scope::scope(scope* parent)
    : parent_(parent) {}

bool scope::has_parent() const {
    return parent_ != nullptr;
}

const scope* scope::parent() const {
    return parent_;
}

const std::set<variable_ptr>& scope::own_vars() const {
    return vars;
}

bool scope::has_own_var(const symbol& sym) const {
    return vars_by_symbol.find(sym) != vars_by_symbol.end();
}

bool scope::has_var(const symbol& sym) const {
    const scope* current = this;
    while (current != nullptr) {
        if (current->has_own_var(sym)) {
            return true;
        }
        current = current->parent_;
    }
    return false;
}

bool scope::has_own_var(const variable_ptr& var) const {
    return vars.find(var) != vars.end();
}

bool scope::has_var(const variable_ptr& var) const {
    const scope* current = this;
    while (current != nullptr) {
        if (current->has_own_var(var)) {
            return true;
        }
        current = current->parent_;
    }
    return false;
}

void scope::add_var(variable_ptr var) {
    // TODO: Maybe check that it doesn't exist already? Not sure, this is a
    //  low level API atm, will see how usage evolves.
    vars_by_symbol.emplace(var->name, var);
    vars.insert(std::move(var));
}

variable_ptr scope::get_own_var(const symbol& sym) const {
    const auto it = vars_by_symbol.find(sym);
    if (it == vars_by_symbol.end()) {
        // TODO: Maybe return nullptr instead? More efficient, but would set a
        //  dangerous precedent for error handling, not sure yet.
        throw var_not_found{};
    }
    return it->second;
}

variable_ptr scope::get_var(const symbol& sym) const {
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

void scope::add_statement(statement_ptr statement) {
    statements.push_back(std::move(statement));
}

const std::vector<statement_ptr>& scope::own_statements() const {
    return statements;
}

}// namespace tema
