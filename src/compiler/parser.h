#pragma once

#include <filesystem>
#include <istream>
#include <stdexcept>
#include <string>

#include "core/module.h"

namespace tema {

struct parse_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

[[nodiscard]] module parse_module(std::istream& stream, std::string file_name);
[[nodiscard]] module parse_module(std::string_view code);
[[nodiscard]] module parse_module(const std::filesystem::path& file_name);

}  // namespace tema
