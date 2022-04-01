#include "variable.h"

namespace tema {

variable::variable(symbol name)
    : name(std::move(name)) {}

variable_ptr var(symbol name) {
    return std::make_shared<const variable>(std::move(name));
}

}// namespace tema
