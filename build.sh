#! /bin/bash

set -e

if [ -z "$OPT" ] || [ "$OPT" -ne "0" ]; then
  FILE=ninja_opt
else
  FILE=ninja_opt
fi

rm -f src/ninja_build && ./generate_ninja.py
ninja -C src -f $FILE
