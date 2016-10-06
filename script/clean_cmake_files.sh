#!/bin/bash
set -eu

rm -r $(find . \( -name "CMakeFiles" -o -name "CMakeCache.txt" -o -name "cmake_install.cmake" -o -name "Makefile" \))
