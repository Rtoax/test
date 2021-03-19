#!/bin/bash
. 0-basc.sh
mkdir build && cd build
cmake $CMAKE_OPTIONS
ninja -j 64
ninja install
