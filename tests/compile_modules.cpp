#include <filesystem>

#include <mcga/test_ext/matchers.hpp>

#include "compiler/compile.h"
#include "compiler/precompiled_module.h"
#include "compiler/print_cxx.h"

using namespace tema;
using namespace mcga::test;
using namespace mcga::matchers;

void compile_and_load_module(const std::filesystem::path& module_path) {
    auto cxx_file = tema::translate_module(module_path);
    expect(cxx_file, std::filesystem::path{module_path}.replace_extension(".tema.cc"));
    const auto dll_file = tema::compile_module(cxx_file);
    expect(dll_file, std::filesystem::path{module_path}.replace_extension(get_compiled_module_extension()));

    tema::precompiled_module module(dll_file);
    const auto mod = module.load_module();
    (void) mod;
}

TEST_CASE("compile & load pre-built modules") {
    for (const auto& entry: std::filesystem::directory_iterator("./modules")) {
        if (entry.is_regular_file()) {
            const auto& module_path = entry.path();
            if (module_path.extension() == ".tema") {
                test({
                             .description = module_path.stem().string(),
                             .timeTicksLimit = 100.0,
                     },
                     [&] {
                         compile_and_load_module(module_path);
                     });
            }
        }
    }
}