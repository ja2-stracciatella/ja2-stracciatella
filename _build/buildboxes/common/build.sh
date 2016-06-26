#!/usr/bin/env bash

[ -d /tmp/build ] && rm -rf /tmp/build
mkdir /tmp/build
cd /tmp/build && cmake -DCMAKE_BUILD_TYPE=Release $2 /home/vagrant/strac
cd /tmp/build && make $1 -j2

# Save error code
e=$?

[ -d /home/vagrant/strac/releases ] || mkdir /home/vagrant/strac/releases
if test -n "$(find /tmp/build/ -maxdepth 1 -name 'ja2-stracciatella*')"
then
    cp /tmp/build/ja2-stracciatella* /home/vagrant/strac/releases
fi

exit ${e}
