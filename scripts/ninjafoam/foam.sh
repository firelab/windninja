#!/bin/bash

cd OpenFOAM-2.2.x
git checkout 8a983dba63b246772c69ed0fa9cc3b3e33a10f92
wget http://www.symscape.com/files/articles/openfoam22x-windows/v3-mingw-openfoam-2-2-x.patch.zip
unzip v3-mingw-openfoam-2-2-x.patch.zip
patch -p0 < v3-mingw-openfoam-2-2-x.patch
rm v3-mingw-openfoam-2-2-x.patch*
# Fix paths...
export FOAM_INST_DIR="$HOME/src/openfoam/mingw";
source $FOAM_INST_DIR/OpenFOAM-2.2.x/etc/bashrc \
       WM_OSTYPE=MSwindows \
       WM_COMPILER=mingw-w64 \
       WM_ARCH_OPTION=64 \
       WM_PRECISION_OPTION=SP \
       WM_CC=x86_64-w64-mingw32-gcc \
       WM_CXX=x86_64-w64-mingw32-g++ \
       compilerInstall=system \
       WM_MPLIB=MSMPI \
       MPI_ARCH_PATH=/home/kyle/Desktop/msi

source $FOAM_INST_DIR/OpenFOAM-2.2.x/etc/bashrc
cd $WM_PROJECT_DIR/wmake/src
make
ln -s $WM_PROJECT_DIR/wmake/platforms/linux64Gcc \
      $WM_PROJECT_DIR/wmake/platforms/linux64mingw-w64

cd $WM_PROJECT_DIR
find . -name Allwmake -exec chmod +x '{}' \;
./Allwmake
