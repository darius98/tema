#ifndef TEMA_EXPORT
#if defined _WIN32 || defined __CYGWIN__
#    define TEMA_EXPORT __declspec(dllexport)
#else
#    if __GNUC__ >= 4
#        define TEMA_EXPORT __attribute__((visibility("default")))
#    else
#        define TEMA_EXPORT
#    endif
#endif
#endif
