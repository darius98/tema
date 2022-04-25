#include "core/statement.h"

#include "mcga/test_ext/matchers.hpp"

using namespace tema;
using namespace mcga::matchers;

TEST_CASE("statement") {
    test("truth", [] {
        const auto a = truth();

        expect(a->is_truth(), isTrue);
        // Check that this doesn't throw.
        (void) a->as_truth();

        expect(a->is_contradiction(), isFalse);
        expect([&] { (void) a->as_contradiction(); }, throwsA<std::bad_variant_access>);

        // Truth is only one
        expect(a, truth());
    });

    test("contradiction", [] {
        const auto a = contradiction();

        expect(a->is_contradiction(), isTrue);
        // Check that this doesn't throw.
        (void) a->as_contradiction();

        expect(a->is_truth(), isFalse);
        expect([&] { (void) a->as_truth(); }, throwsA<std::bad_variant_access>);

        // Contradiction is only one
        expect(a, contradiction());
    });

    test("implication", [] {
        const auto a = implies(contradiction(), truth());
        expect(a->is_implies(), isTrue);
        expect(a->as_implies().from, contradiction());
        expect(a->as_implies().to, truth());

        expect(a->is_equiv(), isFalse);
        expect([&] { (void) a->as_equiv(); }, throwsA<std::bad_variant_access>);
    });

    test("equivalence", [] {
        const auto a = equiv(contradiction(), truth());
        expect(a->is_equiv(), isTrue);
        expect(a->as_equiv().left, contradiction());
        expect(a->as_equiv().right, truth());

        expect(a->is_implies(), isFalse);
        expect([&] { (void) a->as_implies(); }, throwsA<std::bad_variant_access>);
    });

    test("negation", [] {
        const auto a = neg(contradiction());

        expect(a->is_neg(), isTrue);
        expect(a->as_neg().inner, contradiction());

        expect(a->is_truth(), isFalse);
        expect([&] { (void) a->as_truth(); }, throwsA<std::bad_variant_access>);
    });

    test("conjunction", [] {
        const auto a = conj({truth(), conj(truth(), contradiction())});

        expect(a->is_conj(), isTrue);
        expect(a->as_conj().inner, hasSize(2));
        expect(a->as_conj().inner[0], truth());
        expect(a->as_conj().inner[1]->is_conj(), isTrue);
        expect(a->as_conj().inner[1]->as_conj().inner, hasSize(2));
        expect(a->as_conj().inner[1]->as_conj().inner[0], truth());
        expect(a->as_conj().inner[1]->as_conj().inner[1], contradiction());

        expect(a->is_disj(), isFalse);
        expect([&] { (void) a->as_disj(); }, throwsA<std::bad_variant_access>);
    });

    test("variadic conjunction", [] {
        const auto a = conj(truth(), truth(), contradiction());
        expect(a->is_conj(), isTrue);
        expect(a->as_conj().inner, hasSize(3));
        expect(a->as_conj().inner[0], truth());
        expect(a->as_conj().inner[1], truth());
        expect(a->as_conj().inner[2], contradiction());

        const auto b = conj(truth(), truth(), contradiction(), truth(), truth(), contradiction());
        expect(b->is_conj(), isTrue);
        expect(b->as_conj().inner, hasSize(6));
        expect(b->as_conj().inner[0], truth());
        expect(b->as_conj().inner[1], truth());
        expect(b->as_conj().inner[2], contradiction());
        expect(b->as_conj().inner[3], truth());
        expect(b->as_conj().inner[4], truth());
        expect(b->as_conj().inner[5], contradiction());
    });

    test("disjunction", [] {
        const auto a = disj({truth(), disj(truth(), contradiction())});

        expect(a->is_disj(), isTrue);
        expect(a->as_disj().inner, hasSize(2));
        expect(a->as_disj().inner[0], truth());
        expect(a->as_disj().inner[1]->is_disj(), isTrue);
        expect(a->as_disj().inner[1]->as_disj().inner, hasSize(2));
        expect(a->as_disj().inner[1]->as_disj().inner[0], truth());
        expect(a->as_disj().inner[1]->as_disj().inner[1], contradiction());

        expect(a->is_conj(), isFalse);
        expect([&] { (void) a->as_conj(); }, throwsA<std::bad_variant_access>);
    });

    test("variadic disjunction", [] {
        const auto a = disj(truth(), truth(), contradiction());
        expect(a->is_disj(), isTrue);
        expect(a->as_disj().inner, hasSize(3));
        expect(a->as_disj().inner[0], truth());
        expect(a->as_disj().inner[1], truth());
        expect(a->as_disj().inner[2], contradiction());

        const auto b = disj(truth(), truth(), contradiction(), truth(), truth(), contradiction());
        expect(b->is_disj(), isTrue);
        expect(b->as_disj().inner, hasSize(6));
        expect(b->as_disj().inner[0], truth());
        expect(b->as_disj().inner[1], truth());
        expect(b->as_disj().inner[2], contradiction());
        expect(b->as_disj().inner[3], truth());
        expect(b->as_disj().inner[4], truth());
        expect(b->as_disj().inner[5], contradiction());
    });

    test("forall", [] {
        const auto p = var("P");

        const auto a = forall(p, disj(var_stmt(p), truth()));

        expect(a->is_forall(), isTrue);
        expect(a->as_forall().var, p);
        expect(a->as_forall().inner->is_disj(), isTrue);

        expect(a->is_conj(), isFalse);
        expect([&] { (void) a->as_conj(); }, throwsA<std::bad_variant_access>);
    });

    test("variable", [] {
        const auto p = var("P");

        const auto a = var_stmt(p);

        expect(a->is_var(), isTrue);
        expect(a->as_var(), p);

        expect(a->is_truth(), isFalse);
        expect([&] { (void) a->as_truth(); }, throwsA<std::bad_variant_access>);
    });

    test("relationship", [] {
        const auto x = var("x");
        const auto y = var("y");
        const auto a = rel_stmt(relationship{rel_type::eq, var_expr(x), var_expr(y)});

        expect(a->is_rel(), isTrue);
        expect(a->as_rel().left->is_var(), isTrue);
        expect(a->as_rel().left->as_var(), x);
        expect(a->as_rel().right->is_var(), isTrue);
        expect(a->as_rel().right->as_var(), y);

        expect(a->is_var(), isFalse);
        expect([&] { (void) a->as_var(); }, throwsA<std::bad_variant_access>);

        const auto b = rel_stmt(var_expr(x), rel_type::eq, var_expr(y));

        expect(b->is_rel(), isTrue);
        expect(b->as_rel().left->is_var(), isTrue);
        expect(b->as_rel().left->as_var(), x);
        expect(b->as_rel().right->is_var(), isTrue);
        expect(b->as_rel().right->as_var(), y);

        expect(b->is_var(), isFalse);
        expect([&] { (void) b->as_var(); }, throwsA<std::bad_variant_access>);
    });
}
