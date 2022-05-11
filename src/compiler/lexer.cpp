#include "compiler/lexer.h"

#include <cstring>

#include <FlexLexer.h>

#include "compiler/parser.h"

namespace tema {

// TODO: Optimize? Not very important.

const std::map<std::string, int, std::less<>> keyword_table{
        {"var", tok_var},
        {"export", tok_export},
        {"definition", tok_definition},
        {"theorem", tok_theorem},
        {"exercise", tok_exercise},
        {"proof", tok_proof},
        {"missing", tok_missing},
};

bool is_keyword_token(int tok) {
    return std::find_if(keyword_table.begin(), keyword_table.end(), [tok](const auto& pair) {
               return pair.second == tok;
           }) != keyword_table.end();
}

void throw_parse_error(const location& loc, std::string msg) {
    throw parse_error{"Parse error at " + loc.file_name + ":" + std::to_string(loc.line) + ":" + std::to_string(loc.col) + ": " + std::move(msg)};
}

void throw_unexpected_token_error(const location& loc) {
    throw_parse_error(loc, "unexpected token.");
}

void flex_lexer_scanner::update_location_from_last_token() {
    if (last_token == tok_eol) {
        loc.line += 1;
        loc.col = 1;
        return;
    }
    if (is_keyword_token(last_token) ||
        last_token == tok_identifier ||
        last_token == tok_string_literal ||
        last_token == tok_whitespace) {
        loc.col += static_cast<int>(std::strlen(lexer->YYText()));
        return;
    }
    loc.col += 1;
}

flex_lexer_scanner::flex_lexer_scanner(std::istream& in, std::string file_name)
    : lexer(std::make_unique<yyFlexLexer>(&in, nullptr)), loc{std::move(file_name), 1, 1} {}

flex_lexer_scanner::~flex_lexer_scanner() = default;

std::pair<token, std::string_view> flex_lexer_scanner::consume_token(bool allow_eof) {
    if (next_token != -1) {
        last_token = next_token;
        next_token = -1;
    } else {
        do {
            if (last_token >= 0) {
                update_location_from_last_token();
            }
            const auto token_type = lexer->yylex();
            if (token_type < 0) {
                throw_parse_error(loc, "Unknown token '" + std::string(lexer->YYText()) + "'");
            }
            last_token = token_type;
        } while (last_token == tok_whitespace || last_token == tok_eol);
    }
    if (last_token == tok_eof && !allow_eof) {
        throw_parse_error(loc, "Unexpected end of file");
    }
    return {static_cast<token>(last_token), lexer->YYText()};
}

std::string_view flex_lexer_scanner::consume_token_exact(token required_token, std::string_view error_msg) {
    auto [token, text] = consume_token();
    if (token != required_token) {
        throw_parse_error(loc, std::string(error_msg));
    }
    return text;
}

void flex_lexer_scanner::unconsume_last_token() {
    next_token = last_token;
}

const location& flex_lexer_scanner::current_loc() const {
    return loc;
}

}  // namespace tema
