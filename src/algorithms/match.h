#pragma once

#include "statement.h"

#include <map>
#include <optional>

namespace tema {

struct match_result {
    std::map<variable_ptr, statement_ptr> stmt_replacements;
    std::map<variable_ptr, expr_ptr> expr_replacements;
};

[[nodiscard]] std::optional<match_result> match(const expression* law, const expression* application);
[[nodiscard]] std::optional<match_result> match(const statement* law, const statement* application);

}// namespace tema
