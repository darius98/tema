#include "core/module.h"

#include "core/scope.h"

namespace tema {

module::module(std::string name) :name(std::move(name)) {}

std::string_view module::get_name() const {
    return name;
}

const scope& module::get_scope() const {
    return data;
}

const std::vector<module::decl>& module::get_decls() const {
    return decls;
}

void module::add_variable_decl(location loc, bool exported, variable_ptr var) {
    if (exported) {
        data.add_var(var);
    }
    decls.emplace_back(var_decl{std::move(loc), exported, std::move(var)});
}

void module::add_statement_decl(location loc,
                                bool exported,
                                stmt_decl_type type,
                                std::string stmt_name,
                                statement_ptr stmt,
                                std::optional<scope> proof_description) {
    if (exported) {
        data.add_statement(stmt);
    }
    decls.emplace_back(stmt_decl{
            std::move(loc),
            exported,
            type,
            std::move(stmt_name),
            std::move(stmt),
            std::move(proof_description),
    });
}

}  // namespace tema
