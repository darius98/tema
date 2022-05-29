#include "algorithms/deduce.h"

#include "algorithms/match.h"

namespace tema {

// TODO: Optimize for top-level conjunctions (common to have laws of the form A & B & C -> D).

std::optional<statement_ptr> mp_deduce(const statement& law, const statement_ptr& application) {
    if (law.is_implies()) {
        const auto match_result = match(*law.as_implies().from, application);
        if (match_result.has_value()) {
            return apply_vars(law.as_implies().to, match_result.value()).stmt;
        }
    }
    if (law.is_equiv()) {
        const auto match_result = match(*law.as_equiv().left, application);
        if (match_result.has_value()) {
            return apply_vars(law.as_equiv().right, match_result.value()).stmt;
        }

        const auto rev_match_result = match(*law.as_equiv().right, application);
        if (rev_match_result.has_value()) {
            return apply_vars(law.as_equiv().left, rev_match_result.value()).stmt;
        }
    }
    return std::nullopt;
}

}  // namespace tema
