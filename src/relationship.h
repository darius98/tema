#pragma once

#include "expression.h"

namespace tema {

enum class rel_type {
    // Equality / Inequality
    eq,
    n_eq,
    less,
    n_less,
    eq_less,
    n_eq_less,
    greater,
    n_greater,
    eq_greater,
    n_eq_greater,
    // Set theory
    in,
    n_in,
    includes,
    n_includes,
    eq_includes,
    n_eq_includes,
    is_included,
    n_is_included,
    eq_is_included,
    n_eq_is_included,
};

struct relationship {
    rel_type rel;
    expr_ptr left;
    expr_ptr right;
};

}// namespace tema
