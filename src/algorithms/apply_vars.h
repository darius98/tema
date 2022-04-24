#pragma once

#include <map>
#include <set>

#include "algorithms/match.h"
#include "core/statement.h"
#include "core/variable.h"

namespace tema {

struct apply_vars_result {
    statement_ptr stmt;
    std::set<variable_ptr> unmatched_vars;
};

[[nodiscard]] apply_vars_result apply_vars(const statement_ptr& law, const match_result& replacements);

}// namespace tema
