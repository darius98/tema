#pragma once

#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include "core/statement.h"
#include "core/variable.h"

namespace tema {

struct var_not_found : public std::runtime_error {
    var_not_found();
};

struct scope {
private:
    const scope* parent_;

    std::map<symbol, variable_ptr, std::less<>> vars_by_symbol;

    std::vector<statement_ptr> statements;

public:
    explicit scope(const scope* parent = nullptr);

    [[nodiscard]] bool has_parent() const;
    [[nodiscard]] const scope* parent() const;

    // TODO: Don't return set, return an iterator instead
    [[nodiscard]] std::set<variable_ptr> own_vars() const;
    [[nodiscard]] bool has_var(symbol_view sym) const;
    [[nodiscard]] bool has_var(const variable_ptr& var) const;
    [[nodiscard]] bool has_own_var(symbol_view sym) const;
    [[nodiscard]] bool has_own_var(const variable_ptr& var) const;
    [[nodiscard]] variable_ptr get_var(symbol_view sym) const;
    [[nodiscard]] variable_ptr get_own_var(symbol_view sym) const;

    // TODO: Don't return vector, return an iterator instead
    [[nodiscard]] const std::vector<statement_ptr>& own_statements() const;

    // Mutation
    void add_var(variable_ptr var);
    void add_statement(statement_ptr statement);
};

}  // namespace tema
