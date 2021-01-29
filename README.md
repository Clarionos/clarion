## setup 

git submodule update --init --recursive 
cmake -DCMAKE_BUILD_TYPE=Release .
make
./tests/clio/clio_tests
