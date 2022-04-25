#include "core/module.h"

#include "core/scope.h"

namespace tema {

invalid_module_scope::invalid_module_scope()
    : std::runtime_error{"Invalid module scope, has parent scope"} {}

module::module(scope&& data): data(data) {
    if (data.has_parent()) {
        throw invalid_module_scope{};
    }
}

const scope& module::get_scope() const {
    return data;
}

}  // namespace tema
