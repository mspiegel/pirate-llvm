# This creates a ubuntu image with prereqs for building the pirate
# tools.  We include sphinx as that is needed for documentatiob.
FROM centos:7
MAINTAINER Joe Hendrix (jhendrix@galois.com)
WORKDIR /root

RUN yum install -y centos-release-scl \
 && yum install -y devtoolset-7-make devtoolset-7-gcc-c++ llvm-toolset-7-cmake \
                   devtoolset-7-libatomic-devel \
                   libedit-devel python-devel swig \
 && yum clean all \
 && rm -rf /var/cache/yum

ENV PATH="/opt/rh/llvm-toolset-7/root/bin:/opt/rh/devtoolset-7/root/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

# Install ccache 3.7.9
RUN curl -L https://github.com/ccache/ccache/releases/download/v3.7.9/ccache-3.7.9.tar.xz -o ccache.tar.xz \
 && tar xfJ ccache.tar.xz \
 && cd ccache-3.7.9 \
 && ./configure \
 && make \
 && make install \
 && cd .. \
 && rm -rf ccache.tar.xz ccache-3.7.9