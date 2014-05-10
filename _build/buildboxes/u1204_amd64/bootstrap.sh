#!/usr/bin/env bash

apt-get update

apt-get install -y git

# that's for a simple build
apt-get install -y make gcc g++
apt-get install -y libsdl1.2-dev

# that's for building deb package
apt-get install -y pbuilder debhelper

# that's for building and packaging win release
apt-get install -y gcc-mingw-w64 g++-mingw-w64
apt-get install -y zip
