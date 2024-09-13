# libgeohex

GeoHex (https://geohex.org/) v3.2 C99 implementation.

### install

Require: CMake, Make, C99 compiler (gcc, clang)

```
$ git clone "https://github.com/zeriyoshi/libgeohex.git" "libgeohex"
$ cd "libgeohex"
$ mkdir "build"
$ cd "build"
$ cmake ..
$ make -j"$(nproc)"
$ sudo make install
```

### usage

include `geohex/geohex.h` and link `-lgeohex`
