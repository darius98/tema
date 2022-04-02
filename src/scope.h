#pragma once

#include <map>
#include <set>
#include <stdexcept>

#include "statement.h"
#include "variable.h"

namespace tema {

class var_not_found : public std::runtime_error {
public:
    var_not_found();
};

class scope {
    scope* parent_;

    std::set<variable_ptr> vars;
    std::map<symbol, variable_ptr> vars_by_symbol;

    std::vector<statement_ptr> statements;

public:
    explicit scope(scope* parent = nullptr);

    [[nodiscard]] bool has_parent() const;
    [[nodiscard]] const scope* parent() const;

    [[nodiscard]] const std::set<variable_ptr>& own_vars() const;
    [[nodiscard]] bool has_var(const symbol& sym) const;
    [[nodiscard]] bool has_var(const variable_ptr& var) const;
    [[nodiscard]] bool has_own_var(const symbol& sym) const;
    [[nodiscard]] bool has_own_var(const variable_ptr& var) const;
    [[nodiscard]] variable_ptr get_var(const symbol& sym) const;
    [[nodiscard]] variable_ptr get_own_var(const symbol& sym) const;
    void add_var(variable_ptr var);

    [[nodiscard]] const std::vector<statement_ptr>& own_statements() const;
    void add_statement(statement_ptr statement);
};

}// namespace tema
