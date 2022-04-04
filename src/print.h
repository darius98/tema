#pragma once

#include <ostream>

#include "statement.h"

namespace tema {

void print_ascii_to(const statement* statement, std::ostream& to);
std::string print_ascii(const statement* statement);

}
