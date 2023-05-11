#!/bin/bash
if [ -d build ];
then
  rm -r build
fi
if [ -d db ];
then
  rm -r db
fi
mkdir -p build
cd build
cmake -D CMAKE_BUILD_TYPE=Release ..
make -j20