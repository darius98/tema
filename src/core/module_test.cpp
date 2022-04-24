#include "core/module.h"
#include "core/scope.h"

#include "mcga/test_ext/matchers.hpp"

using namespace tema;
using namespace mcga::matchers;

TEST_CASE("module") {
    test("valid constructor", [] {
        scope s;
        const auto x = var("X");
        s.add_var(x);
        s.add_statement("truth", truth());
        module m(std::move(s));
        expect(m.get_scope().own_vars(), std::set<variable_ptr>{x});
        expect(m.get_scope().own_statements(), std::vector<named_statement>{{"truth", truth()}});
    });

    test("invalid constructor", [] {
        scope p;
        scope s{&p};
        expect([&] { module m{std::move(s)}; }, throwsA<invalid_module_scope>);
    });
}
