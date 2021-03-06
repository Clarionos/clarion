#!/usr/bin/env sh
set -e
docker build -t clarion-builder:latest .
docker run \
    --rm \
    --mount type=bind,src="$PWD",target=/workspace \
    --workdir /workspace \
    clarion-builder:latest bash -c "
        set -e
        export WASI_SDK_PREFIX=/opt/wasi-sdk-12.0
        export PATH=\"/opt/node-v14.16.0-linux-x64/bin:$PATH\"
        mkdir -p build
        cd build
        cmake -DCMAKE_BUILD_TYPE=Release ..
        # make -j
        make
        native/tests/clio/clio_tests
"