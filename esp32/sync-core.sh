#!/usr/bin/env bash
# Syncs the shared emulator core (CPU/PPU/Bus/etc.) into this PlatformIO
# project. Run this after any change to the desktop core in ../src or
# ../include/gb.

set -e
cd "$(dirname "$0")"

mkdir -p lib/gbcore/src lib/gbcore/include

for f in ../src/*.cpp; do
    base=$(basename "$f")
    if [ "$base" != "main.cpp" ]; then
        cp "$f" "lib/gbcore/src/$base"
    fi
done

cp ../include/gb/*.h lib/gbcore/include/

echo "Synced $(ls lib/gbcore/src/*.cpp | wc -l) source files and $(ls lib/gbcore/include/*.h | wc -l) headers into lib/gbcore/"
