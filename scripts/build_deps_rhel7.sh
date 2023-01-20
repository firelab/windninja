#!/bin/bash

PREFIX=/usr/local
POPPLER="poppler-0.23.4"
SQLITE="sqlite-snapshot-202301131932"
PROJ="proj-6.3.2"
GDAL="gdal-3.4.3"

#Dependencies
sudo -E yum install -y pkg-config libfontconfig1-dev libcurl4-gnutls-dev libnetcdf-dev libboost-program-options-dev libboost-date-time-dev libgeos-dev libboost-test-dev libsqlite3x-dev sqlite3

#Get and build poppler for PDF support in GDAL
wget https://poppler.freedesktop.org/$POPPLER.tar.xz
tar -xvfz $POPPLER.tar.xz
cd $POPPLER/
./configure --prefix=$PREFIX --enable-xpdf-headers
make
make install
cd ..

#Get and build sqlite3
wget https://www.sqlite.org/snapshot/$SQLITE.tar.gz
tar -xvfz $SQLITE.tar.gz
cd $SQLITE
CFLAGS="-DSQLITE_ENABLE_COLUMN_METADATA=1" ./configure
./configure
make
make install
cd ..

#Get and build proj
wget https://download.osgeo.org/proj/$PROJ.tar.gz
tar -xvfz $PROJ.tar.gz
cd $PROJ
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
./configure --prefix=$PREFIX
make clean
make
make install
cd ..

#Get and build GDAL with poppler support
wget http://download.osgeo.org/gdal/${GDAL:5}/$GDAL.tar.gz
tar -xvf $GDAL.tar.gz
cd $GDAL/
./configure --prefix=$PREFIX --with-poppler=$PREFIX
make
make install
cd ..

#Use OpenFOAM 8
yum -y install dnf-plugins-core
yum -y install epel-release
yum -y install yum-plugin-copr
yum -y copr enable openfoam/openfoam
yum -y install openfoam-selector
yum -y install openfoam
