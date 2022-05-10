#include "compiler/compile.h"
#include "compiler/precompiled_module.h"
#include "compiler/translate.h"

#include <chrono>
#include <fstream>
#include <iostream>

struct timer {
    std::string name;
    std::chrono::high_resolution_clock::time_point start_time{std::chrono::high_resolution_clock::now()};

    explicit timer(std::string name): name(std::move(name)) {
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
                const auto cxx_file = tema::translate_module(module_path);
                t.step("translate");
                const auto dll_file = tema::compile_module(cxx_file, {.extra_flags = {"-I./src"}});
                t.step("CXX compile");
                tema::precompiled_module module(dll_file);
                t.step("DLL load");
                (void)module.load_module();
                t.step("module load");
            }
        }
    }
    return 0;
}
