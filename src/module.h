#pragma once

#include <memory>
#include <stdexcept>

namespace tema {

class scope;

struct invalid_module_scope : std::runtime_error {
    invalid_module_scope();
};

class module {
    std::shared_ptr<const scope> data;

public:
    explicit module(scope&& data);

    [[nodiscard]] const scope& scope() const;
};

}// namespace tema