#!/bin/bash



# Install necessary dependencies
sudo apt install -y libfontconfig1-dev \
                    libcurl4-gnutls-dev \
                    libnetcdf-dev \
                    libboost-program-options-dev \
                    libboost-date-time-dev \
                    libgeos-dev \
                    libboost-test-dev \
                    libsqlite3-dev \
                    sqlite3 \
                    libopenjp2-7-dev \
                    libtiff-dev \
                    libshp-dev

# Install qt6 dependencies
sudo apt install -y libgl1-mesa-dev

# Install qt6 libs
sudo apt install -y qt6-base-dev qt6-base-dev-tools qt6-webengine-dev qt6-webengine-dev-tools libqt6webenginecore6-bin

# Install Poppler, Proj, GDAL
sudo apt install -y \
    libgdal-dev \
    libproj-dev \
    libpoppler-dev

# Install hwloc, for OpenFOAM
sudo apt install -y libhwloc-dev

# Use OpenFOAM 9; OpenFOAM 8 not available for Ubuntu 22.04
# add the dl.openfoam.org repo and install OpenFOAM 9
sudo sh -c "wget -O - https://dl.openfoam.org/gpg.key > /etc/apt/trusted.gpg.d/openfoam.asc"
sudo add-apt-repository -y http://dl.openfoam.org/ubuntu
sudo apt update
sudo apt install -y openfoam9
echo "source /opt/openfoam9/etc/bashrc" >> ~/.bashrc
