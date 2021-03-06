#include "algorithms/equals.h"

#include <mcga/test_ext/matchers.hpp>

#include "algorithms/print_utf8.h"

using namespace tema;
using namespace mcga::matchers;

void impl_expect_equals(bool expected, std::string_view sign, const auto& a, const auto& b, mcga::test::Context context) {
    mcga::test::expectMsg(equals(*a, *b) == expected,
                          print_utf8(*a) + std::string{sign} + print_utf8(*b),
                          std::move(context));
}

void expect_equals(const auto& a, const auto& b, mcga::test::Context context = mcga::test::Context()) {
    impl_expect_equals(true, " == ", a, b, std::move(context));
}

void expect_not_equals(const auto& a, const auto& b, mcga::test::Context context = mcga::test::Context()) {
    impl_expect_equals(false, " != ", a, b, std::move(context));
}

TEST_CASE("algorithms.equals") {
    const auto x = var("X");
    const auto y = var("Y");
    const auto z = var("Z");
    const auto t = var("T");

    const auto u = var("u");
    const auto v = var("V");
    const auto w = var("W");

    test("truth", [] {
        expect_equals(truth(), truth());
        expect_not_equals(truth(), contradiction());
    });

    test("contradiction", [] {
        expect_equals(contradiction(), contradiction());
        expect_not_equals(contradiction(), truth());
    });

    const auto v_stmt = var_stmt(v);
    test("var", [&] {
        expect_equals(var_stmt(v), var_stmt(v));
        expect_equals(v_stmt, v_stmt);
        expect_not_equals(v_stmt, var_stmt(y));
        expect_not_equals(var_stmt(v), truth());
        expect_not_equals(truth(), var_stmt(v));
    });

    test("implies", [&] {
        expect_equals(implies(contradiction(), truth()), implies(contradiction(), truth()));
        expect_not_equals(implies(contradiction(), truth()), equiv(contradiction(), v_stmt));
        expect_not_equals(implies(contradiction(), truth()), implies(contradiction(), v_stmt));
        expect_not_equals(implies(contradiction(), truth()), implies(v_stmt, truth()));
    });

    test("equiv", [&] {
        expect_equals(equiv(contradiction(), truth()), equiv(contradiction(), truth()));
        expect_not_equals(equiv(contradiction(), truth()), implies(contradiction(), v_stmt));
        expect_not_equals(equiv(contradiction(), truth()), equiv(contradiction(), v_stmt));
        expect_not_equals(equiv(contradiction(), truth()), equiv(v_stmt, truth()));
    });

    test("neg", [&] {
        expect_equals(neg(implies(contradiction(), truth())), neg(implies(contradiction(), truth())));
        expect_not_equals(neg(implies(contradiction(), truth())), implies(truth(), contradiction()));
        expect_not_equals(neg(implies(contradiction(), truth())), neg(implies(v_stmt, truth())));
        expect_not_equals(neg(implies(contradiction(), truth())), neg(implies(contradiction(), v_stmt)));
    });

    test("conj", [&] {
        expect_equals(conj(truth(), contradiction()), conj(truth(), contradiction()));
        expect_equals(conj(implies(v_stmt, truth()), contradiction()), conj(implies(var_stmt(v), truth()), contradiction()));
        expect_equals(conj(contradiction(), implies(v_stmt, truth())), conj(contradiction(), implies(var_stmt(v), truth())));
        expect_equals(
                conj(equiv(v_stmt, truth()), implies(v_stmt, truth()), conj(v_stmt, v_stmt)),
                conj(equiv(var_stmt(v), truth()), implies(var_stmt(v), truth()), conj(var_stmt(v), var_stmt(v))));
        expect_not_equals(
                conj(equiv(v_stmt, truth()), implies(v_stmt, truth()), conj(v_stmt, v_stmt)),
                conj(equiv(var_stmt(v), truth()), implies(var_stmt(v), truth())));
        expect_not_equals(
                conj(equiv(v_stmt, truth()), implies(v_stmt, truth()), conj(v_stmt, v_stmt)),
                conj(equiv(var_stmt(v), truth()), implies(var_stmt(v), truth()), disj(var_stmt(v), var_stmt(v))));
    });

    test("disj", [&] {
        expect_equals(disj(truth(), contradiction()), disj(truth(), contradiction()));
        expect_equals(disj(implies(v_stmt, truth()), contradiction()), disj(implies(var_stmt(v), truth()), contradiction()));
        expect_equals(disj(contradiction(), implies(v_stmt, truth())), disj(contradiction(), implies(var_stmt(v), truth())));
        expect_equals(
                disj(equiv(v_stmt, truth()), implies(v_stmt, truth()), disj(v_stmt, v_stmt)),
                disj(equiv(var_stmt(v), truth()), implies(var_stmt(v), truth()), disj(var_stmt(v), var_stmt(v))));
        expect_not_equals(
                disj(equiv(v_stmt, truth()), implies(v_stmt, truth()), disj(v_stmt, v_stmt)),
                disj(equiv(var_stmt(v), truth()), implies(var_stmt(v), truth())));
        expect_not_equals(
                disj(equiv(v_stmt, truth()), implies(v_stmt, truth()), disj(v_stmt, v_stmt)),
                disj(equiv(var_stmt(v), truth()), implies(var_stmt(v), truth()), conj(var_stmt(v), var_stmt(v))));
    });

    test("forall", [&] {
        const auto u_stmt = var_stmt(u);
        const auto w_stmt = var_stmt(w);

        expect_equals(forall(v, v_stmt), forall(v, v_stmt));                              // Same variable
        expect_equals(forall(v, u_stmt), forall(w, u_stmt));                              // Different variable used
        expect_equals(forall(v, truth()), forall(w, truth()));                            // Different variable (not used inside)
        expect_equals(forall(v, disj(u_stmt, v_stmt)), forall(w, disj(u_stmt, w_stmt)));  // Different variable (used inside)
        expect_equals(forall(v, forall(w, disj(v_stmt, w_stmt))),
                      forall(x, forall(y, disj(var_stmt(x), var_stmt(y)))));  // Recursive

        expect_not_equals(forall(v, v_stmt), v_stmt);
        expect_not_equals(forall(v, v_stmt), truth());

        // Two mappings
        expect_equals(forall(v, forall(w, disj(v_stmt, w_stmt))),
                      forall(x, forall(y, disj(var_stmt(x), var_stmt(y)))));
        expect_not_equals(forall(v, forall(w, disj(v_stmt, w_stmt))),
                          forall(x, forall(y, disj(var_stmt(y), var_stmt(x)))));

        // Invalid IR coming in
        expect([&] {
            expect_not_equals(forall(v, forall(v, disj(v_stmt, u_stmt))),
                              forall(w, forall(w, disj(w_stmt, u_stmt))));
        },
               throwsA<std::runtime_error>);
    });

    test("relationships", [&] {
        const auto x_expr = var_expr(x);
        const auto y_expr = var_expr(y);

        expect_equals(rel_stmt(var_expr(x), rel_type::in, var_expr(y)),
                      rel_stmt(var_expr(x), rel_type::in, var_expr(y)));

        // With same pointer
        const auto x_minus_y_expr = binop(x_expr, binop_type::set_difference, y_expr);
        const auto stmt = rel_stmt(x_expr, rel_type::in, binop(x_expr, binop_type::set_difference, y_expr));
        expect_equals(stmt, rel_stmt(x_expr, rel_type::in, x_minus_y_expr));
        expect_equals(stmt, stmt);

        // With bound variables
        expect_equals(forall(x, rel_stmt(var_expr(x), rel_type::includes, var_expr(z))),
                      forall(y, rel_stmt(var_expr(y), rel_type::includes, var_expr(z))));
        expect_equals(forall(x, forall(y, rel_stmt(var_expr(x), rel_type::eq_includes, var_expr(y)))),
                      forall(z, forall(t, rel_stmt(var_expr(z), rel_type::eq_includes, var_expr(t)))));

        // With same pointer
        expect_equals(forall(x, forall(y, rel_stmt(x_expr, rel_type::eq_includes, y_expr))),
                      forall(y, forall(x, rel_stmt(y_expr, rel_type::eq_includes, x_expr))));

        // Not with a relationship
        expect_not_equals(rel_stmt(var_expr(x), rel_type::is_included, var_expr(y)), var_stmt(x));
        // With different relationship type
        expect_not_equals(rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)),
                          rel_stmt(var_expr(x), rel_type::n_eq_is_included, var_expr(y)));
        // With different left side
        expect_not_equals(rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)),
                          rel_stmt(var_expr(y), rel_type::eq_is_included, var_expr(y)));
        // With different right side
        expect_not_equals(rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(y)),
                          rel_stmt(var_expr(x), rel_type::eq_is_included, var_expr(x)));
        // With different bindings
        expect_not_equals(forall(x, forall(y, rel_stmt(var_expr(x), rel_type::eq_includes, var_expr(y)))),
                          forall(z, forall(t, rel_stmt(var_expr(t), rel_type::eq_includes, var_expr(z)))));
    });

    test("on expressions", [&] {
        expect_equals(var_expr(x), var_expr(x));

        const auto x_expr = var_expr(x);
        expect_equals(x_expr, x_expr);

        const auto x_union_y_expr = binop(var_expr(x), binop_type::set_union, var_expr(y));
        expect_equals(x_union_y_expr, x_union_y_expr);
        expect_equals(x_union_y_expr,
                      binop(var_expr(x), binop_type::set_union, var_expr(y)));

        expect_not_equals(var_expr(x), var_expr(y));
        expect_not_equals(var_expr(x), x_union_y_expr);
        expect_not_equals(x_union_y_expr, var_expr(x));

        expect_not_equals(x_union_y_expr,
                          binop(var_expr(x), binop_type::set_difference, var_expr(y)));
        expect_not_equals(x_union_y_expr,
                          binop(var_expr(x), binop_type::set_union, var_expr(x)));
        expect_not_equals(x_union_y_expr,
                          binop(var_expr(y), binop_type::set_union, var_expr(y)));

        expect_equals(call(var_expr(x), {var_expr(y), var_expr(z)}),
                      call(var_expr(x), {var_expr(y), var_expr(z)}));
        expect_equals(call(var_expr(x), {x_union_y_expr, var_expr(z), var_expr(z)}),
                      call(var_expr(x), {binop(var_expr(x), binop_type::set_union, var_expr(y)), var_expr(z), var_expr(z)}));

        expect_not_equals(var_expr(x), call(var_expr(x), {var_expr(y), var_expr(z)}));
        expect_not_equals(call(var_expr(x), {var_expr(y), var_expr(z)}), var_expr(x));
        expect_not_equals(call(var_expr(x), {var_expr(y), var_expr(z)}),
                          call(var_expr(y), {var_expr(y), var_expr(z)}));
        expect_not_equals(call(var_expr(x), {var_expr(y), var_expr(z), var_expr(z)}),
                          call(var_expr(x), {var_expr(y), var_expr(z)}));
        expect_not_equals(call(var_expr(x), {var_expr(y), var_expr(z)}),
                          call(var_expr(x), {var_expr(y), var_expr(z), var_expr(z)}));
        expect_not_equals(call(var_expr(x), {var_expr(y), var_expr(z)}),
                          call(var_expr(x), {var_expr(z), var_expr(z)}));
        expect_not_equals(call(var_expr(x), {var_expr(y), var_expr(z)}),
                          call(var_expr(x), {var_expr(y), var_expr(y)}));
    });
}
