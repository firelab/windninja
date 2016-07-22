---
layout: post
title:  "Building WindNinja on Linux"
color: green
width:   3
height:  1
date:   2016-06-24 23:31:49 +0200
categories: main
---

## Get WindNinja Source

    git clone https://github.com/firelab/windninja.git

## Install Third-Party Libraries

Run `scripts/build_deps.sh` to fetch, build, and install third party libraries.

    cd windninja/scripts/ && ./build_deps.sh

## Start CMake

Create a build directory outside the source tree. For example, if WindNinja is in `~/src/wind/windninja`: `mkdir ~/src/wind/build`.

This build directory is where all of the build-related files will be located (executables, shared libraries, object files, qt moc files, etc.). This is done so that these files don't contaminate the source tree. One advantage of this is that all of these files can be deleted easily by just deleting the build directory.

    cd ~/src/wind/build && cmake-gui ../windninja

Select configure to start the process of setting options and identifying locations of third party libraries, etc. If asked to "Specify the generator for this project" select Unix Makefiles and choose "Use default native compilers".

The general procedure to be used in the cmake gui is to edit the "Value" fields in the Name/Value pair list appropriately, then click the configure button. This procedure must be typically done in an iterative fashion (multiple times) as one edit may add additional requirements. Read below for the possible Options available.

**CMake options:**

Turn on one or both of the following options:

* NINJA_QTGUI -- builds the WindNinja GUI
* NINJA_CLI -- builds the WindNinja CLI

If you plan to use the optional conservation of mass and momentum solver, set:

* NINJAFOAM = ON

The CMake build has several other general options which can be set:

* CMAKE_BUILD_TYPE -- can be set to debug or release
* CMAKE_INSTALL_PREFIX -- defines the install path
* ENABLE_CONSOLE -- enables the GUI console on Windows, which can help with debugging
* OPENMP_SUPPORT -- enables OpenMP support
* SUPPRESS_WARNINGS -- suppresses common, known warnings that are not thought to affect the build
* VERBOSE_WARNINGS -- sets warnings to all, and supercedes SUPRESS_WARNINGS (you may also want to enable CMAKE_VERBOSE_MAKEFILE in the advanced options)

**Third-party libraries:**

CMake will attempt to find all of the third party libraries needed for WindNinja. All are required with the exception of Qt, if the gui is not being built. CMake has default methods for handling libraries. It will usually ask for a path to a lib, a path to an include directory, and possibly a path to a binary that can specify some configuration values for the install. Since all of the libraries are required, the CMake configuration fails when one is not found. This means you must find the paths for each lib in order, as CMake fails each time you click "Configure" on the next unfound lib. After everything is found and set, click "Generate".



## Build WindNinja



If you have correctly configured in the cmake gui and clicked "generate", a Makefile will be built in the build directory. Next run:

    make && sudo make install

You may also need to run:

    sudo ldconfig

make should compile and link WindNinja to all of its dependency libraries. The WindNinja executables should be located in build/src/gui and build/src/cli for the GUI and CLI versions, respectively. The core WindNinja shared library that handles the number crunching part of WindNinja is located in build/src/ninja.

make install should install all of the necessary WindNinja binaries and other files in the location specified earlier in the cmake CMAKE_INSTALL_PREFIX option.



## Set Environment Variables



As of WindNinja 2.2.0, the environment variable WINDNINJA_DATA must be set. This points to datasets that WindNinja needs to run certain functions. If the program is installed (i.e., with make install), it is likely that WindNinja will find the path. Otherwise, the environment variable will need to be set:

    export WINDNINJA_DATA=~/src/wind/windninja/data



## (Optional) Install OpenFOAM 2.2.0



If you want to use the optional conservation of mass and momentum solver, you will need to build and install OpenFOAM 2.2.0. Follow the instructions at [openFOAM](http://openfoam.org/download/2-2-0-source/).

Then build build our custom libraries and applications. For example:

    mkdir -p $FOAM_RUN/../applications
    cp -r ~/src/wind/windninja/src/ninjafoam/* $FOAM_RUN/../applications
    cd $FOAM_RUN/../applications
    wmake libso
    cd utility/applyInit
    wmake

This should build an executable called applyInit in $FOAM_RUN/../platforms/linux64GccDPOpt/bin and a shared library called libWindNinja.so in $FOAM_RUN/../platforms/linux64GccDPOpt/lib.
