include(FetchContent)

FetchContent_Declare(mcga_matchers URL https://github.com/darius98/mcga-matchers/archive/refs/heads/master.zip)
FetchContent_Populate(mcga_matchers)
add_subdirectory(${mcga_matchers_SOURCE_DIR} ${mcga_matchers_BINARY_DIR} EXCLUDE_FROM_ALL)

FetchContent_Declare(mcga_test URL https://github.com/darius98/mcga-test/archive/refs/heads/master.zip)
FetchContent_Populate(mcga_test)
add_subdirectory(${mcga_test_SOURCE_DIR} ${mcga_test_BINARY_DIR} EXCLUDE_FROM_ALL)

set(TEST_PROFRAW_FILES "")
set(TEST_BINARY_FILES "")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh "#!/usr/bin/env sh\n\nset -e\n\n")
file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

macro(add_tema_test NAME)
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} tema mcga_test mcga_matchers)

    set(PROFRAW_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.profraw)
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh "LLVM_PROFILE_FILE=${PROFRAW_FILE_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${NAME} --executor=smooth\n")
    if (COLLECT_COVERAGE)
        target_link_options(${NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        list(APPEND TEST_PROFRAW_FILES ${PROFRAW_FILE_NAME})
        list(APPEND TEST_BINARY_FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME})
    endif ()
endmacro()

add_tema_test(unittests
        algorithms/test_apply_vars.cpp
        algorithms/test_deduce.cpp
        algorithms/test_equals.cpp
        algorithms/test_match.cpp
        algorithms/test_print_ascii.cpp
        test_module.cpp
        test_statement.cpp
        test_scope.cpp)

if (COLLECT_COVERAGE)
    add_custom_target(coverage_collect COMMAND llvm-profdata merge -sparse ${TEST_PROFRAW_FILES} -o ${CMAKE_CURRENT_BINARY_DIR}/merged.profdata)
    add_custom_target(coverage COMMAND llvm-cov show ${TEST_BINARY_FILES}
            -instr-profile=${CMAKE_CURRENT_BINARY_DIR}/merged.profdata
            -show-branches=count
            -show-line-counts
            -use-color -Xdemangler c++filt -Xdemangler -n
            -format=html > ${CMAKE_CURRENT_BINARY_DIR}/coverage.html)
    add_custom_target(coverage_summary COMMAND llvm-cov report ${TEST_BINARY_FILES} -instr-profile=${CMAKE_CURRENT_BINARY_DIR}/merged.profdata)
endif ()
