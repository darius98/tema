#include "modules/propositional_logic.h"

#include "core/scope.h"

namespace tema::modules {

// TODO: Have a way to write these in a text file. Write a minimal parser for statements

static module pl_impl() {
    scope s;
    const auto p_var = var("p");
    const auto q_var = var("q");
    const auto r_var = var("r");

    const auto p = var_stmt(p_var);
    const auto q = var_stmt(q_var);
    const auto r = var_stmt(r_var);
    const auto not_p = neg(p);
    const auto not_q = neg(q);
    const auto p_implies_q = implies(p, q);
    const auto p_and_q = conj(p, q);
    const auto p_or_q = disj(p, q);

    s.add_statement("Truth", truth());
    s.add_statement("Law of excluded middle", disj(p, not_p));
    s.add_statement("Modus Ponens", implies(conj(p, p_implies_q), q));
    s.add_statement("Affirming the conclusion", implies(q, p_implies_q));
    s.add_statement("Contradiction", neg(conj(p, not_p)));
    s.add_statement("Double negation", equiv(neg(not_p), p));
    s.add_statement("Counter position", equiv(p_implies_q, implies(not_q, not_p)));
    s.add_statement("Negation of the premise", implies(not_p, p_implies_q));
    s.add_statement("Modus Tollens", implies(conj(p_implies_q, not_q), not_p));
    s.add_statement("Transitivity of implication", implies(conj(p_implies_q, implies(q, r)), implies(p, r)));
    s.add_statement("DeMorgan law I", equiv(p_or_q, neg(conj(not_p, not_q))));
    s.add_statement("DeMorgan law II", equiv(p_and_q, neg(disj(not_p, not_q))));
    s.add_statement("Import and Export", equiv(implies(p, implies(q, r)), implies(p_and_q, r)));
    s.add_statement("Idempotence of conjunction", equiv(conj(p, p), p));
    s.add_statement("Idempotence of disjunction", equiv(disj(p, p), p));
    s.add_statement("Weakening of conjunction", implies(p_and_q, p));
    s.add_statement("Weakening of disjunction", implies(p, forall(q_var, p_or_q)));
    s.add_statement("Commutativity of conjunction", equiv(p_and_q, conj(q, p)));
    s.add_statement("Commutativity of disjunction", equiv(p_or_q, disj(q, p)));
    s.add_statement("Associativity of conjunction", equiv(conj(p_and_q, r), conj(p, conj(q, r))));
    s.add_statement("Associativity of disjunction", equiv(disj(p_or_q, r), disj(p, disj(q, r))));
    s.add_statement("Absorption of conjunction", equiv(disj(p, p_and_q), p));
    s.add_statement("Absorption of disjunction", equiv(conj(p, p_or_q), p));
    s.add_statement("Distributivity of conjunction", equiv(conj(p, disj(q, r)), disj(p_and_q, conj(p, r))));
    s.add_statement("Distributivity of disjunction", equiv(disj(p, conj(q, r)), conj(p_or_q, disj(p, r))));
    return module{std::move(s)};
}

module pl() {
    static module pl = pl_impl();
    return pl;
}

}// namespace tema::modules
