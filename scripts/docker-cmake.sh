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

LDFLAGS="-Wl,--gc-sections" \
  cmake \
      -DCMAKE_BUILD_TYPE=$type \
      -DCMAKE_C_FLAGS="-ffunction-sections -fdata-sections" \
      -DCMAKE_CXX_FLAGS="-ffunction-sections -fdata-sections" \
      -DCMAKE_INSTALL_PREFIX=/root/dist/pirate-llvm \
      -DLLVM_BUILD_TOOLS=Off \
      -DLLVM_CCACHE_BUILD=On \
      -DLLVM_DISTRIBUTION_COMPONENTS="clang;clang-libraries;clang-resource-headers;lld;lldb;liblldb" \
      -DLLVM_ENABLE_PROJECTS="clang;lld;lldb" \
      -DLLVM_INCLUDE_EXAMPLES=Off \
      -DLLVM_INSTALL_TOOLCHAIN_ONLY=On \
      -DLLVM_TARGETS_TO_BUILD="X86;AArch64;ARM" \
      $CMAKE_FLAGS \
      ../pirate-llvm/llvm

#      \
