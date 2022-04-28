#include "compiler/parser.h"

#include <fstream>
#include <sstream>

#include <FlexLexer.h>

#include "compiler/lexer.h"

namespace tema {

class flex_lexer_scanner {
    yyFlexLexer lexer;
    location loc;
    int last_token = -1;
    int next_token = -1;

    void update_location_from_last_token() {
        if (last_token == tok_eol) {
            loc.line += 1;
            loc.col = 1;
            return;
        }
        if (is_keyword_token(last_token) ||
            last_token == tok_identifier ||
            last_token == tok_string_literal ||
            last_token == tok_whitespace) {
            loc.col += static_cast<int>(std::strlen(lexer.YYText()));
            return;
        }
        loc.col += 1;
    }

public:
    flex_lexer_scanner(std::istream& in, std::string file_name)
        : lexer(&in, nullptr), loc{std::move(file_name), 1, 1} {}

    std::pair<int, const char*> consume_token() {
        if (next_token != -1) {
            last_token = next_token;
            next_token = -1;
        } else {
            do {
                if (last_token >= 0) {
                    update_location_from_last_token();
                }
                const auto token_type = lexer.yylex();
                if (token_type < 0) {
                    throw parse_error{"Unknown token '" + std::string(lexer.YYText()) + "' at " + loc.file_name + ":" + std::to_string(loc.line) + ":" + std::to_string(loc.col)};
                }
                last_token = token_type;
            } while (last_token == tok_whitespace || last_token == tok_eol);
        }
        return {last_token, lexer.YYText()};
    }

    // Note: this only works for one token.
    void unconsume_last_token() {
        next_token = last_token;
    }

    const location& current_loc() const {
        return loc;
    }
};

using partial_statement = std::variant<variable_ptr, expr_ptr, statement_ptr>;
using opt_partial_statement = std::optional<partial_statement>;

opt_partial_statement reduce_operator(int operator_token, auto consume_next) {
    const auto force_stmt = [&](statement_ptr& to) {
        auto from_opt = consume_next();
        if (!from_opt.has_value()) {
            return false;
        }
        auto from = std::move(from_opt).value();
        if (holds_alternative<expr_ptr>(from)) {
            return false;
        }
        if (holds_alternative<variable_ptr>(from)) {
            to = var_stmt(get<variable_ptr>(std::move(from)));
        } else {
            to = get<statement_ptr>(std::move(from));
        }
        return true;
    };
    const auto force_expr = [&](expr_ptr& to) {
        auto from_opt = consume_next();
        if (!from_opt.has_value()) {
            return false;
        }
        auto from = std::move(from_opt).value();
        if (holds_alternative<statement_ptr>(from)) {
            return false;
        }
        if (holds_alternative<variable_ptr>(from)) {
            to = var_expr(get<variable_ptr>(std::move(from)));
        } else {
            to = get<expr_ptr>(std::move(from));
        }
        return true;
    };

    if (unary_statement_factories.contains(operator_token)) {
        const auto factory = unary_statement_factories.find(operator_token)->second;
        statement_ptr forced_stmt_last;
        if (!force_stmt(forced_stmt_last)) {
            return std::nullopt;
        }
        return factory(std::move(forced_stmt_last));
    }

    if (binary_statement_factories.contains(operator_token)) {
        const auto factory = binary_statement_factories.find(operator_token)->second;
        statement_ptr forced_stmt_last;
        if (!force_stmt(forced_stmt_last)) {
            return std::nullopt;
        }
        statement_ptr forced_stmt_prev;
        if (!force_stmt(forced_stmt_prev)) {
            return std::nullopt;
        }
        return factory(std::move(forced_stmt_prev), std::move(forced_stmt_last));
    }

    if (token_binop_map.contains(operator_token)) {
        expr_ptr forced_expr_last;
        if (!force_expr(forced_expr_last)) {
            return std::nullopt;
        }
        expr_ptr forced_expr_prev;
        if (!force_expr(forced_expr_prev)) {
            return std::nullopt;
        }
        const auto type = token_binop_map.find(operator_token)->second;
        return binop(std::move(forced_expr_prev), type, std::move(forced_expr_last));
    }

    if (token_rel_map.contains(operator_token)) {
        expr_ptr forced_expr_last;
        if (!force_expr(forced_expr_last)) {
            return std::nullopt;
        }
        expr_ptr forced_expr_prev;
        if (!force_expr(forced_expr_prev)) {
            return std::nullopt;
        }
        const auto type = token_rel_map.find(operator_token)->second;
        return rel_stmt(std::move(forced_expr_prev), type, std::move(forced_expr_last));
    }
    return std::nullopt;
}

class parser {
    flex_lexer_scanner& scanner;

    [[noreturn]] void throw_parse_error(std::string msg) const {
        const auto& loc = scanner.current_loc();
        throw parse_error{"Parse error at " + loc.file_name + ":" + std::to_string(loc.line) + ":" + std::to_string(loc.col) + ": " + std::move(msg)};
    }

    [[noreturn]] void throw_unexpected_token_error() const {
        throw_parse_error("unexpected token.");
    }

    void parse_var_decl(module& mod, bool is_exported) {
        auto [id_tok, identifier] = scanner.consume_token();
        if (id_tok != tok_identifier) {
            throw_parse_error("Expected variable name (an identifier).");
        }
        mod.add_variable_decl(module::var_decl{
                .loc = scanner.current_loc(),
                .exported = is_exported,
                .var = var(identifier),
        });
    }

    std::string parse_stmt_name() {
        auto [tok, literal] = scanner.consume_token();
        if (tok != tok_string_literal) {
            throw_parse_error("Expected statement name as a string literal");
        }
        auto str_literal = std::string(literal + 1);
        str_literal.erase(str_literal.size() - 1, 1);
        return str_literal;
    }

    variable_ptr lookup_var(const scope& enclosing_scope, const char* text) {
        variable_ptr var;
        try {
            var = enclosing_scope.get_var(symbol_view{text});
        } catch (var_not_found&) {
            throw_parse_error("Unknown variable '" + std::string(text) + "'.");
        }
        return var;
    }

    statement_ptr parse_stmt(const scope& enclosing_scope, bool allow_extend) {
        std::vector<std::variant<variable_ptr, expr_ptr, statement_ptr>> partials;
        std::vector<int> operators;
        const auto can_finish = [&] {
            // TODO: Take into account emptying the operators stack.
            return operators.empty() &&
                   partials.size() == 1 &&
                   (holds_alternative<statement_ptr>(partials[0]) || holds_alternative<variable_ptr>(partials[0]));
        };
        const auto reduce_last_operator = [&] {
            if (operators.empty()) {
                throw_unexpected_token_error();
            }
            auto result = reduce_operator(operators.back(), [&] {
                opt_partial_statement previous;
                if (partials.empty()) {
                    return previous;
                }
                previous = std::move(partials.back());
                partials.pop_back();
                return previous;
            });
            operators.pop_back();
            if (!result.has_value()) {
                throw_unexpected_token_error();
            }
            partials.push_back(std::move(result).value());
        };
        const auto finish = [&] {
            while (!operators.empty()) {
                reduce_last_operator();
            }
            if (!can_finish()) {
                throw_unexpected_token_error();
            }
            if (holds_alternative<variable_ptr>(partials[0])) {
                return var_stmt(get<variable_ptr>(std::move(partials[0])));
            }
            return get<statement_ptr>(std::move(partials[0]));
        };
        while (true) {
            if (!allow_extend && can_finish()) {
                return finish();
            }
            auto [token, text] = scanner.consume_token();
            if (is_keyword_token(token)) {
                scanner.unconsume_last_token();
                return finish();
            }
            switch (token) {
                case tok_identifier: {
                    partials.emplace_back(lookup_var(enclosing_scope, text));
                    break;
                }
                case tok_truth: {
                    partials.emplace_back(truth());
                    break;
                }
                case tok_contradiction: {
                    partials.emplace_back(contradiction());
                    break;
                }
                case tok_open_paren: {
                    operators.push_back(token);
                    break;
                }
                case tok_close_paren: {
                    while (!operators.empty() && operators.back() != tok_open_paren) {
                        reduce_last_operator();
                    }
                    if (operators.empty()) {
                        throw_unexpected_token_error();
                    }
                    operators.pop_back();  // Remove the open paren.
                    break;
                }
                case tok_forall: {
                    const auto [forall_var_token, forall_var_name] = scanner.consume_token();
                    if (forall_var_token != tok_identifier) {
                        throw_parse_error("Expected variable name (an identifier).");
                    }
                    scope forall_scope(&enclosing_scope);
                    auto forall_var = var(forall_var_name);
                    forall_scope.add_var(forall_var);
                    auto forall_stmt = parse_stmt(forall_scope, false);
                    partials.emplace_back(forall(std::move(forall_var), std::move(forall_stmt)));
                    break;
                }
                default: {
                    const auto token_priority_it = token_priority_map.find(token);
                    if (token_priority_it == token_priority_map.end()) {
                        throw_unexpected_token_error();
                    }
                    int token_priority = token_priority_it->second;
                    while (!operators.empty() && token_priority_map.find(operators.back())->second < token_priority) {
                        reduce_last_operator();
                    }
                    operators.push_back(token);
                    break;
                }
            }
        }
    }

    std::optional<scope> parse_proof(const module&) {
        auto [tok, text] = scanner.consume_token();
        if (tok != tok_proof) {
            throw_parse_error("Expected statement proof. Use the 'proof missing' keyword if you plan to add the proof later.");
        }
        const auto [tok2, text2] = scanner.consume_token();
        if (tok2 == tok_missing) {
            return std::nullopt;
        }
        // TODO: Set up a syntax for proofs and parse it here.
        throw_parse_error("Statement proofs are not currently supported. Use the 'proof missing' keyword.");
    }

    void parse_stmt_decl(module& mod, bool is_exported, module::stmt_decl_type stmt_type) {
        auto decl_loc = scanner.current_loc();
        auto stmt_name = parse_stmt_name();
        auto stmt = parse_stmt(mod.get_internal_scope(), true);
        std::optional<scope> proof;
        if (stmt_type != module::stmt_decl_type::definition) {
            proof = parse_proof(mod);
        }

        mod.add_statement_decl(module::stmt_decl{
                .loc = std::move(decl_loc),
                .exported = is_exported || (stmt_type == module::stmt_decl_type::theorem),
                .type = stmt_type,
                .name = std::move(stmt_name),
                .stmt = std::move(stmt),
                .proof_description = std::move(proof),
        });
    }

    bool parse_decl(module& mod) {
        bool is_exported = false;
        auto tok = scanner.consume_token().first;
        if (tok == tok_export) {
            is_exported = true;
            tok = scanner.consume_token().first;
        }
        if (tok == 0) {
            if (is_exported) {
                throw_parse_error("Unexpected 'export' with no declaration.");
            }
            return false;
        }
        switch (tok) {
            case tok_var: {
                parse_var_decl(mod, is_exported);
                break;
            }
            case tok_definition:
                parse_stmt_decl(mod, is_exported, module::stmt_decl_type::definition);
                break;
            case tok_theorem:
                parse_stmt_decl(mod, is_exported, module::stmt_decl_type::theorem);
                break;
            case tok_exercise:
                parse_stmt_decl(mod, is_exported, module::stmt_decl_type::exercise);
                break;
            default:
                throw_parse_error("Expected variable or statement declaration.");
        }
        return true;
    }

public:
    explicit parser(flex_lexer_scanner& scanner)
        : scanner(scanner) {}

    module parse_module() {
        module mod(scanner.current_loc().file_name);
        while (parse_decl(mod)) {
        }
        return mod;
    }
};

module parse_module(std::istream& in, std::string file_name) {
    flex_lexer_scanner scanner(in, std::move(file_name));
    return parser(scanner).parse_module();
}

module parse_module_from_string(std::string_view code) {
    std::stringstream string_stream;
    // TODO: This is quite inefficient, why do we need to copy the data / allocate? We should just
    //  be able to read from the string_view directly.
    string_stream.write(code.data(), static_cast<std::streamsize>(code.size()));
    return parse_module(string_stream, "<anonymous module>");
}

module parse_module_from_file(std::string file_name) {
    std::ifstream file_stream(file_name);
    if (file_stream.fail()) {
        throw parse_error{"Could not open file '" + file_name + "'."};
    }
    return parse_module(file_stream, std::move(file_name));
}

}  // namespace tema
