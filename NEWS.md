Release Notes
=============

WindNinja 3.12.1
---------------
- Fixed issue where the 2-m temperature band was not properly identified in the NBM model, causing diurnal runs to fail (#600)

- Fixed issue where HRRR-EXT and RAP-EXT were improperly identified as HRRR and RAP (#601)

WindNinja 3.12.0
---------------
- Fixed issue in point initialization where requested start/stop times outside the range of the station file times would cause the application to crash (#586)

- Fixed issue causing an inaccurate mesh resolution to sometimes be displayed in the GUI (#564)

- Updated LANDFIRE LFPS endpoints (#562)

- Fixed issue causing some UCAR models to fail (#557)

- Added a DEM smoothing algorithm for cases where the momentum solver diverges due to a low quality mesh (#555)

- Fixed issue causing weather station files to be written to the wrong directory (#545)

- Added a check for a proper datum in weather station files (#541)

- Fixed an issue causing the GUI to crash when a custom output path was not set correctly (#540)

- Upgraded LCP downloads to LF 2023 version 2.4.0 (#539)

- Updated shapefile display tutorial (#536)

- Improved SRTM DEM downloads (#529)

- Added a proper check for surface data bounds in the GUI (#523)

- Enabled pastcasts with historical HRRR forecasts from GCP (#514)

- Fixed inconsistent output file naming in gridded initialization runs (#512)

- Enabled a consistent color scale across time steps for KMZ outputs (#441)

- Improved interpolations in point initialization (#419)

- Added RAP and HRRR extended forecasts (#365)

- Added the NBM forecast (#222)

WindNinja 3.11.2
---------------
- Fix Qt SSL issue causing the DEM download widget map not to load for some users (#535)

- Update Tutorial 1 with details regarding requirement for north "up" projections

WindNinja 3.11.1
---------------
- Fix issue causing diurnal wind component to be arbitrarily increased (#532)

WindNinja 3.11.0
---------------
- Re-enable SRTM data source downloads via OpenTopography API (#395)

- Increased default download time for LCP service (#515)

- Fixed timze zone issue in the GUI related to point initialization runs (#516)

- Added additional time zones from Python timezone finder

- Disabled automatic output buffering unless the FARSITE/FlamMap atmosphere file is written (#511)

- Re-enable server communications (#295, #450, #481)

WindNinja 3.10.0
---------------
- Update CLI documentation, example cfg files, and test data (#415, #434, #270, #252)

- Fix issue related to station fetching that was causing point initialization runs to fail (#506)

- Fix several small miscellaneous issues in the point initialization method (#420)

- Check dependencies/conflicts and improve error messages in the CLI (#238)

- WindNinja can now download station files for time periods prior to the current year

- Fix an issue that was causing output products to be displayed in correctly in ESRI platforms (e.g., ArcMap) (#464, #500)

- Implement better handling of special characters in DEM names (#455)

WindNinja 3.9.0
---------------
- Re-enable ability to download DEM from point and radius or bbox coordinates (#336)

- Speed up solar radiation calculations used in diurnal and non-neutral stability parameterizations (#478)

- Fix a bug that caused the vegetation to be set incorrectly in the GUI (#486)

- Temporary fix for NDFD issue on THREDDS server related to temperature grids (#494)

- Fix issue in terrain shading code used in diurnal and non-neutral stability parameterizations (#476)

- Fix coordinate transformation issue in solar calculations for newer GDAL versions (#475)

- Don't buffer output grids for FARSITE if output clipping is requested (#472)

- Re-enable topofire basemaps in GeoPDF output (#463)

- Add scale bar to map in DEM downloader widget (#422)

WindNinja 3.8.1
---------------
- Update Mapbox API requests for GUI download widget (#483)

- Fixed issue causing an occassional double free of an object in LCP client (#482)

WindNinja 3.8.0
---------------
- Update LANDFIRE services used for downloading LCP files (#400, #411, and #451)

- Add new weather models from NOMADS for Hawaii, Puerto Rico, and Guam (#457)

- Add option to write binary VTK files (#416)

WindNinja 3.7.5
---------------
- Restore GMTED data downloads following a change on the server that provides these data (#452)

- Adjust coefficients used in estimating roughness and displacement heights to avoid wind profile interpolation issues (#456)

WindNinja 3.7.4
---------------
- Fix issue related to URL generation causing LCP downloads to intermittently fail (#449, #448)

- Temporarily disabled start-up messages for server migration (#450) 


WindNinja 3.7.3
---------------
- Update Linux build script for Ubuntu 20.04 (#397)

- Update Tutorial 4 with current weather model specs (#401, #425)

- Fix an interpolation issue in point initialization (#418)

- Fix example cfg for point initialization run syntax (#423)

- Update continuous integration test (#402, #425)

- Fix issue with fuel depth units (#426)

- Fix issue where OpenFOAM output was occasionally incorrectly read from previous time steps (#430) 

- Add debugging option for multi-timestep momentum solver simulations (#431)

- Fix .atm file writing for multi-timestep point initialization runs (#432)

- Fix time zone issue in point initialization (#412)


WindNinja 3.7.2
---------------
- Fix bug related to how roughness grids are set from LCP files (#398)

- Upgrade LCP data to 2016 for CONUS and 2014 for AK and HI (#189)

- Display files with appropriate extensions in the save dialog box of the DEM downloader (#406)

- Ensure resampled output grids cover the original DEM extents (#399)

- Fix bug causing the GUI to crash when the output directory is not populated in the Solve tab (#405)

WindNinja 3.7.1
---------------
- Update DEM viewer for mapbox API migration to Static Tiles API (#394)

WindNinja 3.7.0
---------------
- Add ability to set a custom output path in the GUI (#369)

- Write correct units in weather station kml output (#325)

- Use gridded roughness properly in initializations with LCP files (#366)

- Allow users to set a custom mesh resolution in the CFD solver (#357)

- Write outputs at the mesh resolution as default for CFD runs (#339)

- Correctly report meshing time for CFD runs with weather model initializations (#371)

- Handle how exceptions are passed through to the user via the API vs. the GUI/CLI (#373)

- Properly set diurnal wind indices when stations outside the DEM are used in point initializations (#362)

- Do not use internal VRT warping for band mapping for WRF NetCDF files (#375)

- Properly set Monin-Obukhov length for neutral stability runs (#391)

- Use log interpolation to the output wind height for CFD runs if the output height is not resolved (#76, #392)

WindNinja 3.6.0
---------------
- Fix issue related to time interpolation in point initialization (\#352)

- Add sub-hourly HRRR (\#240)

- Add ability to select forecast times to simulate (\#49)

- Add UTC to list of time zone options (\#353)

- Check for proper file extension on elevation files when reusing a CFD case (\#345)

- CFD model upgrades (\#213, \#145)

WindNinja 3.5.3
---------------
- Update for NOMADS directory structure changes for the GFS (\#341)

WindNinja 3.5.2
---------------
- Fix bug related to lat/lon calculations in DEM downloader (\#327, \#328, \#333)

WindNinja 3.5.1
---------------
- Switch to using HTTPS for NOMADS downloads (\#329)

WindNinja 3.5.0
---------------
- Switch to OpenStreetMaps and Mapbox for DEM download window to fix issues related to Google Maps API terms of service changes (\#287)

WindNinja 3.4.1
---------------
- Fix issue where DEM names containing spaces fail with NinjaFOAM (\#230)

WindNinja 3.4.0
---------------

- Add Mesowest Mesonet API to fetch weather stations for point initialization runs (\#94)

- Add multiple time step point Initialization runs (\#94)

- Convert Tutorials from open office to LaTeX (\#308)

- Add configuration options to CLI (\#297)

- Add "phone home server" (\#295)

- Add various color schemes to google earth outputs (\#296,\#235)

WindNinja 3.3.2
---------------

- Fix bug related to HRRR/RAP upgrade on NOMADS (\#298)

WindNinja 3.3.1
---------------

- Fix bug causing the DEM downloader "Drag Box On Map" option to not work correctly (\#260)

WindNinja 3.3.0
---------------

- Add NOMADS-NAM-NEST-CONUS-3-KM and NOMADS-NAM-NEST-ALASKA-3-KM models (\#245)

- Add knots as a wind speed unit (\#243)

- Fix an issue that was causing NDFD to fail due to a change on the UCAR server (\#257)

WindNinja 3.2.0
---------------

- Enable weather model initialization with the momentum solver

- Fix an issue that was causing NAM models to fail due to an upgrade on the NOMADS server (\#244)

WindNinja 3.1.4
---------------

- Use API key to avoid Google Maps API quota error (#242)

- Set app name/version in User-Agent header (#242)

WindNinja 3.1.3
---------------

- Update lcp fetcher for changes on Landfire server (#236)

WindNinja 3.1.2
---------------

- Fix bug where DEM downloading fails on Windows due to SSL verfification (#231)

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
