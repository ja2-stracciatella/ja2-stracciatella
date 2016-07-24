#!/usr/bin/env bash

apt-get update

# Build dependencies
apt-get install -y cmake make gcc g++ gcc-mingw-w64 g++-mingw-w64 libsdl1.2-dev nsis
