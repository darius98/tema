if (COLLECT_COVERAGE)
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate -fcoverage-mapping)
endif ()

add_library(tema
        core/expression.cpp
        core/module.cpp
        core/scope.cpp
        core/statement.cpp
        core/variable.cpp

        algorithms/apply_vars.cpp
        algorithms/deduce.cpp
        algorithms/equals.cpp
        algorithms/match.cpp
        algorithms/print_utf8.cpp

        modules/propositional_logic.cpp
        )
target_include_directories(tema PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
