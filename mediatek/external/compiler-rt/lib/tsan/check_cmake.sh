#!/bin/bash
set -u
set -e

ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
mkdir -p $ROOT/build
cd $ROOT/build
CC=clang CXX=clang++ cmake -DLLVM_ENABLE_WERROR=ON -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_ASSERTIONS=ON $ROOT/../../../..
make -j64
make check-sanitizer -j64
make check-tsan -j64
make check-asan -j64
make check-msan -j64
