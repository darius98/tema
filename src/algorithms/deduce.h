#pragma once

#include <optional>

#include "algorithms/apply_vars.h"
#include "statement.h"

namespace tema {

[[nodiscard]] std::optional<apply_vars_result> deduce(const statement* law, const statement* application);

}// namespace tema
