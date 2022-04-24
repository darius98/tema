set(CLANG_WARNINGS
        -Werror
        -Wall
        -Wextra # reasonable and standard
        -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps
        # catch hard to track down memory errors
        -Wold-style-cast # warn for c-style casts
        -Wcast-align # warn for potential performance problem casts
        -Wunused # warn on anything being unused
        -Woverloaded-virtual # warn if you overload (not override) a virtual function
        -Wpedantic # warn if non-standard C++ is used
        -Wconversion # warn on type conversions that may lose data
        -Wsign-conversion # warn on sign conversions
        -Wsign-compare
        -Wnull-dereference # warn if a null dereference is detected
        -Wdouble-promotion # warn if float is implicit promoted to double
        -Wformat=2 # warn on security issues around functions that format output (ie printf)
        -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation
        )
set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
        -Wduplicated-cond # warn if if / else chain has duplicated conditions
        -Wduplicated-branches # warn if if / else branches have duplicated code
        -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
        -Wuseless-cast # warn if you perform a cast to the same type
        )

if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set_property(GLOBAL PROPERTY PROJECT_WARNINGS "${CLANG_WARNINGS}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_property(GLOBAL PROPERTY PROJECT_WARNINGS "${GCC_WARNINGS}")
else ()
    message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
    set_property(GLOBAL PROPERTY PROJECT_WARNINGS "")
endif ()

function(AddTargetCompileFlags TARGET)
    get_property(warnings GLOBAL PROPERTY PROJECT_WARNINGS)
    if (warnings)
        target_compile_options(${TARGET} PRIVATE ${warnings})
    endif ()

    target_compile_options(${TARGET} PRIVATE -fPIE -fvisibility=hidden)
    target_link_options(${TARGET} PRIVATE -fPIE -fvisibility=hidden)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${TARGET} PRIVATE -fsanitize=address,undefined -fno-sanitize-recover=undefined)
        target_link_options(${TARGET} PRIVATE -fsanitize=address,undefined)
    endif ()
endfunction()
