#include "propositional_logic.h"

#include "scope.h"

namespace tema::modules {

// TODO: Have a way to write these in a text file. Write a minimal parser for statements

static module pl_impl() {
    scope s;
    const auto p = var("p");
    const auto p_stmt = var_stmt(p);
    const auto q = var("q");
    const auto q_stmt = var_stmt(q);
    const auto r = var("r");
    const auto r_stmt = var_stmt(r);

    s.add_statement("Truth", truth());
    s.add_statement("Law of excluded middle", forall(p, disj(p_stmt, neg(p_stmt))));
    s.add_statement("Modus Ponens", forall(p, forall(q, implies(conj(p_stmt, implies(p_stmt, q_stmt)), q_stmt))));
    s.add_statement("Affirming the conclusion", forall(p, forall(q, implies(p_stmt, implies(q_stmt, p_stmt)))));
    s.add_statement("Contradiction", forall(p, neg(conj(p_stmt, neg(p_stmt)))));
    s.add_statement("Double negation", forall(p, equiv(p_stmt, neg(neg(p_stmt)))));
    s.add_statement("Counter position", forall(p, forall(q, equiv(implies(p_stmt, q_stmt), implies(neg(q_stmt), neg(p_stmt))))));
    s.add_statement("Negation of the premise", forall(p, forall(q, implies(neg(p_stmt), implies(p_stmt, q_stmt)))));
    s.add_statement("Modus Tollens", forall(p, forall(q, implies(conj(implies(p_stmt, q_stmt), neg(q_stmt)), neg(p_stmt)))));
    s.add_statement("Transitivity of implication", forall(p, forall(q, forall(r, implies(conj(implies(p_stmt, q_stmt), implies(q_stmt, r_stmt)), implies(p_stmt, r_stmt))))));
    s.add_statement("DeMorgan law I", forall(p, forall(q, equiv(disj(p_stmt, q_stmt), neg(conj(neg(p_stmt), neg(q_stmt)))))));
    s.add_statement("DeMorgan law II", forall(p, forall(q, equiv(conj(p_stmt, q_stmt), neg(disj(neg(p_stmt), neg(q_stmt)))))));
    s.add_statement("Import and Export", forall(p, forall(q, forall(r, equiv(implies(p_stmt, implies(q_stmt, r_stmt)), implies(conj(p_stmt, q_stmt), r_stmt))))));
    s.add_statement("Idempotence of conjunction", forall(p, equiv(p_stmt, conj(p_stmt, p_stmt))));
    s.add_statement("Idempotence of disjunction", forall(p, equiv(p_stmt, disj(p_stmt, p_stmt))));
    s.add_statement("Weakening of conjunction", forall(p, forall(q, implies(conj(p_stmt, q_stmt), p_stmt))));
    s.add_statement("Weakening of disjunction", forall(p, forall(q, implies(p_stmt, disj(p_stmt, q_stmt)))));
    s.add_statement("Commutativity of conjunction", forall(p, forall(q, equiv(conj(p_stmt, q_stmt), conj(q_stmt, p_stmt)))));
    s.add_statement("Commutativity of disjunction", forall(p, forall(q, equiv(disj(p_stmt, q_stmt), disj(q_stmt, p_stmt)))));
    s.add_statement("Associativity of conjunction", forall(p, forall(q, forall(r, equiv(conj(conj(p_stmt, q_stmt), r_stmt), conj(p_stmt, conj(q_stmt, r_stmt)))))));
    s.add_statement("Associativity of disjunction", forall(p, forall(q, forall(r, equiv(disj(disj(p_stmt, q_stmt), r_stmt), disj(p_stmt, disj(q_stmt, r_stmt)))))));
    s.add_statement("Absorption of conjunction", forall(p, forall(q, equiv(p_stmt, disj(p_stmt, conj(p_stmt, q_stmt))))));
    s.add_statement("Absorption of disjunction", forall(p, forall(q, equiv(p_stmt, conj(p_stmt, disj(p_stmt, q_stmt))))));
    s.add_statement("Distributivity of conjunction", forall(p, forall(q, forall(r, equiv(conj(p_stmt, disj(q_stmt, r_stmt)), disj(conj(p_stmt, q_stmt), conj(p_stmt, r_stmt)))))));
    s.add_statement("Distributivity of disjunction", forall(p, forall(q, forall(r, equiv(disj(p_stmt, conj(q_stmt, r_stmt)), conj(disj(p_stmt, q_stmt), disj(p_stmt, r_stmt)))))));
    return module{std::move(s)};
}

module pl() {
    static module pl = pl_impl();
    return pl;
}

}// namespace tema::modules
