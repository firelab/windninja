#!/bin/bash

# Install necessary dependencies
sudo apt install -y \
    build-essential \
    cmake \
    wget \
    pkg-config \
    libfontconfig1-dev \
    libcurl4-gnutls-dev \
    libnetcdf-dev \
    libboost-program-options-dev \
    libboost-date-time-dev \
    libboost-test-dev \
    libgeos-dev \
    libsqlite3-dev \
    sqlite3 \
    libopenjp2-7-dev \
    libtiff-dev \
    libshp-dev \
    libpoppler-dev \
    libproj-dev \
    proj-bin \
    proj-data \
    libgdal-dev \
    gdal-bin \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-webengine-dev \
    qt6-webengine-dev-tools

# Use OpenFOAM 9; OpenFOAM 8 not available for Ubuntu 22.04
# add the dl.openfoam.org repo and install OpenFOAM 9
sudo sh -c "wget -O - https://dl.openfoam.org/gpg.key > /etc/apt/trusted.gpg.d/openfoam.asc"
sudo add-apt-repository http://dl.openfoam.org/ubuntu
sudo apt update
sudo apt install -y openfoam9
echo "source /opt/openfoam9/etc/bashrc" >> ~/.bashrc
