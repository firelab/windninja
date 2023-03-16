WindNinja
=========
[![example workflow](https://github.com/firelab/windninja/actions/workflows/testing.yml/badge.svg)](https://github.com/firelab/windninja/actions)
[![builds.sr.ht status](https://builds.sr.ht/~ksshannon/windninja.svg)](https://builds.sr.ht/~ksshannon/windninja?)
[![DOI](https://zenodo.org/badge/21244/firelab/windninja.svg)](https://zenodo.org/badge/latestdoi/21244/firelab/windninja)

WindNinja is a diagnostic wind model developed for use in wildland fire modeling.

Web:
http://firelab.org/project/windninja

Source & wiki:
https://github.com/firelab/windninja

FAQ:
https://github.com/firelab/windninja/wiki/Frequently-Asked-Questions

IRC:
irc://irc.freenode.net/#windninja

[Building on Linux](https://github.com/firelab/windninja/wiki/Building-WindNinja-on-Linux)

[Building on Windows](https://github.com/firelab/windninja/wiki/Building-WindNinja-on-Windows-using-the-MSVC-compiler-and-gisinternals.com-dependencies)

Directories:
 * autotest    -> testing suite
 * cmake       -> cmake support scripts
 * data        -> testing data
 * doc         -> documentation
 * images      -> splash image and icons for gui
 * src         -> source files

Dependencies (versions are versions we build against for the Windows installer):
 * Boost 1.46:
    * boost_date_time
    * boost_program_options
    * boost_test
 * NetCDF 4.1.1
 * GDAL 2.2.2
    * NetCDF support
    * PROJ.4 support
    * GEOS support
    * CURL support
 * Qt 4.8.5
    * QtGui
    * QtCore
    * QtNetwork/Phonon
    * QtWebKit
 * [OpenFOAM 2.2.x](https://github.com/OpenFOAM/OpenFOAM-2.2.x)

See INSTALL for more information (coming soon)

See CREDITS for authors

See NEWS for release information

=========
<img src="images/bsb.jpg" alt="Example output"  />

