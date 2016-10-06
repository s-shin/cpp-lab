#!/bin/bash
set -eu

cd deps/libuv
./gyp_uv.py -f xcode
xcodebuild -ARCHS="x86_64" -project uv.xcodeproj -configuration Release -target All
cd ../..
