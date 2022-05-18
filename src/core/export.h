#ifndef TEMA_EXPORT
#if defined _WIN32 || defined __CYGWIN__
#    define TEMA_EXPORT __declspec(dllexport)  // NOLINT(cppcoreguidelines-macro-usage)
#else
#    if __GNUC__ >= 4
#        define TEMA_EXPORT __attribute__((visibility("default")))  // NOLINT(cppcoreguidelines-macro-usage)
#    else
#        define TEMA_EXPORT  // NOLINT(cppcoreguidelines-macro-usage)
#    endif
#endif
#endif
