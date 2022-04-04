#include "print.h"

#include <mcga/test_ext/matchers.hpp>

using namespace tema;
using namespace mcga::matchers;

TEST("print_ascii") {
    const auto t = truth();
    expect(print_ascii(t.get()), "T");

    const auto f = contradiction();
    expect(print_ascii(f.get()), "F");

    const auto x = var("X");
    const auto x_stmt = var_stmt(x);
    expect(print_ascii(x_stmt.get()), "X");

    const auto fx = forall(x, disj(x_stmt, neg(x_stmt)));
    expect(print_ascii(fx.get()), "\\/X((X)|(~(X)))");

    const auto y = var("Y");
    const auto fy = forall(x, exists(y, implies(x_stmt, var_stmt(y))));
    expect(print_ascii(fy.get()), "\\/X(EY((X)->(Y)))");

    const auto fz = neg(exists(x, conj(x_stmt, neg(x_stmt))));
    expect(print_ascii(fz.get()), "~(EX((X)&(~(X))))");

    const auto ft = forall(x, equiv(x_stmt, neg(neg(x_stmt))));
    expect(print_ascii(ft.get()), "\\/X((X)<->(~(~(X))))");
}

TEST("print_ascii_to") {
    const auto p = var("P");
    const auto q = var("Q");
    const auto r = var("R");
    const auto expr = equiv(
            implies(var_stmt(p), implies(var_stmt(q), var_stmt(r))),
            implies(var_stmt(q), implies(var_stmt(p), var_stmt(r))));
    std::stringstream sout;
    print_ascii_to(expr.get(), sout);
    expect(!sout.fail());
    expect(!sout.bad());
    expect(sout.str(), "((P)->((Q)->(R)))<->((Q)->((P)->(R)))");
}
