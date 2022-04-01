#pragma once

#include <memory>
#include <string>

namespace tema {

// In the future, this will become more complex (e.g. to support greek symbols).
using symbol = std::string;

struct variable {
    symbol name;
    // set_ptr type;  // TODO

    explicit variable(symbol name);
};

using variable_ptr = std::shared_ptr<const variable>;

variable_ptr var(symbol name);

}// namespace tema
