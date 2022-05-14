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

    // Provide the path to a custom compiler. Defaults to the value provided by CMake during build time.
    std::filesystem::path cxx_compiler_path{default_cxx_compiler_path};

    // The sysroot to use with "-isysroot". Defaults to the value provided by CMake during build time. Only used on macOS.
    std::filesystem::path apple_sysroot{default_apple_sysroot_path};

    std::vector<std::string> extra_flags{};
};

std::pair<std::filesystem::path, std::vector<std::string>> get_compilation_command(const std::filesystem::path& cxx_file, compile_options options = {});
std::filesystem::path compile_module(const std::filesystem::path& cxx_file, compile_options options = {});

}  // namespace tema
