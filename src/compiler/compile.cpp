#include "compiler/compile.h"

#include <array>
#include <vector>

namespace tema {

namespace {

constexpr std::array common_compile_flags{
        "-shared",
        "-fPIC",
        "-fPIE",
        "-fvisibility=hidden",
        "-std=c++20",
        "-ltema_core",
};

constexpr std::array debug_compile_flags{
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

constexpr std::array release_compile_flags = std::array{
        "-O3",
};

const char* get_compiler_path(const char* cxx_option) {
    if (cxx_option != nullptr && cxx_option[0] != '\0') {
        return cxx_option;
    }
    const auto cxx_env = std::getenv("CXX");
    if (cxx_env != nullptr && cxx_env[0] != '\0') {
        return cxx_env;
    }
    return "cc";
}

consteval const char* get_compiled_module_extension() {
#ifdef TEMA_APPLE
    return ".tema.dylib";
#else
    return ".tema.so";
#endif
}

}  // namespace

std::filesystem::path compile_module(const std::filesystem::path& cxx_file, const compile_options& options) {
    std::filesystem::path output = options.output_file;
    if (output.empty()) {
        output = cxx_file;
        output.replace_extension(get_compiled_module_extension());
    }

    std::vector<const char*> compile_command{get_compiler_path(options.cxx_compiler_path.c_str())};
    compile_command.insert(compile_command.end(), common_compile_flags.begin(), common_compile_flags.end());
    if (options.debug) {
        compile_command.insert(compile_command.end(), debug_compile_flags.begin(), debug_compile_flags.end());
    } else {
        compile_command.insert(compile_command.end(), release_compile_flags.begin(), release_compile_flags.end());
    }
    if (!options.extra_flags.empty()) {
        compile_command.resize(compile_command.size() + options.extra_flags.size());
        for (const auto& flag: options.extra_flags) {
            compile_command.push_back(flag.c_str());
        }
    }
    compile_command.push_back("-o");
    compile_command.push_back(output.c_str());
    compile_command.push_back(cxx_file.c_str());
    // TODO: Invoke compile_command!
//    auto proc = mcga::proc::Subprocess::Fork();
    return output;
}

}  // namespace tema
