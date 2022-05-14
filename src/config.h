#pragma once

namespace tema {

enum class platform_os: char {
    apple = 'A',
    linux = 'L',
    windows = 'W',

    target = TEMA_PLATFORM_TARGET_OS,
};

#ifndef TEMA_PLATFORM_MACOS_SYSROOT
static_assert(platform_os::target != platform_os::apple, "Building on apple without providing TEMA_PLATFORM_MACOS_SYSROOT build flag.");
#define TEMA_PLATFORM_MACOS_SYSROOT ""
#endif
constexpr std::string_view default_apple_sysroot = TEMA_PLATFORM_MACOS_SYSROOT;

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