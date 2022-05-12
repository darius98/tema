#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "compiler/print_cxx.h"
#include "config.h"
#include "core/module.h"

namespace tema {

constexpr std::string_view get_compiled_module_extension() {
    if constexpr (is_apple()) {
        return ".tema.dylib";
    } else {
        return ".tema.so";
    }
}

struct compile_options {
    // Compile using "-O0 -g", warning flags, address and UB sanitizers. If false, will use "-O3".
    bool debug{false};

    // "-" goes to stdout.
    // "" goes to <input_file_without_extensions>.tema.(dylib|so) if <input_file> is a file, or stdout if input is stdin.
    std::filesystem::path output_file{};

    // Provide the path to a custom compiler. If empty, falls back to the CXX environment variable. If empty, falls back to "cc".
    std::filesystem::path cxx_compiler_path{};

    std::vector<std::string> extra_flags{};
};

std::pair<std::filesystem::path, std::vector<std::string>> get_compilation_command(const std::filesystem::path& cxx_file, compile_options options = {});
std::filesystem::path compile_module(const std::filesystem::path& cxx_file, compile_options options = {});

}  // namespace tema
