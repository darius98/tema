#pragma once

#include <filesystem>
#include <ostream>
#include <string>

#include "core/module.h"

namespace tema {

struct print_cxx_options {
    // Not used atm.
    bool debug{false};

    // Include the proofs in the generated module.
    bool include_proofs{false};
};

void print_cxx_to(const module& mod, std::ostream& to, const print_cxx_options& options = {});

// If output_file is "-", output goes to stdout.
void print_cxx_to(const module& mod, const std::filesystem::path& output_file, const print_cxx_options& options = {});

// Print the C++ code for the tema module to a string.
[[nodiscard]] std::string print_cxx(const module& mod, const print_cxx_options& options = {});

// Parse "input_file" as a tema module, then print it in C++ code to "output_file".
// If output_file is empty, output goes to <input_file_without_extension>.tema.cc
std::filesystem::path translate_module(const std::filesystem::path& input_file,
                                       const std::filesystem::path& output_file = "",
                                       const print_cxx_options& options = {});

}  // namespace tema
