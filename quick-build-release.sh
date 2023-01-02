#! /bin/bash

rm -rf cmake-build-release-my
mkdir cmake-build-release-my
cd cmake-build-release-my
cmake .. -DCMAKE_BUILD_TYPE=Release . 
make
cd .. 
./cmake-build-release-my/smirVideoTool
