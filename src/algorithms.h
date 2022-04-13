#pragma once

#include "statement.h"

#include <map>
#include <optional>
#include <ostream>
#include <set>

namespace tema {

[[nodiscard]] auto equals(const statement* a, const statement* b) -> bool;

using match_result = std::map<variable_ptr, statement_ptr>;

[[nodiscard]] auto match(const statement* law, const statement* application) -> std::optional<match_result>;

struct apply_vars_result {
    statement_ptr stmt;
    std::set<variable_ptr> unmatched_vars;
};

[[nodiscard]] auto apply_vars(const statement* law, const match_result& replacements) -> apply_vars_result;

[[nodiscard]] auto deduce(const statement* law, const statement* application) -> std::optional<apply_vars_result>;

void print_ascii_to(const statement* statement, std::ostream& to);
[[nodiscard]] auto print_ascii(const statement* statement) -> std::string;

}// namespace tema
