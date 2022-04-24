#include "core/scope.h"

#include <mcga/test_ext/matchers.hpp>

using namespace tema;
using namespace mcga::matchers;

void expect_var_not_found(symbol_view sym, scope& s) {
    using mcga::test::expect;
    expect(s.has_var(sym), isFalse);
    expect(s.has_own_var(sym), isFalse);
    expect(s.has_var(var(symbol{sym})), isFalse);
    expect(s.has_own_var(var(symbol{sym})), isFalse);
    expect([&] { (void) s.get_var(sym); }, throwsA<var_not_found>);
    expect([&] { (void) s.get_own_var(sym); }, throwsA<var_not_found>);
}

void expect_statement_not_found(statement_name_view name, scope& s) {
    using mcga::test::expect;
    expect(s.has_statement(name), isFalse);
    expect(s.has_own_statement(name), isFalse);
    expect([&] { (void) s.get_statement(name); }, throwsA<statement_not_found>);
    expect([&] { (void) s.get_own_statement(name); }, throwsA<statement_not_found>);
}

TEST_CASE("scope") {
    group("without parent", [] {
        test("constructor", [] {
            scope s;
            expect(s.has_parent(), isFalse);
            expect(s.parent(), isNull);
            expect(s.own_vars(), isEmpty);
            expect_var_not_found("P", s);
            expect(s.own_statements(), isEmpty);
            expect_statement_not_found("truth", s);
        });

        test("add a variable", [] {
            scope s;
            const auto x = var("X");
            s.add_var(x);
            expect(s.own_vars(), std::set<variable_ptr>{x});
            expect(s.has_var(x), isTrue);
            expect(s.has_var("X"), isTrue);
            expect(s.has_own_var(x), isTrue);
            expect(s.has_own_var("X"), isTrue);
            expect(s.get_var("X"), x);
            expect(s.get_own_var("X"), x);
            // Different variable doesn't exist.
            expect_var_not_found("P", s);
            // Different variable with same name.
            expect(s.has_var(var("X")), isFalse);
            expect(s.has_own_var(var("X")), isFalse);

            // Doesn't change statements
            expect(s.own_statements(), isEmpty);
            expect_statement_not_found("truth", s);
        });

        test("add a statement", [] {
            scope s;
            s.add_statement("truth", truth());
            expect(s.own_statements(), hasSize(1));
            expect(s.own_statements()[0], named_statement{"truth", truth()});
            expect(s.has_statement("truth"), isTrue);
            expect(s.has_own_statement("truth"), isTrue);
            expect(s.get_statement("truth"), named_statement{"truth", truth()});
            expect(s.get_own_statement("truth"), named_statement{"truth", truth()});
            expect_statement_not_found("other", s);

            // No variables added.
            expect(s.own_vars(), isEmpty);
            expect_var_not_found("P", s);
        });

        test("add a statement by direct construction", [] {
            scope s;
            s.add_statement(named_statement{"truth", truth()});
            expect(s.own_statements(), hasSize(1));
            expect(s.own_statements()[0], named_statement{"truth", truth()});
            expect(s.has_statement("truth"), isTrue);
            expect(s.has_own_statement("truth"), isTrue);
            expect(s.get_statement("truth"), named_statement{"truth", truth()});
            expect(s.get_own_statement("truth"), named_statement{"truth", truth()});
            expect_statement_not_found("other", s);

            // No variables added.
            expect(s.own_vars(), isEmpty);
            expect_var_not_found("P", s);
        });
    });

    group("with parent", [] {
        test("constructor", [] {
            scope p;
            p.add_var(var("X"));
            p.add_statement("truth", truth());
            scope s{&p};
            expect(s.has_parent(), isTrue);
            expect(s.parent(), &p);
            expect(s.own_vars(), isEmpty);
            expect_var_not_found("P", s);
            expect(s.own_statements(), isEmpty);
            expect_statement_not_found("other", s);
        });

        test("variable from parent", [] {
            const auto x = var("X");

            scope p;
            p.add_var(x);
            p.add_statement("truth", truth());

            scope s{&p};
            expect(s.own_vars(), isEmpty);
            expect(s.has_var(x), isTrue);
            expect(s.has_var("X"), isTrue);
            expect(s.has_own_var(x), isFalse);
            expect(s.has_own_var("X"), isFalse);
            expect(s.get_var("X"), x);
            expect([&s] { (void) s.get_own_var("X"); }, throwsA<var_not_found>);
            // Different variable doesn't exist.
            expect_var_not_found("P", s);
            // Different variable with same name.
            expect(s.has_var(var("X")), isFalse);
            expect(s.has_own_var(var("X")), isFalse);
        });

        test("add a variable", [] {
            const auto x = var("X");
            const auto y = var("Y");
            scope p;
            p.add_var(y);
            scope s{&p};
            s.add_var(x);
            expect(s.own_vars(), std::set<variable_ptr>{x});
            expect(s.has_var(x), isTrue);
            expect(s.has_var("X"), isTrue);
            expect(s.has_own_var(x), isTrue);
            expect(s.has_own_var("X"), isTrue);
            expect(s.get_var("X"), x);
            expect(s.get_own_var("X"), x);
            // Check parent variable is still accessible
            expect(s.has_var(y), isTrue);
            expect(s.has_var("Y"), isTrue);
            expect(s.has_own_var(y), isFalse);
            expect(s.has_own_var("Y"), isFalse);
            expect(s.get_var("Y"), y);
            expect([&s] { (void) s.get_own_var("Y"); }, throwsA<var_not_found>);
            // Different variable doesn't exist.
            expect_var_not_found("P", s);
            // Different variable with same name.
            expect(s.has_var(var("X")), isFalse);
            expect(s.has_own_var(var("X")), isFalse);
        });

        test("shadow variable from parent", [] {
            const auto x = var("X");
            scope p;
            p.add_var(x);
            scope s{&p};
            expect(s.has_var(x), isTrue);
            expect(s.has_var("X"), isTrue);
            expect(s.has_own_var(x), isFalse);
            expect(s.has_own_var("X"), isFalse);
            expect(s.get_var("X"), x);
            expect([&s] { (void) s.get_own_var("X"); }, throwsA<var_not_found>);
            const auto x2 = var("X");
            s.add_var(x2);
            expect(s.has_var(x), isTrue);// TODO: should this be true? This variable cannot be accessed through the symbol API...
            expect(s.has_var(x2), isTrue);
            expect(s.has_var("X"), isTrue);
            expect(s.has_own_var(x), isFalse);
            expect(s.has_own_var(x2), isTrue);
            expect(s.has_own_var("X"), isTrue);
            expect(s.get_var("X"), x2);
            expect(s.get_own_var("X"), x2);
        });

        test("statement from parent", [] {
            scope p;
            p.add_statement("truth", truth());

            scope s{&p};
            expect(s.own_statements(), isEmpty);
            expect(s.has_statement("truth"), isTrue);
            expect(s.has_own_statement("truth"), isFalse);
            expect(s.get_statement("truth"), named_statement{"truth", truth()});
            expect([&s] { (void) s.get_own_statement("truth"); }, throwsA<statement_not_found>);
            // Different statement doesn't exist.
            expect_statement_not_found("other", s);
        });

        test("add a statement", [] {
            scope p;
            p.add_statement("truth", truth());
            scope s{&p};
            s.add_statement("contra", contradiction());
            expect(s.own_statements(), std::vector<named_statement>{{"contra", contradiction()}});
            expect(s.has_statement("contra"), isTrue);
            expect(s.has_own_statement("contra"), isTrue);
            expect(s.get_statement("contra"), named_statement{"contra", contradiction()});
            expect(s.get_own_statement("contra"), named_statement{"contra", contradiction()});
            // Check parent statement is still accessible
            expect(s.has_statement("truth"), isTrue);
            expect(s.has_own_statement("truth"), isFalse);
            expect(s.get_statement("truth"), named_statement{"truth", truth()});
            // Different statement doesn't exist.
            expect_statement_not_found("other", s);
        });

        test("shadow statement from parent", [] {
            const auto stmt = implies(contradiction(), truth());
            scope p;
            p.add_statement("stmt", stmt);
            scope s{&p};
            expect(s.has_statement("stmt"), isTrue);
            expect(s.has_own_statement("stmt"), isFalse);
            expect(s.get_statement("stmt"), named_statement{"stmt", stmt});
            expect([&s] { (void) s.get_own_statement("stmt"); }, throwsA<statement_not_found>);
            const auto stmt2 = equiv(truth(), neg(contradiction()));
            s.add_statement("stmt", stmt2);
            expect(s.has_statement("stmt"), isTrue);
            expect(s.has_own_statement("stmt"), isTrue);
            expect(s.get_statement("stmt"), named_statement{"stmt", stmt2});
            expect(s.get_own_statement("stmt"), named_statement{"stmt", stmt2});
        });
    });
}
