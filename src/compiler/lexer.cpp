#include "compiler/lexer.h"

#include <cstring>

#include <FlexLexer.h>

#include "compiler/parser.h"

namespace tema {

// TODO: Optimize? Not very important.

namespace {

const std::map<std::string, int, std::less<>>& get_keyword_table() {
    static const std::map<std::string, int, std::less<>> keyword_table{
            {"var", tok_var},
            {"export", tok_export},
            {"definition", tok_definition},
            {"theorem", tok_theorem},
            {"exercise", tok_exercise},
            {"proof", tok_proof},
            {"missing", tok_missing},
    };
    return keyword_table;
}

}  // namespace

int get_keyword_or_identifier(const char* token_text) {
    const auto& keyword_table = get_keyword_table();
    const auto k = keyword_table.find(token_text);
    if (k != keyword_table.end()) {
        return k->second;
    }
    return token::tok_identifier;
}

bool is_keyword_token(int tok) {
    const auto& keyword_table = get_keyword_table();
    return std::find_if(keyword_table.begin(), keyword_table.end(), [tok](const auto& pair) {
               return pair.second == tok;
           }) != keyword_table.end();
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
    : lexer(std::make_unique<yyFlexLexer>(&in, nullptr)), file_name(std::move(file_name)) {}

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
                throw_parse_error("Unknown token '" + std::string(lexer->YYText()) + "'");
            }
            last_token = token_type;
        } while (last_token == tok_whitespace || last_token == tok_eol);
    }
    if (last_token == tok_eof && !allow_eof) {
        throw_parse_error("Unexpected end of file");
    }
    return {static_cast<token>(last_token), lexer->YYText()};
}

std::string_view flex_lexer_scanner::consume_token_exact(token required_token, std::string_view error_msg) {
    auto [token, text] = consume_token();
    if (token != required_token) {
        throw_parse_error(std::string(error_msg));
    }
    return text;
}

void flex_lexer_scanner::unconsume_last_token() {
    next_token = last_token;
}

const file_location& flex_lexer_scanner::current_loc() const {
    return loc;
}

void flex_lexer_scanner::throw_parse_error(std::string msg) {
    throw parse_error{"Parse error at " + file_name + ":" + std::to_string(loc.line) + ":" + std::to_string(loc.col) + ": " + std::move(msg)};
}

void flex_lexer_scanner::throw_unexpected_token_error() {
    throw_parse_error("unexpected token.");
}

}  // namespace tema
