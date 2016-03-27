Testing WindNinja with configuration files
==========================================

This folder contains tests that run WindNinja as a whole using various
configuration files.  Any file in the `pass` and `fail` folders that end with
`.cfg` will be run.  If the file is in the `pass` folder, WindNinja should run
the configuration file and return 0, suceeding.  Files in the `fail` folder
will be expected to fail, meaning returning 1.  Uncaught exceptions do not
count as passing.

The program `parse_input` is a helper program that extracts the dem file from
the config file.  If it can find the `elevation_file` tag it returns the file
name (no path) of the input DEM.  This should be located in the `data` folder
in the source tree.

Process
-------

Cmake will generate the helper executables.
