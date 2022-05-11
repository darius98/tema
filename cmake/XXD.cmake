find_program(XXD_PATH xxd REQUIRED)

function(GenerateXXD FILE VAR_NAME)
    execute_process(COMMAND ${XXD_PATH} -i INPUT_FILE ${FILE} OUTPUT_VARIABLE XXD_CODE)
    file(GENERATE
            OUTPUT ${FILE}.xxd.h
            CONTENT "#pragma once\n\ninline constexpr const char ${VAR_NAME}[] = {\n${XXD_CODE}, 0\n};")
endfunction()
