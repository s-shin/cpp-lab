## Build

If homebrew says

```
For compilers to find this software you may need to set:
    LDFLAGS:  -L/usr/local/opt/openssl/lib
    CPPFLAGS: -I/usr/local/opt/openssl/include
```

then `OPENSSL_INCLUDE_DIR` and `OPENSSL_LIBRARIES` should be specified.

Example:

```sh
cmake .. -GNinja -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl
```
