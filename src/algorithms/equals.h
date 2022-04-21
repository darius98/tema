#pragma once

#include "statement.h"

namespace tema {

[[nodiscard]] bool equals(const expression* a, const expression* b);
[[nodiscard]] bool equals(const statement* a, const statement* b);

}// namespace tema
