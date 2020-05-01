FROM centos:7
MAINTAINER Joe Hendrix (jhendrix@galois.com)
WORKDIR /root

#RUN yum -y install \
#  libstdc++ libstdc++-devel libedit make binutils bzip2 gcc

# valgrind is not in the repo and doesn't offer prebuilt binaries, so do it from source
# RUN curl -L https://sourceware.org/pub/valgrind/valgrind-3.15.0.tar.bz2 | tar xj \
#   && cd valgrind-3.15.0 \
#   && ./configure \
#   && make \
#   && make install
RUN yum install -y centos-release-scl \
 && yum install -y devtoolset-7-make llvm-toolset-7-cmake \
                   libedit \
 && yum clean all \
 && rm -rf /var/cache/yum

COPY pirate-llvm /usr/local
RUN cd /usr/local/bin \
 && ln -s clang cc \
 && ln -s clang++ c++ \
 && ln -s lld ld