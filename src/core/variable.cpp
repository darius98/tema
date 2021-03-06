#include "core/variable.h"

namespace tema {

variable::variable(symbol name)
    : name(std::move(name)) {}

variable_ptr var(symbol_view name) {
    return std::make_shared<const variable>(symbol{name});
}

}  // namespace tema
