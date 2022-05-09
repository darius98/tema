#pragma once

#include <string>
#include <ostream>

#include "core/module.h"

namespace tema {

struct print_cxx_options {
    // Not used atm.
    bool debug{false};

    // Include the proofs in the generated module.
    bool include_proofs{false};
};

void print_cxx_to(const module& mod, std::ostream& to, const print_cxx_options& options = {});
[[nodiscard]] std::string print_cxx(const module& mod, const print_cxx_options& options = {});

}  // namespace tema
