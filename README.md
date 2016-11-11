C++ Laboratory

```sh
# Build dependencies
git submodule update --init
deps/build.sh

# Build project
mkdir build
cd build
cmake .. -GNinja
ninja
ls ../bin
```
