#include "compiler/parser.h"

#include <string>
#include <vector>

#include <mcga/test_ext/matchers.hpp>

#include "algorithms/equals.h"
#include "algorithms/print_utf8.h"

using namespace tema;
using namespace mcga::matchers;

void expect_equals(const statement_ptr& a, const statement_ptr& b, mcga::test::Context context = mcga::test::Context()) {
    mcga::test::expectMsg(equals(a.get(), b.get()),
                          print_utf8(a.get()) + " == " + print_utf8(b.get()),
                          std::move(context));
}

void fail_to_parse_module(std::string_view code, mcga::test::Context context = mcga::test::Context()) {
    mcga::test::expect([code] {
        (void) parse_module(code);
    },
                       throwsA<parse_error>, std::move(context));
}

void fail_to_parse_stmts(std::initializer_list<const char*> vars, const std::vector<std::string>& stmts) {
    std::string module_header;
    for (const auto& var: vars) {
        module_header += "export var " + std::string{var} + "\n";
    }
    for (const auto& stmt: stmts) {
        auto module_code = module_header + "theorem \"random-" + std::to_string(&stmt - stmts.data()) + "\" " + std::string{stmt} + " proof missing\n";
        try {
            (void) parse_module(std::string_view{module_code});
            mcga::test::fail(stmt + " did not throw");
        } catch (const parse_error&) {
        } catch (...) {
            mcga::test::fail(stmt + " threw something else");
        }
    }
}

tema::module parse_stmts(std::initializer_list<const char*> vars, const std::vector<std::string>& stmts) {
    std::string module_code;
    for (const auto& var: vars) {
        module_code += "export var " + std::string{var} + "\n";
    }
    for (const auto& stmt: stmts) {
        module_code += "theorem \"random-" + std::to_string(&stmt - stmts.data()) + "\" " + std::string{stmt} + " proof missing\n";
    }
    return parse_module(std::string_view{module_code});
}

TEST_CASE("compiler parser") {
    test("invalid declaration", [] {
        // Exported nothing
        fail_to_parse_module("export");
        // Variable without name
        fail_to_parse_module("export var");
        // Variable with string literal
        fail_to_parse_module("export var \"p q\"");
        // Exported theorem
        fail_to_parse_module("export theorem \"Truth\" ⊤ proof missing");
        // Theorem missing proof clause
        fail_to_parse_module("theorem \"Truth\" ⊤");
        // Exported exercise
        fail_to_parse_module("export exercise \"Truth\" ⊤ proof missing");
        // Exercise missing proof clause
        fail_to_parse_module("exercise \"Truth\" ⊤");
        // Exercise missing statement
        fail_to_parse_module("exercise \"Truth\" proof missing");
        // Exercise missing double quotes around name
        fail_to_parse_module("exercise Truth ⊤");
        // Exported definition
        fail_to_parse_module("export definition \"Truth\" ⊤");
        // Not a declaration
        fail_to_parse_module("⊤");
        fail_to_parse_module("export ⊤");
    });

    test("valid declarations", [] {
        auto mod = parse_module(std::string_view{R"(
    var p
    export var q
    definition "Truth-definition" ⊤
    theorem "Truth-theorem" ⊤ proof missing
    exercise "Truth-exercise" ⊤ proof missing
)"});
        expect(mod.get_decls(), hasSize(5));

        expect(holds_alternative<var_decl>(mod.get_decls()[0]), isTrue);
        expect(get<var_decl>(mod.get_decls()[0]).exported, isFalse);
        expect(get<var_decl>(mod.get_decls()[0]).var->name, "p");

        expect(holds_alternative<var_decl>(mod.get_decls()[1]), isTrue);
        expect(get<var_decl>(mod.get_decls()[1]).exported, isTrue);
        expect(get<var_decl>(mod.get_decls()[1]).var->name, "q");

        expect(holds_alternative<stmt_decl>(mod.get_decls()[2]), isTrue);
        expect(get<stmt_decl>(mod.get_decls()[2]).exported, isTrue);
        expect(get<stmt_decl>(mod.get_decls()[2]).type, stmt_decl_type::definition);
        expect(get<stmt_decl>(mod.get_decls()[2]).name, "Truth-definition");
        expect(get<stmt_decl>(mod.get_decls()[2]).stmt, truth());

        expect(holds_alternative<stmt_decl>(mod.get_decls()[3]), isTrue);
        expect(get<stmt_decl>(mod.get_decls()[3]).exported, isTrue);
        expect(get<stmt_decl>(mod.get_decls()[3]).type, stmt_decl_type::theorem);
        expect(get<stmt_decl>(mod.get_decls()[3]).name, "Truth-theorem");
        expect(get<stmt_decl>(mod.get_decls()[3]).stmt, truth());

        expect(holds_alternative<stmt_decl>(mod.get_decls()[4]), isTrue);
        expect(get<stmt_decl>(mod.get_decls()[4]).exported, isFalse);
        expect(get<stmt_decl>(mod.get_decls()[4]).type, stmt_decl_type::exercise);
        expect(get<stmt_decl>(mod.get_decls()[4]).name, "Truth-exercise");
        expect(get<stmt_decl>(mod.get_decls()[4]).stmt, truth());
    });

    test("valid statements", [] {
        module mod = parse_stmts(
                {"p", "q", "A", "B", "elem"},
                {
                        "⊤",
                        "¬⊥",
                        "p",
                        "(p∧(p→q))→q",
                        "¬¬p⟷p",
                        "∀x x",
                        "∀xyz (xyz∨¬xyz)",
                        "A⊆B ⟷ (∀t (t∈A → t∈B))",
                        "elem∈A∪B ⟷ (elem∈A ∨ elem∈B)",
                });
        const auto& scope = mod.get_internal_scope();
        const auto p = scope.get_var("p");
        const auto q = scope.get_var("q");
        expect_equals(scope.own_statements()[0], truth());
        expect_equals(scope.own_statements()[1], neg(contradiction()));
        expect_equals(scope.own_statements()[2], var_stmt(p));
        expect_equals(scope.own_statements()[3], implies(conj(var_stmt(p), implies(var_stmt(p), var_stmt(q))), var_stmt(q)));
        expect_equals(scope.own_statements()[4], equiv(neg(neg(var_stmt(p))), var_stmt(p)));

        const auto x = scope.own_statements()[5]->as_forall().var;
        expect(x->name, "x");
        expect_equals(scope.own_statements()[5], forall(x, var_stmt(x)));

        const auto y = scope.own_statements()[6]->as_forall().var;
        expect(y->name, "xyz");
        expect_equals(scope.own_statements()[6], forall(y, disj(var_stmt(y), neg(var_stmt(y)))));

        const auto a = scope.get_var("A");
        const auto b = scope.get_var("B");
        const auto t = scope.own_statements()[7]->as_equiv().right->as_forall().var;
        expect(t->name, "t");
        expect_equals(scope.own_statements()[7],
                      equiv(rel_stmt(var_expr(a), rel_type::eq_is_included, var_expr(b)),
                            forall(t, implies(rel_stmt(var_expr(t), rel_type::in, var_expr(a)),
                                              rel_stmt(var_expr(t), rel_type::in, var_expr(b))))));

        const auto elem = scope.get_var("elem");
        expect_equals(scope.own_statements()[8],
                      equiv(rel_stmt(var_expr(elem), rel_type::in, binop(var_expr(a), binop_type::set_union, var_expr(b))),
                            disj(rel_stmt(var_expr(elem), rel_type::in, var_expr(a)),
                                 rel_stmt(var_expr(elem), rel_type::in, var_expr(b)))));
    });

    test("invalid statements", [] {
        fail_to_parse_stmts({"p", "q", "e"},
                            {
                                    "¬¬",
                                    "⊤⊤¬",
                                    "p q",                       // No operator between two identifiers
                                    "∨ p q",                     // Prefix notation
                                    "p q ∨",                     // Postfix notation
                                    "(p∨)¬q",                    // Mismatched parens
                                    "(p∨¬q",                     // Unclosed paren
                                    "(p",                        // Unclosed paren
                                    "p∨q∨(",                     // Trailing open paren
                                    "p∨¬q)",                     // Unopened paren
                                    "p∨()¬q",                    // Random paired parens
                                    "p∪q∪q∪q",                   // Top-level expression
                                    "(p∨¬r)",                    // Unknown variable
                                    "∀xyz (xyz∨¬r)",             // Unknown variable inside forall
                                    "e∈q∪(p∨¬p) ⟷ (e∈p ∨ e∈q)",  // Statement inside expression
                                    "e∈q∪() ⟷ (e∈p ∨ e∈q)",      // Empty parens inside expression
                            });
    });
}
