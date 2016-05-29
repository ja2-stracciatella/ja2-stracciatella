#!/usr/bin/env bash

apt-get update

# Build dependencies
apt-get install -y make gcc g++
apt-get install -y libsdl1.2-dev libboost-all-dev

# Deb Paackage
apt-get install -y git pbuilder debhelper

# Windows Cross-Compilation
apt-get install -y gcc-mingw-w64 g++-mingw-w64 zip
