#include "algorithms.h"

#include "mcga/test_ext/matchers.hpp"

using namespace tema;
using namespace mcga::matchers;

TEST("algorithms.print_ascii") {
    const auto t = truth();
    expect(print_ascii(t.get()), "T");

    const auto f = contradiction();
    expect(print_ascii(f.get()), "F");

    const auto p = var("p");
    const auto p_stmt = var_stmt(p);
    expect(print_ascii(p_stmt.get()), "p");

    const auto f1 = disj(p_stmt, neg(p_stmt));
    expect(print_ascii(f1.get()), "p|~p");

    const auto q = var("q");

    const auto f2 = implies(p_stmt, var_stmt(q));
    expect(print_ascii(f2.get()), "p->q");

    const auto f3 = neg(conj(p_stmt, neg(p_stmt)));
    expect(print_ascii(f3.get()), "~(p&~p)");

    const auto f4 = equiv(p_stmt, neg(neg(p_stmt)));
    expect(print_ascii(f4.get()), "p<->~~p");

    const auto f5 = equiv(p_stmt, var_stmt(q));
    expect(print_ascii(f5.get()), "p<->q");
}

TEST("algorithms.print_ascii_to") {
    const auto p = var("p");
    const auto q = var("q");
    const auto r = var("r");
    const auto expr = equiv(
            implies(var_stmt(p), implies(var_stmt(q), var_stmt(r))),
            implies(var_stmt(q), implies(var_stmt(p), var_stmt(r))));
    std::stringstream sout;
    print_ascii_to(expr.get(), sout);
    expect(!sout.fail());
    expect(!sout.bad());
    expect(sout.str(), "(p->(q->r))<->(q->(p->r))");
}
