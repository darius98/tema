cmake_minimum_required(VERSION 3.22)
project(tema)

include(cmake/MCGADeps.cmake)
include(cmake/ConfigFlex.cmake)
include(cmake/Coverage.cmake)
include(cmake/Tema.cmake)
include(cmake/Flags.cmake)

InitTemaTests()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

AddTemaCoverageTarget()

AddTemaExecutable(scratch_pad
        SOURCES scratch/scratch_pad.cpp
        DEPS tema_compiler tema_algorithms)
