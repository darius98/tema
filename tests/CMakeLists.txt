include(FetchContent)

FetchContent_Declare(mcga_matchers URL https://github.com/darius98/mcga-matchers/archive/refs/heads/master.zip)
FetchContent_MakeAvailable(mcga_matchers)

FetchContent_Declare(mcga_test URL https://github.com/darius98/mcga-test/archive/refs/heads/master.zip)
FetchContent_MakeAvailable(mcga_test)

enable_testing()
function(add_tema_test NAME)
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} tema mcga_test mcga_matchers)
    add_test(${NAME} ${NAME})
endfunction()

add_tema_test(unittests
        ${CMAKE_CURRENT_SOURCE_DIR}/test_module.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_print.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_statement.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test_scope.cpp)
