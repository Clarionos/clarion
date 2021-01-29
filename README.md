## setup 

git submodule update --init --recursive 
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
./tests/clio/clio_tests
