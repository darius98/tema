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
        to << "\\/" << expr.var->name << "(";
        expr.inner->accept(*this);
        to << ")";
    }
    void operator()(const statement::exists& expr) {
        to << "E" << expr.var->name << "(";
        expr.inner->accept(*this);
        to << ")";
    }
    void operator()(const statement::implies& expr) {
        to << "(";
        expr.from->accept(*this);
        to << ")->(";
        expr.to->accept(*this);
        to << ")";
    }
    void operator()(const statement::equiv& expr) {
        to << "(";
        expr.left->accept(*this);
        to << ")<->(";
        expr.right->accept(*this);
        to << ")";
    }
    void operator()(const statement::neg& expr) {
        to << "~(";
        expr.inner->accept(*this);
        to << ")";
    }
    void operator()(const statement::conj& expr) {
        bool first = true;
        for (const auto& term: expr.inner) {
            if (!first) {
                to << "&";
            } else {
                first = false;
            }
            to << "(";
            term->accept(*this);
            to << ")";
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
            to << "(";
            term->accept(*this);
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
