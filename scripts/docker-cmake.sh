#!/bin/sh

# This configures and builds pirate-llvm inside a docker container.
set -e
type=$1
case $type in
    Release) ;;
    Debug) ;;
    *) echo "Expected build type of Debug or Release"
       exit
       ;;
esac

CMAKE_FLAGS=""

# Use ld.lld if it is in path.
if type -P ld.lld; then
    CMAKE_FLAGS="-DLLVM_USE_LINKER=lld"
fi

cmake -DCMAKE_BUILD_TYPE=$type \
      -DCMAKE_INSTALL_PREFIX=/root/dist/pirate-llvm \
      -DLLVM_CCACHE_BUILD=On \
      -DLLVM_ENABLE_PROJECTS="clang;lld;lldb" \
      -DLLVM_INCLUDE_EXAMPLES=Off \
      -DLLVM_TARGETS_TO_BUILD="X86;AArch64;ARM" \
      $CMAKE_FLAGS \
      ../pirate-llvm/llvm