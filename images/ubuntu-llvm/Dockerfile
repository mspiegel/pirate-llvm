# This creates a ubuntu image with prereqs for building clang
FROM ubuntu:latest
MAINTAINER Joe Hendrix (jhendrix@galois.com)
RUN apt-get update \
 && apt-get install --no-install-recommends -y cmake make \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /root

COPY pirate-llvm /usr/local
RUN cd /usr/local/bin && ln -s clang cc && ln -s clang++ c++
