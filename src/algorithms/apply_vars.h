#pragma once

#include <map>
#include <set>

#include "statement.h"
#include "variable.h"

namespace tema {

struct apply_vars_result {
    statement_ptr stmt;
    std::set<variable_ptr> unmatched_vars;
};

[[nodiscard]] apply_vars_result apply_vars(const statement* law, const std::map<variable_ptr, statement_ptr>& replacements);

}// namespace tema
