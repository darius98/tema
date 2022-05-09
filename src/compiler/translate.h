#pragma once

#include <filesystem>

#include "compiler/print_cxx.h"

namespace tema {

// If output_file is "-", output goes to stdout.
// If output_file is "", goes to <input_file_without_extensions>.tema.cc if <input_file> is a file, or stdout if input is stdin.
std::filesystem::path translate_module(const std::filesystem::path& input_file,
                                       const std::filesystem::path& output_file = "",
                                       const print_cxx_options& options = {});

}  // namespace tema
