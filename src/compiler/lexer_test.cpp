#include "compiler/lexer.h"

#include "mcga/test_ext/matchers.hpp"

using namespace tema;
using namespace mcga::matchers;

TEST_CASE("compiler lexer") {
    test("is_keyword_token", [] {
        expect(is_keyword_token(tok_proof), isTrue);
        expect(is_keyword_token(tok_open_paren), isFalse);
    });
}
