#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "core/scope.h"

namespace tema {

struct location {
    std::string file_name;
    int line;
    int col;
};

struct module {
    struct var_decl {
        location loc;
        bool exported;
        variable_ptr var;
    };

    enum class stmt_decl_type {
        definition,
        theorem,
        exercise,
    };

    struct stmt_decl {
        location loc;
        bool exported;
        stmt_decl_type type;
        std::string name;
        statement_ptr stmt;
        std::optional<scope> proof_description;
    };

    using decl = std::variant<var_decl, stmt_decl>;

private:
    std::string name;
    scope data;
    std::vector<decl> decls;

public:
    explicit module(std::string name);

    [[nodiscard]] std::string_view get_name() const;
    [[nodiscard]] const scope& get_scope() const;
    [[nodiscard]] const std::vector<decl>& get_decls() const;

    void add_variable_decl(location loc, bool exported, variable_ptr var);
    void add_statement_decl(location loc,
                            bool exported,
                            stmt_decl_type type,
                            std::string stmt_name,
                            statement_ptr stmt,
                            std::optional<scope> proof_description);
};

}  // namespace tema
