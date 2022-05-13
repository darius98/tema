#include "compiler/print_cxx.h"

#include <sstream>

namespace tema {

// TODO: Loading a module is currently barely twice as fast. Implement the optimizations below.
// TODO: Generate a direct constructor to tema::module, with the pre-built scopes & decl vector.
// TODO: consolidate statements that are equal and store in intermediate variables.
// TODO: consolidate non-nested forall variables from across the whole module.
// TODO: batch allocate variables, statements and expressions.

namespace {

std::string_view to_cxx(rel_type type) noexcept {
    static constexpr std::string_view table[] = {
            "eq",                // eq = 0
            "n_eq",              // n_eq = 1
            "less",              // less = 2
            "n_less",            // n_less = 3
            "eq_less",           // eq_less = 4
            "n_eq_less",         // n_eq_less = 5
            "greater",           // greater = 6
            "n_greater",         // n_greater = 7
            "eq_greater",        // eq_greater = 8
            "n_eq_greater",      // n_eq_greater = 9
            "in",                // in = 10
            "n_in",              // n_in = 11
            "includes",          // includes = 12
            "n_includes",        // n_includes = 13
            "eq_includes",       // eq_includes = 14
            "n_eq_includes",     // n_eq_includes = 15
            "is_included",       // is_included = 16
            "n_is_included",     // n_is_included = 17
            "eq_is_included",    // eq_is_included = 18
            "n_eq_is_included",  // n_eq_is_included = 19
    };
    return table[static_cast<std::underlying_type_t<rel_type>>(type)];
}

std::string_view to_cxx(binop_type type) noexcept {
    static constexpr std::string_view table[] = {
            "set_union",           // set_union = 0,
            "set_intersection",    // set_intersection = 1,
            "set_difference",      // set_difference = 2,
            "set_sym_difference",  // set_sym_difference = 3,
    };
    return table[static_cast<std::underlying_type_t<binop_type>>(type)];
}

std::string_view to_cxx(module::stmt_decl_type type) noexcept {
    static constexpr std::string_view table[] = {
            "definition",
            "theorem",
            "exercise",
    };
    return table[static_cast<std::underlying_type_t<module::stmt_decl_type>>(type)];
}

struct print_cxx_expression_visitor {
    const std::map<variable_ptr, std::string>& var_names;
    std::ostream& to;

    print_cxx_expression_visitor(const std::map<variable_ptr, std::string>& var_names, std::ostream& to)
        : var_names{var_names}, to{to} {}

    void operator()(const variable_ptr& var) const {
        to << "var_expr(" << var_names.find(var)->second << ")";
    }

    void operator()(const expression::binop& binop) const {
        to << "binop(";
        binop.left->accept(*this);
        to << ", binop_type::" << to_cxx(binop.type) << ", ";
        binop.right->accept(*this);
        to << ")";
    }
};

struct print_cxx_statement_visitor {
    const std::map<variable_ptr, std::string>& var_names;
    std::ostream& to;

    explicit print_cxx_statement_visitor(const std::map<variable_ptr, std::string>& var_names, std::ostream& to)
        : var_names{var_names}, to{to} {}

    void operator()(const statement::truth&) const {
        to << "truth()";
    }
    void operator()(const statement::contradiction&) const {
        to << "contradiction()";
    }
    void operator()(const statement::implies& expr) const {
        to << "implies(";
        expr.from->accept(*this);
        to << ", ";
        expr.to->accept(*this);
        to << ")";
    }
    void operator()(const statement::equiv& expr) const {
        to << "equiv(";
        expr.left->accept(*this);
        to << ", ";
        expr.right->accept(*this);
        to << ")";
    }
    void operator()(const statement::neg& expr) const {
        to << "neg(";
        expr.inner->accept(*this);
        to << ")";
    }
    void operator()(const statement::conj& expr) const {
        to << "conj({";
        bool first = true;
        std::for_each(expr.inner.begin(), expr.inner.end(), [&](const statement_ptr& term) {
            if (!first) {
                to << ", ";
            } else {
                first = false;
            }
            term->accept(*this);
        });
        to << "})";
    }
    void operator()(const statement::disj& expr) const {
        to << "disj({";
        bool first = true;
        std::for_each(expr.inner.begin(), expr.inner.end(), [&](const statement_ptr& term) {
            if (!first) {
                to << ", ";
            } else {
                first = false;
            }
            term->accept(*this);
        });
        to << "})";
    }
    void operator()(const statement::forall& expr) const {
        to << "forall(" << var_names.find(expr.var)->second << ", ";
        expr.inner->accept(*this);
        to << ")";
    }
    void operator()(const variable_ptr& var) const {
        to << "var_stmt(" << var_names.find(var)->second << ")";
    }
    void operator()(const relationship& rel) const {
        to << "rel_stmt(";
        rel.left->accept(print_cxx_expression_visitor{var_names, to});
        to << ", rel_type::" << to_cxx(rel.type) << ", ";
        rel.right->accept(print_cxx_expression_visitor{var_names, to});
        to << ")";
    }
};

struct var_name_discovery_visitor {
    int& idx;
    std::map<variable_ptr, std::string>& var_names;

    var_name_discovery_visitor(int& idx, std::map<variable_ptr, std::string>& var_names)
        : idx{idx}, var_names{var_names} {}

    void operator()(const statement::truth&) const {
    }
    void operator()(const statement::contradiction&) const {
    }
    void operator()(const statement::implies& expr) {
        expr.from->accept(*this);
        expr.to->accept(*this);
    }
    void operator()(const statement::equiv& expr) {
        expr.left->accept(*this);
        expr.right->accept(*this);
    }
    void operator()(const statement::neg& expr) {
        expr.inner->accept(*this);
    }
    void operator()(const statement::conj& expr) {
        std::for_each(expr.inner.begin(), expr.inner.end(), [&](const statement_ptr& term) {
            term->accept(*this);
        });
    }
    void operator()(const statement::disj& expr) {
        std::for_each(expr.inner.begin(), expr.inner.end(), [&](const statement_ptr& term) {
            term->accept(*this);
        });
    }
    void operator()(const statement::forall& expr) {
        add_var(expr.var);
        expr.inner->accept(*this);
    }
    void operator()(const relationship& rel) {
        rel.left->accept(*this);
        rel.right->accept(*this);
    }

    void operator()(const variable_ptr& var) {
        add_var(var);
    }

    void operator()(const expression::binop& binop) {
        binop.left->accept(*this);
        binop.right->accept(*this);
    }

    void add_var(const variable_ptr& var) {
        if (!var_names.contains(var)) {
            var_names.emplace(var, "v_" + std::to_string(idx));
            idx += 1;
        }
    }
};

std::map<variable_ptr, std::string> discover_variables(const module& mod) {
    int idx = 0;
    std::map<variable_ptr, std::string> var_names;
    var_name_discovery_visitor discovery_vis{idx, var_names};
    for (const auto& decl: mod.get_decls()) {
        if (holds_alternative<module::var_decl>(decl)) {
            auto var_name = "v_" + std::to_string(idx);
            var_names.emplace(get<module::var_decl>(decl).var, std::move(var_name));
            idx += 1;
        } else {
            get<module::stmt_decl>(decl).stmt->accept(discovery_vis);
        }
    }
    return var_names;
}

void print_module_header(const module&, std::ostream& to) {
    to << R"(#include "core/export.h"
#include "core/module.h"

using namespace tema;

)";
}

void print_module_vars(const std::map<variable_ptr, std::string>& vars, std::ostream& to) {
    for (const auto& [var, name]: vars) {
        to << "  const auto " << name << " = var(\"" << var->name << "\");\n";
    }
    to << "\n";
}

void print_proof(const scope&, std::ostream& to) {
    // TODO: Implement when adding proofs.
    to << "std::nullopt";
}

void print_module_decls(const module& mod, const std::map<variable_ptr, std::string>& vars, const print_cxx_options& options, std::ostream& to) {
    for (const auto& decl: mod.get_decls()) {
        if (holds_alternative<module::var_decl>(decl)) {
            const auto& var_decl = get<module::var_decl>(decl);
            to << "  mod.add_variable_decl(module::var_decl{\n"
                  "    .loc = {\""
               << var_decl.loc.file_name << "\", " << var_decl.loc.line << ", " << var_decl.loc.col << "},\n"
               << "    .exported = " << (var_decl.exported ? "true" : "false") << ",\n"
               << "    .var = " << vars.find(var_decl.var)->second << ",\n"
               << "  });\n";
        } else {
            const auto& stmt_decl = get<module::stmt_decl>(decl);
            to << "  mod.add_statement_decl(module::stmt_decl{\n"
                  "    .loc = {\""
               << stmt_decl.loc.file_name << "\", " << stmt_decl.loc.line << ", " << stmt_decl.loc.col << "},\n"
               << "    .exported = " << (stmt_decl.exported ? "true" : "false") << ",\n"
               << "    .type = module::stmt_decl_type::" << to_cxx(stmt_decl.type) << ",\n"
               << "    .name = \"" << stmt_decl.name << "\",\n"
               << "    .stmt = ";
            stmt_decl.stmt->accept(print_cxx_statement_visitor{vars, to});
            to << ",\n"
                  "    .proof_description = ";
            if (options.include_proofs) {
                if (stmt_decl.proof_description.has_value()) {
                    print_proof(stmt_decl.proof_description.value(), to);
                } else {
                    to << "std::nullopt";
                }
            } else {
                to << "std::nullopt";
            }
            to << ",\n  });\n";
        }
    }
}

void print_tema_module_function(const module& mod, std::ostream& to, const print_cxx_options& options) {
    to << R"(
::tema::module tema_module() asm("_tema_module");
TEMA_EXPORT ::tema::module tema_module() {
  ::tema::module mod{")"
       << mod.get_name() << R"("};
)";
    const auto var_names = discover_variables(mod);
    print_module_vars(var_names, to);
    print_module_decls(mod, var_names, options, to);
    to << R"(
  return mod;
})";
}

}  // namespace

void print_cxx_to(const module& mod, std::ostream& to, const print_cxx_options& options) {
    print_module_header(mod, to);
    print_tema_module_function(mod, to, options);
    to.flush();
}

std::string print_cxx(const module& mod, const print_cxx_options& options) {
    std::stringstream sout;
    print_cxx_to(mod, sout, options);
    return std::move(sout).str();
}

}  // namespace tema
