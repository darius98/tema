#include "algorithms/deduce.h"

#include <mcga/test_ext/matchers.hpp>

#include "algorithms/equals.h"
#include "algorithms/print_utf8.h"

using namespace tema;
using namespace mcga::matchers;
using namespace mcga::test;

void expect_mp_deduce(const statement_ptr& law, const statement_ptr& application, const statement_ptr& expected_conclusion, Context context = Context()) {
    const auto result = mp_deduce(*law, application);
    const auto message = "mp_deduce " +
                         print_utf8(*expected_conclusion) +
                         " from law " +
                         print_utf8(*law) +
                         " applied as " +
                         print_utf8(*application);
    expectMsg(result.has_value(), message, context);
    expectMsg(equals(*result.value(), *expected_conclusion), message, context);
}

void expect_not_mp_deduce(const statement_ptr& law, const statement_ptr& application, Context context = Context()) {
    const auto result = mp_deduce(*law, application);
    expectMsg(!result.has_value(),
              print_utf8(*application) +
                      " does not mp_deduce anything from " +
                      print_utf8(*law),
              std::move(context));
}

TEST_CASE("algorithms.mp_deduce") {
    const auto p = var("P");
    const auto q = var("Q");
    const auto r = var("R");
    const auto s = var("S");

    test("from implication", [&] {
        const auto law = implies(conj(var_stmt(p), implies(var_stmt(p), var_stmt(q))), var_stmt(q));
        expect_mp_deduce(law, conj(truth(), implies(truth(), contradiction())), contradiction());
        expect_mp_deduce(law, conj(conj(truth(), disj(truth(), contradiction())), implies(conj(truth(), disj(truth(), contradiction())), equiv(truth(), truth()))), equiv(truth(), truth()));
    });

    test("from equivalence (left-to-right)", [&] {
        const auto law = equiv(neg(neg(var_stmt(p))), var_stmt(p));
        expect_mp_deduce(law, neg(neg(truth())), truth());
        expect_mp_deduce(law, neg(neg(implies(var_stmt(q), var_stmt(r)))), implies(var_stmt(q), var_stmt(r)));
    });

    test("from equivalence (right to left)", [&] {
        const auto law = equiv(disj(var_stmt(p), var_stmt(p)), var_stmt(p));
        expect_mp_deduce(law, truth(), disj(truth(), truth()));
        expect_mp_deduce(law, var_stmt(q), disj(var_stmt(q), var_stmt(q)));
    });

    test("with unmatched variables", [&] {
        const auto law = implies(disj(var_stmt(p), var_stmt(q)), disj(var_stmt(p), var_stmt(r), var_stmt(q), var_stmt(s)));
        expect_mp_deduce(law, disj(truth(), contradiction()), disj(truth(), var_stmt(r), contradiction(), var_stmt(s)));
    });

    test("without match (implication)", [&] {
        const auto law = implies(neg(neg(var_stmt(p))), var_stmt(p));
        expect_not_mp_deduce(law, truth());
    });

    test("without match (equivalence)", [&] {
        const auto law = equiv(neg(neg(var_stmt(p))), neg(neg(neg(neg(var_stmt(p))))));
        expect_not_mp_deduce(law, truth());
    });

    test("without match (neither implication nor equivalence)", [&] {
        const auto law = neg(conj(var_stmt(p), neg(var_stmt(p))));
        expect_not_mp_deduce(law, truth());
    });
}
