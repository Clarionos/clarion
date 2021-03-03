Install wasi-sdk in Ubuntu:

```
WASI_SDK_PATH=~/work/wasi-sdk-12.0

cd ~/work
wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-12/wasi-sdk-12.0-linux.tar.gz
tar xf wasi-sdk-12.0-linux.tar.gz
```

Compile a.cpp:

```
cd tests/web

$WASI_SDK_PATH/bin/clang++ --sysroot=$WASI_SDK_PATH/share/wasi-sysroot a.cpp -o a.wasm -Os -Wl,--export-table -std=c++20
```
