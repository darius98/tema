#include "algorithms/match.h"

#include "mcga/test_ext/matchers.hpp"

#include "algorithms/equals.h"
#include "algorithms/print_utf8.h"

using namespace tema;
using namespace mcga::matchers;
using namespace mcga::test;

void expect_matches(const auto& law,
                    const auto& application,
                    const std::map<variable_ptr, statement_ptr>& expected_stmt_repls,
                    const std::map<variable_ptr, expr_ptr>& expected_expr_repls = {},
                    const Context& context = Context()) {
    const auto result = match(law.get(), application.get());
    expectMsg(result.has_value(),
              print_utf8(application.get()) +
                      " matches " +
                      print_utf8(law.get()),
              context);
    for (const auto& [var, repl]: result.value().stmt_replacements) {
        expectMsg(expected_stmt_repls.contains(var), "Unexpected replacement " + var->name + " (replaced with '" + print_utf8(repl.get()) + "')", context);
        expect(equals(repl.get(), expected_stmt_repls.find(var)->second.get()), context);
    }
    expect(result.value().stmt_replacements, hasSize(expected_stmt_repls.size()), context);

    for (const auto& [var, repl]: result.value().expr_replacements) {
        expectMsg(expected_expr_repls.contains(var), "Unexpected replacement " + var->name + " (replaced with '" + print_utf8(repl.get()) + "')", context);
        expect(equals(repl.get(), expected_expr_repls.find(var)->second.get()), context);
    }
    expect(result.value().expr_replacements, hasSize(expected_expr_repls.size()), context);
}

void expect_not_matches(const statement_ptr& law, const statement_ptr& application, Context context = Context()) {
    const auto result = match(law.get(), application.get());
    expectMsg(!result.has_value(),
              print_utf8(application.get()) +
                      " does not match " +
                      print_utf8(law.get()),
              std::move(context));
}

TEST_CASE("algorithms.match") {
    const auto p = var("P");
    const auto q = var("Q");

    group("matches", [&] {
        test("modus ponens left half (conj + implies)", [&] {
            const auto law = conj(var_stmt(p), implies(var_stmt(p), var_stmt(q)));
            const auto application = conj(implies(truth(), contradiction()), implies(implies(truth(), contradiction()), contradiction()));
            expect_matches(law, application, {
                                                     {p, implies(truth(), contradiction())},
                                                     {q, contradiction()},
                                             });
        });

        test("replace variable with itself", [&] {
            const auto law = equiv(disj(var_stmt(p), var_stmt(q)), disj(var_stmt(q), var_stmt(p)));
            const auto application = equiv(disj(truth(), var_stmt(q)), disj(var_stmt(q), truth()));
            expect_matches(law, application, {
                                                     {p, truth()},
                                                     {q, var_stmt(q)},
                                             });
        });

        test("Law without vars (truth + contradiction)", [] {
            const auto law = equiv(implies(truth(), contradiction()), contradiction());
            const auto application = equiv(implies(truth(), contradiction()), contradiction());
            expect_matches(law, application, {});
        });

        test("replace variable with variable (neg)", [&] {
            const auto law = equiv(var_stmt(p), neg(neg(var_stmt(p))));
            const auto application = equiv(var_stmt(q), neg(neg(var_stmt(q))));
            expect_matches(law, application, {{p, var_stmt(q)}});
        });

        test("ignore variable (forall)", [&] {
            const auto law = equiv(var_stmt(q), forall(p, disj(var_stmt(p), neg(var_stmt(q)))));
            const auto application = equiv(truth(), forall(p, disj(var_stmt(p), neg(truth()))));
            expect_matches(law, application, {{q, truth()}});
        });
    });

    group("no match", [&] {
        test("non-matching pattern - different disj arity", [&] {
            const auto law = equiv(var_stmt(p), disj(var_stmt(p), var_stmt(p)));
            const auto application = equiv(var_stmt(q), disj(var_stmt(q), var_stmt(q), var_stmt(q)));
            expect_not_matches(law, application);
        });

        test("non-matching pattern - disj with conj", [&] {
            const auto law = equiv(var_stmt(p), disj(var_stmt(p), var_stmt(p)));
            const auto application = equiv(var_stmt(q), conj(var_stmt(q), var_stmt(q)));
            expect_not_matches(law, application);
        });

        test("non-matching pattern - different conj arity", [&] {
            const auto law = equiv(var_stmt(p), conj(var_stmt(p), var_stmt(p)));
            const auto application = equiv(var_stmt(q), conj(var_stmt(q), var_stmt(q), var_stmt(q)));
            expect_not_matches(law, application);
        });

        test("non-matching pattern - conj with disj", [&] {
            const auto law = equiv(var_stmt(p), conj(var_stmt(p), var_stmt(p)));
            const auto application = equiv(var_stmt(q), disj(var_stmt(q), var_stmt(q)));
            expect_not_matches(law, application);
        });

        test("non-matching pattern - non-matching equiv node", [&] {
            const auto law = equiv(var_stmt(p), conj(var_stmt(p), var_stmt(p)));
            expect_not_matches(law, truth());
            expect_not_matches(law, contradiction());
            // TODO: Maybe this one should match? As equiv implies implies.
            expect_not_matches(law, implies(var_stmt(q), conj(var_stmt(q), var_stmt(q))));

            const auto law2 = equiv(conj(var_stmt(p), var_stmt(p)), var_stmt(p));
            expect_not_matches(law2, truth());
            expect_not_matches(law2, contradiction());
            // TODO: Maybe this one should match? As equiv implies implies (even in reverse).
            expect_not_matches(law2, implies(var_stmt(q), conj(var_stmt(q), var_stmt(q))));

            // TODO: Maybe this one should match? It's just a reversed conjunction.
            expect_not_matches(law2, equiv(var_stmt(p), conj(var_stmt(p), var_stmt(p))));
        });

        test("non-matching pattern - non-matching implies node", [&] {
            const auto law = implies(var_stmt(p), conj(var_stmt(p), var_stmt(p)));
            expect_not_matches(law, truth());
            expect_not_matches(law, contradiction());
            expect_not_matches(law, equiv(var_stmt(q), conj(var_stmt(q), var_stmt(q))));
        });

        test("non-matching pattern - non-matching neg node", [&] {
            const auto law = neg(conj(var_stmt(p), var_stmt(p)));
            expect_not_matches(law, truth());
            expect_not_matches(law, neg(disj(var_stmt(p), var_stmt(p))));
        });

        test("non-matching pattern - non-matching forall node", [&] {
            const auto law = equiv(var_stmt(q), forall(p, disj(var_stmt(p), neg(var_stmt(q)))));
            expect_not_matches(law, equiv(var_stmt(p), truth()));
            expect_not_matches(law, equiv(var_stmt(p), disj(var_stmt(p), neg(var_stmt(q)))));
        });

        test("invalid forall matching", [&] {
            const auto x = var("X");
            const auto y = var("Y");

            expect_not_matches(forall(p, forall(q, implies(var_stmt(p), var_stmt(q)))),
                               forall(x, forall(y, implies(var_stmt(y), var_stmt(x)))));
            expect_not_matches(forall(p, forall(q, implies(var_stmt(p), var_stmt(q)))),
                               forall(x, forall(y, implies(var_stmt(x), truth()))));

            // Invalid IR coming in
            expect([&] {
                expect_not_matches(forall(p, forall(p, disj(var_stmt(p), var_stmt(p)))),
                                   forall(q, forall(q, disj(var_stmt(q), var_stmt(q)))));
            },
                   throwsA<std::runtime_error>);
        });

        test("different variable replacements", [&] {
            const auto law = conj(var_stmt(p), implies(var_stmt(p), var_stmt(q)));
            const auto application = conj(implies(truth(), contradiction()), implies(implies(truth(), truth()), contradiction()));
            expect_not_matches(law, application);
        });

        test("different variable replacements for two variables", [&] {
            const auto law = implies(disj(var_stmt(p), var_stmt(q)), disj(var_stmt(q), var_stmt(p)));
            const auto application = implies(disj(truth(), contradiction()), disj(truth(), contradiction()));
            expect_not_matches(law, application);
        });
    });

    group("expressions", [&] {
        const auto x = var("x");
        const auto y = var("y");
        const auto s = var("s");
        const auto t = var("t");

        test("match relationship statements", [&] {
            expect_matches(rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)),
                           rel_stmt(var_expr(s), rel_type::eq_is_included, var_expr(t)),
                           {},
                           {
                                   {x, var_expr(s)},
                                   {y, var_expr(t)},
                           });

            // With bound variable
            expect_matches(forall(x, disj(
                                             rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)),
                                             rel_stmt(var_expr(x), rel_type::n_eq_is_included, var_expr(y)))),
                           forall(s, disj(
                                             rel_stmt(var_expr(s), rel_type::eq_is_included, var_expr(t)),
                                             rel_stmt(var_expr(s), rel_type::n_eq_is_included, var_expr(t)))),
                           {},
                           {
                                   {y, var_expr(t)},
                           });
        });

        test("not a match relationship statements", [&] {
            // match with non-relationship
            expect_not_matches(rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)), var_stmt(p));

            // match with different relationship type
            expect_not_matches(rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)),
                               rel_stmt(var_expr(x), rel_type::n_eq_includes, var_expr(y)));

            // doesn't match left side
            expect_not_matches(disj(rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)),
                                    rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y))),
                               disj(rel_stmt(var_expr(s), rel_type::eq_is_included, var_expr(t)),
                                    rel_stmt(var_expr(t), rel_type::eq_is_included, var_expr(t))));

            // doesn't match right side
            expect_not_matches(disj(rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)),
                                    rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y))),
                               disj(rel_stmt(var_expr(s), rel_type::eq_is_included, var_expr(t)),
                                    rel_stmt(var_expr(s), rel_type::eq_is_included, var_expr(s))));

            // doesn't match bound variable
            expect_not_matches(forall(x, disj(
                                                 rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)),
                                                 rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)))),
                               forall(s, disj(
                                                 rel_stmt(var_expr(s), rel_type::eq_is_included, var_expr(t)),
                                                 rel_stmt(var_expr(s), rel_type::eq_is_included, var_expr(s)))));
            expect_not_matches(forall(x, disj(
                                                 rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)),
                                                 rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)))),
                               forall(s, disj(
                                                 rel_stmt(var_expr(s), rel_type::eq_is_included, var_expr(t)),
                                                 rel_stmt(var_expr(t), rel_type::eq_is_included, var_expr(t)))));
        });

        test("match both expressions and statements", [&] {
            // Matching both expressions and statements
            expect_matches(disj(var_stmt(p), rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y))),
                           disj(contradiction(), rel_stmt(var_expr(s), rel_type::eq_is_included, var_expr(t))),
                           {
                                   {p, contradiction()},
                           },
                           {
                                   {x, var_expr(s)},
                                   {y, var_expr(t)},
                           });
        });

        test("match expression", [&] {
            expect_matches(var_expr(x), var_expr(s), {}, {{x, var_expr(s)}});
        });

        test("not a match expression", [&] {
            // TODO: Not possible atm.
        });
    });
}
