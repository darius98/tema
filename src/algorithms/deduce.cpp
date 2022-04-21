#include "deduce.h"

#include "match.h"

namespace tema {

// TODO: Optimize for top-level conjunctions (common to have laws of the form A & B & C -> D).

std::optional<apply_vars_result> deduce(const statement* law, const statement* application) {
    if (law->is_implies()) {
        const auto match_result = match(law->as_implies().from.get(), application);
        if (match_result.has_value()) {
            // TODO: After implementing apply_vars for relationships, change this back!
            return apply_vars(law->as_implies().to.get(), match_result.value().stmt_replacements);
        }
    }
    if (law->is_equiv()) {
        const auto match_result = match(law->as_equiv().left.get(), application);
        if (match_result.has_value()) {
            // TODO: After implementing apply_vars for relationships, change this back!
            return apply_vars(law->as_equiv().right.get(), match_result.value().stmt_replacements);
        }

        const auto rev_match_result = match(law->as_equiv().right.get(), application);
        if (rev_match_result.has_value()) {
            // TODO: After implementing apply_vars for relationships, change this back!
            return apply_vars(law->as_equiv().left.get(), rev_match_result.value().stmt_replacements);
        }
    }
    return std::nullopt;
}

}// namespace tema
