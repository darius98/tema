#include "compiler/precompiled_module.h"

#include <dlfcn.h>

#include "config.h"

namespace tema {

namespace {

// Weird difference in dlsym interface, but sure...
constexpr auto tema_module_symbol_name = is_apple() ? "tema_module" : "_tema_module";

}  // namespace

precompiled_module::precompiled_module(const std::filesystem::path& dll_path)
    : handle(dlopen(dll_path.c_str(), RTLD_LOCAL | RTLD_LAZY)) {
    if (handle == nullptr) {
        throw std::runtime_error{"Failed to open precompiled module at " + dll_path.string() + ", dlopen error: " + std::string{dlerror()}};  // NOLINT(concurrency-mt-unsafe)
    }
    tema_module = reinterpret_cast<module (*)()>(dlsym(handle, tema_module_symbol_name));  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    if (tema_module == nullptr) {
        dlclose(handle);
        throw std::runtime_error{"Failed to open precompiled module at " + dll_path.string() + ", dlsym error: " + std::string{dlerror()}};  // NOLINT(concurrency-mt-unsafe)
    }
}

precompiled_module::precompiled_module(precompiled_module&& other) noexcept
    : handle(other.handle), tema_module(other.tema_module) {
    other.handle = nullptr;
    other.tema_module = nullptr;
}

precompiled_module& precompiled_module::operator=(precompiled_module&& other) noexcept {
    if (this != &other) {
        handle = std::exchange(other.handle, nullptr);
        tema_module = std::exchange(other.tema_module, nullptr);
    }
    return *this;
}

precompiled_module::~precompiled_module() {
    if (handle != nullptr) {
        dlclose(handle);
    }
}

module precompiled_module::load_module() {
    return tema_module();
}

}  // namespace tema
