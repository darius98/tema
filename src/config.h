#pragma once

namespace tema {

enum class platform_os: char {
    apple = 'A',
    linux = 'L',
    windows = 'W',

    target = TEMA_PLATFORM_TARGET_OS,
};

constexpr std::string_view default_cxx_compiler_path = TEMA_DEFAULT_CXX_COMPILER_PATH;

#ifndef TEMA_DEFAULT_APPLE_SYSROOT
static_assert(platform_os::target != platform_os::apple, "Building on apple without providing TEMA_DEFAULT_APPLE_SYSROOT build flag.");
#define TEMA_DEFAULT_APPLE_SYSROOT ""
#endif
constexpr std::string_view default_apple_sysroot = TEMA_DEFAULT_APPLE_SYSROOT;

constexpr bool is_apple() {
    return platform_os::target == platform_os::apple;
}

constexpr bool is_linux() {
    return platform_os::target == platform_os::linux;
}

constexpr bool is_windows() {
    return platform_os::target == platform_os::windows;
}

}  // namespace tema