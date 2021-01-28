#! /bin/bash

find . -iname '*.o' -exec rm {} \;
find . -iname '*.a' -exec rm {} \;
find . -iname '.ninja_*' -exec rm {} \;
find src -perm +111 -type f -exec rm {} \;
rm -f src/ninja_build
