#include "compiler/compile.h"

#include <array>
#include <iostream>
#include <vector>

#include <mcga/proc.hpp>

#include "config.h"

extern char **environ;

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
    };
}

std::vector<std::string> get_apple_compile_flags() {
    return {
            "-undefined",
            "dynamic_lookup",
    };
}

std::size_t concatenate_vectors_get_part_size(const mcga::meta::one_of<std::string, std::vector<std::string>> auto& part) {
    if constexpr (mcga::meta::cvref_same_as<decltype(part), std::string>) {
        return std::size_t(1);
    } else {
        return part.size();
    }
}

void concatenate_vectors_append(std::vector<std::string>& result, const mcga::meta::one_of<std::string, std::vector<std::string>> auto&& part) {
    if constexpr (mcga::meta::cvref_same_as<decltype(part), std::string>) {
        result.push_back(std::move(part));
    } else {
        for (auto it = part.begin(); it != part.end(); it++) {
            result.push_back(std::move(*it));
        }
    }
}

std::vector<std::string> concatenate_vectors(mcga::meta::one_of<std::string, std::vector<std::string>> auto&&... parts) {
    const auto size = (concatenate_vectors_get_part_size(parts) + ...);
    std::vector<std::string> result;
    result.reserve(size);
    (concatenate_vectors_append(result, std::move(parts)), ...);
    return result;
}

std::string get_compiler_path(std::string cxx_option) {
    if (!cxx_option.empty()) {
        return cxx_option;
    }
    const auto cxx_env = std::getenv("CXX");
    if (cxx_env != nullptr && cxx_env[0] != '\0') {
        return cxx_env;
    }
    return "/usr/bin/c++";
}

std::filesystem::path get_output_path(std::filesystem::path output_path, const std::filesystem::path& input_path) {
    if (output_path.empty()) {
        output_path = input_path;
        while (output_path.has_extension()) {
            output_path.replace_extension("");
        }
        output_path.replace_extension(get_compiled_module_extension());
    }
    return output_path;
}

}  // namespace

std::pair<std::filesystem::path, std::vector<std::string>> get_compilation_command(const std::filesystem::path& cxx_file,
                                                                                   compile_options options) {
    auto output_path = get_output_path(options.output_file, cxx_file);
    auto compile_command = concatenate_vectors(
            get_compiler_path(std::move(options.cxx_compiler_path)),
            get_common_compile_flags(),
            options.debug ? get_debug_compile_flags() : get_release_compile_flags(),
            is_apple() ? get_apple_compile_flags() : std::vector<std::string>{},
            std::string{"-o"},
            output_path.string(),
            std::move(options.extra_flags),
            cxx_file.string());
    return {output_path, compile_command};
}

std::filesystem::path compile_module(const std::filesystem::path& cxx_file, compile_options options) {
    auto [output_path, compile_command] = get_compilation_command(cxx_file, std::move(options));
    std::vector<char*> args;
    args.reserve(compile_command.size());
    std::transform(compile_command.begin(), compile_command.end(), std::back_inserter(args), [&](std::string& s) {
        return s.data();
    });
    args.push_back(nullptr);
    // Forward environment variables as well.
    auto proc = mcga::proc::Subprocess::Invoke(args[0], args.data(), environ);
    proc->waitBlocking();
    if (!proc->isExited() || proc->getReturnCode() != 0) {
        // TODO: Include compiler error! Better error message!
        // TODO(@branch): Remove!
        for (const auto& part: compile_command) {
            std::cout << part << " ";
        }
        std::cout << "\n";
        std::cout << "errno: " << errno << " " << strerror(errno) << "\n";
        throw std::runtime_error{"Compilation failed."};
    }
    return output_path;
}

}  // namespace tema
