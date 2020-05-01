# This creates a ubuntu image with prereqs for building clang
FROM ubuntu:latest
MAINTAINER Joe Hendrix (jhendrix@galois.com)
WORKDIR /root

# This gets a recent cmake from kitware and then also installs come common development tools.
RUN apt-get update \
 && apt-get install --no-install-recommends -y libstdc++-7-dev cmake libedit2 make binutils valgrind \
 && rm -rf /var/lib/apt/lists/*

COPY pirate-llvm /usr/local
RUN cd /usr/local/bin \
 && ln -s clang cc \
 && ln -s clang++ c++ \
 && ln -s lld ld
