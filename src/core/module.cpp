#include "core/module.h"

#include "core/scope.h"

namespace tema {

module::module(std::string name, std::string file_name)
    : name(std::move(name)), file_name(std::move(file_name)) {}

std::string_view module::get_name() const {
    return name;
}

std::string_view module::get_file_name() const {
    return file_name;
}

const scope& module::get_internal_scope() const {
    return internal_scope;
}

const scope& module::get_public_scope() const {
    return public_scope;
}

const std::vector<decl>& module::get_decls() const {
    return decls;
}

void module::add_variable_decl(var_decl var) {
    internal_scope.add_var(var.var);
    if (var.exported) {
        public_scope.add_var(var.var);
    }
    decls.emplace_back(std::move(var));
}

void module::add_statement_decl(stmt_decl stmt) {
    internal_scope.add_statement(stmt.stmt);
    if (stmt.exported) {
        public_scope.add_statement(stmt.stmt);
    }
    decls.emplace_back(std::move(stmt));
}

}  // namespace tema
