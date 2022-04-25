#pragma once

#include <memory>
#include <stdexcept>

#include "core/scope.h"

namespace tema {

struct invalid_module_scope : std::runtime_error {
    invalid_module_scope();
};

struct module {
private:
    scope data;

public:
    explicit module(scope&& data);

    [[nodiscard]] const scope& get_scope() const;
};

}  // namespace tema
