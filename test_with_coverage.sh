#!/usr/bin/env sh

cmake --build .build/debug --target unittests
LLVM_PROFILE_FILE=".build/unittests.profraw" ./.build/debug/tests/unittests --executor=smooth
llvm-profdata merge -sparse .build/unittests.profraw -o .build/unittests.profdata
llvm-cov show ./.build/debug/tests/unittests -instr-profile=.build/unittests.profdata \
  -show-branches=count \
  -show-line-counts \
  -use-color -Xdemangler c++filt -Xdemangler -n \
  -format=html > .build/coverage.html
llvm-cov report ./.build/debug/tests/unittests -instr-profile=.build/unittests.profdata
open .build/coverage.html
