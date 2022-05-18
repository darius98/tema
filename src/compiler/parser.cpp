#include "compiler/parser.h"

#include <fstream>
#include <sstream>

#include <mcga/meta/tpack.hpp>

#include "compiler/lexer.h"

namespace tema {

namespace {

auto& get_token_priority_map() {
    static const std::map<int, int> table{
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
    return table;
}

auto& get_token_unary_stmt_factory_map() {
    static const std::map<int, statement_ptr (*)(statement_ptr)> table{
            {tok_neg, [](statement_ptr a) {
                 return neg(std::move(a));
             }},
    };
    return table;
}

auto& get_token_binary_stmt_factory_map() {
    static const std::map<int, statement_ptr (*)(statement_ptr, statement_ptr)> table{
            {tok_implies, [](statement_ptr a, statement_ptr b) {
                 return implies(std::move(a), std::move(b));
             }},
            {tok_equiv, [](statement_ptr a, statement_ptr b) {
                 return equiv(std::move(a), std::move(b));
             }},
            {tok_conj, [](statement_ptr a, statement_ptr b) {
                 return conj({std::move(a), std::move(b)});
             }},
            {tok_disj, [](statement_ptr a, statement_ptr b) {
                 return disj({std::move(a), std::move(b)});
             }},
    };
    return table;
}

auto& get_token_binary_expr_op_map() {
    static const std::map<int, binop_type> table{
            {tok_set_union, binop_type::set_union},
            {tok_set_intersection, binop_type::set_intersection},
            {tok_set_difference, binop_type::set_difference},
            {tok_set_sym_difference, binop_type::set_sym_difference},
    };
    return table;
}

auto& get_token_rel_op_map() {
    static const std::map<int, rel_type> table{
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
    return table;
}

using partial_statement = std::variant<variable_ptr, expr_ptr, statement_ptr>;

class reverse_polish_notation_builder {
    enum class expectation {
        expect_operand,
        expect_operator,
    };

    expectation state = expectation::expect_operand;
    std::vector<partial_statement> partials;
    std::vector<int> operators;

    const file_location& loc;
    std::string file_name;

    void check_expectation(expectation from, expectation to) {
        if (state != from) {
            throw_unexpected_token_error(file_name, loc);
        }
        state = to;
    }

    template<mcga::meta::one_of<statement_ptr, expr_ptr> As>
    [[nodiscard]] As pop_last_partial() {
        if (partials.empty()) {
            throw_unexpected_token_error(file_name, loc);
        }
        auto partial = std::move(partials.back());
        partials.pop_back();
        if (holds_alternative<As>(partial)) {
            return get<As>(std::move(partial));
        }
        if (holds_alternative<variable_ptr>(partial)) {
            if constexpr (std::is_same_v<As, statement_ptr>) {
                return var_stmt(get<variable_ptr>(std::move(partial)));
            } else {
                return var_expr(get<variable_ptr>(std::move(partial)));
            }
        }
        throw_unexpected_token_error(file_name, loc);
    }

    template<mcga::meta::one_of<statement_ptr, expr_ptr> As>
    [[nodiscard]] std::pair<As, As> pop_last_2_partials() {
        As last = pop_last_partial<As>();
        As before_last = pop_last_partial<As>();
        return std::pair{std::move(before_last), std::move(last)};
    }

    statement_ptr try_reduce_unary_stmt(int operator_token) {
        const auto it = get_token_unary_stmt_factory_map().find(operator_token);
        if (it == get_token_unary_stmt_factory_map().end()) {
            return nullptr;
        }
        return (it->second)(pop_last_partial<statement_ptr>());
    }

    statement_ptr try_reduce_binary_stmt(int operator_token) {
        const auto it = get_token_binary_stmt_factory_map().find(operator_token);
        if (it == get_token_binary_stmt_factory_map().end()) {
            return nullptr;
        }
        return std::apply(it->second, pop_last_2_partials<statement_ptr>());
    }

    expr_ptr try_reduce_binary_expr(int operator_token) {
        const auto it = get_token_binary_expr_op_map().find(operator_token);
        if (it == get_token_binary_expr_op_map().end()) {
            return nullptr;
        }
        auto [expr1, expr2] = pop_last_2_partials<expr_ptr>();
        return binop(std::move(expr1), it->second, std::move(expr2));
    }

    statement_ptr try_reduce_relationship(int operator_token) {
        const auto it = get_token_rel_op_map().find(operator_token);
        if (it == get_token_rel_op_map().end()) {
            return nullptr;
        }
        auto [expr1, expr2] = pop_last_2_partials<expr_ptr>();
        return rel_stmt(std::move(expr1), it->second, std::move(expr2));
    }

    partial_statement reduce_operator(int operator_token) {
        if (auto stmt = try_reduce_unary_stmt(operator_token); stmt) {
            return stmt;
        }
        if (auto stmt = try_reduce_binary_stmt(operator_token); stmt) {
            return stmt;
        }
        if (auto expr = try_reduce_binary_expr(operator_token); expr) {
            return expr;
        }
        if (auto stmt = try_reduce_relationship(operator_token); stmt) {
            return stmt;
        }
        throw_unexpected_token_error(file_name, loc);
    }

    void reduce_operators_until(auto predicate) {
        while (!operators.empty() && predicate()) {
            int operator_token = operators.back();
            operators.pop_back();
            partials.push_back(reduce_operator(operator_token));
        }
    }

public:
    reverse_polish_notation_builder(
            const file_location& loc,
            std::string file_name)
        : loc(loc), file_name(std::move(file_name)) {}

    bool can_finish() {
        return operators.empty() &&
               partials.size() == 1 &&
               (holds_alternative<statement_ptr>(partials[0]) ||
                holds_alternative<variable_ptr>(partials[0]));
    }

    statement_ptr finish() {
        reduce_operators_until([] { return true; });
        if (!can_finish()) {
            throw_unexpected_token_error(file_name, loc);
        }
        if (holds_alternative<variable_ptr>(partials[0])) {
            return var_stmt(get<variable_ptr>(std::move(partials[0])));
        }
        return get<statement_ptr>(std::move(partials[0]));
    }

    void add_partial(partial_statement partial) {
        check_expectation(expectation::expect_operand, expectation::expect_operator);
        partials.push_back(std::move(partial));
    }

    void open_paren() {
        check_expectation(expectation::expect_operand, expectation::expect_operand);
        operators.push_back(tok_open_paren);
    }

    void close_paren() {
        check_expectation(expectation::expect_operator, expectation::expect_operator);
        reduce_operators_until([this] { return operators.back() != tok_open_paren; });
        if (operators.empty()) {
            throw_unexpected_token_error(file_name, loc);
        }
        operators.pop_back();  // Remove the open paren.
    }

    void add_operator(int operator_token) {
        // TODO: A more generic unary operator check.
        if (operator_token == tok_neg) {
            check_expectation(expectation::expect_operand, expectation::expect_operand);
        } else {
            check_expectation(expectation::expect_operator, expectation::expect_operand);
        }
        int token_priority = get_token_priority_map().find(operator_token)->second;
        reduce_operators_until([this, token_priority] {
            return get_token_priority_map().find(operators.back())->second < token_priority;
        });
        operators.push_back(operator_token);
    }
};

// This function uses a "reverse polish notation"-based algorithm to parse statements,
// except when encountering a forall node where it does a recursive call.
// Similar to the "shunting-yard algorithm".
statement_ptr parse_stmt(flex_lexer_scanner& scanner, const scope& enclosing_scope, bool allow_extend) {  // NOLINT(misc-no-recursion)
    // This keeps a reference to the location, so is always up to date.
    reverse_polish_notation_builder rpn_builder(scanner.current_loc(), std::string{scanner.get_file_name()});
    while (true) {
        if (!allow_extend && rpn_builder.can_finish()) {
            return rpn_builder.finish();
        }
        auto [token, text] = scanner.consume_token(true);
        if (token == tok_eof || is_keyword_token(token)) {
            scanner.unconsume_last_token();
            return rpn_builder.finish();
        }
        if (token == tok_forall) {
            auto forall_var_name = scanner.consume_token_exact(tok_identifier, "Expected variable name (an identifier).");
            scope forall_scope(&enclosing_scope);
            auto forall_var = var(symbol{forall_var_name});
            forall_scope.add_var(forall_var);
            auto forall_stmt = parse_stmt(scanner, forall_scope, false);
            rpn_builder.add_partial(forall(std::move(forall_var), std::move(forall_stmt)));
            continue;
        }
        switch (token) {
            case tok_identifier: {
                variable_ptr var;
                try {
                    var = enclosing_scope.get_var(symbol_view{text});
                } catch (const var_not_found&) {
                    throw_parse_error(std::string{scanner.get_file_name()},
                                      scanner.current_loc(),
                                      "Unknown variable '" + std::string(text) + "'.");
                }
                rpn_builder.add_partial(std::move(var));
                break;
            }
            case tok_truth: {
                rpn_builder.add_partial(truth());
                break;
            }
            case tok_contradiction: {
                rpn_builder.add_partial(contradiction());
                break;
            }
            case tok_open_paren: {
                rpn_builder.open_paren();
                break;
            }
            case tok_close_paren: {
                rpn_builder.close_paren();
                break;
            }
            default: {
                rpn_builder.add_operator(token);
                break;
            }
        }
    }
}

void parse_var_decl(flex_lexer_scanner& scanner, module& mod, bool is_exported) {
    auto identifier = scanner.consume_token_exact(tok_identifier, "Expected variable name (an identifier).");
    mod.add_variable_decl(var_decl{
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
    throw_parse_error(std::string{scanner.get_file_name()},
                      scanner.current_loc(),
                      "Statement proofs are not currently supported. Use the 'proof missing' keyword.");
}

void parse_stmt_decl(flex_lexer_scanner& scanner, module& mod, stmt_decl_type stmt_type) {
    auto decl_loc = scanner.current_loc();
    auto stmt_name = parse_stmt_name(scanner);
    auto stmt = parse_stmt(scanner, mod.get_internal_scope(), true);
    std::optional<scope> proof;
    if (stmt_type != stmt_decl_type::definition) {
        proof = parse_proof(scanner, mod);
    }

    mod.add_statement_decl(stmt_decl{
            .loc = decl_loc,
            .exported = stmt_type == stmt_decl_type::definition || stmt_type == stmt_decl_type::theorem,
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
            parse_stmt_decl(scanner, mod, stmt_decl_type::definition);
            break;
        case tok_theorem:
            parse_stmt_decl(scanner, mod, stmt_decl_type::theorem);
            break;
        case tok_exercise:
            parse_stmt_decl(scanner, mod, stmt_decl_type::exercise);
            break;
        default:
            throw_parse_error(std::string{scanner.get_file_name()},
                              scanner.current_loc(),
                              "Expected variable or statement declaration.");
    }
    return true;
}

}  // namespace

module parse_module(std::istream& stream, const std::filesystem::path& file_name) {
    flex_lexer_scanner scanner(stream, file_name);
    module mod(file_name.stem(), file_name);
    while (parse_decl(scanner, mod)) {
    }
    return mod;
}

module parse_module(std::string_view code) {
    std::stringstream string_stream;
    // TODO: This is quite inefficient, why do we need to copy the data / allocate? We should just
    //  be able to read from the string_view directly.
    string_stream.write(code.data(), static_cast<std::streamsize>(code.size()));
    return parse_module(string_stream, "<anonymous module>");
}

}  // namespace tema
