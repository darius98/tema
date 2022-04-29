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

AddFlexLibrary(tema_compiler_lexer compiler/flex_lexer.l)
AddTemaLibrary(tema_compiler
        SOURCES
        compiler/lexer.cpp
        compiler/parser.cpp

        DEPS
        tema_core tema_compiler_lexer

        TESTS
        compiler/lexer_test.cpp
        compiler/parser_test.cpp

        TESTS_DEPS
        tema_algorithms)
