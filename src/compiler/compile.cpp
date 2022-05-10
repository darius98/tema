#include "compiler/compile.h"

#include <array>
#include <vector>

#include "mcga/proc.hpp"

namespace tema {

namespace {

std::vector<std::string> get_debug_compile_flags() {
    return {
            "-g",
            "-O0",
            "-Werror",
            "-Wall",
            "-Wextra",
            "-Wnon-virtual-dtor",
            "-Wold-style-cast",
            "-Wcast-align",
            "-Wunused",
            "-Woverloaded-virtual",
            "-Wpedantic",
            "-Wconversion",
            "-Wsign-conversion",
            "-Wsign-compare",
            "-Wnull-dereference",
            "-Wdouble-promotion",
            "-Wformat=2",
            "-Wimplicit-fallthrough",
            "-fsanitize=address,undefined",
            "-fno-sanitize-recover=undefined",
    };
}

std::vector<std::string> get_release_compile_flags() {
    return {
            "-O3",
    };
}


std::vector<std::string> get_common_compile_flags() {
    return {
            "-shared",
            "-fPIC",
            "-fvisibility=hidden",
            "-std=c++20",
            "-o",  // This is the last one, to be followed by the output path.
    };
}

std::vector<std::string> get_apple_compile_flags() {
    return {
            "-undefined",
            "dynamic_lookup",
    };
}

auto str_data_getter(std::string& s) {
    return s.data();
}

char* get_compiler_path(char* cxx_option) {
    static char default_compiler[] = "/usr/bin/cc";
    if (cxx_option != nullptr && cxx_option[0] != '\0') {
        return cxx_option;
    }
    const auto cxx_env = std::getenv("CXX");
    if (cxx_env != nullptr && cxx_env[0] != '\0') {
        return cxx_env;
    }
    return default_compiler;
}

consteval const char* get_compiled_module_extension() {
#ifdef TEMA_APPLE
    return ".tema.dylib";
#else
    return ".tema.so";
#endif
}

}  // namespace

std::filesystem::path compile_module(const std::filesystem::path& cxx_file_path, compile_options options) {
    static auto debug_compile_flags = get_debug_compile_flags();
    static auto release_compile_flags = get_release_compile_flags();
    static auto common_compile_flags = get_common_compile_flags();
    static auto apple_compile_flags = get_apple_compile_flags();
    std::string cxx_file = cxx_file_path.string();
    if (options.output_file.empty()) {
        options.output_file = cxx_file_path;
        while (options.output_file.has_extension()) {
            options.output_file.replace_extension("");
        }
        options.output_file.replace_extension(get_compiled_module_extension());
    }
    std::string output = std::move(options.output_file).string();

    auto cxx_compiler_path = std::move(options.cxx_compiler_path).string();
    std::vector<char*> compile_command{get_compiler_path(cxx_compiler_path.data())};
    if (options.debug) {
        std::transform(debug_compile_flags.begin(), debug_compile_flags.end(), std::back_inserter(compile_command), str_data_getter);
    } else {
        std::transform(release_compile_flags.begin(), release_compile_flags.end(), std::back_inserter(compile_command), str_data_getter);
    }
    std::transform(common_compile_flags.begin(), common_compile_flags.end(), std::back_inserter(compile_command), str_data_getter);
    compile_command.push_back(output.data());
#ifdef TEMA_APPLE
    std::transform(apple_compile_flags.begin(), apple_compile_flags.end(), std::back_inserter(compile_command), str_data_getter);
#endif
    if (!options.extra_flags.empty()) {
        compile_command.reserve(compile_command.size() + options.extra_flags.size());
        for (auto& flag: options.extra_flags) {
            compile_command.push_back(flag.data());
        }
    }
    compile_command.push_back(cxx_file.data());
    compile_command.push_back(nullptr);
    auto proc = mcga::proc::Subprocess::Invoke(compile_command[0], compile_command.data());
    proc->waitBlocking();
    if (!proc->isExited() || proc->getReturnCode() != 0) {
        // TODO: Better error handling of fatal errors.
        std::abort();
    }
    return output;
}

}  // namespace tema
