#pragma once

#include <istream>
#include <string>
#include <stdexcept>

#include "core/module.h"

namespace tema {

struct parse_error: std::runtime_error {
    using std::runtime_error::runtime_error;
};

[[nodiscard]] module parse_module_stream(std::istream& stream, std::string file_name);
[[nodiscard]] module parse_module_code(std::string_view code);
[[nodiscard]] module parse_module_file(std::string file_name);

}  // namespace tema
