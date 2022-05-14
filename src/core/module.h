#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "core/scope.h"

namespace tema {

struct file_location {
    int line;
    int col;

    bool operator==(const file_location&) const = default;
};

struct module {
    struct var_decl {
        file_location loc;
        bool exported;
        variable_ptr var;
    };

    enum class stmt_decl_type {
        definition = 0,
        theorem = 1,
        exercise = 2,
    };

    struct stmt_decl {
        file_location loc;
        bool exported;
        stmt_decl_type type;
        std::string name;
        statement_ptr stmt;
        std::optional<scope> proof_description;
    };

    using decl = std::variant<var_decl, stmt_decl>;

private:
    std::string name;
    std::string file_name;
    scope internal_scope;
    scope public_scope;
    std::vector<decl> decls;

public:
    explicit module(std::string name, std::string file_name);

    [[nodiscard]] std::string_view get_name() const;
    [[nodiscard]] std::string_view get_file_name() const;
    [[nodiscard]] const scope& get_internal_scope() const;
    [[nodiscard]] const scope& get_public_scope() const;
    [[nodiscard]] const std::vector<decl>& get_decls() const;

    void add_variable_decl(var_decl var);
    void add_statement_decl(stmt_decl stmt);
};

}  // namespace tema
