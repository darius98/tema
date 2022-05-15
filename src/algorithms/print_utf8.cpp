#include "algorithms/print_utf8.h"

#include <sstream>

namespace tema {

std::string_view to_utf8(rel_type type) noexcept {
    static constexpr std::string_view table[] = {
            "=",  // eq = 0
            "≠",  // n_eq = 1
            "<",  // less = 2
            "≮",  // n_less = 3
            "≤",  // eq_less = 4
            "≰",  // n_eq_less = 5
            ">",  // greater = 6
            "≯",  // n_greater = 7
            "≥",  // eq_greater = 8
            "≱",  // n_eq_greater = 9
            "∈",  // in = 10
            "∉",  // n_in = 11
            "⊃",  // includes = 12
            "⊅",  // n_includes = 13
            "⊇",  // eq_includes = 14
            "⊉",  // n_eq_includes = 15
            "⊂",  // is_included = 16
            "⊄",  // n_is_included = 17
            "⊆",  // eq_is_included = 18
            "⊈",  // n_eq_is_included = 19
    };
    return table[static_cast<std::underlying_type_t<rel_type>>(type)];
}

std::string_view to_utf8(binop_type type) noexcept {
    static constexpr std::string_view table[] = {
            "∪",   // set_union = 0,
            "∩",   // set_intersection = 1,
            "\\",  // set_difference = 2,
            "⊖",   // set_sym_difference = 3,
    };
    return table[static_cast<std::underlying_type_t<binop_type>>(type)];
}

struct print_utf8_visitor {
    std::ostream& to;

    explicit print_utf8_visitor(std::ostream& to)
        : to{to} {}

    void operator()(const statement::truth&) const {
        to << "⊤";
    }
    void operator()(const statement::contradiction&) const {
        to << "⊥";
    }
    void operator()(const statement::implies& expr) const {
        visit_sub_statement(*expr.from);
        to << "→";
        visit_sub_statement(*expr.to);
    }
    void operator()(const statement::equiv& expr) const {
        visit_sub_statement(*expr.left);
        to << "⟷";
        visit_sub_statement(*expr.right);
    }
    void operator()(const statement::neg& expr) const {
        to << "¬";
        visit_sub_statement(*expr.inner);
    }
    void operator()(const statement::conj& expr) const {
        bool first = true;
        std::for_each(expr.inner.begin(), expr.inner.end(), [&](const statement_ptr& term) {
            if (!first) {
                to << "∧";
            } else {
                first = false;
            }
            visit_sub_statement(*term);
        });
    }
    void operator()(const statement::disj& expr) const {
        bool first = true;
        std::for_each(expr.inner.begin(), expr.inner.end(), [&](const statement_ptr& term) {
            if (!first) {
                to << "∨";
            } else {
                first = false;
            }
            visit_sub_statement(*term);
        });
    }
    void operator()(const statement::forall& expr) const {
        to << "∀" << expr.var->name << " ";
        visit_sub_statement(*expr.inner);
    }
    void operator()(const statement::var_stmt& var) const {
        to << var.var->name;
    }
    void operator()(const relationship& rel) const {
        visit_sub_expr(*rel.left);
        to << to_utf8(rel.type);
        visit_sub_expr(*rel.right);
    }
    void operator()(const variable_ptr& var) const {
        to << var->name;
    }
    void operator()(const expression::binop& binop) const {
        visit_sub_expr(*binop.left);
        to << to_utf8(binop.type);
        visit_sub_expr(*binop.right);
    }

    void visit_sub_expr(const expression& expr) const {
        if (expr.is_var()) {
            expr.accept(*this);
        } else {
            to << "(";
            expr.accept(*this);
            to << ")";
        }
    }

    void visit_sub_statement(const statement& stmt) const {
        if (stmt.is_var() || stmt.is_neg() || stmt.is_truth() || stmt.is_contradiction()) {
            stmt.accept(*this);
        } else {
            to << "(";
            stmt.accept(*this);
            to << ")";
        }
    }
};

void print_utf8_to(const expression& expr, std::ostream& to) {
    expr.accept(print_utf8_visitor{to});
}

std::string print_utf8(const expression& expr) {
    std::stringstream sout;
    print_utf8_to(expr, sout);
    return std::move(sout).str();
}

void print_utf8_to(const statement& stmt, std::ostream& to) {
    stmt.accept(print_utf8_visitor{to});
}

std::string print_utf8(const statement& stmt) {
    std::stringstream sout;
    print_utf8_to(stmt, sout);
    return std::move(sout).str();
}

}  // namespace tema
