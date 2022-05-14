AddTemaLibrary(tema_core
        SOURCES
        core/expression.cpp
        core/module.cpp
        core/relationship.cpp
        core/scope.cpp
        core/statement.cpp
        core/variable.cpp

        DEPS
        mcga_meta

        TESTS
        core/expression_test.cpp
        core/module_test.cpp
        core/relationship_test.cpp
        core/statement_test.cpp
        core/scope_test.cpp
        core/variable_test.cpp)

# These headers are required for pre-compiling modules.
file(GLOB tema_core_headers core/*.h)
install(FILES ${tema_core_headers} DESTINATION include/tema/core/)
install(DIRECTORY ${mcga_meta_SOURCE_DIR}/include DESTINATION .)

AddTemaLibrary(tema_algorithms
        SOURCES
        algorithms/apply_vars.cpp
        algorithms/deduce.cpp
        algorithms/equals.cpp
        algorithms/match.cpp
        algorithms/print_utf8.cpp

        DEPS
        tema_core mcga_meta

        TESTS
        algorithms/apply_vars_test.cpp
        algorithms/deduce_test.cpp
        algorithms/equals_test.cpp
        algorithms/match_test.cpp
        algorithms/print_utf8_test.cpp)

AddFlexLibrary(tema_compiler_lexer compiler/lexer_flex.l)
target_link_libraries(tema_compiler_lexer PUBLIC tema_core)
AddTemaLibrary(tema_compiler
        SOURCES
        compiler/compile.cpp
        compiler/lexer.cpp
        compiler/parser.cpp
        compiler/precompiled_module.cpp
        compiler/print_cxx.cpp

        DEPS
        tema_compiler_lexer mcga_proc

        TESTS
        compiler/compile_test.cpp
        compiler/lexer_test.cpp
        compiler/parser_test.cpp
        compiler/precompiled_module_test.cpp
        compiler/print_cxx_test.cpp

        TESTS_DEPS
        tema_algorithms)
if (NOT APPLE)
    target_link_libraries(tema_compiler PRIVATE dl)
endif ()
