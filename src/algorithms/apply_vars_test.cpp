#include "algorithms/apply_vars.h"

#include <mcga/test_ext/matchers.hpp>

#include "algorithms/equals.h"
#include "algorithms/print_utf8.h"

using namespace tema;
using namespace mcga::matchers;
using namespace mcga::test;

void expect_apply_vars(const statement_ptr& law,
                       std::map<variable_ptr, statement_ptr> stmt_replacements,
                       std::map<variable_ptr, expr_ptr> expr_replacements,
                       const statement_ptr& expected_application,
                       std::set<variable_ptr> expected_unmatched_vars = {},
                       Context context = Context()) {
    const auto result = apply_vars(law, match_result{std::move(stmt_replacements), std::move(expr_replacements)});
    expectMsg(equals(*result.stmt, *expected_application),
              print_utf8(*law) + " ===> " + print_utf8(*expected_application) + " (got '" + print_utf8(*result.stmt) + "')",
              context);
    expect(result.unmatched_vars, std::move(expected_unmatched_vars), std::move(context));
}

TEST_CASE("algorithms.apply_vars") {
    const auto p = var("P");
    const auto q = var("Q");
    const auto r = var("R");

    test("replace no variables (none used)", [] {
        const auto law = implies(neg(contradiction()), truth());
        expect_apply_vars(law, {}, {}, implies(neg(contradiction()), truth()));
    });

    test("replace no variables (one used)", [&] {
        const auto law = implies(var_stmt(p), contradiction());
        expect_apply_vars(law, {}, {}, implies(var_stmt(p), contradiction()), {p});
    });

    test("replace no variables (two used)", [&] {
        const auto law = equiv(contradiction(), disj(var_stmt(p), var_stmt(q)));
        expect_apply_vars(law, {}, {}, equiv(contradiction(), disj(var_stmt(p), var_stmt(q))), {p, q});

        const auto law2 = equiv(conj(var_stmt(p), var_stmt(q)), contradiction());
        expect_apply_vars(law2, {}, {}, equiv(conj(var_stmt(p), var_stmt(q)), contradiction()), {p, q});
    });

    test("replace no variables (two used in an expression)", [&] {
        const auto law = implies(contradiction(), rel_stmt(var_expr(p), rel_type::eq, binop(var_expr(q), binop_type::set_union, var_expr(q))));
        expect_apply_vars(law, {}, {}, law, {p, q});
    });

    test("replace one variable (one used)", [&] {
        const auto law = implies(contradiction(), var_stmt(p));
        expect_apply_vars(law,
                          {
                                  {p, implies(var_stmt(q), var_stmt(q))},
                          },
                          {},
                          implies(contradiction(), implies(var_stmt(q), var_stmt(q))));

        const auto law2 = implies(var_stmt(p), contradiction());
        expect_apply_vars(law2,
                          {
                                  {p, implies(var_stmt(q), var_stmt(q))},
                          },
                          {},
                          implies(implies(var_stmt(q), var_stmt(q)), contradiction()));
    });

    test("replace different variable than used", [&] {
        const auto law = implies(contradiction(), var_stmt(p));
        expect_apply_vars(law,
                          {
                                  {q, implies(var_stmt(q), var_stmt(q))},
                          },
                          {},
                          implies(contradiction(), var_stmt(p)), {p});
    });

    test("replace one variable (out of 2 used)", [&] {
        const auto law = equiv(contradiction(), disj(var_stmt(p), var_stmt(q)));
        expect_apply_vars(law,
                          {
                                  {p, implies(var_stmt(q), var_stmt(q))},
                          },
                          {},
                          equiv(contradiction(), disj(implies(var_stmt(q), var_stmt(q)), var_stmt(q))), {q});

        const auto law2 = equiv(conj(var_stmt(p), var_stmt(q)), contradiction());
        expect_apply_vars(law2,
                          {
                                  {p, implies(var_stmt(q), var_stmt(q))},
                          },
                          {},
                          equiv(conj(implies(var_stmt(q), var_stmt(q)), var_stmt(q)), contradiction()), {q});
    });

    test("replace both variables", [&] {
        const auto law = equiv(contradiction(), conj(var_stmt(p), var_stmt(q)));
        expect_apply_vars(law,
                          {
                                  {p, implies(truth(), contradiction())},
                                  {q, neg(truth())},
                          },
                          {},
                          equiv(contradiction(), conj(implies(truth(), contradiction()), neg(truth()))));

        const auto law2 = equiv(disj(var_stmt(p), var_stmt(q)), contradiction());
        expect_apply_vars(law2,
                          {
                                  {p, implies(truth(), contradiction())},
                                  {q, neg(truth())},
                          },
                          {},
                          equiv(disj(implies(truth(), contradiction()), neg(truth())), contradiction()));
    });

    test("replace variable in expression", [&] {
        const auto t = var("t");
        const auto law = implies(contradiction(), rel_stmt(var_expr(p), rel_type::in, binop(var_expr(q), binop_type::set_union, var_expr(r))));
        expect_apply_vars(law,
                          {},
                          {
                                  {q, var_expr(t)},
                          },
                          implies(contradiction(), rel_stmt(var_expr(p), rel_type::in, binop(var_expr(t), binop_type::set_union, var_expr(r)))), {p, r});
        expect_apply_vars(law,
                          {},
                          {
                                  {r, var_expr(t)},
                          },
                          implies(contradiction(), rel_stmt(var_expr(p), rel_type::in, binop(var_expr(q), binop_type::set_union, var_expr(t)))), {p, q});
        expect_apply_vars(law,
                          {},
                          {
                                  {p, var_expr(t)},
                          },
                          implies(contradiction(), rel_stmt(var_expr(t), rel_type::in, binop(var_expr(q), binop_type::set_union, var_expr(r)))), {q, r});
        expect_apply_vars(law,
                          {},
                          {
                                  {p, var_expr(t)},
                                  {q, var_expr(t)},
                                  {r, var_expr(t)},
                          },
                          implies(contradiction(), rel_stmt(var_expr(t), rel_type::in, binop(var_expr(t), binop_type::set_union, var_expr(t)))));
    });

    test("replace variables in call expression", [&] {
        const auto x = var("X");
        const auto y = var("Y");
        const auto z = var("Z");
        const auto t = var("T");

        const auto law = rel_stmt(var_expr(p), rel_type::in, call(var_expr(x), {var_expr(y), binop(var_expr(y), binop_type::set_union, var_expr(z))}));

        expect_apply_vars(law, {}, {}, law, {p, x, y, z});
        expect_apply_vars(law, {}, {
                                           {x, binop(var_expr(t), binop_type::set_union, var_expr(t))},
                                   },
                          rel_stmt(var_expr(p), rel_type::in, call(binop(var_expr(t), binop_type::set_union, var_expr(t)), {var_expr(y), binop(var_expr(y), binop_type::set_union, var_expr(z))})),
                          {p, y, z});
        expect_apply_vars(law, {}, {
                                           {y, var_expr(t)},
                                   },
                          rel_stmt(var_expr(p), rel_type::in, call(var_expr(x), {var_expr(t), binop(var_expr(t), binop_type::set_union, var_expr(z))})),
                          {p, x, z});
        expect_apply_vars(law, {}, {
                                           {z, var_expr(t)},
                                   },
                          rel_stmt(var_expr(p), rel_type::in, call(var_expr(x), {var_expr(y), binop(var_expr(y), binop_type::set_union, var_expr(t))})),
                          {p, x, y});
        expect_apply_vars(law, {}, {
                                           {x, var_expr(t)},
                                           {y, var_expr(t)},
                                           {z, var_expr(t)},
                                   },
                          rel_stmt(var_expr(p), rel_type::in, call(var_expr(t), {var_expr(t), binop(var_expr(t), binop_type::set_union, var_expr(t))})),
                          {p});
    });

    test("replace variable with reference to another does not work recursively", [&] {
        const auto law = equiv(var_stmt(p), neg(neg(var_stmt(q))));
        expect_apply_vars(law,
                          {
                                  {p, implies(var_stmt(q), truth())},
                                  {q, neg(truth())},
                          },
                          {},
                          equiv(implies(var_stmt(q), truth()), neg(neg(neg(truth())))));
    });

    group("forall", [&] {
        const auto law = equiv(disj(var_stmt(p), var_stmt(r)), forall(q, disj(var_stmt(q), var_stmt(p))));

        test("replace the bound variable inside forall", [&] {
            expect_apply_vars(law,
                              {
                                      {p, truth()},
                                      {q, neg(truth())},
                              },
                              {},
                              equiv(disj(truth(), var_stmt(r)), disj(neg(truth()), truth())), {r});
        });

        test("Replace a non-bound variable inside forall", [&] {
            expect_apply_vars(law,
                              {
                                      {p, truth()},
                              },
                              {},
                              equiv(disj(truth(), var_stmt(r)), forall(q, disj(var_stmt(q), truth()))), {r});
        });

        test("Keep the forall the same", [&] {
            expect_apply_vars(law,
                              {
                                      {r, contradiction()},
                              },
                              {},
                              equiv(disj(var_stmt(p), contradiction()), forall(q, disj(var_stmt(q), var_stmt(p)))), {p});
        });

        test("Replace forall-bound variable in expression", [&] {
            const auto t = var("t");
            const auto law = forall(q, rel_stmt(var_expr(q), rel_type::eq, var_expr(p)));
            expect_apply_vars(law,
                              {},
                              {
                                      {q, var_expr(t)},
                              },
                              rel_stmt(var_expr(t), rel_type::eq, var_expr(p)), {p});
        });

        test("Replace non-bound variable in expression inside forall", [&] {
            const auto t = var("t");
            const auto law = forall(q, rel_stmt(var_expr(q), rel_type::eq, var_expr(p)));
            expect_apply_vars(law,
                              {},
                              {
                                      {p, var_expr(t)},
                              },
                              forall(q, rel_stmt(var_expr(q), rel_type::eq, var_expr(t))));
        });

        test("Invalid IR coming in", [&] {
            expect([&] {
                expect_apply_vars(forall(p, forall(p, disj(var_stmt(p), var_stmt(p)))),
                                  {
                                          {p, contradiction()},
                                  },
                                  {},
                                  disj(contradiction(), contradiction()));
            },
                   throwsA<std::runtime_error>);
        });
    });
}
