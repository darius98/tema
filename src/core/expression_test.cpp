#include "core/expression.h"

#include <mcga/test_ext/matchers.hpp>

using namespace tema;
using namespace mcga::matchers;

TEST_CASE("expression") {
    test("var_expr factory", [] {
        const auto x = var("X");

        const auto expr = var_expr(x);
        expect(expr->is_var(), isTrue);
        expect(expr->as_var(), x);

        expect(expr->is_binop(), isFalse);
        expect([&] { (void) expr->as_binop(); }, throwsA<std::bad_variant_access>);
    });

    test("factory binop(left, type, right)", [] {
        const auto x = var("X");
        const auto y = var("Y");

        const auto expr = binop(var_expr(x), binop_type::set_union, var_expr(y));
        expect(expr->is_binop(), isTrue);
        expect(expr->as_binop().type, binop_type::set_union);
        expect(expr->as_binop().left->is_var(), isTrue);
        expect(expr->as_binop().left->as_var(), x);
        expect(expr->as_binop().right->is_var(), isTrue);
        expect(expr->as_binop().right->as_var(), y);

        expect(expr->is_var(), isFalse);
        expect([&] { (void) expr->as_var(); }, throwsA<std::bad_variant_access>);
    });

    test("factory binop(expression::binop)", [] {
        const auto x = var("X");
        const auto y = var("Y");

        const auto expr = binop(var_expr(x), binop_type::set_union, var_expr(y));
        expect(expr->is_binop(), isTrue);
        expect(expr->as_binop().type, binop_type::set_union);
        expect(expr->as_binop().left->is_var(), isTrue);
        expect(expr->as_binop().left->as_var(), x);
        expect(expr->as_binop().right->is_var(), isTrue);
        expect(expr->as_binop().right->as_var(), y);

        expect(expr->is_var(), isFalse);
        expect([&] { (void) expr->as_var(); }, throwsA<std::bad_variant_access>);
    });
}
