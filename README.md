## Build

Set the `WASI_SDK_PREFIX` environment variable before building (see architecture-specific instructions below). Alternatively, use cmake's `-DWASI_SDK_PREFIX=....` option. Also make sure `nodejs 14`, `npm 6.14`, and `yarn 1.22` are in your path.

```
git submodule update --init --recursive 
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j

native/tests/clio/clio_tests
mkdir -p clariondata
NODE_PATH=dist CLARION_WASM_PATH=wasm/tests/web/a.wasm node dist/clariond/index.js
NODE_PATH=dist node dist/clintrinsics/tester.js wasm/tests/clintrinsics/test-clintrinsics.wasm
NODE_PATH=dist node dist/clintrinsics/tester.js wasm/tests/clio/clio_tests.wasm
```

## Ubuntu 20.04

```
sudo apt-get update
sudo apt-get install -yq     \
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
