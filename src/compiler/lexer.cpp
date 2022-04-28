#include "compiler/lexer.h"

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
        {tok_neg, 1},
        {tok_implies, 2},
        {tok_equiv, 2},
        {tok_conj, 2},
        {tok_disj, 2},
        {tok_eq, 3},
        {tok_n_eq, 3},
        {tok_less, 3},
        {tok_n_less, 3},
        {tok_eq_less, 3},
        {tok_n_eq_less, 3},
        {tok_greater, 3},
        {tok_n_greater, 3},
        {tok_eq_greater, 3},
        {tok_n_eq_greater, 3},
        {tok_in, 3},
        {tok_n_in, 3},
        {tok_includes, 3},
        {tok_n_includes, 3},
        {tok_eq_includes, 3},
        {tok_n_eq_includes, 3},
        {tok_is_included, 3},
        {tok_n_is_included, 3},
        {tok_eq_is_included, 3},
        {tok_n_eq_is_included, 3},
        {tok_set_union, 4},
        {tok_set_intersection, 4},
        {tok_set_difference, 4},
        {tok_set_sym_difference, 4},
        {tok_open_paren, 5},
};

const std::map<int, statement_ptr (*)(statement_ptr)> unary_statement_factories{
        {tok_neg, neg},
};

const std::map<int, statement_ptr (*)(statement_ptr, statement_ptr)> binary_statement_factories{
        {tok_implies, implies},
        {tok_equiv, equiv},
        {tok_conj, [](statement_ptr a, statement_ptr b) {
             return conj(std::move(a), std::move(b));
         }},
        {tok_disj, [](statement_ptr a, statement_ptr b) {
             return disj(std::move(a), std::move(b));
         }},
};

const std::map<int, binop_type> token_binop_map{
        {tok_set_union, binop_type::set_union},
        {tok_set_intersection, binop_type::set_intersection},
        {tok_set_difference, binop_type::set_difference},
        {tok_set_sym_difference, binop_type::set_sym_difference},
};

const std::map<int, rel_type> token_rel_map{
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

}  // namespace tema
