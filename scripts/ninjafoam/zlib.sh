#!/bin/bash

# This script downloads and cross compiles zlib.
#
# Prerequesites: apt.sh

mkdir zlib
cd zlib
wget http://zlib.net/zlib-1.2.8.tar.gz
tar -xvf zlib-1.2.8.tar.gz
cd zlib-1.2.8

sed -e s/"PREFIX ="/"PREFIX = x86_64-w64-mingw32-"/ -i win32/Makefile.gcc
make -f win32/Makefile.gcc
# Check these
sudo BINARY_PATH=/usr/lib/gcc/i686-w64-mingw32/4.8 \
     INCLUDE_PATH=/usr/lib/gcc/i686-w64-mingw32/4.8/include \
     LIBRARY_PATH=/usr/lib/gcc/i686-w64-mingw32/4.8 \
     make -f win32/Makefile.gcc install
cd ..

