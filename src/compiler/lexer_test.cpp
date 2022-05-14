#include "compiler/lexer.h"

#include <mcga/test_ext/matchers.hpp>

#include "compiler/parser.h"

using namespace tema;
using namespace mcga::matchers;

TEST("is_keyword_token") {
    expect(is_keyword_token(tok_proof), isTrue);
    expect(is_keyword_token(tok_open_paren), isFalse);
}

TEST("throw_parse_error and throw_unexpected_token_error") {
    expect([&] {
        throw_parse_error("", {}, "msg");
    },
           throwsA<parse_error>);

    expect([&] {
        throw_unexpected_token_error("", {});
    },
           throwsA<parse_error>);
}

TEST_CASE("flex_lexer_scanner") {
    std::unique_ptr<std::stringstream> stream;
    std::unique_ptr<flex_lexer_scanner> scanner;

    setUp([&] {
        stream = std::make_unique<std::stringstream>();
        scanner = std::make_unique<flex_lexer_scanner>(*stream, "/file");
    });

    tearDown([&] {
        scanner.reset();
        stream.reset();
    });

    test("consume_token EOF unexpected", [&] {
        *stream << "";
        expect([&] {
            (void) scanner->consume_token();
        },
               throwsA<parse_error>);
    });

    test("consume_token EOF expected", [&] {
        *stream << "";
        expect(scanner->consume_token(true), std::pair{tok_eof, std::string_view{""}});
    });

    test("consume_token unknown token", [&] {
        *stream << "?";
        expect([&] {
            (void) scanner->consume_token();
        },
               throwsA<parse_error>);
    });

    test("consume_token_exact expected token", [&] {
        *stream << "export var xyz = ()";
        expect(scanner->consume_token_exact(tok_export, ""), "export");
        expect(scanner->consume_token_exact(tok_var, ""), "var");
        expect(scanner->consume_token_exact(tok_identifier, ""), "xyz");
        expect(scanner->consume_token_exact(tok_eq, ""), "=");
        expect(scanner->consume_token_exact(tok_open_paren, ""), "(");
        expect(scanner->consume_token_exact(tok_close_paren, ""), ")");
    });

    test("consume_token_exact unexpected token", [&] {
        *stream << "export";
        expect([&] {
            scanner->consume_token_exact(tok_identifier, "");
        },
               throwsA<parse_error>);
    });

    test("unconsume_token()", [&] {
        *stream << "export var more proof";
        expect(scanner->consume_token_exact(tok_export, ""), "export");
        expect(scanner->consume_token_exact(tok_var, ""), "var");
        expect(scanner->consume_token_exact(tok_identifier, ""), "more");
        scanner->unconsume_last_token();
        expect(scanner->consume_token_exact(tok_identifier, ""), "more");
        expect(scanner->consume_token_exact(tok_proof, ""), "proof");
    });

    test("file_location", [&] {
        *stream << "var stuff export \n\n   \t\"String literal\"   xx<>\n";
        (void) scanner->consume_token();
        expect(scanner->current_loc(), file_location{1, 1});
        (void) scanner->consume_token();
        expect(scanner->current_loc(), file_location{1, 5});
        (void) scanner->consume_token();
        expect(scanner->current_loc(), file_location{1, 11});
        (void) scanner->consume_token();
        expect(scanner->current_loc(), file_location{3, 5});
        (void) scanner->consume_token();
        expect(scanner->current_loc(), file_location{3, 24});
        (void) scanner->consume_token();
        expect(scanner->current_loc(), file_location{3, 26});
        (void) scanner->consume_token();
        expect(scanner->current_loc(), file_location{3, 27});
    });
}
