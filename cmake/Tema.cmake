set_property(GLOBAL PROPERTY TEST_PROFRAW_FILES)
set_property(GLOBAL PROPERTY TEST_BINARY_FILES)
set_property(GLOBAL PROPERTY SOURCE_FILES)

function(InitTemaTests)
    file(WRITE ${CMAKE_BINARY_DIR}/run_tests.sh
            "#!/usr/bin/env sh\n"
            "\n"
            "set -e\n"
            "\n"
            )
    if (APPLE)
        file(APPEND ${CMAKE_BINARY_DIR}/run_tests.sh
                "export MallocNanoZone=0\n")
    endif ()

    file(CHMOD ${CMAKE_BINARY_DIR}/run_tests.sh PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
endfunction()

function(AddTemaTarget TYPE NAME)
    cmake_parse_arguments(P "NO_COVERAGE" "" "SOURCES;TESTS;DEPS;TESTS_DEPS" ${ARGN})
    if (TYPE STREQUAL "LIBRARY")
        add_library(${NAME} STATIC ${P_SOURCES})
        get_property(tmp GLOBAL PROPERTY SOURCE_FILES)
        foreach (source_file ${P_SOURCES})
            list(APPEND tmp ${CMAKE_CURRENT_SOURCE_DIR}/${source_file})
        endforeach ()
        set_property(GLOBAL PROPERTY SOURCE_FILES "${tmp}")
    elseif (TYPE STREQUAL "EXECUTABLE")
        add_executable(${NAME} ${P_SOURCES})
        if (APPLE)
            target_link_options(${NAME} PRIVATE -all_load)
        endif ()
    else ()
        message(FATAL_ERROR "Invalid type '${TYPE}' provided for AddTemaTarget")
    endif ()
    AddTargetCompileFlags(${NAME})
    if ((NOT P_NO_COVERAGE) AND COLLECT_COVERAGE)
        target_compile_options(${NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(${NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    endif ()
    if (P_DEPS)
        target_link_libraries(${NAME} PUBLIC ${P_DEPS})
    endif ()
    if (P_TESTS)
        AddTemaTest(test_${NAME} SOURCES ${P_TESTS} DEPS ${NAME} ${P_TESTS_DEPS})
    endif ()
endfunction()

function(AddTemaLibrary NAME)
    AddTemaTarget(LIBRARY ${NAME} ${ARGN})
endfunction()

function(AddTemaExecutable NAME)
    AddTemaTarget(EXECUTABLE ${NAME} ${ARGN})
endfunction()

function(AddTemaTest NAME)
    cmake_parse_arguments(P "NO_COVERAGE" "" "SOURCES;DEPS;ENV" ${ARGN})
    AddTemaExecutable(${NAME}
            NO_COVERAGE
            SOURCES ${P_SOURCES}
            DEPS ${P_DEPS} mcga_test_static mcga_matchers)
    set(PROFRAW_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.profraw)
    set(COMMAND "")
    foreach (TEST_ENV_VAR ${P_ENV})
        set(COMMAND "${COMMAND} ${TEST_ENV_VAR}")
    endforeach ()
    set(COMMAND "${COMMAND} LLVM_PROFILE_FILE=${PROFRAW_FILE_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${NAME} --executor=smooth")
    string(STRIP "${COMMAND}" COMMAND)
    file(APPEND ${CMAKE_BINARY_DIR}/run_tests.sh "\necho 'Running ${NAME}\\n\t${COMMAND}'\n")
    file(APPEND ${CMAKE_BINARY_DIR}/run_tests.sh "${COMMAND}\n")
    file(APPEND ${CMAKE_BINARY_DIR}/run_tests.sh "echo '\\n'\n")
    if (COLLECT_COVERAGE)
        target_link_options(${NAME} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        get_property(tmp GLOBAL PROPERTY TEST_PROFRAW_FILES)
        list(APPEND tmp ${PROFRAW_FILE_NAME})
        set_property(GLOBAL PROPERTY TEST_PROFRAW_FILES "${tmp}")

        get_property(tmp GLOBAL PROPERTY TEST_BINARY_FILES)
        list(APPEND tmp ${CMAKE_CURRENT_BINARY_DIR}/${NAME})
        set_property(GLOBAL PROPERTY TEST_BINARY_FILES "${tmp}")
    endif ()
endfunction()
