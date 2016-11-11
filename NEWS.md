Release Notes
=============

WindNinja 3.1.1
---------------

- Fix issue where the input wind height was not properly set in the GUI (\#223)

WindNinja 3.1.0
---------------

 - Fix PDF output legend units (\#217)

 - Make TopoFire the default basemap for PDF output (\#209)

 - Check for no data values in the DEM in CLI runs (\#207)

 - Fix segfault issue related to time zone info in the GUI (\#198)

 - NOMADS HRRR and RAP forecast durations lengthened (\#181)

 - Reduce target number of cells for CFD solver component (\#173)

 - Fix hyperlinks in tutorials (\#191)

 - Use a single mesh for multiple runs (\#176 )

 - Add ability to use an existing case for momentum solver runs (\#176)

 - Handle spaces and other unsupported characters in DEM filename with momentum solver (\#180)

 - Add UAB solver enhancements (\#118)

 - Better warning when solver fails to converge (\#184 )

 - Add a clear button to clear entered domain average inputs (\#165)

 - Fix memory consumption issue with multiple runs (\#166)

 - Properly update feet and meters in output windows (\#167)

 - Prevent duplicate domain average runs (\#163)

 - Dispaly text representation of progress in dialog (\#112)

WindNinja 3.0.1
---------------

2016-06-07

* Fix GFS issue related to time variable (\#149)

* Speed up STL file writing (\#147)

* Fix a bug related to OpenFOAM meshing on small domains (\#152)

WindNinja 3.0.0
---------------

* Add OpenFOAM based momentum solver( \#41, \#43, \#44, \#45, \#47, \#48, \#52,
\#54, \#55, \#62, \#71, \#73, \#75, \#77, \#83, \#90, \#106, \#115, \#116,
\#123, \#124, \#127, \#132)

* Add Geospatial PDF output(\#75, \#87, \#88, \#91, \#92, \#100, \#101, \#115,
\#123, \#127)

* Fix segfault in DEM reading in CLI (\#86)

* Fix GMTED elevation source (\#69, \#78)

WindNinja 2.5.4
---------------

2015-10-06

* Re-enable GMTED as it disables LCP.

WindNinja 2.5.3
---------------

2015-10-02

* Disable GMTED due to lack of server(#69)

* Fix GFS from thredds(#63)

WindNinja 2.5.2
---------------

2015-02-18

* Fixed bug where nomads forecasts wouldn't download properly due to a missing
  drive letter in the path.  This was only present when the working directory
  was different than C:\ (temporary file path root)(#35).

* Make sure progress finishes on the gui (#36).

* Fix NOMADS output file naming (#37).

WindNinja 2.5.1
---------------

2015-02-11

WindNinja 2.5.0 had a major bug that crashed the application when more than one
domain average initialization was run.  All users are encouraged to upgrade
immediately.

* Fixed regression in 2.5.0 that didn't allow more than one domain averaged
  run (#33).

* Handle timezone mismatches more effectively when the geospatial timezone data
  doesn't match the boost tz database (#25).


WindNinja 2.5.0
---------------

2015-01-23

* Add NOMADS server as a coarse scale weather model data source.

* The iterative solver for point initialization was modified to make it more
  stable and, in many cases, slightly faster.

* Fixed bugs related to forecast models not downloading correctly.

* Fixed bug where command line interface wouldn't fill no_data in files it
  downloaded.

* Fixed bug where zero values in forecasts caused solver to hang.

* Fixed scenario where output height may exceed domain height on extremely
  small DEMs.

* Fixed potential race condition in point input/output.

* Fixed command line interface (CLI) so it shows help if no arguments are
  supplied.

WindNinja 2.4.1
---------------

2015-01-24

* Unreleased version with bug fixes from 2.5.0, but without NOMADS support.

WindNinja 2.4.0
---------------

* Fix UCAR server endpoints, incorporated features from the trunk.

WindNinja 2.3.0
---------------

* This release adds a new feature to allow automated downloads of FARSITE .lcp
  files from the LANDFIRE server for the U.S.

* Two bugs were fixed (unknown).

WindNinja 2.2.0
---------------

* Elevation file grabber
    * Built in Google Maps window for navigation and graphical selection of area.

    * Three different elevation data sets to choose from.

    * Added a command line version, also.

* Added better handling of non-neutral stability flows.

* Added feature to optionally fill no data values in elevation files
  using interpolation/extrapolation.

* Time zone is automatically selected based on location of elevation file.

* All elevation files now require valid projection information.

* Changed mesh functionality so WindNinja output files now completely
  cover the input domain.  This will facilitate better compatibility
  with FlamMap requirements.

* Fixed a bug where weather model initialized runs weren't working due
  to a change on the data server.

* Fixed a bug where lcp files with only five bands were not reading correctly.

* Fixed a bug where simulations of very large domains were
  inaccurately interpolating winds to the output height surface.

* Added ability to specify speed units in output files.

* Removed several third party dependency libraries to make
  compiling/linking easier for other software developers.

WindNinja 2.1.3
---------------

* Fix an issue where the RUC weather model initialization wasn't working.  This
  was because the National Weather Service upgraded to a new model called RAP
  (Rapid Refresh) and RUC was subsequently not available on their server.  We
  have converted WindNinja to use the new RAP model.  The main advantage of the
  new RAP model is that the domain now covers all of North America (RUC only
  covered the 48 contiguous US states).  So now users in Alaska, Canada, and
  Mexico can us the RAP initialization.

* Fix a bug that occurred when users freshly installed WindNinja and tried to
  do a weather model initialized run without first setting the time zone,
  WindNinja would produce a fatal error.  This has been fixed by defaulting the
  time zone to a valid one if it is not set by the user.

WindNinja 2.1.2
---------------

* A 64-bit version is now available (for 64-bit Windows XP, Vista, and 7
  operating systems).  The main advantage of this new 64-bit version is the
  ability to run simulations on larger areas by accessing more computer memory.
  Additionally, the 64-bit version ran about 12% faster than the 32-bit version
  on our test machine.

* Fixed a bug where the NDFD weather model initialization wasn't working.

* Fixed an installation bug occurring on some machines that was related to
  linkage with the Microsoft Visual Studio runtime libraries.

* Fixed a bug related to output file resolutions not working correctly when the
  output resolution was less than the elevation file resolution.

* Added better messaging on run failure in the graphical user interface.

* Transitioned the building of WindNinja to the cmake build system.

*  Added support for wind initialization using generic coarse weather model
  output.  The required file format is described here:
  https://collab.firelab.org/software/projects/windninja/wiki/WxModelFormat

WindNinja 2.1.1
---------------

* Fix example point initialization files bundled with WindNinja

* New build system (CMake)

WindNinja 2.1.0
---------------

* Coarse weather model initialization:

  This feature lets you automatically download National Weather Service weather
  model output for your DEM area and do WindNinja runs to "down scale".
  Currently, there are 4 different models to chose from.  You can also
  optionally write output files from the "raw" weather model outputs (in
  addition to the usual output files from WindNinja).

* Point initialization:

  This new feature lets you define location(s) of known wind speed and
  direction to initialize a WindNinja run.  WindNinja will use this information
  and produce a simulation that matches your inputs at the defined location(s).
  Note that you can only do one simulation at a time, and that a simulation
  takes a bit longer than other WindNinja simulations because internally there
  are several simulations being run.  Right now, to do a point initialization
  run, you have to write a text file that identifies the locations, the speeds
  and directions, etc.  The locations can either be in lat/lon or x/y
  coordinates in the DEMs projection.  As an example for now, there is one of
  these files for the Mackay example dem in the example files folder called
  "mackay_wx_stations.csv".

* Clip output files:

  This new functionality just lets you optionally clip off a buffer around your
  output files.  You specify how much to clip by entering a percent of the
  domain average total north-south and east-west extents.  So 10% would clip
  off a 10% "doughnut" from all the output files.  This could be used to
  prevent people from seeing or using (in FARSITE for example) the outer
  portions of your simulations since these areas might be less accurate due to
  boundary effects.

* Write atm files:

  This option just writes an atm file for your simulation.  For "normal" runs
  (input speed and direction), an atm file is written for each run.  For
  weather model initialized runs, one atm file is written with multiple lines
  (ie. each time that was simulated).

* Better time zone support:

  Internally, we have switched to a more advanced representation of time
  (daylight savings, leap year, etc.).  For the user, this just means that you
  chose the timezone based on the location of the simulation, and WindNinja
  determines if it is daylight savings or not (based on the date), the UTC
  offset, etc.

WindNinja 2.0.3
---------------

* Fix support of old style projection files.

* Fix support for DEM files in the southern hemisphere.

* Coarse Scale Weather Model Initialization

  WindNinja will download a forecast for your DEM area and do several runs
  based on forecasts from various sources

* Point Initialization
    * WindNinja will allow a user to select a point on the domain where a wind
       speed and direction are known (a weather station or personal observation).

    * WindNinja will then do its best to match that point's wind speed and
       direction.

WindNinja 2.0.2
---------------

* Fix bug in the diurnal model

* Fix lcp reading.


WindNinja 2.0.1
---------------

* Add time zone support

* Fancy splash screen

* QGIS shapefile rotation support


WindNinja 2.0.0
---------------

* Simulation of diurnal winds – A slope flow model has been incorporated to
  simulate buoyancy driven upslope and downslope flows. Solar heating and
  shadows are computed as part of the ground-to-air heat transfer calculation.
  Simulation of diurnal winds can be turned on or off using a check box. The
  additional inputs required for diurnal wind simulation are minimal, but
  include the date, time, percent cloud cover, air temperature, and
  latitude/longitude (which can usually be obtained automatically from the DEM
  file).

* Multithreading capability – Multiple processor/core computers can now be used to
  shorten simulation times. Speed up is nearly perfect when simulating multiple
  runs. For example, a dual-core computer doing 2 wind runs will only take the
  time of a single wind run.

* Faster solver – Considerable improvements to the solver have also cut
  simulation times (usually in the 8 - 45 second range).

* Newer, more modern GUI – The Graphic User Interface (GUI) has been updated to
  include a tree view, console output, more feedback to the user, a progress
  bar, and many other features. The new GUI will also make it much easier to
  improve WindNinja in the future, such as adding an OpenGL 3D graphics window,
  built in Google Earth to view wind vectors, and to run WindNinja on other
  operating systems such as Linux and Mac OS.

* Additional elevation import options – Elevation data can now be imported using 4
  different file formats: Arc/Info ASCII Raster (*.asc), FARSITE landscape file
  (*.lcp), GeoTiff (*.tif), and ERDAS Imagine (*.img).

* Gridded roughness – Gridded roughness (surface drag) values can be
  automatically imported from a FARSITE landscape file (*.lcp) using the fuels
  and canopy information.

* Additional output options – The resolution of each type of output file
  (Google Earth, fire behavior, and shape file) can now be specified
  independently. Also, an option to use the wind simulation mesh resolution has
  been added and more control over Google Earth output files is now possible.


