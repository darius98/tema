#pragma once

#include <memory>
#include <string>

namespace tema {

using symbol = std::string;  // TODO: In the future, this will become more complex (e.g. support greek symbols, sub/super-script etc.).
using symbol_view = std::string_view;

struct variable {
    symbol name;

    explicit variable(symbol name);
};

using variable_ptr = std::shared_ptr<const variable>;

[[nodiscard]] variable_ptr var(symbol_view name);

}  // namespace tema
