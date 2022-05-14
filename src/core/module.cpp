#include "core/module.h"

#include "core/scope.h"
#include "export.h"

namespace tema {

TEMA_EXPORT module::module(std::string name, std::string file_name)
    : name(std::move(name)), file_name(std::move(file_name)) {}

TEMA_EXPORT module::module(std::string name, std::string file_name, std::vector<decl> decls)
    : name(std::move(name)), file_name(std::move(file_name)) {
    // TODO: Optimize.
    this->decls.reserve(decls.size());
    for (auto& decl: decls) {
        if (holds_alternative<var_decl>(decl)) {
            add_variable_decl(get<var_decl>(std::move(decl)));
        } else {
            add_statement_decl(get<stmt_decl>(std::move(decl)));
        }
    }
}

TEMA_EXPORT std::string_view module::get_name() const {
    return name;
}

TEMA_EXPORT std::string_view module::get_file_name() const {
    return file_name;
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
