#include "compiler/parser.h"

#include <fstream>
#include <sstream>

#include "compiler/lexer.h"
#include "util/tpack.h"

namespace tema {

static const std::map<int, int> token_priority_map{
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

static const std::map<int, statement_ptr (*)(statement_ptr)> token_unary_stmt_factory_map{
        {tok_neg, neg},
};

static const std::map<int, statement_ptr (*)(statement_ptr, statement_ptr)> token_binary_stmt_factory_map{
        {tok_implies, implies},
        {tok_equiv, equiv},
        {tok_conj, [](statement_ptr a, statement_ptr b) {
             return conj({std::move(a), std::move(b)});
         }},
        {tok_disj, [](statement_ptr a, statement_ptr b) {
             return disj({std::move(a), std::move(b)});
         }},
};

static const std::map<int, binop_type> token_binary_expr_op_map{
        {tok_set_union, binop_type::set_union},
        {tok_set_intersection, binop_type::set_intersection},
        {tok_set_difference, binop_type::set_difference},
        {tok_set_sym_difference, binop_type::set_sym_difference},
};

static const std::map<int, rel_type> token_rel_op_map{
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

using partial_statement = std::variant<variable_ptr, expr_ptr, statement_ptr>;

class reverse_polish_notation_builder {
    enum class expectation {
        expect_operand,
        expect_operator,
    };

    expectation state = expectation::expect_operand;
    std::vector<partial_statement> partials;
    std::vector<int> operators;

    void check_expectation(expectation from, expectation to, const location& loc) {
        if (state != from) {
            throw_unexpected_token_error(loc);
        }
        state = to;
    }

    template<util::one_of<statement_ptr, expr_ptr> As>
    [[nodiscard]] As pop_last_partial(As (*var_wrapper)(variable_ptr), const location& loc) {
        if (partials.empty()) {
            throw_unexpected_token_error(loc);
        }
        auto partial = std::move(partials.back());
        partials.pop_back();
        if (holds_alternative<As>(partial)) {
            return get<As>(std::move(partial));
        } else if (holds_alternative<variable_ptr>(partial)) {
            return var_wrapper(get<variable_ptr>(std::move(partial)));
        }
        throw_unexpected_token_error(loc);
    }

    template<util::one_of<statement_ptr, expr_ptr> As>
    [[nodiscard]] std::pair<As, As> pop_last_2_partials(As (*var_wrapper)(variable_ptr), const location& loc) {
        auto last = pop_last_partial(var_wrapper, loc);
        auto before_last = pop_last_partial(var_wrapper, loc);
        return std::pair{std::move(before_last), std::move(last)};
    }

    statement_ptr try_reduce_unary_stmt(int operator_token, const location& loc) {
        const auto it = token_unary_stmt_factory_map.find(operator_token);
        if (it == token_unary_stmt_factory_map.end()) {
            return nullptr;
        }
        return (it->second)(pop_last_partial(var_stmt, loc));
    }

    statement_ptr try_reduce_binary_stmt(int operator_token, const location& loc) {
        const auto it = token_binary_stmt_factory_map.find(operator_token);
        if (it == token_binary_stmt_factory_map.end()) {
            return nullptr;
        }
        return std::apply(it->second, pop_last_2_partials(var_stmt, loc));
    }

    expr_ptr try_reduce_binary_expr(int operator_token, const location& loc) {
        const auto it = token_binary_expr_op_map.find(operator_token);
        if (it == token_binary_expr_op_map.end()) {
            return nullptr;
        }
        auto [expr1, expr2] = pop_last_2_partials(var_expr, loc);
        return binop(std::move(expr1), it->second, std::move(expr2));
    }

    statement_ptr try_reduce_relationship(int operator_token, const location& loc) {
        const auto it = token_rel_op_map.find(operator_token);
        if (it == token_rel_op_map.end()) {
            return nullptr;
        }
        auto [expr1, expr2] = pop_last_2_partials(var_expr, loc);
        return rel_stmt(std::move(expr1), it->second, std::move(expr2));
    }

    partial_statement reduce_operator(int operator_token, const location& loc) {
        if (auto stmt = try_reduce_unary_stmt(operator_token, loc); stmt) {
            return stmt;
        }
        if (auto stmt = try_reduce_binary_stmt(operator_token, loc); stmt) {
            return stmt;
        }
        if (auto expr = try_reduce_binary_expr(operator_token, loc); expr) {
            return expr;
        }
        if (auto stmt = try_reduce_relationship(operator_token, loc); stmt) {
            return stmt;
        }
        throw_unexpected_token_error(loc);
    }

    void reduce_operators(const location& loc, auto predicate) {
        while (!operators.empty() && predicate()) {
            int operator_token = operators.back();
            operators.pop_back();
            partials.push_back(reduce_operator(operator_token, loc));
        }
    }

public:
    bool can_finish() {
        return operators.empty() &&
               partials.size() == 1 &&
               (holds_alternative<statement_ptr>(partials[0]) ||
                holds_alternative<variable_ptr>(partials[0]));
    }

    statement_ptr finish(const location& loc) {
        reduce_operators(loc, [] { return true; });
        if (!can_finish()) {
            throw_unexpected_token_error(loc);
        }
        if (holds_alternative<variable_ptr>(partials[0])) {
            return var_stmt(get<variable_ptr>(std::move(partials[0])));
        }
        return get<statement_ptr>(std::move(partials[0]));
    }

    void add_partial(partial_statement partial, const location& loc) {
        check_expectation(expectation::expect_operand, expectation::expect_operator, loc);
        partials.push_back(std::move(partial));
    }

    void open_paren(const location& loc) {
        check_expectation(expectation::expect_operand, expectation::expect_operand, loc);
        operators.push_back(tok_open_paren);
    }

    void close_paren(const location& loc) {
        check_expectation(expectation::expect_operator, expectation::expect_operator, loc);
        reduce_operators(loc, [this] { return operators.back() != tok_open_paren; });
        if (operators.empty()) {
            throw_unexpected_token_error(loc);
        }
        operators.pop_back();  // Remove the open paren.
    }

    void add_operator(int operator_token, const location& loc) {
        // TODO: A more generic unary operator check.
        if (operator_token == tok_neg) {
            check_expectation(expectation::expect_operand, expectation::expect_operand, loc);
        } else {
            check_expectation(expectation::expect_operator, expectation::expect_operand, loc);
        }
        int token_priority = token_priority_map.find(operator_token)->second;
        reduce_operators(loc, [this, token_priority] {
            return token_priority_map.find(operators.back())->second < token_priority;
        });
        operators.push_back(operator_token);
    }
};

// This function uses a "reverse polish notation"-based algorithm to parse statements,
// except when encountering a forall node where it does a recursive call.
// Similar to the "shunting-yard algorithm".
statement_ptr parse_stmt(flex_lexer_scanner& scanner, const scope& enclosing_scope, bool allow_extend) {
    const location& loc = scanner.current_loc();  // This is a reference, so always up to date.
    reverse_polish_notation_builder rpn_builder;
    while (true) {
        if (!allow_extend && rpn_builder.can_finish()) {
            return rpn_builder.finish(loc);
        }
        auto [token, text] = scanner.consume_token(true);
        if (token == tok_eof || is_keyword_token(token)) {
            scanner.unconsume_last_token();
            return rpn_builder.finish(loc);
        }
        if (token == tok_forall) {
            auto forall_var_name = scanner.consume_token_exact(tok_identifier, "Expected variable name (an identifier).");
            scope forall_scope(&enclosing_scope);
            auto forall_var = var(symbol{forall_var_name});
            forall_scope.add_var(forall_var);
            auto forall_stmt = parse_stmt(scanner, forall_scope, false);
            rpn_builder.add_partial(forall(std::move(forall_var), std::move(forall_stmt)), loc);
            continue;
        }
        switch (token) {
            case tok_identifier: {
                variable_ptr var;
                try {
                    var = enclosing_scope.get_var(symbol_view{text});
                } catch (const var_not_found&) {
                    throw_parse_error(loc, "Unknown variable '" + std::string(text) + "'.");
                }
                rpn_builder.add_partial(std::move(var), loc);
                break;
            }
            case tok_truth: {
                rpn_builder.add_partial(truth(), loc);
                break;
            }
            case tok_contradiction: {
                rpn_builder.add_partial(contradiction(), loc);
                break;
            }
            case tok_open_paren: {
                rpn_builder.open_paren(loc);
                break;
            }
            case tok_close_paren: {
                rpn_builder.close_paren(loc);
                break;
            }
            default: {
                rpn_builder.add_operator(token, loc);
                break;
            }
        }
    }
}

void parse_var_decl(flex_lexer_scanner& scanner, module& mod, bool is_exported) {
    auto identifier = scanner.consume_token_exact(tok_identifier, "Expected variable name (an identifier).");
    mod.add_variable_decl(module::var_decl{
            .loc = scanner.current_loc(),
            .exported = is_exported,
            .var = var(symbol{identifier}),
    });
}

std::string parse_stmt_name(flex_lexer_scanner& scanner) {
    auto literal = scanner.consume_token_exact(tok_string_literal, "Expected statement name as a string literal");
    auto str_literal = std::string(literal.substr(1));
    str_literal.erase(str_literal.size() - 1, 1);
    return str_literal;
}

std::optional<scope> parse_proof(flex_lexer_scanner& scanner, const module&) {
    scanner.consume_token_exact(tok_proof, "Expected statement proof. Use the 'proof missing' keyword if you plan to add the proof later.");
    const auto [tok2, text2] = scanner.consume_token();
    if (tok2 == tok_missing) {
        return std::nullopt;
    }
    // TODO: Set up a syntax for proofs and parse it here.
    throw_parse_error(scanner.current_loc(), "Statement proofs are not currently supported. Use the 'proof missing' keyword.");
}

void parse_stmt_decl(flex_lexer_scanner& scanner, module& mod, module::stmt_decl_type stmt_type) {
    auto decl_loc = scanner.current_loc();
    auto stmt_name = parse_stmt_name(scanner);
    auto stmt = parse_stmt(scanner, mod.get_internal_scope(), true);
    std::optional<scope> proof;
    if (stmt_type != module::stmt_decl_type::definition) {
        proof = parse_proof(scanner, mod);
    }

    mod.add_statement_decl(module::stmt_decl{
            .loc = std::move(decl_loc),
            .exported = stmt_type == module::stmt_decl_type::definition ||
                        stmt_type == module::stmt_decl_type::theorem,
            .type = stmt_type,
            .name = std::move(stmt_name),
            .stmt = std::move(stmt),
            .proof_description = std::move(proof),
    });
}

bool parse_decl(flex_lexer_scanner& scanner, module& mod) {
    auto tok = scanner.consume_token(true).first;
    if (tok == tok_eof) {
        return false;
    }
    if (tok == tok_export || tok == tok_var) {
        bool is_exported = (tok == tok_export);
        if (is_exported) {
            scanner.consume_token_exact(tok_var, "Unexpected token: only variable declarations can be manually exported.");
        }
        parse_var_decl(scanner, mod, is_exported);
        return true;
    }
    switch (tok) {
        case tok_definition:
            parse_stmt_decl(scanner, mod, module::stmt_decl_type::definition);
            break;
        case tok_theorem:
            parse_stmt_decl(scanner, mod, module::stmt_decl_type::theorem);
            break;
        case tok_exercise:
            parse_stmt_decl(scanner, mod, module::stmt_decl_type::exercise);
            break;
        default:
            throw_parse_error(scanner.current_loc(), "Expected variable or statement declaration.");
    }
    return true;
}

module parse_module_stream(std::istream& in, std::string file_name) {
    flex_lexer_scanner scanner(in, std::move(file_name));
    module mod(scanner.current_loc().file_name);
    while (parse_decl(scanner, mod)) {
    }
    return mod;
}

module parse_module_code(std::string_view code) {
    std::stringstream string_stream;
    // TODO: This is quite inefficient, why do we need to copy the data / allocate? We should just
    //  be able to read from the string_view directly.
    string_stream.write(code.data(), static_cast<std::streamsize>(code.size()));
    return parse_module_stream(string_stream, "<anonymous module>");
}

module parse_module_file(std::string file_name) {
    std::ifstream file_stream(file_name);
    if (file_stream.fail()) {
        throw parse_error{"Could not open file '" + file_name + "'."};
    }
    return parse_module_stream(file_stream, std::move(file_name));
}

}  // namespace tema
