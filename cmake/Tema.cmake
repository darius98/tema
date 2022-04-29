set_property(GLOBAL PROPERTY TEST_PROFRAW_FILES)
set_property(GLOBAL PROPERTY TEST_BINARY_FILES)

function(InitTemaTests)
    file(WRITE ${CMAKE_BINARY_DIR}/run_tests.sh "#!/usr/bin/env sh\n\nset -e\n\n")
    file(CHMOD ${CMAKE_BINARY_DIR}/run_tests.sh PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
endfunction()

function(AddTemaTarget TYPE NAME)
    cmake_parse_arguments(P "" "" "SOURCES;TESTS;DEPS;TESTS_DEPS" ${ARGN})
    if (TYPE STREQUAL "LIBRARY")
        add_library(${NAME} ${P_SOURCES})
    elseif (TYPE STREQUAL "EXECUTABLE")
        add_executable(${NAME} ${P_SOURCES})
    else ()
        message(FATAL_ERROR "Invalid type '${TYPE}' provided for AddTemaTarget")
    endif ()
    AddTargetCompileFlags(${NAME})
    if (COLLECT_COVERAGE)
        target_compile_options(${NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(${NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    endif ()
    if (P_DEPS)
        target_link_libraries(${NAME} PRIVATE ${P_DEPS})
    endif ()
    if (P_TESTS)
        add_executable(${NAME}_test ${P_TESTS})
        AddTargetCompileFlags(${NAME}_test)
        target_link_libraries(${NAME}_test PRIVATE ${NAME} mcga_test_static mcga_matchers)
        if (P_TESTS_DEPS)
            target_link_libraries(${NAME}_test PRIVATE ${P_TESTS_DEPS})
        endif ()

        set(PROFRAW_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_test.profraw)
        set(COMMAND "MallocNanoZone=0 LLVM_PROFILE_FILE=${PROFRAW_FILE_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_test --executor=smooth")
        file(APPEND ${CMAKE_BINARY_DIR}/run_tests.sh "echo 'Tests for ${NAME}\\n\t${COMMAND}'\n")
        file(APPEND ${CMAKE_BINARY_DIR}/run_tests.sh "${COMMAND}\n")
        file(APPEND ${CMAKE_BINARY_DIR}/run_tests.sh "echo '\\n'\n")
        if (COLLECT_COVERAGE)
            target_link_options(${NAME}_test PRIVATE -fprofile-instr-generate -fcoverage-mapping)
            get_property(tmp GLOBAL PROPERTY TEST_PROFRAW_FILES)
            list(APPEND tmp ${PROFRAW_FILE_NAME})
            set_property(GLOBAL PROPERTY TEST_PROFRAW_FILES "${tmp}")

            get_property(tmp GLOBAL PROPERTY TEST_BINARY_FILES)
            list(APPEND tmp ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_test)
            set_property(GLOBAL PROPERTY TEST_BINARY_FILES "${tmp}")
        endif ()
    endif ()
endfunction()

function(AddTemaLibrary NAME)
    AddTemaTarget(LIBRARY ${NAME} ${ARGN})
endfunction()

function(AddTemaExecutable NAME)
    AddTemaTarget(EXECUTABLE ${NAME} ${ARGN})
endfunction()

function(AddTemaCoverageTarget)
    if (COLLECT_COVERAGE)
        get_property(test_profraw_files GLOBAL PROPERTY TEST_PROFRAW_FILES)
        get_property(test_binary_files GLOBAL PROPERTY TEST_BINARY_FILES)

        add_custom_target(coverage_collect COMMAND llvm-profdata merge -sparse ${test_profraw_files} -o ${CMAKE_BINARY_DIR}/merged.profdata)

        set(BINARY_FILE_ARGS "")
        set(FIRST_ARG ON)
        foreach (binary_file ${test_binary_files})
            if (FIRST_ARG)
                set(FIRST_ARG OFF)
            else ()
                list(APPEND BINARY_FILE_ARGS "-object")
            endif ()
            list(APPEND BINARY_FILE_ARGS ${binary_file})
        endforeach ()

        add_custom_target(coverage_report COMMAND llvm-cov show ${BINARY_FILE_ARGS}
                -instr-profile=${CMAKE_BINARY_DIR}/merged.profdata
                -show-branches=count
                -show-line-counts
                -use-color -Xdemangler c++filt -Xdemangler -n
                -line-coverage-lt=100
                -ignore-filename-regex=".*FlexLexer.*"
                -format=html > ${CMAKE_BINARY_DIR}/coverage.html)
        add_dependencies(coverage_report coverage_collect)
        add_custom_target(coverage COMMAND llvm-cov report ${BINARY_FILE_ARGS}
                -instr-profile=${CMAKE_BINARY_DIR}/merged.profdata
                -show-region-summary=0
                -ignore-filename-regex=".*FlexLexer.*"
                )
        add_dependencies(coverage coverage_report)
    else ()
        add_custom_target(coverage COMMAND echo "coverage not supported in this build")
    endif ()
endfunction()
