#!/bin/bash

# Zesty (17.04), LLVM 3.7
VER=zesty
LLVM_VER=3.7

echo "deb http://llvm.org/apt/$VER/ llvm-toolchain-$VER-$LLVM_VER main
deb-src http://llvm.org/apt/$VER/ llvm-toolchain-$VER-$LLVM_VER main" | \
  sudo tee /etc/apt/sources.list.d/llvm.list
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get update

# All versions
sudo apt-get -y install bison build-essential cmake flex git libedit-dev \
  libllvm$LLVM_VER llvm-$LLVM_VER-dev libclang-$LLVM_VER-dev python zlib1g-dev libelf-dev

# For Lua support
sudo apt-get -y install luajit luajit-5.1-dev

# Needed to statically define USDT probes
sudo apt-get -y install systemtap-sdt-dev
