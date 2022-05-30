set(clang_tidy_extra_paths "")
if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    # For local development, for llvm installed with brew.
    list(APPEND clang_tidy_extra_paths "/usr/local/opt/llvm/bin/")
endif ()
find_program(CLANG_TIDY clang-tidy PATHS ${clang_tidy_extra_paths})

function (AddStaticAnalysisTargets)
    get_property(source_files GLOBAL PROPERTY SOURCE_FILES)
    if (CLANG_TIDY)
        add_custom_target(tidy ${CLANG_TIDY} "--warnings-as-errors=*" -p ${CMAKE_BINARY_DIR} ${source_files}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                USES_TERMINAL)
    endif ()
endfunction ()
