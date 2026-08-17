#!/bin/bash

# Install necessary dependencies
sudo apt install -y pkg-config \
                    libboost-program-options-dev \
                    libboost-date-time-dev \
                    libboost-test-dev \
                    libshp-dev

# Install qt6 
sudo apt install -y libgl1-mesa-dev qt6-base-dev qt6-webengine-dev qt6-webengine-dev-tools libqt6webenginecore6-bin

# Install GDAL
sudo apt install -y libgdal-dev

# Install hwloc, for OpenFOAM
sudo apt install -y libhwloc-dev

# Use OpenFOAM 11. OpenFOAM 9 is also a supported version for 22.04. 
# add the dl.openfoam.org repo and install OpenFOAM 9
sudo sh -c "wget -O - https://dl.openfoam.org/gpg.key > /etc/apt/trusted.gpg.d/openfoam.asc"
sudo add-apt-repository -y http://dl.openfoam.org/ubuntu
sudo apt update
sudo apt install -y openfoam11
echo "source /opt/openfoam11/etc/bashrc" >> ~/.bashrc
