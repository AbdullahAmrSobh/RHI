FROM ubuntu:18.04

LABEL maintainer="Ivan.Subotic@unibas.ch"

# Silence debconf messages
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections

# The CMake version which should be downloaded and installed
ENV CMAKE_VERSION 3.14.5

# Install build dependencies
RUN \
    sed -i 's/# \(.*multiverse$\)/\1/g' /etc/apt/sources.list && \
    apt-get -qq update && apt-get -y install \
    build-essential \
    git \
    libllvm-8-ocaml-dev libllvm8 llvm-8 llvm-8-dev llvm-8-doc llvm-8-examples llvm-8-runtime \
    clang-8 clang-tools-8 clang-8-doc libclang-common-8-dev libclang-8-dev libclang1-8 clang-format-8 python-clang-8 \
    libfuzzer-8-dev \
    lldb-8 \
    lld-8 \
    libc++-8-dev libc++abi-8-dev \
    libomp-8-dev \
    g++-7 \
    libwebsocketpp-dev \
    ninja-build \
    make \
    wget

ENV CC clang-8
ENV CXX clang++-8

# Install newer CMake version (Ubuntu 18.04 default is CMake 3.10.2)
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.tar.gz && \
    tar -zxvf cmake-${CMAKE_VERSION}.tar.gz && \
    cd cmake-${CMAKE_VERSION} && \
    ./bootstrap && \
    make && \
    make install

CMD ["bash"]