#!/bin/bash

#This is a limited build without GUI and without OpenFOAM
#GUI and OpenFOAM support to be added soon
PREFIX=/usr/local
POPPLER="poppler-22.02.0"
PROJ="proj-9.5.1"
GDAL="gdal-3.4.1"

#Dependencies
sudo -E apt install -y pkg-config libfontconfig1-dev libcurl4-gnutls-dev libnetcdf-dev \
                       libboost-program-options-dev libboost-date-time-dev libgeos-dev \
                       libboost-test-dev libsqlite3-dev sqlite3 libgeotiff-dev libopenjp2-7-dev

# Install Poppler for PDF support in GDAL
wget https://poppler.freedesktop.org/$POPPLER.tar.xz
tar -xvf $POPPLER.tar.xz
cd $POPPLER/
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=$PREFIX \
      -DTESTDATADIR=$PWD/testfiles \
      -DENABLE_UNSTABLE_API_ABI_HEADERS=ON \
      ..
make -j$(nproc)  # Use all available cores for faster compilation
sudo make install
cd ../..

# Install PROJ
wget https://download.osgeo.org/proj/$PROJ.tar.gz
tar xvfz $PROJ.tar.gz
cd $PROJ
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=$PREFIX \
      ..
make -j$(nproc)  # Use all available cores for faster compilation
sudo make install
cd ../..

# Install GDAL with Poppler support
wget https://download.osgeo.org/gdal/${GDAL:5}/$GDAL.tar.gz
tar -xvf $GDAL.tar.gz 
cd $GDAL/
./configure --prefix=$PREFIX --with-poppler=$PREFIX --with-unix-stdio-64=no
make -j$(nproc)
sudo make install
cd ..

## bad luck trying to build OpenFOAM 8 and 9 from source, they compiled fine, but they had weird runtime IO errors that imply the need to build with an older compiler
## https://openfoam.org/download/8-source/, https://openfoam.org/download/9-source/
## I did find a workaround though: went to the .deb package from the openfoam website, downloaded it, and unpacked it.
##  https://dl.openfoam.org/ubuntu/dists/focal/main/binary-amd64/openfoam9_20211122_amd64.deb
##  dpkg-deb -x openfoam9_20211122_amd64.deb .
##  sudo mv opt/openfoam9/ /opt/
##  source /opt/openfoam9/etc/bashrc   <-- I usually put this in my /home/<usr>/.bashrc, then "source /home/<usr>/.bashrc"
##
## Note, to get the dependencies, I did the following, from https://openfoam.org/download/source/software-for-compilation/
##  sudo apt-get install build-essential cmake git ca-certificates flex
##  sudo sh -c "wget -O - https://dl.openfoam.org/gpg.key > /etc/apt/trusted.gpg.d/openfoam.asc"
##  sudo add-apt-repository http://dl.openfoam.org/ubuntu
##  sudo apt install openfoam-nopv-deps   <--- well, technically I tried the full paraview "openfoam-deps" version, alongside "paraview-dev", but in theory just the no-paraview version should be enough of the dependencies.
