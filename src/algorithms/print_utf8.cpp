#include "print_utf8.h"

#include <sstream>

namespace tema {

std::string_view to_utf8(rel_type rel) noexcept {
    static constexpr std::string_view table[] = {
            "=",// eq = 0
            "≠",// n_eq = 1
            "<",// less = 2
            "≮",// n_less = 3
            "≤",// eq_less = 4
            "≰",// n_eq_less = 5
            ">",// greater = 6
            "≯",// n_greater = 7
            "≥",// eq_greater = 8
            "≱",// n_eq_greater = 9
            "∈",// in = 10
            "∉",// n_in = 11
            "⊃",// includes = 12
            "⊅",// n_includes = 13
            "⊇",// eq_includes = 14
            "⊉",// n_eq_includes = 15
            "⊂",// is_included = 16
            "⊄",// n_is_included = 17
            "⊆",// eq_is_included = 18
            "⊈",// n_eq_is_included = 19
    };
    return table[static_cast<std::underlying_type_t<rel_type>>(rel)];
}

struct print_utf8_expression_visitor {
    std::ostream& to;

    explicit print_utf8_expression_visitor(std::ostream& to)
        : to{to} {}

    void operator()(const variable_ptr& var) {
        to << var->name;
    }
};

struct print_utf8_statement_visitor {
    std::ostream& to;

    explicit print_utf8_statement_visitor(std::ostream& to)
        : to{to} {}

    void operator()(const statement::truth&) {
        to << "⊤";
    }
    void operator()(const statement::contradiction&) {
        to << "⊥";
    }
    void operator()(const statement::implies& expr) {
        visit_sub_statement(*expr.from);
        to << "→";
        visit_sub_statement(*expr.to);
    }
    void operator()(const statement::equiv& expr) {
        visit_sub_statement(*expr.left);
        to << "⟷";
        visit_sub_statement(*expr.right);
    }
    void operator()(const statement::neg& expr) {
        to << "¬";
        visit_sub_statement(*expr.inner);
    }
    void operator()(const statement::conj& expr) {
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
    void operator()(const statement::disj& expr) {
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
    void operator()(const statement::forall& expr) {
        to << "∀" << expr.var->name << " ";
        visit_sub_statement(*expr.inner);
    }
    void operator()(const variable_ptr& var) {
        to << var->name;
    }
    void operator()(const relationship& rel) const {
        rel.left->accept(print_utf8_expression_visitor{to});
        to << to_utf8(rel.type);
        rel.right->accept(print_utf8_expression_visitor{to});
    }

    void visit_sub_statement(const statement& expr) {
        if (expr.is_var() || expr.is_neg() || expr.is_truth() || expr.is_contradiction()) {
            expr.accept(*this);
        } else {
            to << "(";
            expr.accept(*this);
            to << ")";
        }
    }
};

void print_utf8_to(const statement* statement, std::ostream& to) {
    statement->accept(print_utf8_statement_visitor{to});
}

std::string print_utf8(const statement* statement) {
    std::stringstream sout;
    print_utf8_to(statement, sout);
    return std::move(sout).str();
}

}// namespace tema
