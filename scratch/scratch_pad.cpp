#include "compiler/compile.h"
#include "compiler/parser.h"
#include "compiler/translate.h"

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

tema::module load_module(const std::string& so_filename) {
    timer t{"load " + so_filename};
    const auto dl = dlopen(so_filename.c_str(), RTLD_LOCAL | RTLD_LAZY);
    std::cout << dlerror() << "\n";
    const auto symbol = reinterpret_cast<tema::module (*)()>(dlsym(dl, "_tema_module"));
    std::cout << dlerror() << "\n" << symbol << "\n";
    return symbol();
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i += 1) {
        std::filesystem::path module_path = argv[i];
        module_path.replace_extension(".tema.dylib");
//        std::cout << module_path.string() << "\n";
//        const auto cxx_file = tema::translate_module(module_path);
//        std::cout << cxx_file.string() << "\n";
//        const auto dll_file = tema::compile_module(cxx_file, {
//                                                                     .extra_flags = {"-I./src"}
//                                                             });
//        std::cout << dll_file.string() << "\n";
        const auto mod_compiled = load_module(module_path);
    }
    return 0;
}
