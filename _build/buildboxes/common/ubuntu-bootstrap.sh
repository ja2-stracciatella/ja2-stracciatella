#!/usr/bin/env bash

add-apt-repository ppa:likemartinma/devel
apt-get update

# Build dependencies
apt-get install -y cmake make gcc g++ gcc-mingw-w64 g++-mingw-w64 libsdl2-dev nsis
