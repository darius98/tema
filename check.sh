#!/usr/bin/env sh

set -e

echo "\nConfigure & build project\n"
mkdir -p .build
cmake -DCMAKE_BUILD_TYPE=Debug -B.build/debug -H.
cmake --build .build/debug -j8

echo "\nRun tests\n"
LLVM_PROFILE_FILE=".build/unittests.profraw" ./.build/debug/tests/unittests --executor=smooth
llvm-profdata merge -sparse .build/unittests.profraw -o .build/unittests.profdata
llvm-cov show ./.build/debug/tests/unittests -instr-profile=.build/unittests.profdata \
  -show-branches=count \
  -show-line-counts \
  -use-color -Xdemangler c++filt -Xdemangler -n \
  -format=html > .build/coverage.html

echo "\nLLVM Coverage Report:\n"
llvm-cov report ./.build/debug/tests/unittests -instr-profile=.build/unittests.profdata
open .build/coverage.html
