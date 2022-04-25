#pragma once

#include <istream>
#include <string>

#include "core/module.h"

namespace tema {

[[nodiscard]] module parse_module_from_stream(std::istream& stream);
[[nodiscard]] module parse_module_from_string(std::string_view data);
[[nodiscard]] module parse_module_from_file(std::string_view file_name);

}  // namespace tema
