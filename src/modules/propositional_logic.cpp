#include "propositional_logic.h"

#include "scope.h"

namespace tema::modules {

// TODO: Have a way to write these in a text file. Write a minimal parser for statements
// TODO: Have a way to name things in a module.

static module pl_impl() {
    scope s;
    const auto p = var("p");
    const auto p_stmt = var_stmt(p);
    const auto q = var("q");
    const auto q_stmt = var_stmt(q);
    const auto r = var("r");
    const auto r_stmt = var_stmt(r);

    // law of excluded middle: \/p p|~p
    s.add_statement(forall(p, disj(p_stmt, neg(p_stmt))));
    // modus ponens: \/p\/q (p&(p->q))->q
    s.add_statement(forall(p, forall(q, implies(conj(p_stmt, implies(p_stmt, q_stmt)), q_stmt))));
    // affirming the conclusion: p -> (q -> p)
    s.add_statement(forall(p, forall(q, implies(p_stmt, implies(q_stmt, p_stmt)))));
    // contradiction: ~(p&~p)
    s.add_statement(forall(p, neg(conj(p_stmt, neg(p_stmt)))));
    // double negation: p<->~~p
    s.add_statement(forall(p, equiv(p_stmt, neg(neg(p_stmt)))));
    // counter position: (p->q)<->(~q->~p)
    s.add_statement(forall(p, forall(q, equiv(implies(p_stmt, q_stmt), implies(neg(q_stmt), neg(p_stmt))))));
    // negation of the premise: ~p->(p->q)
    s.add_statement(forall(p, forall(q, implies(neg(p_stmt), implies(p_stmt, q_stmt)))));
    // modus tollens: ((p->q)&~q)->~p
    s.add_statement(forall(p, forall(q, implies(conj(implies(p_stmt, q_stmt), neg(q_stmt)), neg(p_stmt)))));
    // transitivity of implication: ((p->q)&(q->r))->(p->r)
    s.add_statement(forall(p, forall(q, forall(r, implies(conj(implies(p_stmt, q_stmt), implies(q_stmt, r_stmt)), implies(p_stmt, r_stmt))))));
    // deMorgan Law I: (p|q)<->~(~p&~q)
    s.add_statement(forall(p, forall(q, equiv(disj(p_stmt, q_stmt), neg(conj(neg(p_stmt), neg(q_stmt)))))));
    // deMorgan Law II: (p&q)<->~(~p|~q)
    s.add_statement(forall(p, forall(q, equiv(conj(p_stmt, q_stmt), neg(disj(neg(p_stmt), neg(q_stmt)))))));
    // import and export: (p->(q->r))<->(p&q->r)
    s.add_statement(forall(p, forall(q, forall(r, equiv(implies(p_stmt, implies(q_stmt, r_stmt)), implies(conj(p_stmt, q_stmt), r_stmt))))));
    // idempotence: p <-> p&p <-> p|p
    s.add_statement(forall(p, equiv(p_stmt, conj(p_stmt, p_stmt))));
    s.add_statement(forall(p, equiv(p_stmt, disj(p_stmt, p_stmt))));
    // weakening: (p&q)->p  p->(p|q)
    s.add_statement(forall(p, forall(q, implies(conj(p_stmt, q_stmt), p_stmt))));
    s.add_statement(forall(p, forall(q, implies(p_stmt, disj(p_stmt, q_stmt)))));
    // commutativity: (p&q)<->(q&p) (p|q)<->(q|p)
    s.add_statement(forall(p, forall(q, equiv(conj(p_stmt, q_stmt), conj(q_stmt, p_stmt)))));
    s.add_statement(forall(p, forall(q, equiv(disj(p_stmt, q_stmt), disj(q_stmt, p_stmt)))));
    // associativity: ((p&q)&r)<->(p&(q&r)) ((p|q)|r)<->(p|(q|r))
    s.add_statement(forall(p, forall(q, forall(r, equiv(conj(conj(p_stmt, q_stmt), r_stmt), conj(p_stmt, conj(q_stmt, r_stmt)))))));
    s.add_statement(forall(p, forall(q, forall(r, equiv(disj(disj(p_stmt, q_stmt), r_stmt), disj(p_stmt, disj(q_stmt, r_stmt)))))));
    // absorption: p|(p&q)<->p p&(p|q)<->p
    s.add_statement(forall(p, forall(q, equiv(p_stmt, disj(p_stmt, conj(p_stmt, q_stmt))))));
    s.add_statement(forall(p, forall(q, equiv(p_stmt, conj(p_stmt, disj(p_stmt, q_stmt))))));
    // distributivity: p&(q|r)<->(p&q)|(p&r) p|(q&r)<->(p|q)&(p|r)
    s.add_statement(forall(p, forall(q, forall(r, equiv(conj(p_stmt, disj(q_stmt, r_stmt)), disj(conj(p_stmt, q_stmt), conj(p_stmt, r_stmt)))))));
    s.add_statement(forall(p, forall(q, forall(r, equiv(disj(p_stmt, conj(q_stmt, r_stmt)), conj(disj(p_stmt, q_stmt), disj(p_stmt, r_stmt)))))));
    return module{std::move(s)};
}

module pl() {
    static module pl = pl_impl();
    return pl;
}

}// namespace tema::modules
