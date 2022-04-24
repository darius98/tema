#pragma once

#include "core/statement.h"

#include <map>
#include <optional>

namespace tema {

struct match_result {
    std::map<variable_ptr, statement_ptr> stmt_replacements;
    std::map<variable_ptr, expr_ptr> expr_replacements;
};

[[nodiscard]] std::optional<match_result> match(const expression* law, expr_ptr application);
[[nodiscard]] std::optional<match_result> match(const statement* law, statement_ptr application);

}  // namespace tema
