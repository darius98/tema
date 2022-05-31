#!/usr/bin/env sh

set -e

CHECK_TYPE=${1:-test}
CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Debug}"

export CMAKE_BUILD_DIR=.build/${CMAKE_BUILD_TYPE}
export CMAKE_INSTALL_DIR=${CMAKE_BUILD_DIR}/_install

echo "\nConfigure\n"
mkdir -p ${CMAKE_BUILD_DIR}
cmake -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -B${CMAKE_BUILD_DIR} -H.

case "${CHECK_TYPE}" in
  test)
    echo "\nBuild\n"
    cmake --build ${CMAKE_BUILD_DIR} -j8

    # We need the project installed before running the tests
    echo "\nInstall locally\n"
    cmake --build ${CMAKE_BUILD_DIR} --target install

    echo "\nRun tests\n"
    ${CMAKE_BUILD_DIR}/run_tests.sh

    echo "\nLLVM Coverage Report:\n"
    cmake --build ${CMAKE_BUILD_DIR} --target coverage
    echo "\nFull coverage available at file://$(realpath ${CMAKE_BUILD_DIR}/coverage.html)"
    ;;

  analyze)
    echo "\nRunning Clang-Tidy\n"
    cmake --build ${CMAKE_BUILD_DIR} --target tidy
    ;;

  *)
    echo '\nUnknown check type "'${CHECK_TYPE}'". Supported: "test", "analyze".\n'
    exit 1
    ;;
esac
