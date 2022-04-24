#pragma once

#include <memory>
#include <stdexcept>

namespace tema {

struct scope;

struct invalid_module_scope : std::runtime_error {
    invalid_module_scope();
};

struct module {
private:
    std::shared_ptr<const scope> data;

public:
    explicit module(scope&& data);

    [[nodiscard]] const scope& get_scope() const;
};

}  // namespace tema