cmake_minimum_required(VERSION 3.22)
project(tema)

# Add the sanitizers globally, because of known false positives of ASan
# on macOS when only part of the executables is instrumented. See for example
# https://github.com/google/sanitizers/wiki/AddressSanitizerContainerOverflow#false-positives
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=undefined)
    add_link_options(-fsanitize=address,undefined)
endif ()
add_compile_options(-fvisibility=hidden)
add_link_options(-fvisibility=hidden)

include(cmake/MCGADeps.cmake)
include(cmake/ConfigFlex.cmake)
include(cmake/Coverage.cmake)
include(cmake/Tema.cmake)
include(cmake/Flags.cmake)

InitTemaTests()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)

AddTemaCoverageTarget()

add_custom_target(check ${CMAKE_SOURCE_DIR}/check.sh ${CMAKE_BUILD_TYPE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        USES_TERMINAL)

AddTemaExecutable(scratch_pad
        SOURCES scratch/scratch_pad.cpp
        DEPS tema_compiler tema_algorithms)
