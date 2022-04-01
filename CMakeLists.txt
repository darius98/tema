cmake_minimum_required(VERSION 3.22)
project(tema)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(SANITIZER_COMPILE_OPTIONS -fsanitize=address,undefined -fno-sanitize-recover=undefined)
set(SANITIZER_LINK_OPTIONS -fsanitize=address,undefined)

set(CLANG_WARNINGS
        -Werror
        -Wall
        -Wextra # reasonable and standard
        -Wshadow # warn the user if a variable declaration shadows one from a parent context
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

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    add_compile_options(${CLANG_WARNINGS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(${GCC_WARNINGS})
else()
    message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
endif()

include(FetchContent)

FetchContent_Declare(mcga_cli URL https://github.com/darius98/mcga-cli/archive/refs/heads/master.zip)
FetchContent_MakeAvailable(mcga_cli)

FetchContent_Declare(mcga_matchers URL https://github.com/darius98/mcga-matchers/archive/refs/heads/master.zip)
FetchContent_MakeAvailable(mcga_matchers)

FetchContent_Declare(mcga_test URL https://github.com/darius98/mcga-test/archive/refs/heads/master.zip)
FetchContent_MakeAvailable(mcga_test)

add_library(tema
        ${CMAKE_CURRENT_SOURCE_DIR}/src/statement.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/variable.cpp
        )
target_include_directories(tema PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)

enable_testing()
function(add_tema_test NAME)
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} tema mcga_test mcga_matchers)
    add_test(${NAME} ${NAME})
endfunction()

add_tema_test(test_statement tests/test_statement.cpp)
