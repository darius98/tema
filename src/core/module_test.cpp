#include "core/module.h"
#include "core/scope.h"

#include "mcga/test_ext/matchers.hpp"

using namespace tema;
using namespace mcga::matchers;

TEST_CASE("module") {
    test("constructor", [&] {
        module mod("module name");
        expect(mod.get_name(), isEqualTo("module name"));
        expect(mod.get_scope().has_parent(), isFalse);
        expect(mod.get_scope().own_vars(), isEmpty);
        expect(mod.get_scope().own_statements(), isEmpty);
        expect(mod.get_decls(), isEmpty);
    });

    test("add variable declarations", [&] {
        module mod("module name");
        const auto x = var("X");
        const auto y = var("Y");
        mod.add_variable_decl({"/var/X.tema", 2, 5}, false, x);
        expect(mod.get_decls(), hasSize(1));
        expect(holds_alternative<module::var_decl>(mod.get_decls()[0]), isTrue);
        expect(get<module::var_decl>(mod.get_decls()[0]).loc.file_name, "/var/X.tema");
        expect(get<module::var_decl>(mod.get_decls()[0]).loc.line, 2);
        expect(get<module::var_decl>(mod.get_decls()[0]).loc.col, 5);
        expect(get<module::var_decl>(mod.get_decls()[0]).exported, isFalse);
        expect(get<module::var_decl>(mod.get_decls()[0]).var, x);
        // Not exported
        expect(mod.get_scope().own_vars(), isEmpty);
        expect(mod.get_scope().own_statements(), isEmpty);

        mod.add_variable_decl({"/var/Y.tema", 3, 4}, true, y);
        expect(mod.get_decls(), hasSize(2));
        expect(holds_alternative<module::var_decl>(mod.get_decls()[1]), isTrue);
        expect(get<module::var_decl>(mod.get_decls()[1]).loc.file_name, "/var/Y.tema");
        expect(get<module::var_decl>(mod.get_decls()[1]).loc.line, 3);
        expect(get<module::var_decl>(mod.get_decls()[1]).loc.col, 4);
        expect(get<module::var_decl>(mod.get_decls()[1]).exported, isTrue);
        expect(get<module::var_decl>(mod.get_decls()[1]).var, y);
        // Exported
        expect(mod.get_scope().own_vars(), hasSize(1));
        expect(mod.get_scope().own_vars(), std::set<variable_ptr>{y});
        expect(mod.get_scope().own_statements(), isEmpty);
    });

    test("add statement declarations", [&] {
        module mod("module name");
        const auto x = truth();
        const auto y = contradiction();
        mod.add_statement_decl({"/var/X.tema", 2, 5}, false, module::stmt_decl_type::definition, "Truth", x, std::nullopt);
        expect(mod.get_decls(), hasSize(1));
        expect(holds_alternative<module::stmt_decl>(mod.get_decls()[0]), isTrue);
        expect(get<module::stmt_decl>(mod.get_decls()[0]).loc.file_name, "/var/X.tema");
        expect(get<module::stmt_decl>(mod.get_decls()[0]).loc.line, 2);
        expect(get<module::stmt_decl>(mod.get_decls()[0]).loc.col, 5);
        expect(get<module::stmt_decl>(mod.get_decls()[0]).exported, isFalse);
        expect(get<module::stmt_decl>(mod.get_decls()[0]).name, "Truth");
        expect(get<module::stmt_decl>(mod.get_decls()[0]).type, module::stmt_decl_type::definition);
        expect(get<module::stmt_decl>(mod.get_decls()[0]).stmt, x);
        expect(get<module::stmt_decl>(mod.get_decls()[0]).proof_description, std::nullopt);
        // Not exported
        expect(mod.get_scope().own_vars(), isEmpty);
        expect(mod.get_scope().own_statements(), isEmpty);

        mod.add_statement_decl({"/var/Y.tema", 3, 4}, true, module::stmt_decl_type::theorem, "Contradiction", y, std::nullopt);
        expect(mod.get_decls(), hasSize(2));
        expect(holds_alternative<module::stmt_decl>(mod.get_decls()[1]), isTrue);
        expect(get<module::stmt_decl>(mod.get_decls()[1]).loc.file_name, "/var/Y.tema");
        expect(get<module::stmt_decl>(mod.get_decls()[1]).loc.line, 3);
        expect(get<module::stmt_decl>(mod.get_decls()[1]).loc.col, 4);
        expect(get<module::stmt_decl>(mod.get_decls()[1]).exported, isTrue);
        expect(get<module::stmt_decl>(mod.get_decls()[1]).name, "Contradiction");
        expect(get<module::stmt_decl>(mod.get_decls()[1]).type, module::stmt_decl_type::theorem);
        expect(get<module::stmt_decl>(mod.get_decls()[1]).stmt, y);
        expect(get<module::stmt_decl>(mod.get_decls()[1]).proof_description, std::nullopt);
        // Exported
        expect(mod.get_scope().own_vars(), isEmpty);
        expect(mod.get_scope().own_statements(), hasSize(1));
    });
}
