#pragma once

#include "statement.h"

#include <ostream>

namespace tema {

void print_ascii_to(const statement* statement, std::ostream& to);
[[nodiscard]] std::string print_ascii(const statement* statement);

}// namespace tema
