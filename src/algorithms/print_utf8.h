#pragma once

#include "core/statement.h"

#include <ostream>

namespace tema {

void print_utf8_to(const expression& expr, std::ostream& to);
[[nodiscard]] std::string print_utf8(const expression& expr);

void print_utf8_to(const statement& stmt, std::ostream& to);
[[nodiscard]] std::string print_utf8(const statement& stmt);

}  // namespace tema
