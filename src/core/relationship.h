#pragma once

#include <string>

#include "core/expression.h"

namespace tema {

enum class rel_type {
    // Equality
    eq = 0,
    n_eq = 1,
    // Ordering
    less = 2,
    n_less = 3,
    eq_less = 4,
    n_eq_less = 5,
    greater = 6,
    n_greater = 7,
    eq_greater = 8,
    n_eq_greater = 9,
    // Set theory
    in = 10,
    n_in = 11,
    includes = 12,
    n_includes = 13,
    eq_includes = 14,
    n_eq_includes = 15,
    is_included = 16,
    n_is_included = 17,
    eq_is_included = 18,
    n_eq_is_included = 19,
};

struct relationship {
    rel_type type;
    expr_ptr left;
    expr_ptr right;
};

[[nodiscard]] std::string_view to_utf8(rel_type rel) noexcept;

}// namespace tema
