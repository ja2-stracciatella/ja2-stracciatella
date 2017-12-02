#!/bin/sh
#

BUNDLE="`echo "$0" | sed -e 's/\/Contents\/MacOS\/JA2 Stracciatella//'`"
RESOURCES="$BUNDLE/Contents/Resources"

echo "running $0"
echo "BUNDLE: $BUNDLE"
echo "RESOURCES: $RESOURCES"

exec "$RESOURCES/ja2-launcher"
