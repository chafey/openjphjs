#!/bin/sh
mkdir -p build
#(cd build && emconfigure cmake -DCMAKE_BUILD_TYPE=Debug ..)
(cd build && emconfigure cmake ..)
(cd build && emmake make VERBOSE=1 -j 16)
cp ./build/src/openjphjs.js ./dist
cp ./build/src/openjphjs.wasm ./dist
(cd test/node; npm run test)
