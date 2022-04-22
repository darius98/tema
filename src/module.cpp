#include "module.h"

#include "scope.h"

namespace tema {

invalid_module_scope::invalid_module_scope()
    : std::runtime_error{"Invalid module scope, has parent scope"} {}

module::module(scope&& data) {
    // Check and throw before allocating, to avoid memory leaks
    if (data.has_parent()) {
        throw invalid_module_scope{};
    }
    this->data = std::make_shared<const scope>(std::move(data));
}

const scope& module::get_scope() const {
    return *data;
}

}// namespace tema
