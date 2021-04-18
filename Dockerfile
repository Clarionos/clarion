FROM ubuntu:focal

COPY . /clarion

ENV WASI_SDK_PREFIX=/opt/wasi-sdk-12.0
ENV PATH=/opt/node-v14.16.0-linux-x64/bin:$PATH

RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get update \
    && apt-get install -yq  \
        build-essential \
        binaryen \
        cmake \
        curl \
        git \
        libboost-all-dev \
        libssl-dev \
        libgmp-dev \
    && apt-get clean -yq \
    && rm -rf /var/lib/apt/lists/*

RUN cd /opt \
    && curl -LO https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-12/wasi-sdk-12.0-linux.tar.gz \
    && tar xf wasi-sdk-12.0-linux.tar.gz \
    && curl -LO https://nodejs.org/dist/v14.16.0/node-v14.16.0-linux-x64.tar.xz \
    && tar xf node-v14.16.0-linux-x64.tar.xz \
    && npm i -g yarn

RUN cd /clarion && \
        mkdir build && \
        cd build && \
        cmake -DCMAKE_BUILD_TYPE=Release .. && \
        make -j$(nproc) && \
        ctest -j$(nproc) && \
        mkdir -p clariondata
