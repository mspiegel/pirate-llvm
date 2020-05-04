# This creates a ubuntu image with prereqs for building the pirate
# tools.  We include sphinx as that is needed for documentatiob.
FROM ubuntu:focal
MAINTAINER Joe Hendrix (jhendrix@galois.com)
WORKDIR /root

ENV TZ UTC
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update \
 && apt-get install --no-install-recommends -y \
            ccache clang-9 cmake git libedit-dev lld-9 make python3-dev \
            python3-pip swig \
 && rm -rf /var/lib/apt/lists/*

# Install sphinx
RUN pip3 install --no-cache-dir -U Sphinx

# Add links
RUN ln -s /usr/bin/clang-9 /usr/local/bin/cc \
 && ln -s /usr/bin/clang++-9 /usr/local/bin/c++ \
 && ln -s /usr/bin/ld.lld-9 /usr/local/bin/ld
