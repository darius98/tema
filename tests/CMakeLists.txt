include(FetchContent)

FetchContent_Declare(mcga_matchers URL https://github.com/darius98/mcga-matchers/archive/refs/heads/master.zip)
FetchContent_MakeAvailable(mcga_matchers)

FetchContent_Declare(mcga_test URL https://github.com/darius98/mcga-test/archive/refs/heads/master.zip)
FetchContent_MakeAvailable(mcga_test)

enable_testing()
function(add_tema_test NAME)
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} tema mcga_test mcga_matchers)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(${NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    endif ()

    add_test(${NAME} ${NAME})
endfunction()

add_tema_test(unittests
        algorithms/test_apply_vars.cpp
        algorithms/test_deduce.cpp
        algorithms/test_equals.cpp
        algorithms/test_match.cpp
        algorithms/test_print_ascii.cpp
        test_module.cpp
        test_statement.cpp
        test_scope.cpp)
