#! /bin/bash

find . -iname '*.h' -exec clang-format -i {} \;
find . -iname '*.c' -exec clang-format -i {} \;
find . -iname '*.hpp' -exec clang-format -i {} \;
find . -iname '*.cpp' -exec clang-format -i {} \;
