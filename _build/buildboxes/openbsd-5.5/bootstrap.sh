#!/bin/sh

# installing missing xbase55
test -f /usr/X11R6/README || (cd /tmp && curl -O ftp://ftp.openbsd.org/pub/OpenBSD/5.5/amd64/xbase55.tgz)
test -f /usr/X11R6/README || (cd /tmp && sudo tar -C / -xzphf xbase55.tgz)

test -f /usr/local/include/SDL/SDL.h || sudo PKG_PATH=ftp://ftp.openbsd.org/pub/OpenBSD/5.5/packages/amd64/ pkg_add -v sdl-1.2.15p6
test -f /usr/local/bin/gmake         || sudo PKG_PATH=ftp://ftp.openbsd.org/pub/OpenBSD/5.5/packages/amd64/ pkg_add -v gmake
test -f /usr/local/bin/egcc          || sudo PKG_PATH=ftp://ftp.openbsd.org/pub/OpenBSD/5.5/packages/amd64/ pkg_add -v gcc-4.6.4p7
test -f /usr/local/bin/eg++          || sudo PKG_PATH=ftp://ftp.openbsd.org/pub/OpenBSD/5.5/packages/amd64/ pkg_add -v g++-4.6.4p4
