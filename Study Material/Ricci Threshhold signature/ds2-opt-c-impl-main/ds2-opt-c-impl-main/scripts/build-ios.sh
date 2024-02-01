#!/bin/zsh

mkdir -p build-ios
cd build-ios
cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake \
 -DENABLE_BITCODE=0 -DPLATFORM=OS64 -DIS_IOS=1