#include "compiler/parser.h"

#include "mcga/test_ext/matchers.hpp"

using namespace tema;
using namespace mcga::matchers;

void expect_fails_to_parse(std::string_view code, mcga::test::Context context = mcga::test::Context()) {
    mcga::test::expect([code] {
        (void) parse_module_from_string(code);
    },
                       throwsA<parse_error>, std::move(context));
}

TEST_CASE("compiler parser") {
    test("missing declaration", [] {
        expect_fails_to_parse("export");
        expect_fails_to_parse("export var x export");
    });

    // TODO: More tests!
}
