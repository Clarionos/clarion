## Build

```
git submodule update --init --recursive 
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
native/tests/clio/clio_tests
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
```

## Browser experiments

See [tests/web/README.md](tests/web/README.md)
