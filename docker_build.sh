#!/usr/bin/env sh
set -e
docker build -t clarion-builder:latest .
docker run \
    --rm \
    --mount type=bind,src="$PWD",target=/workspace \
    --workdir /workspace \
    clarion-builder:latest bash -c "
        set -e
        mkdir -p build-docker
        cd build-docker
        cmake -DCMAKE_BUILD_TYPE=Release ..
        make -j
        ctest -j10
        mkdir -p clariondata
        NODE_PATH=dist CLARION_WASM_PATH=a.wasm node dist/clariond/index.js
"
