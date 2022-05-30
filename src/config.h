#pragma once

namespace tema {

enum class platform_os: char {
    apple = 'A',
    linux = 'L',
    windows = 'W',

    target = TEMA_PLATFORM_TARGET_OS,
};

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