#include "scope.h"

namespace tema {

var_not_found::var_not_found()
    : std::runtime_error{"variable not found"} {}

statement_not_found::statement_not_found()
    : std::runtime_error{"statement not found"} {}

scope::scope(scope* parent)
    : parent_(parent) {}

bool scope::has_parent() const {
    return parent_ != nullptr;
}

const scope* scope::parent() const {
    return parent_;
}

std::set<variable_ptr> scope::own_vars() const {
    std::set<variable_ptr> vars;
    std::transform(vars_by_symbol.begin(), vars_by_symbol.end(), std::inserter(vars, vars.end()), [](const auto& p) {
        return p.second;
    });
    return vars;
}

bool scope::has_own_var(symbol_view sym) const {
    return vars_by_symbol.find(sym) != vars_by_symbol.end();
}

bool scope::has_var(symbol_view sym) const {
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
    const auto it = vars_by_symbol.find(var->name);
    return it != vars_by_symbol.end() && it->second == var;
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
}

variable_ptr scope::get_own_var(symbol_view sym) const {
    const auto it = vars_by_symbol.find(sym);
    if (it == vars_by_symbol.end()) {
        // TODO: Maybe return nullptr instead? More efficient, but would set a
        //  dangerous precedent for error handling, not sure yet.
        throw var_not_found{};
    }
    return it->second;
}

variable_ptr scope::get_var(symbol_view sym) const {
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

const std::vector<named_statement>& scope::own_statements() const {
    return statements;
}

// TODO: Better (or additional) data structure for this.
static auto find_statement_it(const std::vector<named_statement>& statements, statement_name_view name) {
    return std::find_if(statements.begin(), statements.end(), [&name](const named_statement& stmt) {
        return stmt.name == name;
    });
}

bool scope::has_statement(statement_name_view name) const {
    const scope* current = this;
    while (current != nullptr) {
        if (current->has_own_statement(name)) {
            return true;
        }
        current = current->parent_;
    }
    return false;
}

bool scope::has_own_statement(statement_name_view name) const {
    return find_statement_it(statements, name) != statements.end();
}

const named_statement& scope::get_statement(statement_name_view name) const {
    const scope* current = this;
    while (current != nullptr) {
        const auto it = find_statement_it(current->statements, name);
        if (it != current->statements.end()) {
            return *it;
        }
        current = current->parent_;
    }
    // TODO: same as in get_own_var
    throw statement_not_found{};
}

const named_statement& scope::get_own_statement(statement_name_view name) const {
    const auto it = find_statement_it(statements, name);
    if (it == statements.end()) {
        // TODO: Same as in get_own_var
        throw statement_not_found{};
    }
    return *it;
}

void scope::add_statement(named_statement statement) {
    statements.push_back(std::move(statement));
}

void scope::add_statement(std::string name, statement_ptr stmt) {
    add_statement(named_statement{
            .name = std::move(name),
            .stmt = std::move(stmt),
    });
}

}// namespace tema
