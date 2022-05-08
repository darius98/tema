#include "core/module.h"

#include "core/scope.h"
#include "util/export.h"

namespace tema {

TEMA_EXPORT module::module(std::string name)
    : name(std::move(name)) {}

TEMA_EXPORT std::string_view module::get_name() const {
    return name;
}

TEMA_EXPORT const scope& module::get_internal_scope() const {
    return internal_scope;
}

TEMA_EXPORT const scope& module::get_public_scope() const {
    return public_scope;
}

TEMA_EXPORT const std::vector<module::decl>& module::get_decls() const {
    return decls;
}

TEMA_EXPORT void module::add_variable_decl(var_decl var) {
    internal_scope.add_var(var.var);
    if (var.exported) {
        public_scope.add_var(var.var);
    }
    decls.emplace_back(std::move(var));
}

TEMA_EXPORT void module::add_statement_decl(stmt_decl stmt) {
    internal_scope.add_statement(stmt.stmt);
    if (stmt.exported) {
        public_scope.add_statement(stmt.stmt);
    }
    decls.emplace_back(std::move(stmt));
}

}  // namespace tema
