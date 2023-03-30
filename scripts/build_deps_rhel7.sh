#!/bin/bash

SOURCE=~/sources/windninja
WINDNINJA_VER=3.8.0
PREFIX=~/apps/windninja/$WINDNINJA_VER
MAKE_OPTIONS=-j10

POPPLER="poppler-0.23.4"
SQLITE="sqlite-snapshot-202301131932"
PROJ="proj-6.3.2"
GDAL="gdal-3.4.3"

mkdir -p $SOURCE
mkdir -p $PREFIX

# Clear variable 
export LD_LIBRARY_PATH

# Dependencies
os_release=$(cat /etc/system-release-cpe | cut -d ':' -f5)
if [[ ${os_release::1} -ge 8 ]]
then
  echo "$os_release >= 8"
  sudo dnf install -y pkgconf-pkg-config fontconfig-devel libcurl-devel netcdf-devel boost-devel geos-devel libsqlite3x-devel jasper-devel wget
  export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
else
  echo "$os_release < 8"
  sudo yum install -y pkgconfig fontconfig-devel libcurl-devel netcdf-devel boost-devel geos-devel libsqlite3x-devel jasper-devel wget
  export PKG_CONFIG_PATH=/usr/lib64/pkgconfig
fi

cd $SOURCE

# Get and build poppler for PDF support in GDAL
wget https://poppler.freedesktop.org/$POPPLER.tar.xz
tar -xvf $POPPLER.tar.xz
cd $POPPLER/
make clean
./configure --prefix=$PREFIX --enable-xpdf-headers
make $MAKE_OPTIONS
make install
cd ..


# Get and build sqlite3
wget https://www.sqlite.org/snapshot/$SQLITE.tar.gz
tar -xvf $SQLITE.tar.gz
cd $SQLITE
CFLAGS="-DSQLITE_ENABLE_COLUMN_METADATA=1"
make clean
./configure --prefix=$PREFIX
make $MAKE_OPTIONS
make install
cd ..


# Get and build proj
wget https://download.osgeo.org/proj/$PROJ.tar.gz
tar -xvf $PROJ.tar.gz
cd $PROJ
# rhel 8: export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
# rhel 7: export PKG_CONFIG_PATH=/usr/lib64/pkgconfig
make clean
export SQLITE3_CFLAGS="-I$PREFIX/include"
export SQLITE3_LIBS="-L$PREFIX/lib -lsqlite3"
./configure --prefix=$PREFIX
make $MAKE_OPTIONS
make install
cd ..


# Get and build GDAL with poppler support
wget http://download.osgeo.org/gdal/${GDAL:5}/$GDAL.tar.gz
tar -xvf $GDAL.tar.gz
cd $GDAL
make clean
./configure --prefix=$PREFIX --with-poppler=$PREFIX --with-proj=$PREFIX  --with-sqlite3=$PREFIX --without-idb
make $MAKE_OPTIONS
make install
cd ..


# Install windninja
curl -L https://github.com/firelab/windninja/archive/refs/tags/$WINDNINJA_VER.tar.gz --output windninja-$WINDNINJA_VER.tar.gz
tar -xvf windninja-$WINDNINJA_VER.tar.gz
mkdir build
cd build
export WINDNINJA_DATA=$SOURCE/windninja-$WINDNINJA_VER/data
export CXXFLAGS=-std=c++11
ccmake -DCMAKE_LIBRARY_PATH=$PREFIX/lib \
    -DCMAKE_INCLUDE_DIR=$PREFIX/include \
    -DCMAKE_INSTALL_PREFIX=$PREFIX \
    -DNINJA_QTGUI=OFF \
    -DNINJAFOAM=OFF \
    -DGDAL_CONFIG=$PREFIX/bin/gdal-config \
    -DGDAL_INCLUDE_DIR=$PREFIX/include \
    -DGDAL_LIBRARY=$PREFIX/lib/libgdal.so \
    ../windninja-$WINDNINJA_VER

# Press (c) -> (c) -> (g)

make $MAKE_OPTIONS
make install
export CXXFLAGS

cd $PREFIX

mkdir -p $PREFIX/modulefiles/windninja
cat <<EOF > $PREFIX/modulefiles/windninja/$WINDNINJA_VER.lua
-- -*- lua -*-
--
whatis([[Name : WindNinja]])
whatis([[Version : $WINDNINJA_VER]])
--
prepend_path("PATH", "$PREFIX/bin", ":")
prepend_path("LD_LIBRARY_PATH", "$PREFIX/lib", ":")
prepend_path("LIBRARY_PATH", "$PREFIX/lib", ":")
prepend_path("MANPATH", "$PREFIX/share/man", ":")
prepend_path("PKG_CONFIG_PATH", "$PREFIX/lib/pkgconfig", ":")
--
EOF
