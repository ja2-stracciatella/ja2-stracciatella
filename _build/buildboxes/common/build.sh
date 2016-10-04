#!/usr/bin/env bash

[ -d /tmp/build ] && rm -rf /tmp/build
mkdir /tmp/build
cd /tmp/build && cmake -DCMAKE_BUILD_TYPE=Release $2 /tmp/ja2-stracciatella && make $1 -j2

# Save error code
e=$?

[ -d /tmp/ja2-stracciatella/releases ] || mkdir /tmp/ja2-stracciatella/releases
if test -n "$(find /tmp/build/ -maxdepth 1 -name 'ja2-stracciatella*')"
then
    cp /tmp/build/ja2-stracciatella* /tmp/ja2-stracciatella/releases
fi

exit ${e}
