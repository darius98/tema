#include <filesystem>
#include <fstream>

#include <mcga/test_ext/matchers.hpp>

#include "compiler/parser.h"

using namespace tema;
using namespace mcga::test;
using namespace mcga::matchers;

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
                         std::ifstream file_stream(module_path);
                         const auto mod = tema::parse_module(file_stream, module_path);
                         (void) mod;
                     });
            }
        }
    }
}