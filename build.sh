#! /bin/bash

set -e

if [ -z "$OPT" ] || [ "$OPT" -ne "0" ]; then
    echo "Debug build"
    FILE=ninja_dbg
else
    FILE=ninja_opt
fi

rm -f src/ninja_build && ./generate_ninja.py
ninja -C src -f $FILE
