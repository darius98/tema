#pragma once

#include <optional>

#include "algorithms/apply_vars.h"
#include "core/statement.h"

namespace tema {

// Try to deduce a statement from a law using modus ponens.
// For this to work, "law" must be of the form A->B or A<->B, and "application" must *match* A, thus deducing
// that B is true with the replacements from the *match* of "application" to A.
[[nodiscard]] std::optional<statement_ptr> mp_deduce(const statement& law, const statement_ptr& application);

}  // namespace tema
