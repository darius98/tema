#pragma once

#include <cstdio>

#include <map>
#include <string>

#include "core/statement.h"

namespace tema {

// Used in the flex lexer definition file.
extern const std::map<std::string, int, std::less<>> keyword_table;
extern const std::map<int, int> token_priority_map;
extern const std::map<int, statement_ptr(*)(statement_ptr)> unary_statement_factories;
extern const std::map<int, statement_ptr(*)(statement_ptr, statement_ptr)> binary_statement_factories;
extern const std::map<int, binop_type> token_binop_map;
extern const std::map<int, rel_type> token_rel_map;

bool is_keyword_token(int tok);

enum token {
    tok_eof = 0,

    tok_export,
    tok_definition,
    tok_theorem,
    tok_exercise,
    tok_proof,
    tok_missing,
    tok_var,

    // Constants with their own symbol
    tok_truth,
    tok_contradiction,

    // PL
    tok_neg,
    tok_implies,
    tok_equiv,
    tok_conj,
    tok_disj,
    tok_forall,

    // Relationships
    tok_eq,
    tok_n_eq,
    tok_less,
    tok_n_less,
    tok_eq_less,
    tok_n_eq_less,
    tok_greater,
    tok_n_greater,
    tok_eq_greater,
    tok_n_eq_greater,
    tok_in,
    tok_n_in,
    tok_includes,
    tok_n_includes,
    tok_eq_includes,
    tok_n_eq_includes,
    tok_is_included,
    tok_n_is_included,
    tok_eq_is_included,
    tok_n_eq_is_included,

    // Operators
    tok_set_union,
    tok_set_intersection,
    tok_set_difference,
    tok_set_sym_difference,

    tok_open_paren,
    tok_close_paren,

    // Identifiers
    tok_identifier,
    tok_string_literal,

    // Others
    tok_eol,
    tok_whitespace,
};

}  // namespace tema
