#include "core/relationship.h"

#include <mcga/test_ext/matchers.hpp>

using namespace tema;
using namespace mcga::matchers;

TEST_CASE("variable") {
    test("var factory", [] {
        const auto x = var("X");
        expect(x->name, "X");
        expect(x.use_count(), 1);

        const auto y = var("Y");
        expect(y->name, "Y");
        expect(y.use_count(), 1);

        const auto lo = var("A_much_longer_variable_naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaame");
        expect(lo->name, "A_much_longer_variable_naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaame");
        expect(lo.use_count(), 1);
    });
}
