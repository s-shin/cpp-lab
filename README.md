C++ Laboratory

```sh
# Build dependencies
git submodule update --init
deps/prepare.sh build

# Build project
mkdir build
cd build
cmake .. -GNinja
ninja
ls ../bin
```
