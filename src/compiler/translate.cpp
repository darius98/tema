#include "compiler/translate.h"

#include <fstream>
#include <iostream>

#include "compiler/parser.h"

namespace tema {

std::filesystem::path translate_module(const std::filesystem::path& input_file,
                                       const std::filesystem::path& output_file,
                                       const print_cxx_options& options) {
    std::ifstream file_stream(input_file);
    if (file_stream.fail()) {
        throw parse_error{"Could not open file '" + input_file.string() + "'."};
    }
    const auto mod = parse_module(file_stream, input_file.string());

    std::filesystem::path output_path;
    std::ostream* out;
    std::ofstream file_out;
    if (output_file == "-") {
        out = &std::cout;
        output_path = "";
    } else {
        if (!output_file.empty()) {
            output_path = output_file;
        } else {
            output_path = input_file;
            output_path.replace_extension(".tema.cc");
        }
        file_out.open(output_path);
        out = &file_out;
    }
    print_cxx_to(mod, *out, options);
    return output_path;
}

}  // namespace tema
