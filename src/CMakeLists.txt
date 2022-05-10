AddTemaLibrary(tema_core
        SOURCES
        core/expression.cpp
        core/module.cpp
        core/relationship.cpp
        core/scope.cpp
        core/statement.cpp
        core/variable.cpp

        TESTS
        core/expression_test.cpp
        core/module_test.cpp
        core/relationship_test.cpp
        core/statement_test.cpp
        core/scope_test.cpp
        core/variable_test.cpp)

AddTemaLibrary(tema_algorithms
        SOURCES
        algorithms/apply_vars.cpp
        algorithms/deduce.cpp
        algorithms/equals.cpp
        algorithms/match.cpp
        algorithms/print_utf8.cpp

        DEPS
        tema_core

        TESTS
        algorithms/apply_vars_test.cpp
        algorithms/deduce_test.cpp
        algorithms/equals_test.cpp
        algorithms/match_test.cpp
        algorithms/print_utf8_test.cpp)

GenerateXXD(${CMAKE_CURRENT_SOURCE_DIR}/util/export.h xxd_export_h_code)
AddFlexLibrary(tema_compiler_lexer compiler/lexer_flex.l)
AddTemaLibrary(tema_compiler
        SOURCES
        compiler/compile.cpp
        compiler/lexer.cpp
        compiler/parser.cpp
        compiler/print_cxx.cpp
        compiler/translate.cpp

        DEPS
        tema_core tema_compiler_lexer mcga_proc

        TESTS
        compiler/compile_test.cpp
        compiler/lexer_test.cpp
        compiler/parser_test.cpp
        compiler/print_cxx_test.cpp
        compiler/translate_test.cpp

        TESTS_DEPS
        tema_algorithms)
