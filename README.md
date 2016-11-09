C++ Laboratory

```sh
git submodule update --init

### Build dependencies

# gflags
cd deps/gflags/
mkdir build-ninja
cd build-ninja
cmake .. -GNinja
ninja
mv include/* ../../include/
mv lib/* ../../lib/
cd ../../..

# libuv
cd deps/libuv
./gyp_uv.py -f ninja
ninja -C out/Release # or ninja -C out/Debug
cp build/Release/libuv.a ../lib/
cp -R include/ ../include/uv
cd ../..

# asio
cp -R deps/asio/asio/include/asio deps/include/asio
cp -R deps/asio/asio/include/asio.hpp deps/include/

### Build project

mkdir build
cd build
cmake .. -GNinja
ninja
ls ../bin
```
