#!/bin/bash

# This configures and builds pirate-llvm inside a docker container.
set -e

mkdir -p /root/build
cd /root/build
export CCACHE_DIR=/root/cache/ccache
export CCACHE_COMPRESS=true
export CCACHE_MAXSIZE=400M
cmake -G Ninja \
      -DCMAKE_C_COMPILER=clang-9 \
      -DCMAKE_CXX_COMPILER=clang++-9 \
      -DCMAKE_INSTALL_PREFIX=/root/dist/pirate-llvm \
      -DCMAKE_BUILD_TYPE=Release \
      -DLLVM_BUILD_TOOLS=Off \
      -DLLVM_CCACHE_BUILD=On \
      -DLLVM_DISTRIBUTION_COMPONENTS="clang;lld;clang-resource-headers" \
      -DLLVM_ENABLE_PROJECTS="clang;lld" \
      -DLLVM_INCLUDE_EXAMPLES=Off \
      -DLLVM_INSTALL_TOOLCHAIN_ONLY=On \
      -DLLVM_TARGETS_TO_BUILD="X86;AArch64;ARM" \
      ../pirate-llvm/llvm
ninja -j 2 install-distribution
