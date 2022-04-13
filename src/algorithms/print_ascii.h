#pragma once

#include "statement.h"

#include <ostream>

namespace tema {

void print_ascii_to(const statement* statement, std::ostream& to);
[[nodiscard]] auto print_ascii(const statement* statement) -> std::string;

}// namespace tema
