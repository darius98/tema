cmake_minimum_required(VERSION 3.22)
project(tema)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build with coverage
if (CMAKE_BUILD_TYPE MATCHES "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(COLLECT_COVERAGE_DEFAULT ON)
else ()
    set(COLLECT_COVERAGE_DEFAULT OFF)
endif ()
option(COLLECT_COVERAGE "Collect Clang source-based coverage" ${COLLECT_COVERAGE_DEFAULT})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
    add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=undefined)
    add_link_options(-fsanitize=address,undefined)
else ()
    set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

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

        #        -Wsign-conversion # warn on sign conversions
        -Wno-sign-conversion  # TODO: Re-enable at some point.
        -Wno-sign-compare  # TODO: Re-enable at some point.

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
    add_compile_options(${CLANG_WARNINGS})
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(${GCC_WARNINGS})
else ()
    message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
endif ()

include(FetchContent)

FetchContent_Declare(mcga_cli URL https://github.com/darius98/mcga-cli/archive/refs/heads/master.zip)
FetchContent_Populate(mcga_cli)
add_subdirectory(${mcga_cli_SOURCE_DIR} ${mcga_cli_BINARY_DIR} EXCLUDE_FROM_ALL)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
