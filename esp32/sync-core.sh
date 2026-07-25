#!/usr/bin/env bash
# Syncs the shared emulator core (CPU/PPU/Bus/etc.) into this PlatformIO
# project. Run this after any change to the desktop core in ../src or
# ../include/gb.
#
# This is a plain copy, not a symlink -- this Windows account doesn't have
# Developer Mode enabled, which Windows requires for directory symlinks
# without admin rights. That means these files WILL go stale if you edit the
# desktop core and forget to rerun this script. Enabling Developer Mode
# (Settings > Privacy & Security > For Developers) would let us switch to a
# true symlink later and remove this footgun entirely.
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
