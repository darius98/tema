#include "compiler/parser.h"
#include "compiler/print_cxx.h"

#include <chrono>
#include <fstream>
#include <iostream>

#include <dlfcn.h>

struct timer {
    std::string name;
    std::chrono::high_resolution_clock::time_point start_time{std::chrono::high_resolution_clock::now()};

    ~timer() {
        const auto end_time = std::chrono::high_resolution_clock::now();
        const auto duration = (end_time - start_time);
        std::cout << name << " took " << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << "us\n";
    }
};

tema::module timed_parse_module(const std::filesystem::path& in_filename) {
    timer t{"parse " + in_filename.string()};
    try {
        return tema::parse_module(in_filename);
    } catch (const tema::parse_error& err) {
        std::cerr << "Failed to parse: " << err.what();
        exit(1);
    }
}

void print_module(const tema::module& mod, const char* out_filename) {
    std::ofstream fout(out_filename);
    tema::print_cxx_to(mod, fout);
}

void compile_module(const std::string& target) {
    std::string s = "cmake --build .build/release/ --target " + std::string{target};
    auto compile_result = system(s.c_str());
    if (compile_result != 0) {
        std::cerr << "Failed to compile!\n";
        exit(1);
    }
}

tema::module load_module(const std::string& so_filename) {
    timer t{"load " + so_filename};
    const auto dl = dlopen(so_filename.c_str(), RTLD_LOCAL | RTLD_LAZY);
    const auto symbol = reinterpret_cast<tema::module (*)()>(dlsym(dl, "_get_module"));
    return symbol();
}

int main(int argc, char** argv) {
    if (argc % 4 != 1) {
        std::cerr << "Invalid arguments.\n";
        return 1;
    }
    for (int i = 1; i < argc; i += 4) {
        const auto mod_interpreted = timed_parse_module(argv[i]);
        print_module(mod_interpreted, argv[i + 1]);
        compile_module(argv[i + 2]);
        const auto mod_compiled = load_module(argv[i + 3]);
    }
    return 0;
}
