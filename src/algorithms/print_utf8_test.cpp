#include "algorithms/print_utf8.h"

#include <mcga/test_ext/matchers.hpp>

using namespace tema;
using namespace mcga::matchers;

TEST_CASE("algorithms.print_utf8") {
    test("print_utf8", [] {
        const auto t = truth();
        expect(print_utf8(*t), "⊤");

        const auto f = contradiction();
        expect(print_utf8(*f), "⊥");

        const auto p = var("p");
        const auto p_stmt = var_stmt(p);
        expect(print_utf8(*p_stmt), "p");

        const auto f1 = disj(p_stmt, neg(p_stmt));
        expect(print_utf8(*f1), "p∨¬p");

        const auto q = var("q");

        const auto f2 = implies(p_stmt, var_stmt(q));
        expect(print_utf8(*f2), "p→q");

        const auto f3 = neg(conj(p_stmt, neg(p_stmt)));
        expect(print_utf8(*f3), "¬(p∧¬p)");

        const auto f4 = equiv(p_stmt, neg(neg(p_stmt)));
        expect(print_utf8(*f4), "p⟷¬¬p");

        const auto f5 = equiv(p_stmt, var_stmt(q));
        expect(print_utf8(*f5), "p⟷q");

        const auto f6 = forall(p, var_stmt(q));
        expect(print_utf8(*f6), "∀p q");

        const auto f7 = forall(p, disj(var_stmt(p), var_stmt(q)));
        expect(print_utf8(*f7), "∀p (p∨q)");

        const auto f8 = forall(p, rel_stmt(var_expr(q), rel_type::n_eq, var_expr(q)));
        expect(print_utf8(*f8), "∀p (q≠q)");
    });

    test("print_utf8_to", [] {
        const auto p = var("p");
        const auto q = var("q");
        const auto r = var("r");
        const auto expr = equiv(
                implies(var_stmt(p), implies(var_stmt(q), var_stmt(r))),
                implies(var_stmt(q), implies(var_stmt(p), var_stmt(r))));
        std::stringstream sout;
        print_utf8_to(*expr, sout);
        expect(!sout.fail());
        expect(!sout.bad());
        expect(sout.str(), "(p→(q→r))⟷(q→(p→r))");
    });

    test("On expressions", [] {
        const auto x = var("X");
        const auto y = var("Y");
        const auto z = var("Z");

        const auto x_expr = var_expr(x);
        expect(print_utf8(*x_expr), "X");

        const auto x_union_y_expr = binop(var_expr(x), binop_type::set_union, var_expr(y));
        expect(print_utf8(*x_union_y_expr), "X∪Y");

        const auto x_union_y_union_z_expr = binop(var_expr(x), binop_type::set_union, binop(var_expr(y), binop_type::set_union, var_expr(z)));
        expect(print_utf8(*x_union_y_union_z_expr), "X∪(Y∪Z)");

        std::stringstream sout;
        print_utf8_to(*x_union_y_expr, sout);
        expect(!sout.fail());
        expect(!sout.bad());
        expect(sout.str(), "X∪Y");
    });
}
