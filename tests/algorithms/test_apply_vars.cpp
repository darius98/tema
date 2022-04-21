#include "algorithms/apply_vars.h"

#include "mcga/test_ext/matchers.hpp"

#include "algorithms/equals.h"
#include "algorithms/print_utf8.h"

using namespace tema;
using namespace mcga::matchers;
using namespace mcga::test;

void expect_apply_vars(const statement_ptr& law, const std::map<variable_ptr, statement_ptr>& replacements, const statement_ptr& expected_application, std::set<variable_ptr> expected_unmatched_vars, Context context = Context()) {
    const auto result = apply_vars(law.get(), replacements);
    expectMsg(equals(result.stmt.get(), expected_application.get()),
              print_utf8(law.get()) + " ===> " + print_utf8(expected_application.get()),
              context);
    expect(result.unmatched_vars, std::move(expected_unmatched_vars), std::move(context));
}

TEST_CASE("algorithms.apply_vars") {
    const auto p = var("P");
    const auto q = var("Q");
    const auto r = var("R");

    test("replace no variables (none used)", [] {
        const auto law = implies(neg(contradiction()), truth());
        expect_apply_vars(law, {}, implies(neg(contradiction()), truth()), {});
    });

    test("replace no variables (one used)", [&] {
        const auto law = implies(var_stmt(p), contradiction());
        expect_apply_vars(law, {}, implies(var_stmt(p), contradiction()), {p});
    });

    test("replace no variables (two used)", [&] {
        const auto law = equiv(contradiction(), disj(var_stmt(p), var_stmt(q)));
        expect_apply_vars(law, {}, equiv(contradiction(), disj(var_stmt(p), var_stmt(q))), {p, q});

        const auto law2 = equiv(conj(var_stmt(p), var_stmt(q)), contradiction());
        expect_apply_vars(law2, {}, equiv(conj(var_stmt(p), var_stmt(q)), contradiction()), {p, q});
    });

    test("replace one variable (one used)", [&] {
        const auto law = implies(contradiction(), var_stmt(p));
        expect_apply_vars(law,
                          {
                                  {p, implies(var_stmt(q), var_stmt(q))},
                          },
                          implies(contradiction(), implies(var_stmt(q), var_stmt(q))), {});

        const auto law2 = implies(var_stmt(p), contradiction());
        expect_apply_vars(law2,
                          {
                                  {p, implies(var_stmt(q), var_stmt(q))},
                          },
                          implies(implies(var_stmt(q), var_stmt(q)), contradiction()), {});
    });

    test("replace different variable than used", [&] {
        const auto law = implies(contradiction(), var_stmt(p));
        expect_apply_vars(law,
                          {
                                  {q, implies(var_stmt(q), var_stmt(q))},
                          },
                          implies(contradiction(), var_stmt(p)), {p});
    });

    test("replace one variable (out of 2 used)", [&] {
        const auto law = equiv(contradiction(), disj(var_stmt(p), var_stmt(q)));
        expect_apply_vars(law,
                          {
                                  {p, implies(var_stmt(q), var_stmt(q))},
                          },
                          equiv(contradiction(), disj(implies(var_stmt(q), var_stmt(q)), var_stmt(q))), {q});

        const auto law2 = equiv(conj(var_stmt(p), var_stmt(q)), contradiction());
        expect_apply_vars(law2,
                          {
                                  {p, implies(var_stmt(q), var_stmt(q))},
                          },
                          equiv(conj(implies(var_stmt(q), var_stmt(q)), var_stmt(q)), contradiction()), {q});
    });

    test("replace both variables", [&] {
        const auto law = equiv(contradiction(), conj(var_stmt(p), var_stmt(q)));
        expect_apply_vars(law,
                          {
                                  {p, implies(truth(), contradiction())},
                                  {q, neg(truth())},
                          },
                          equiv(contradiction(), conj(implies(truth(), contradiction()), neg(truth()))), {});

        const auto law2 = equiv(disj(var_stmt(p), var_stmt(q)), contradiction());
        expect_apply_vars(law2,
                          {
                                  {p, implies(truth(), contradiction())},
                                  {q, neg(truth())},
                          },
                          equiv(disj(implies(truth(), contradiction()), neg(truth())), contradiction()), {});
    });

    test("replace variable with reference to another does not work recursively", [&] {
        const auto law = equiv(var_stmt(p), neg(neg(var_stmt(q))));
        expect_apply_vars(law,
                          {
                                  {p, implies(var_stmt(q), truth())},
                                  {q, neg(truth())},
                          },
                          equiv(implies(var_stmt(q), truth()), neg(neg(neg(truth())))), {});
    });

    test("forall", [&] {
        // replace variable from forall expression
        const auto law = equiv(disj(var_stmt(p), var_stmt(r)), forall(q, disj(var_stmt(q), var_stmt(p))));
        expect_apply_vars(law,
                          {
                                  {p, truth()},
                                  {q, neg(truth())},
                          },
                          equiv(disj(truth(), var_stmt(r)), disj(neg(truth()), truth())), {r});
        // Replace something inside forall (not the q variable though).
        expect_apply_vars(law,
                          {
                                  {p, truth()},
                          },
                          equiv(disj(truth(), var_stmt(r)), forall(q, disj(var_stmt(q), truth()))), {r});
        // Keep the forall the same
        expect_apply_vars(law,
                          {
                                  {r, contradiction()},
                          },
                          equiv(disj(var_stmt(p), contradiction()), forall(q, disj(var_stmt(q), var_stmt(p)))), {p});

        // Invalid IR coming in
        expect([&] {
            expect_apply_vars(forall(p, forall(p, disj(var_stmt(p), var_stmt(p)))),
                              {
                                      {p, contradiction()},
                              },
                              disj(contradiction(), contradiction()), {});
        },
               throwsA<std::runtime_error>);
    });
}
