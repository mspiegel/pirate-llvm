# This creates a ubuntu image with prereqs for building the pirate
# tools.  We include sphinx as that is needed for documentatiob.
FROM ubuntu:latest
MAINTAINER Joe Hendrix (jhendrix@galois.com)

RUN apt-get update \
 && apt-get install --no-install-recommends -y apt-transport-https ca-certificates gnupg software-properties-common wget \
 && wget -qO - https://apt.kitware.com/keys/kitware-archive-latest.asc | apt-key add - \
 && apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main' \
 && apt-get update \
 && apt-get install --no-install-recommends -y ccache clang-9 cmake git make ninja-build python3-pip \
 && rm -rf /var/lib/apt/lists/*

# Install sphinx
RUN pip3 install -U Sphinx

# Add links
RUN ln -s /usr/bin/clang-9 /usr/local/bin/cc \
 && ln -s /usr/bin/clang++-9 /usr/local/bin/c++
