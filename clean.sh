#! /bin/bash

find . -name '*.o' -exec rm {} \;
find . -name '*.a' -exec rm {} \;
find . -name '*.out' -exec rm {} \;
