---
layout: post
title:  "Building WindNinja on Windows"
color: purple
width:   6
height:  1
date:   2016-06-24 11:31:49 +0200
categories: main
---


To build WindNinja on Windows, we recommend building all the third party libraries from source (described below) and **not** downloading pre-built binaries.  This will prevent issues related to incompatibilities of various runtime libraries the codes rely on (version differences and debug/release differences).  The instructions below describe how to build a release version for both 32-bit and 64-bit systems.  As such, a specific directory structure is recommended, however, all of this (debug/release, 32/64, and directory structure) could be modified.  Building is done with the shell (command line) version of the Microsoft Visual Studio C/C++ Compiler which can normally be accessed at Start->Microsoft Visual Studio 2010->Visual Studio Tools and choosing the Visual Studio Command Prompt (2010) for 32-bit compiles and Visual Studio x64 Win64 Command Prompt (2010) for 64-bit compiles.  The commands listed in the instructions below should be run from these command prompts.  Be sure to use the correct one (32- or 64-bit) for your desired build (including building the third party libraries).  Linking incompatible builds together leads to problems (for example, linking a 64-bit WindNinja build with a 32-bit third party library) and is a very common mistake when building both.

The instructions below describe a 32-bit (x86) build, and unless mentioned, a 64-bit build can be accomplished by simply replacing x86 with x64 and using the x64 Visual Studio Command Prompt.

## Build directory structure

First, build a directory structure like this:

    C:\src
        \archives -- Contains zips, tar.gz, etc. of third party source.
        \windninja -- Contains windninja source code and binaries.
            \build32 -- Contains 32-bit windninja build
            \build64 -- Contains 64-bit windninja build
        \x64 -- Contains 64-bit third party binaries and source.
            \bin -- Contains the third party binaries (.exe, .dll).
            \data -- Contains third party data files (mostly GDAL/PROJ projection info).
            \html -- Contains some third party info (related to GDAL file formats).
            \include -- Contains the third party header files.
            \lib -- Contains the third party static and stub libraries.
            \share -- Misc. manuals and other files.
            \src -- Contains the third party extracted source.
        \x86 -- Contains 32-bit third party binaries and source.
            \bin -- Contains the third party binaries (.exe, .dll).
            \data -- Contains third party data files (mostly GDAL/PROJ projection info).
            \html -- Contains some third party info (related to GDAL file formats).
            \include -- Contains the third party header files.
            \lib -- Contains the third party static and stub libraries.
            \share -- Misc. manuals and other files.
            \src -- Contains the third party extracted source.

The top level `\src` directory is in `C:\` here, but could be anywhere you have write access.

## Get third party library source code

Download the third party library archives listed below to `C:\src\archives`.  Then unzip/extract them into both `C:\src\x86` and `C:\src\x64`.

* boost 1.46.1        http://sourceforge.net/projects/boost/files/boost/1.46.1/boost_1_46_1.zip/download
* cURL 7.40.0         http://curl.haxx.se/download/curl-7.40.0.zip
* GDAL 1.11.1         http://download.osgeo.org/gdal/1.11.1/gdal1111.zip
* GEOS 3.4.2          http://download.osgeo.org/geos/geos-3.4.2.tar.bz2
* netCDF 4.3.2        ftp://ftp.unidata.ucar.edu/pub/netcdf/old/netcdf-4.3.2.zip
* PROJ 4.8.0          http://download.osgeo.org/proj/proj-4.8.0.tar.gz
* Qt 4.8.5            https://download.qt.io/archive/qt/4.8/4.8.5/qt-everywhere-opensource-src-4.8.5.zip
* zlib 1.2.8          http://zlib.net/zlib-1.2.8.tar.gz

## Build zlib

`cd` to `C:\src\x86\src\zlib-1.2.8` and make a build directory:

`mkdir C:\src\x86\src\zlib-1.2.8\build`

`cd` to this build directory and run

`cmake-gui ..\`

Edit these entries:

    CMAKE/CMAKE_BUILD_TYPE  =  Release
    INSTALL/INSTALL_BIN_DIR  =  C:/src/x86/bin
    INSTALL/INSTALL_INC_DIR  =  C:/src/x86/include
    INSTALL/INSTALL_LIB_DIR  =  C:/src/x86/lib
    INSTALL/INSTALL_MAN_DIR  =  C:/src/x86/share/man
    INSTALL/INSTALL_PKGCONFIG_DIR  =  C:/src/x86/share

then click configure and generate at the bottom of the window and close the window.
From the shell again, run:

`nmake`

`nmake install`

## Build boost

`cd` into `C:\src\x86\src\boost` and run

`bootstrap`

Then, for 32-bit, run

`bjam install --prefix=C:/src/x86 --with-date_time --with-program_options --with-test --type=complete`

For 64-bit, run:

`bjam install --prefix=C:/src/x64 --with-date_time --with-program_options --with-test --type=complete --toolset=msvc-10.0 address-model=64`

## Build cURL

This requires zlib to already be built.

`cd` into the `winbuild` directory.

For 32-bit, run

`Nmake –f Makefile.vc mode=dll WITH_ZLIB=dll WITH_DEVEL=C:\src\x86 DEBUG=no MACHINE=x86`

For 64-bit, run

`Nmake –f Makefile.vc mode=dll WITH_ZLIB=dll WITH_DEVEL=C:\src\x64 DEBUG=no MACHINE=x64`

The `WITH_DEVEL` flag tells cURL where to find zlib.

Now copy all the development files made (`.lib`, `.dll`, `.h`, etc.) from `curl\builds\libcurl-vc-x86-release-dll-zlib-dll-ipv6-sspi-winssl\bin` and `\include` and `\lib` to the `C:\src\x86\bin` `\include` and `\lib` directories.

## Build GEOS

Go to `C:\src\x86\src\geos-3.4.2\include\geos` and change filename `version.h.vc` to `version.h`.  (It might be possible that `bootstrap.bat` would do this?).

**NOTE**: Building geos-3.4.2 with VS2013 requires the following changes:

https://trac.osgeo.org/geos/changeset/3981

https://trac.osgeo.org/geos/ticket/616

Make a build directory in `C:\src\x86\src\geos-3.4.2`.  `cd` there and run:

`cmake-gui ../`

Run configure.  Now edit the following entries:

    Ungrouped Entries/BUILD_TESTING = unchecked
    CMAKE/CMAKE_BUILD_TYPE = Release
    CMAKE/CMAKE_INSTALL_PREFIX = C:/src/x86
    GEOS/GEOS_ENABLE_TESTS = unchecked

Run configure and generate then close the window. Ignore the following warnings:

    message("*** sh-compatible command not found, cannot create geos_svn_revision.h")
    message("*** Check SVN revision and create revision header manually:")
    message("*** echo '#define GEOS_SVN_REVISION XYZ' > ${CMAKE_SOURCE_DIR}/geos_svn_revision.h")

Run

`nmake` or alternatively `C:\bin\jom.exe –j10`

Run

`nmake install`

## Build PROJ

Open the file `C:\src\x86\src\proj-4.8.0\nmake.opt` and edit the `INSTDIR` line to be:

`INSTDIR=C:\src\x86`

Now download the file:

`http://download.osgeo.org/proj/proj-datumgrid-1.5.zip`

and put it in `C:\src\x86\src\proj-4.8.0\nad` and unzip.

Then `cd` into `C:\src\x86\src\proj-4.8.0` and run:

`nmake -f makefile.vc`

`nmake -f makefile.vc install-all`

## Build netCDF

Make a `build` directory in `C:\src\x86\src\netcdf-4.3.2` and `cd` to there.  Run

`cmake-gui ../`

Edit the entries:

    BUILD/BUILD_TESTING = unchecked
    CMAKE/CMAKE_BUILD_TYPE = Release
    ENABLE/ENABLE_NETCDF4 = unchecked
    ENABLE/ENABLE_NETCDF_4 = unchecked
    USE/USE_HDF5 = unchecked
    USE/USE_NETCDF4 = unchecked
    Ungrouped Entries/ENABLE_DAP = unchecked
    CMAKE/CMAKE_INSTALL_PREFIX = C:\src\x86
    Ungrouped Entries/BUILD_TESTSETS = unchecked

Then do configure and generate and close the window.  Run

`nmake` or alternatively `C:\bin\jom.exe –j10`

`nmake install`

## Build GDAL

Open `C:\src\x86\src\gdal-1.11.1\nmake.opt` and uncomment and edit the following lines:

    GDAL_HOME = "C:\src\x86"
    NETCDF_PLUGIN = NO
    NETCDF_SETTING=yes
    NETCDF_LIB=C:\src\x86\lib\netcdf.lib
    NETCDF_INC_DIR=C:\src\x86\include
    CURL_INC = -IC:\src\x86\include
    CURL_LIB = C:\src\x86\lib\libcurl.lib wsock32.lib wldap32.lib winmm.lib    (note the filename “libcurl.lib”)
    GEOS_CFLAGS = -IC:\src\x86\include -IC:\src\x86\include\geos -DHAVE_GEOS
    GEOS_LIB     = C:\src\x86\lib\geos_c.lib    (note the filename “geos_c.lib”)

If this is a 64-bit compile, also uncomment out the line:

    WIN64=YES

Run

`nmake –f makefile.vc`

`nmake –f makefile.vc devinstall`

`nmake –f makefile.vc install`

## Building Qt

**NOTE**: Building qt-4.8.5 with VS2013 requires the following changes:

http://stackoverflow.com/questions/18080625/qt-4-8-visual-studio-2013-compiling-error

From `C:\src\x86\src\qt-everywhere-opensource-src-4.8.5` run:

`configure -prefix C:\src\x86 -opensource`

`nmake`

`nmake install`

## Build WindNinja

`cd` into `C:\src\windninja\build32` or `\build64` and run:

`cmake-gui ../`

Do configure.  Now add a new entry called `BOOST_ROOT=C:/src/x64` and do configure again.  This should set all the other boost paths correctly.

Using this same method of iteratively clicking configure and then editing entries, set the paths to the NETCDF and GDAL entries, note that `NETCDF/NETCDF_LIBRARIES` should just be set to `EMPTY`. Set `QT_QMAKE_EXECUTABLE` to `C:/src/x64/bin/qmake.exe`

Set:

    NINJA/NINJA_CLI = checked
    NINJA/NINJA_QTGUI = checked
    WITH/WITH_LCP_CLIENT = checked
    WITH/WITH_NOMADS_SUPPORT = checked
    Ungrouped Entries/ENABLE_GMTED = checked
    Ungrouped Entries/FIRELAB_PACKAGE = checked
    Ungrouped Entries/STABILITY = checked
    BUILD/BUILD_FETCH_DEM = checked
    BUILD/BUILD_TESTING = unchecked

Append `/DNINJA_ENABLE_CALL_HOME` to the `CMAKE/CMAKE_CXX_FLAGS` entry.

_If you are planning to build a tagged release package with a WindNinja version number,  open the file VERSION in the WindNinja source root and edit to the desired version number.  In the WindNinja cmake-gui, set the GENERATE_VERSION_INFO=checked and set the GIT_COMMAND  to the path to your git.exe.  These steps would only typically be done by Missoula Firelab personnel._

Then run:

`nmake`

The WindNinja binaries (`.exe`, `.dll`, etc.) should have successfully compiled and are located in directories under `C:\src\windninja\build32\src`.

To build the WindNinja installer, run

`nmake package`
