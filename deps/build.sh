#!/bin/bash
set -eu

SCRIPT_DIR="$(cd $(dirname ${BASH_SOURCE:-$0}); pwd)"

cd "${SCRIPT_DIR}"

rm -rf include
mkdir include
rm -rf lib
mkdir lib

echo "### gflags ###"
cd src/gflags
mkdir -p build-ninja
cd build-ninja
cmake .. -GNinja
ninja
cp -R include/* ../../../include/
cp lib/* ../../../lib/
cd ../../..
echo "=> OK"

echo
echo "### libuv ###"
cd src/libuv
./gyp_uv.py -f ninja
ninja -C out/Release # or ninja -C out/Debug
cp out/Release/libuv.a ../../lib/
cp -R include/ ../../include/uv
cd ../..
echo "=> OK"

echo
echo "### asio ###"
cp -R src/asio/asio/include/asio include/asio
cp -R src/asio/asio/include/asio.hpp include/
echo "=> OK"