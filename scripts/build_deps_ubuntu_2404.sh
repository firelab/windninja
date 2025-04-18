#!/bin/bash

#This is a limited build without GUI and without OpenFOAM
#GUI and OpenFOAM support to be added soon
PREFIX=/usr/local
POPPLER="poppler-0.23.4"
PROJ="proj-9.5.1"
GDAL="gdal-3.2.1"

#Dependencies
sudo -E apt install -y pkg-config libfontconfig1-dev libcurl4-gnutls-dev libnetcdf-dev \
                       libboost-program-options-dev libboost-date-time-dev libgeos-dev \
                       libboost-test-dev libsqlite3-dev sqlite3 libgeotiff-dev

#Get and build poppler for PDF support in GDAL
wget https://poppler.freedesktop.org/$POPPLER.tar.xz
tar -xvf $POPPLER.tar.xz 
cd $POPPLER/
./configure --prefix=$PREFIX --enable-xpdf-headers
make
sudo make install
cd ..

#Get and build proj
wget https://download.osgeo.org/proj/$PROJ.tar.gz
tar xvfz $PROJ.tar.gz
cd $PROJ
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --build . --target install
cd ../..

#Get and build GDAL with poppler support
wget https://download.osgeo.org/gdal/${GDAL:5}/$GDAL.tar.gz
tar -xvf $GDAL.tar.gz 
cd $GDAL/
./configure --prefix=$PREFIX --with-poppler=$PREFIX
make -j 8
sudo make install
cd ..

