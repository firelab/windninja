#!/bin/bash

# Install necessary dependencies
sudo apt install -y pkg-config \
                    libboost-program-options-dev \
                    libboost-date-time-dev \
                    libboost-test-dev \
                    libshp-dev

# Install qt6 
sudo apt install -y qt6-base-dev qt6-webengine-dev

# Install GDAL
sudo apt install -y libgdal-dev

# Install hwloc, for OpenFOAM
sudo apt install -y libhwloc-dev

# Use OpenFOAM 14; OpenFOAM 9 and 11 are not available for Ubuntu 26.04
# add the dl.openfoam.org repo and install OpenFOAM 11
sudo sh -c "wget -O - https://dl.openfoam.org/gpg.key > /etc/apt/trusted.gpg.d/openfoam.asc"
sudo add-apt-repository -y http://dl.openfoam.org/ubuntu
sudo apt update
sudo apt install -y openfoam14
echo "source /opt/openfoam14/etc/bashrc" >> ~/.bashrc
