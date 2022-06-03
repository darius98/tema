# Build with coverage
if (CMAKE_BUILD_TYPE MATCHES "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(COLLECT_COVERAGE_DEFAULT ON)
else ()
    set(COLLECT_COVERAGE_DEFAULT OFF)
endif ()
option(COLLECT_COVERAGE "Collect Clang source-based coverage" ${COLLECT_COVERAGE_DEFAULT})

function(AddCoverageTarget)
    if (COLLECT_COVERAGE)
        set(EXTRA_PATHS "")
        if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
            set(EXTRA_PATHS "/Library/Developer/CommandLineTools/usr/bin/")
        endif ()

        find_program(LLVM_COV llvm-cov REQUIRED PATHS ${EXTRA_PATHS})
        find_program(LLVM_PROFDATA llvm-profdata REQUIRED PATHS ${EXTRA_PATHS})

        get_property(test_profraw_files GLOBAL PROPERTY TEST_PROFRAW_FILES)
        get_property(test_binary_files GLOBAL PROPERTY TEST_BINARY_FILES)

        add_custom_target(coverage_collect COMMAND ${LLVM_PROFDATA} merge -sparse ${test_profraw_files} -o ${CMAKE_BINARY_DIR}/merged.profdata)

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

        set(COVERAGE_COMMON_ARGS ${BINARY_FILE_ARGS}
                -instr-profile=${CMAKE_BINARY_DIR}/merged.profdata
                -ignore-filename-regex=\".*FlexLexer.*\"
                -ignore-filename-regex=\".*_deps.*\")

        add_custom_target(coverage_report COMMAND ${LLVM_COV} show ${COVERAGE_COMMON_ARGS}
                -show-branches=count
                -show-line-counts
                -use-color -Xdemangler c++filt -Xdemangler -n
                -line-coverage-lt=100
                -format=html
                > ${CMAKE_BINARY_DIR}/coverage.html)
        add_dependencies(coverage_report coverage_collect)

        add_custom_target(coverage_export COMMAND ${LLVM_COV} export ${COVERAGE_COMMON_ARGS}
                > ${CMAKE_BINARY_DIR}/coverage_export.json)
        add_dependencies(coverage_export coverage_collect)

        add_custom_target(coverage COMMAND ${LLVM_COV} report ${COVERAGE_COMMON_ARGS}
                -show-region-summary=0)
        add_dependencies(coverage coverage_report coverage_export)
    else ()
        add_custom_target(coverage COMMAND echo "coverage not supported in this build")
    endif ()
endfunction()
