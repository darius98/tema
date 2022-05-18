#pragma once

#include <filesystem>

#include "core/module.h"

namespace tema {

struct precompiled_module {
private:
    void* handle;

    module (*tema_module)();

public:
    explicit precompiled_module(const std::filesystem::path& dll_path);

    precompiled_module(const precompiled_module&) = delete;
    precompiled_module& operator=(const precompiled_module&) = delete;

    precompiled_module(precompiled_module&& other) noexcept;
    precompiled_module& operator=(precompiled_module&& other) noexcept;

    ~precompiled_module();

    module load_module();
};

}  // namespace tema
