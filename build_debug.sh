#!/bin/bash
if [ -d build ];
then
  rm -r build
fi
mkdir -p build
cd build
cmake ..
make -j20