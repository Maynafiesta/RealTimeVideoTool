#! /bin/bash

rm -rf cmake-build-debug-my
mkdir cmake-build-debug-my
cd cmake-build-debug-my
cmake .. -DCMAKE_BUILD_TYPE=Debug . 
make
cd .. 
./cmake-build-debug-my/smirVideoTool
