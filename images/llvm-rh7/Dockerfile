FROM registry.access.redhat.com/ubi7/ubi

RUN yum -y install \
  libstdc++ libstdc++-devel libedit make binutils bzip2 gcc

# valgrind is not in the repo and doesn't offer prebuilt binaries, so do it from source
# RUN curl -L https://sourceware.org/pub/valgrind/valgrind-3.15.0.tar.bz2 | tar xj \
#   && cd valgrind-3.15.0 \
#   && ./configure \
#   && make \
#   && make install

# Get prebuilt binary cmake
RUN curl -L https://github.com/Kitware/CMake/releases/download/v3.16.5/cmake-3.16.5-Linux-x86_64.tar.gz | tar zxv \
  && ln -s /root/cmake-3.16.5-Linux-x86_64/bin/cmake /usr/local/bin/cmake

COPY pirate-llvm /usr/local
RUN cd /usr/local/bin \
 && ln -s clang cc \
 && ln -s clang++ c++ \
 && ln -s lld ld

WORKDIR /root
