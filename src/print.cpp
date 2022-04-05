#include "print.h"

#include <sstream>

namespace tema {

namespace {

struct print_visitor {
    std::ostream& to;

    explicit print_visitor(std::ostream& to)
        : to{to} {}

    void operator()(const statement::truth&) {
        to << "T";
    }
    void operator()(const statement::contradiction&) {
        to << "F";
    }
    void operator()(const variable_ptr& var) {
        to << var->name;
    }
    void operator()(const statement::forall& expr) {
        to << "\\/" << expr.var->name << " ";
        if (expr.inner->is_forall() || expr.inner->is_exists()) {
            expr.inner->accept(*this);
        } else {
            visit_sub_statement(*expr.inner);
        }
    }
    void operator()(const statement::exists& expr) {
        to << "E" << expr.var->name << " ";
        if (expr.inner->is_forall() || expr.inner->is_exists()) {
            expr.inner->accept(*this);
        } else {
            visit_sub_statement(*expr.inner);
        }
    }
    void operator()(const statement::implies& expr) {
        visit_sub_statement(*expr.from);
        to << "->";
        visit_sub_statement(*expr.to);
    }
    void operator()(const statement::equiv& expr) {
        visit_sub_statement(*expr.left);
        to << "<->";
        visit_sub_statement(*expr.right);
    }
    void operator()(const statement::neg& expr) {
        to << "~";
        visit_sub_statement(*expr.inner);
    }
    void operator()(const statement::conj& expr) {
        bool first = true;
        for (const auto& term: expr.inner) {
            if (!first) {
                to << "&";
            } else {
                first = false;
            }
            visit_sub_statement(*term);
        }
    }
    void operator()(const statement::disj& expr) {
        bool first = true;
        for (const auto& term: expr.inner) {
            if (!first) {
                to << "|";
            } else {
                first = false;
            }
            visit_sub_statement(*term);
        }
    }

private:
    void visit_sub_statement(const statement& expr) {
        if (expr.is_var() || expr.is_neg()) {
            expr.accept(*this);
        } else {
            to << "(";
            expr.accept(*this);
            to << ")";
        }
    }
};

}// namespace

void print_ascii_to(const statement* statement, std::ostream& to) {
    statement->accept(print_visitor{to});
}

std::string print_ascii(const statement* statement) {
    std::stringstream sout;
    print_ascii_to(statement, sout);
    return std::move(sout).str();
}

}// namespace tema
