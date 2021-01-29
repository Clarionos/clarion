## setup 

git submodule update --init --recursive 
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
./tests/clio/clio_tests

## Ubuntu 20.04

```
sudo apt-get update
sudo apt-get install -yq     \
    binutils-dev             \
    bison                    \
    build-essential          \
    cmake                    \
    flex                     \
    git                      \
    gperf                    \
    libboost-all-dev         \
    libcap-dev               \
    libdouble-conversion-dev \
    libdwarf-dev             \
    libelf-dev               \
    libevent-dev             \
    libgflags-dev            \
    libgoogle-glog-dev       \
    libgmp-dev               \
    libiberty-dev            \
    libjemalloc-dev          \
    libkrb5-dev              \
    liblz4-dev               \
    liblzma-dev              \
    libnuma-dev              \
    libsasl2-dev             \
    libsnappy-dev            \
    libsodium-dev            \
    libssl-dev               \
    libtool                  \
    libunwind8-dev           \
    libzstd-dev              \
    m4                       \
    pkg-config               \
    unzip                    \
    wget                     \
    zlib1g-dev
```
