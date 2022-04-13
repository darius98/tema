#pragma once

#include "statement.h"

#include <map>
#include <optional>

namespace tema {

using match_result = std::map<variable_ptr, statement_ptr>;

[[nodiscard]] std::optional<match_result> match(const statement* law, const statement* application);

}// namespace tema
