#!/bin/bash

PREFIX=/usr/local
POPPLER="poppler-0.23.4"
PROJ="proj-4.8.0"
GDAL="gdal-2.2.2"

#Ubuntu
sudo apt-get install libfontconfig1-dev libcurl4-gnutls-dev libnetcdf-dev qt4-dev-tools libqtwebkit-dev libboost-program-options-dev libboost-date-time-dev libgeos-dev libboost-test-dev

#CentOs
#sudo yum install boost-devel.x86_64 fontconfig-devel.x86_64 libcurl-devel.x86_64 epel-release netcdf-devel qt-devel qtwebkit-devel

#If the Boost version is < 1.41.1, a newer Boost version will need to be built from source
#e.g., CentOS 6.9 comes with Boost 1.41.0

#Get and build poppler for PDF support in GDAL
wget http://poppler.freedesktop.org/$POPPLER.tar.xz
tar -xvf $POPPLER.tar.xz 
cd $POPPLER/
./configure --prefix=$PREFIX --enable-xpdf-headers
make
sudo make install
cd ..

#Get and build proj
wget http://download.osgeo.org/proj/$PROJ.tar.gz
tar xvfz $PROJ.tar.gz
cd $PROJ
./configure --prefix=$PREFIX
make clean
make
sudo make install
sudo cp $PREFIX/include/proj_api.h $PREFIX/lib
cd ..

#Get and build GDAL with poppler support
wget http://download.osgeo.org/gdal/2.2.2/$GDAL.tar.gz
tar -xvf $GDAL.tar.gz 
cd $GDAL/
./configure --prefix=$PREFIX --with-poppler=$PREFIX
make -j 8
sudo make install
cd ..

