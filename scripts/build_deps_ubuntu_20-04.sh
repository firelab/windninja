#!/bin/bash

PREFIX=/usr/local
POPPLER="poppler-0.23.4"
PROJ="proj-4.8.0"
GDAL="gdal-3.2.1"

#Ubuntu
sudo apt install libfontconfig1-dev libcurl4-gnutls-dev libnetcdf-dev libboost-program-options-dev libboost-date-time-dev libgeos-dev libboost-test-dev

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
wget http://download.osgeo.org/gdal/${GDAL:5}/$GDAL.tar.gz
tar -xvf $GDAL.tar.gz 
cd $GDAL/
./configure --prefix=$PREFIX --with-poppler=$PREFIX
make -j 8
sudo make install
cd ..

#Add qt4 libs from ppa
#See here for more info:
#https://ubuntuhandbook.org/index.php/2020/07/install-qt4-ubuntu-20-04/
sudo add-apt-repository ppa:rock-core/qt4
sudo apt update
sudo apt install libqt4-dev libqtwebkit-dev 
