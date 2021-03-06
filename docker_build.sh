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
        native/tests/clio/clio_tests
        NODE_PATH=dist CLARION_WASM_PATH=wasm/tests/web/a.wasm node dist/clariond/index.js
"
