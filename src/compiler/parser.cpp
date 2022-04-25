#include "compiler/parser.h"

#include <fstream>
#include <sstream>

namespace tema {

module parse_module_from_stream(std::istream&) {
    // TODO: Implement
    return module(scope{});
}

module parse_module_from_string(std::string_view data) {
    std::stringstream string_stream;
    string_stream << data;  // TODO: This is quite inefficient, why do we need to copy the data / allocate?
    return parse_module_from_stream(string_stream);
}

module parse_module_from_file(std::string_view file_name) {
    std::ifstream file_stream(file_name);
    if (file_stream.fail()) {
        throw std::runtime_error{"Could not open file '" + std::string(file_name) + "'."};
    }
    return parse_module_from_stream(file_stream);
}

}  // namespace tema
