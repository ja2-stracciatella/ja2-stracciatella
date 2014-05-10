#!/usr/bin/env bash

apt-get update

apt-get install -y git

# that's for a simple build
apt-get install -y make gcc g++
apt-get install -y libsdl1.2-dev

# that's for building deb package
apt-get install -y pbuilder debhelper
