# ClarionOS

![Build](https://github.com/bytemaster/clarion/actions/workflows/build.yml/badge.svg?branch=main)

![Clarion Logo](./logo.jpg)

-   Introduction: https://github.com/bytemaster/clarion/discussions/2
-   Discussions: https://eoscommunity.org/c/clarion
-   Telegram: https://t.me/clarionos

## Build

Set the `WASI_SDK_PREFIX` environment variable before building (see architecture-specific instructions below). Alternatively, use cmake's `-DWASI_SDK_PREFIX=....` option. Also make sure `nodejs 14`, `npm 6.14`, and `yarn 1.22` are in your path.

```sh
git submodule update --init --recursive
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j

ctest -j10
CLARION_WASM_PATH=a.wasm node dist/clariond

# to run the dev environment (it spins up and watches the PWA and ClarionD)
make dev
# open your browser on http://localhost:9025
```

## Ubuntu 20.04

```sh
sudo apt-get update
sudo apt-get install -yq     \
    binaryen                 \
    build-essential          \
    cmake                    \
    git                      \
    libboost-all-dev         \
    libssl-dev               \
    libgmp-dev

export WASI_SDK_PREFIX=~/work/wasi-sdk-12.0
export PATH=~/work/node-v14.16.0-linux-x64/bin:$PATH

cd ~/work
wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-12/wasi-sdk-12.0-linux.tar.gz
tar xf wasi-sdk-12.0-linux.tar.gz

wget https://nodejs.org/dist/v14.16.0/node-v14.16.0-linux-x64.tar.xz
tar xf node-v14.16.0-linux-x64.tar.xz
npm i -g yarn
```
