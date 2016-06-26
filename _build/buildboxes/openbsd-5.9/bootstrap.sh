#!/bin/sh

# installing missing xbase55
test -f /usr/X11R6/README || (cd /tmp && curl -O http://ftp.openbsd.org/pub/OpenBSD/5.9/amd64/xbase59.tgz)
test -f /usr/X11R6/README || (cd /tmp && tar -C / -xzphf xbase59.tgz)

pkg_add -v gmake cmake sdl boost
