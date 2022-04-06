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

class statement_not_found: public std::runtime_error {
public:
    statement_not_found();
};

// TODO: In the future, this will become more complex.
using statement_name = std::string; // TODO: statement_name_view

struct named_statement {
    statement_name name;
    statement_ptr stmt;

    bool operator==(const named_statement&) const = default;
};

class scope {
    scope* parent_;

    std::set<variable_ptr> vars;
    std::map<symbol, variable_ptr> vars_by_symbol;

    std::vector<named_statement> statements;

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

    [[nodiscard]] const std::vector<named_statement>& own_statements() const;
    [[nodiscard]] bool has_statement(const statement_name& name) const;
    [[nodiscard]] bool has_own_statement(const statement_name& name) const;
    [[nodiscard]] const named_statement& get_statement(const statement_name& name) const;
    [[nodiscard]] const named_statement& get_own_statement(const statement_name& name) const;
    void add_statement(named_statement statement);
    void add_statement(statement_name name, statement_ptr stmt);
};

}// namespace tema
