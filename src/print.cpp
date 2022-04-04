#include "print.h"

#include "statement_visitor.h"

#include <sstream>

namespace tema {

void print_ascii_to(const statement* statement, std::ostream& to) {
    statement->accept(statement_visitor{
            [&](const statement::truth&) { to << "T"; },
            [&](const statement::contradiction&) { to << "F"; },
            [&](const variable_ptr& var) { to << var->name; },
            [&](auto& visitor, const statement::forall& expr) {
                to << "\\/" << expr.var->name << "(";
                expr.inner->accept(visitor);
                to << ")";
            },
            [&](auto& visitor, const statement::exists& expr) {
                to << "E" << expr.var->name << "(";
                expr.inner->accept(visitor);
                to << ")";
            },
            [&](auto& visitor, const statement::implies& expr) {
                to << "(";
                expr.from->accept(visitor);
                to << ")->(";
                expr.to->accept(visitor);
                to << ")";
            },
            [&](auto& visitor, const statement::equiv& expr) {
                to << "(";
                expr.left->accept(visitor);
                to << ")<->(";
                expr.right->accept(visitor);
                to << ")";
            },
            [&](auto& visitor, const statement::neg& expr) {
                to << "~(";
                expr.inner->accept(visitor);
                to << ")";
            },
            [&](auto& visitor, const statement::conj& expr) {
                bool first = true;
                for (const auto& term: expr.inner) {
                    if (!first) {
                        to << "&";
                    } else {
                        first = false;
                    }
                    to << "(";
                    term->accept(visitor);
                    to << ")";
                }
            },
            [&](auto& visitor, const statement::disj& expr) {
                bool first = true;
                for (const auto& term: expr.inner) {
                    if (!first) {
                        to << "|";
                    } else {
                        first = false;
                    }
                    to << "(";
                    term->accept(visitor);
                    to << ")";
                }
            },
    });
}

std::string print_ascii(const statement* statement) {
    std::stringstream sout;
    print_ascii_to(statement, sout);
    return std::move(sout).str();
}

}// namespace tema
