#include "compiler/parser.h"

#include <fstream>
#include <sstream>

#include "compiler/lexer.h"
#include "util/tpack.h"

namespace tema {

using partial_statement = std::variant<variable_ptr, expr_ptr, statement_ptr>;

class reverse_polish_notation_builder {
    std::vector<partial_statement> partials;
    std::vector<int> operators;

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

public:
    void reduce_last_operator(const location& loc) {
        if (operators.empty()) {
            throw_unexpected_token_error(loc);
        }
        int operator_token = operators.back();
        operators.pop_back();
        partials.push_back(reduce_operator(operator_token, loc));
    }

    bool can_finish() {
        return operators.empty() &&
               partials.size() == 1 &&
               (holds_alternative<statement_ptr>(partials[0]) ||
                holds_alternative<variable_ptr>(partials[0]));
    }

    statement_ptr finish(const location& loc) {
        while (!operators.empty()) {
            reduce_last_operator(loc);
        }
        if (!can_finish()) {
            throw_unexpected_token_error(loc);
        }
        if (holds_alternative<variable_ptr>(partials[0])) {
            return var_stmt(get<variable_ptr>(std::move(partials[0])));
        }
        return get<statement_ptr>(std::move(partials[0]));
    }

    void add_partial(partial_statement partial) {
        partials.push_back(std::move(partial));
    }

    void open_paren() {
        operators.push_back(tok_open_paren);
    }

    void close_paren(const location& loc) {
        while (!operators.empty() && operators.back() != tok_open_paren) {
            reduce_last_operator(loc);
        }
        if (operators.empty()) {
            throw_unexpected_token_error(loc);
        }
        operators.pop_back();  // Remove the open paren.
    }

    void add_operator(const location& loc, int operator_token) {
        const auto token_priority_it = token_priority_map.find(operator_token);
        if (token_priority_it == token_priority_map.end()) {
            throw_unexpected_token_error(loc);
        }
        int token_priority = token_priority_it->second;
        while (!operators.empty() && token_priority_map.find(operators.back())->second < token_priority) {
            reduce_last_operator(loc);
        }
        operators.push_back(operator_token);
    }
};

// This function uses a "reverse polish notation"-based algorithm to parse statements,
// except when encountering a forall node where it does a recursive call.
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
            auto forall_var = var(forall_var_name);
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
                    throw_parse_error(loc, "Unknown variable '" + std::string(text) + "'.");
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
                rpn_builder.close_paren(loc);
                break;
            }
            default: {
                rpn_builder.add_operator(loc, token);
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
            .var = var(identifier),
    });
}

std::string parse_stmt_name(flex_lexer_scanner& scanner) {
    auto literal = scanner.consume_token_exact(tok_string_literal, "Expected statement name as a string literal");
    auto str_literal = std::string(literal + 1);
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
    bool is_exported = false;
    if (tok == tok_export) {
        is_exported = true;
        scanner.consume_token_exact(tok_var, "Unexpected token: only variable declarations can be manually exported.");
    }
    switch (tok) {
        case tok_var: {
            parse_var_decl(scanner, mod, is_exported);
            break;
        }
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

module parse_module(flex_lexer_scanner& scanner) {
    module mod(scanner.current_loc().file_name);
    while (parse_decl(scanner, mod)) {
    }
    return mod;
}

module parse_module_stream(std::istream& in, std::string file_name) {
    flex_lexer_scanner scanner(in, std::move(file_name));
    return parse_module(scanner);
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
