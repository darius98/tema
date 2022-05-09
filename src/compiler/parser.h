#pragma once

#include <filesystem>
#include <istream>
#include <stdexcept>
#include <string>

#include "core/module.h"
#include "util/export.h"

namespace tema {

struct TEMA_EXPORT parse_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

[[nodiscard]] module parse_module(std::istream& stream, const std::filesystem::path& file_name);
[[nodiscard]] module parse_module(std::string_view code);

}  // namespace tema
