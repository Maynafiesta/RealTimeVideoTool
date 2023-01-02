#! /bin/bash

rm -rf cmake-build-debug-my
mkdir cmake-build-debug-my
cd cmake-build-debug-my || exit
cmake .. -DCMAKE_BUILD_TYPE=Debug . 
make -j "$(nproc)"
cd .. || exit
./cmake-build-debug-my/smirVideoTool
