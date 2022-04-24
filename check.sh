#!/usr/bin/env sh

set -e

CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Debug}"
BUILD_DIR="${BUILD_DIR:-${CMAKE_BUILD_TYPE}}"

BUILD_DIR_FULL=.build/${BUILD_DIR}

echo "\nConfigure & build project\n"
mkdir -p ${BUILD_DIR_FULL}
cmake -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -B${BUILD_DIR_FULL} -H.
cmake --build ${BUILD_DIR_FULL} -j8

echo "\nRun tests\n"
${BUILD_DIR_FULL}/run_tests.sh

echo "\nLLVM Coverage Report:\n"
cmake --build ${BUILD_DIR_FULL} --target coverage
