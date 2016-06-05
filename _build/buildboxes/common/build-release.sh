#!/usr/bin/env bash

[ -d /tmp/build ] && rm /tmp/build
mkdir /tmp/build
cd /tmp/build && cmake -DCMAKE_BUILD_TYPE=Release $1 /home/vagrant/strac
cd /tmp/build && make package -j2

[ -d /home/vagrant/strac/releases ] || mkdir /home/vagrant/strac/releases
cp /tmp/build/ja2-stracciatella* /home/vagrant/strac/releases