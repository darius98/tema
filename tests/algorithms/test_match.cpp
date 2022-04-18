#include "algorithms/match.h"

#include "mcga/test_ext/matchers.hpp"

#include "algorithms/equals.h"
#include "algorithms/print_ascii.h"

using namespace tema;
using namespace mcga::matchers;
using namespace mcga::test;

void expect_matches(const statement_ptr& law, const statement_ptr& application, const match_result& expected_repls, const Context& context = Context()) {
    const auto result = match(law.get(), application.get());
    expectMsg(result.has_value(),
              print_ascii(application.get()) +
                      " matches " +
                      print_ascii(law.get()),
              context);
    for (const auto& [var, repl]: result.value()) {
        expectMsg(expected_repls.contains(var), "Unexpected replacement " + var->name + " (replaced with '" + print_ascii(repl.get()) + "')", context);
        expect(equals(repl.get(), expected_repls.find(var)->second.get()), context);
    }
    expect(result.value(), hasSize(expected_repls.size()), context);
}

void expect_not_matches(const statement_ptr& law, const statement_ptr& application, Context context = Context()) {
    const auto result = match(law.get(), application.get());
    expectMsg(!result.has_value(),
              print_ascii(application.get()) +
                      " does not match " +
                      print_ascii(law.get()),
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
}
