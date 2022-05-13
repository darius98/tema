#!/usr/bin/env sh

set -e

CMAKE_BUILD_TYPE="${1:-Debug}"
BUILD_DIR="${2:-${CMAKE_BUILD_TYPE}}"

export CMAKE_BUILD_DIR=.build/${BUILD_DIR}
export CMAKE_INSTALL_DIR=${CMAKE_BUILD_DIR}/_install

echo "\nConfigure & build project\n"
mkdir -p ${CMAKE_BUILD_DIR}
cmake -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -B${CMAKE_BUILD_DIR} -H.
cmake --build ${CMAKE_BUILD_DIR} -j8

# We need the project installed before running the tests
cmake --build ${CMAKE_BUILD_DIR} --target install

echo "\nRun tests\n"
${CMAKE_BUILD_DIR}/run_tests.sh

echo "\nLLVM Coverage Report:\n"
cmake --build ${CMAKE_BUILD_DIR} --target coverage
