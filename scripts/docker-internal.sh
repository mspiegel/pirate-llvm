#!/bin/sh

# This configures and builds pirate-llvm inside a docker container.
set -e
type=$1

mkdir -p /root/build
cd /root/build
. ./docker-cmake.sh $type

export CCACHE_DIR=/root/cache/ccache
export CCACHE_COMPRESS=true
export CCACHE_MAXSIZE=400M
ninja -j 2 install-distribution
