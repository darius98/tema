#include "compiler/compile.h"
#include "compiler/parser.h"
#include "compiler/precompiled_module.h"

#include <chrono>
#include <fstream>
#include <iostream>

struct timer {
    std::string name;
    std::chrono::high_resolution_clock::time_point start_time{std::chrono::high_resolution_clock::now()};

    explicit timer(std::string name)
        : name(std::move(name)) {
        std::cout << this->name << ":\n";
    }

    void step(std::string_view step_name) {
        const auto end_time = std::chrono::high_resolution_clock::now();
        const auto duration = (end_time - start_time);
        std::cout << "\t" << step_name << "\t" << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << "us\n";
        start_time = end_time;
    }
};

int main() {
    for (const auto& entry: std::filesystem::directory_iterator("./modules")) {
        if (entry.is_regular_file()) {
            const auto& module_path = entry.path();
            if (module_path.extension() == ".tema") {
                timer t{module_path.stem().string()};
                std::ifstream file_stream(module_path);
                const auto mod = tema::parse_module(file_stream, module_path.string());
                t.step("parse tema");
                auto cxx_file = module_path;
                cxx_file.replace_extension(".tema.cc");
                std::ofstream out_file(cxx_file);
                tema::print_cxx_to(mod, out_file);
                t.step("print c++");
                const auto dll_file = tema::compile_module(
                        cxx_file, {
                                          .extra_flags = {
                                                  "-I./src",
                                                  "-I./.build/debug/_deps/mcga_meta-src/include",
                                          },
                                  });
                t.step("compile c++");
                tema::precompiled_module module(dll_file);
                t.step("load DLL");
                (void) module.load_module();
                t.step("load tema");
            }
        }
    }
    return 0;
}
