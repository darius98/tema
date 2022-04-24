AddTemaLibrary(tema_core
        SOURCES
        core/expression.cpp
        core/module.cpp
        core/scope.cpp
        core/statement.cpp
        core/variable.cpp

        TESTS
        core/module_test.cpp
        core/statement_test.cpp
        core/scope_test.cpp)

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
        algorithms/print_utf8_test.cpp
        )

AddTemaLibrary(tema_modules
        SOURCES
        modules/propositional_logic.cpp

        DEPS
        tema_core
        )
