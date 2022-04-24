# Build with coverage
if (CMAKE_BUILD_TYPE MATCHES "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(COLLECT_COVERAGE_DEFAULT ON)
else ()
    set(COLLECT_COVERAGE_DEFAULT OFF)
endif ()
option(COLLECT_COVERAGE "Collect Clang source-based coverage" ${COLLECT_COVERAGE_DEFAULT})
