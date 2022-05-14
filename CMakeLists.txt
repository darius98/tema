cmake_minimum_required(VERSION 3.22)
project(tema)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
