#include "compiler/precompiled_module.h"

#include <dlfcn.h>

namespace tema {

precompiled_module::precompiled_module(const std::filesystem::path& dll_path) {
    handle = dlopen(dll_path.c_str(), RTLD_LOCAL | RTLD_LAZY);
    if (handle == nullptr) {
        throw std::runtime_error{"Failed to open precompiled module at " + dll_path.string() + ", dlopen error: " + std::string{dlerror()}};
    }
    tema_module = reinterpret_cast<module(*)()>(dlsym(handle, "tema_module"));
    if (tema_module == nullptr) {
        dlclose(handle);
        throw std::runtime_error{"Failed to open precompiled module at " + dll_path.string() + ", dlsym error: " + std::string{dlerror()}};
    }
}

precompiled_module::~precompiled_module() {
    dlclose(handle);
}

module precompiled_module::load_module() {
    return tema_module();
}

}  // namespace tema
