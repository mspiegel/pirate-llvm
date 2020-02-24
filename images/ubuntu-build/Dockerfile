# This creates a ubuntu image with prereqs for building clang
FROM ubuntu:latest
MAINTAINER Joe Hendrix (jhendrix@galois.com)
RUN apt-get update \
 && apt-get install --no-install-recommends -y ccache clang-9 cmake git ninja-build python3 \
 && rm -rf /var/lib/apt/lists/*