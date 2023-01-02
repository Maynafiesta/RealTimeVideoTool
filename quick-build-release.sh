#! /bin/bash

rm -rf cmake-build-release-my
mkdir cmake-build-release-my
cd cmake-build-release-my || exit
cmake .. -DCMAKE_BUILD_TYPE=Release . 
make -j "$(nproc)"
cd ..  || exit
./cmake-build-release-my/smirVideoTool
