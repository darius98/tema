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

const std::map<int, int> token_priority_map{
        {tok_set_union, 1},
        {tok_set_intersection, 1},
        {tok_set_difference, 1},
        {tok_set_sym_difference, 1},

        {tok_eq, 2},
        {tok_n_eq, 2},
        {tok_less, 2},
        {tok_n_less, 2},
        {tok_eq_less, 2},
        {tok_n_eq_less, 2},
        {tok_greater, 2},
        {tok_n_greater, 2},
        {tok_eq_greater, 2},
        {tok_n_eq_greater, 2},
        {tok_in, 2},
        {tok_n_in, 2},
        {tok_includes, 2},
        {tok_n_includes, 2},
        {tok_eq_includes, 2},
        {tok_n_eq_includes, 2},
        {tok_is_included, 2},
        {tok_n_is_included, 2},
        {tok_eq_is_included, 2},
        {tok_n_eq_is_included, 2},

        {tok_neg, 3},
        {tok_implies, 4},
        {tok_equiv, 4},
        {tok_conj, 4},
        {tok_disj, 4},

        {tok_open_paren, 5},
};

const std::map<int, statement_ptr (*)(statement_ptr)> token_unary_stmt_factory_map{
        {tok_neg, neg},
};

const std::map<int, statement_ptr (*)(statement_ptr, statement_ptr)> token_binary_stmt_factory_map{
        {tok_implies, implies},
        {tok_equiv, equiv},
        {tok_conj, [](statement_ptr a, statement_ptr b) {
             return conj(std::move(a), std::move(b));
         }},
        {tok_disj, [](statement_ptr a, statement_ptr b) {
             return disj(std::move(a), std::move(b));
         }},
};

const std::map<int, binop_type> token_binary_expr_op_map{
        {tok_set_union, binop_type::set_union},
        {tok_set_intersection, binop_type::set_intersection},
        {tok_set_difference, binop_type::set_difference},
        {tok_set_sym_difference, binop_type::set_sym_difference},
};

const std::map<int, rel_type> token_rel_op_map{
        {tok_eq, rel_type::eq},
        {tok_n_eq, rel_type::n_eq},
        {tok_less, rel_type::less},
        {tok_n_less, rel_type::n_less},
        {tok_eq_less, rel_type::eq_less},
        {tok_n_eq_less, rel_type::n_eq_less},
        {tok_greater, rel_type::greater},
        {tok_n_greater, rel_type::n_greater},
        {tok_eq_greater, rel_type::eq_greater},
        {tok_n_eq_greater, rel_type::n_eq_greater},
        {tok_in, rel_type::in},
        {tok_n_in, rel_type::n_in},
        {tok_includes, rel_type::includes},
        {tok_n_includes, rel_type::n_includes},
        {tok_eq_includes, rel_type::eq_includes},
        {tok_n_eq_includes, rel_type::n_eq_includes},
        {tok_is_included, rel_type::is_included},
        {tok_n_is_included, rel_type::n_is_included},
        {tok_eq_is_included, rel_type::eq_is_included},
        {tok_n_eq_is_included, rel_type::n_eq_is_included},
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

std::pair<int, const char*> flex_lexer_scanner::consume_token(bool allow_eof) {
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
    return {last_token, lexer->YYText()};
}

const char* flex_lexer_scanner::consume_token_exact(int required_token, const char* error_msg) {
    auto [token, text] = consume_token();
    if (token != required_token) {
        throw_parse_error(loc, error_msg);
    }
    return text;
}

// Note: this only works for one token.
void flex_lexer_scanner::unconsume_last_token() {
    next_token = last_token;
}

const location& flex_lexer_scanner::current_loc() const {
    return loc;
}

}  // namespace tema
