# This creates a ubuntu image with prereqs for building the pirate
# tools.  We include sphinx as that is needed for documentatiob.
FROM ubuntu:latest
MAINTAINER Joe Hendrix (jhendrix@galois.com)

RUN apt-get update \
 && apt-get install --no-install-recommends -y ccache clang-9 cmake git ninja-build python3-pip \
 && rm -rf /var/lib/apt/lists/*

# Install sphinx
RUN pip3 install -U Sphinx
