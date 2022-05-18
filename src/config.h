#pragma once

namespace tema {

enum class platform_os: char {
    apple = 'A',
    linux = 'L',
    windows = 'W',

    target = TEMA_PLATFORM_TARGET_OS,
};

constexpr std::string_view default_install_path = TEMA_DEFAULT_INSTALL_PATH;
constexpr std::string_view default_cxx_compiler_path = TEMA_DEFAULT_CXX_COMPILER_PATH;
constexpr std::string_view default_apple_sysroot_path = TEMA_DEFAULT_APPLE_SYSROOT_PATH;  // NOLINT(readability-redundant-string-init)

consteval bool is_apple() {
    return platform_os::target == platform_os::apple;
}

consteval bool is_linux() {
    return platform_os::target == platform_os::linux;
}

consteval bool is_windows() {
    return platform_os::target == platform_os::windows;
}

}  // namespace tema