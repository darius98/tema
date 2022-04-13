#include "algorithms/equals.h"

#include "mcga/test_ext/matchers.hpp"

#include "algorithms/print_ascii.h"

using namespace tema;
using namespace mcga::matchers;

void impl_expect_equals(bool expected, const char* sign, const statement_ptr& a, const statement_ptr& b, mcga::test::Context context) {
    mcga::test::expectMsg(equals(a.get(), b.get()) == expected,
                          print_ascii(a.get()) + sign + print_ascii(b.get()),
                          std::move(context));
}

void expect_equals(const statement_ptr& a, const statement_ptr& b, mcga::test::Context context = mcga::test::Context()) {
    impl_expect_equals(true, " == ", a, b, std::move(context));
}

void expect_not_equals(const statement_ptr& a, const statement_ptr& b, mcga::test::Context context = mcga::test::Context()) {
    impl_expect_equals(false, " != ", a, b, std::move(context));
}

TEST_CASE("algorithms.equals") {
    test("truth", [] {
        expect_equals(truth(), truth());
        expect_not_equals(truth(), contradiction());
    });

    test("contradiction", [] {
        expect_equals(contradiction(), contradiction());
        expect_not_equals(contradiction(), truth());
    });

    const auto v = var("V");
    const auto v_stmt = var_stmt(v);
    test("var", [&] {
        expect_equals(var_stmt(v), var_stmt(v));
        expect_equals(v_stmt, v_stmt);
        expect_not_equals(v_stmt, var_stmt(var("Y")));
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
}
