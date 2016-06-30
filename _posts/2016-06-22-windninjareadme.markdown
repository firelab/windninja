---
layout: post
title:  "WindNinja Readme"
color:  blue
width:   3
height:  1
date:   2016-03-30 11:31:49 +0200
categories: main
---
[WindNinja](http://firelab.org/project/windninja)
=========
[![Build Status](https://travis-ci.org/firelab/windninja.svg?branch=master)](https://travis-ci.org/firelab/windninja)
[![DOI](https://zenodo.org/badge/21244/firelab/windninja.svg)](https://zenodo.org/badge/latestdoi/21244/firelab/windninja)

WindNinja is a diagnostic wind model developed for use in fire modeling.

Web:
http://firelab.org/project/windninja

Source & wiki:
https://github.com/firelab/windninja

IRC:
irc://irc.freenode.net/#windninja

Legacy hosting/wiki:
https://collab.firelab.org/software/projects/windninja
https://collab.firelab.org/svn/windninja


[Building on Linux](https://github.com/firelab/windninja/wiki/Building-WindNinja-on-Linux)

[Building on Windows](https://github.com/firelab/windninja/wiki/Building-WindNinja-on-Windows-using-the-MSVC-compiler-and-gisinternals.com-dependencies)

Directories:
 * autotest    -> testing suite
 * cmake       -> cmake support scripts
 * data        -> testing data
 * doc         -> documentation
 * images      -> splash image and icons for gui
 * src         -> source files

Dependencies (versions are versions we build against):


 * Boost 1.46:


    * boost_date_time


    * boost_program_options


    * boost_test


 * NetCDF 4.1.1


 * GDAL 1.11.1


    * NetCDF support


    * PROJ.4 support


    * GEOS support


    * CURL support


 * Qt 4.7.4


    * QtGui


    * QtCore


    * QtNetwork/Phonon



    * QtWebKit


 * [OpenFOAM 2.2.x](https://github.com/OpenFOAM/OpenFOAM-2.2.x)

See [About](http://firelab.github.io/windninja/about/) for more information
See [FAQ](http://firelab.github.io/windninja/faq/)

See [CREDITS](http://firelab.org/project/windninja) for authors

See [CHANGE LOG](http://firelab.github.io/windninja/log/) for release information
