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
    // "" goes to <input_file_without_extensions>.tema.(dylib|so) if
    // <input_file> is a file, or stdout if input is stdin.
    std::filesystem::path output_file{};

    // Path to the file_location where tema is installed. Defaults to the value
    // provided by CMake at build time.
    std::filesystem::path install_path{default_install_path};

    // Path to the compiler to use. Defaults to the value provided by CMake
    // at build time (the compiler used to compile tema itself).
    std::filesystem::path cxx_compiler_path{default_cxx_compiler_path};

    // The sysroot to use with "-isysroot". Defaults to the value provided
    // by CMake at build time. Only used on macOS.
    std::filesystem::path apple_sysroot{default_apple_sysroot_path};

    std::vector<std::string> extra_flags{};
};

std::filesystem::path compile_module(const std::filesystem::path& cxx_file, const compile_options& options = {});

}  // namespace tema
