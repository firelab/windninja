#!/bin/bash

# Install necessary dependencies
apt install -y pkg-config \
                    libboost-program-options-dev \
                    libboost-date-time-dev \
                    libboost-test-dev \
                    libshp-dev

# Install GDAL
apt install -y libgdal-dev

# Install hwloc, for OpenFOAM
apt install -y libhwloc-dev

# Use OpenFOAM 11; OpenFOAM 9 not available for Ubuntu 24.04
# add the dl.openfoam.org repo and install OpenFOAM 11
sh -c "wget -O - https://dl.openfoam.org/gpg.key > /etc/apt/trusted.gpg.d/openfoam.asc"
add-apt-repository -y http://dl.openfoam.org/ubuntu
apt update
apt install -y openfoam11
echo "source /opt/openfoam11/etc/bashrc" >> ~/.bashrc
