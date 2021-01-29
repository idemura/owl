#! /bin/bash

find src -perm +111 -type f -name '*_test' -exec {} \;
