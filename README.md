# info
## language name:
Qnnn
## file extension:
.qn
# use
```sh
python ./build.py ./include ./src ./build/app.exe
```
or
```sh
g++ ./include/lexer.h ./include/parser.h ./include/qlib.h ./src/lexer.cpp ./src/main.cpp ./src/parser.cpp ./src/qlib.cpp -o ./build/app.exe
```
# environment
## compiler
```sh
$ g++ -v
使用內建 specs。
COLLECT_GCC=/usr/bin/g++
COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-pc-msys/13.2.0/lto-wrapper.exe
目的：x86_64-pc-msys
配置為：/c/S/B/src/gcc-13.2.0/configure --build=x86_64-pc-msys --prefix=/usr --libexecdir=/usr/lib --enable-bootstrap --enable-shared --enable-shared-libgcc --enable-static --enable-version-specific-runtime-libs --with-arch=nocona --with-tune=generic --disable-multilib --enable-__cxa_atexit --with-dwarf2 --enable-languages=c,c++,lto --enable-graphite --enable-threads=posix --enable-libatomic --enable-libgomp --disable-libitm --enable-libquadmath --enable-libquadmath-support --disable-libssp --disable-win32-registry --disable-symvers --with-gnu-ld --with-gnu-as --disable-isl-version-check --enable-checking=release --without-libiconv-prefix --without-libintl-prefix --with-system-zlib --enable-linker-build-id --enable-libstdcxx-filesystem-ts
執行緒模型：posix
支援的 LTO 壓縮演算法：zlib
gcc 版本 13.2.0 (GCC)
```
## cpp version:
__cplusplus = 201703L
