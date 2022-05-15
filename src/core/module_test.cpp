#include "core/module.h"
#include "core/scope.h"

#include <mcga/test_ext/matchers.hpp>

using namespace tema;
using namespace mcga::matchers;

TEST_CASE("module") {
    test("constructor", [&] {
        module mod("stuff", "/stuff.tema");
        expect(mod.get_name(), "stuff");
        expect(mod.get_file_name(), "/stuff.tema");
        expect(mod.get_internal_scope().has_parent(), isFalse);
        expect(mod.get_internal_scope().own_vars(), isEmpty);
        expect(mod.get_internal_scope().own_statements(), isEmpty);
        expect(mod.get_public_scope().has_parent(), isFalse);
        expect(mod.get_public_scope().own_vars(), isEmpty);
        expect(mod.get_public_scope().own_statements(), isEmpty);
        expect(mod.get_decls(), isEmpty);
    });

    test("add variable declarations", [&] {
        module mod("stuff", "/stuff.tema");
        const auto x = var("X");
        const auto y = var("Y");
        mod.add_variable_decl(var_decl{
                .loc = {2, 5},
                .exported = false,
                .var = x,
        });
        expect(mod.get_decls(), hasSize(1));
        expect(holds_alternative<var_decl>(mod.get_decls()[0]), isTrue);
        expect(get<var_decl>(mod.get_decls()[0]).loc.line, 2);
        expect(get<var_decl>(mod.get_decls()[0]).loc.col, 5);
        expect(get<var_decl>(mod.get_decls()[0]).exported, isFalse);
        expect(get<var_decl>(mod.get_decls()[0]).var, x);
        // Not exported
        expect(mod.get_internal_scope().own_vars(), hasSize(1));
        expect(mod.get_internal_scope().own_vars(), std::set<variable_ptr>{x});
        expect(mod.get_internal_scope().own_statements(), isEmpty);
        expect(mod.get_public_scope().own_vars(), isEmpty);
        expect(mod.get_public_scope().own_statements(), isEmpty);

        mod.add_variable_decl(var_decl{
                .loc = {3, 4},
                .exported = true,
                .var = y,
        });
        expect(mod.get_decls(), hasSize(2));
        expect(holds_alternative<var_decl>(mod.get_decls()[1]), isTrue);
        expect(get<var_decl>(mod.get_decls()[1]).loc.line, 3);
        expect(get<var_decl>(mod.get_decls()[1]).loc.col, 4);
        expect(get<var_decl>(mod.get_decls()[1]).exported, isTrue);
        expect(get<var_decl>(mod.get_decls()[1]).var, y);
        // Exported
        expect(mod.get_internal_scope().own_vars(), hasSize(2));
        expect(mod.get_internal_scope().own_vars(), std::set<variable_ptr>{x, y});
        expect(mod.get_internal_scope().own_statements(), isEmpty);
        expect(mod.get_public_scope().own_vars(), hasSize(1));
        expect(mod.get_public_scope().own_vars(), std::set<variable_ptr>{y});
        expect(mod.get_public_scope().own_statements(), isEmpty);
    });

    test("add statement declarations", [&] {
        module mod("stuff", "/stuff.tema");
        const auto x = truth();
        const auto y = contradiction();

        mod.add_statement_decl(stmt_decl{
                .loc = {2, 5},
                .exported = false,
                .type = stmt_decl_type::definition,
                .name = "Truth",
                .stmt = x,
                .proof_description = std::nullopt,
        });
        expect(mod.get_decls(), hasSize(1));
        expect(holds_alternative<stmt_decl>(mod.get_decls()[0]), isTrue);
        expect(get<stmt_decl>(mod.get_decls()[0]).loc.line, 2);
        expect(get<stmt_decl>(mod.get_decls()[0]).loc.col, 5);
        expect(get<stmt_decl>(mod.get_decls()[0]).exported, isFalse);
        expect(get<stmt_decl>(mod.get_decls()[0]).name, "Truth");
        expect(get<stmt_decl>(mod.get_decls()[0]).type, stmt_decl_type::definition);
        expect(get<stmt_decl>(mod.get_decls()[0]).stmt, x);
        expect(get<stmt_decl>(mod.get_decls()[0]).proof_description, std::nullopt);
        // Not exported
        expect(mod.get_internal_scope().own_vars(), isEmpty);
        expect(mod.get_internal_scope().own_statements(), hasSize(1));
        expect(mod.get_internal_scope().own_statements(), std::vector<statement_ptr>{x});
        expect(mod.get_public_scope().own_vars(), isEmpty);
        expect(mod.get_public_scope().own_statements(), isEmpty);

        mod.add_statement_decl(stmt_decl{
                .loc = {3, 4},
                .exported = true,
                .type = stmt_decl_type::theorem,
                .name = "Contradiction",
                .stmt = y,
                .proof_description = std::nullopt,
        });
        expect(mod.get_decls(), hasSize(2));
        expect(holds_alternative<stmt_decl>(mod.get_decls()[1]), isTrue);
        expect(get<stmt_decl>(mod.get_decls()[1]).loc.line, 3);
        expect(get<stmt_decl>(mod.get_decls()[1]).loc.col, 4);
        expect(get<stmt_decl>(mod.get_decls()[1]).exported, isTrue);
        expect(get<stmt_decl>(mod.get_decls()[1]).name, "Contradiction");
        expect(get<stmt_decl>(mod.get_decls()[1]).type, stmt_decl_type::theorem);
        expect(get<stmt_decl>(mod.get_decls()[1]).stmt, y);
        expect(get<stmt_decl>(mod.get_decls()[1]).proof_description, std::nullopt);
        // Exported
        expect(mod.get_internal_scope().own_vars(), isEmpty);
        expect(mod.get_internal_scope().own_statements(), hasSize(2));
        expect(mod.get_internal_scope().own_statements(), std::vector<statement_ptr>{x, y});
        expect(mod.get_public_scope().own_vars(), isEmpty);
        expect(mod.get_public_scope().own_statements(), hasSize(1));
        expect(mod.get_public_scope().own_statements(), std::vector<statement_ptr>{y});
    });
}
