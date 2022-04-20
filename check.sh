#!/usr/bin/env sh

set -e

echo "\nConfigure & build project\n"
mkdir -p .build/debug
cmake -DCMAKE_BUILD_TYPE=Debug -B.build/debug -H.
cmake --build .build/debug -j8

echo "\nRun tests\n"
.build/debug/tests/run_tests.sh
cmake --build .build/debug --target coverage_collect
cmake --build .build/debug --target coverage

echo "\nLLVM Coverage Report:\n"
cmake --build .build/debug --target coverage_summary
