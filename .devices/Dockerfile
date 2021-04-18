# Arch has poackages that are actually up to date and configurations are simpler because of it.
# this "lopsided" image is amd64 + arm64 which is perfect until we must support riscv64

FROM lopsided/archlinux

COPY . /clarion

ENV WASI_SDK_PREFIX=/opt/wasi-sdk

RUN pacman -Syyu --noconfirm base-devel binaryen cmake curl git boost git openssl gmp clang ninja yarn python

RUN git clone https://github.com/WebAssembly/wasi-sdk/ --branch wasi-sdk-12 --recursive

RUN cd wasi-sdk && \
        make -j24 && \
        cp -r build/install/opt/wasi-sdk /opt

RUN cd /clarion && \
                mkdir build && \
                cd build && \
                cmake -DCMAKE_BUILD_TYPE=Release .. && \
                make -j$(nproc) && \
                ctest -j$(nproc) && \
                mkdir -p clariondata
