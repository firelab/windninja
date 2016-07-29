
NOMADS Weather Model Information
================================

General
-------

NOMADS is the National Weather Service data dissemination server/service.  WindNinja can now download weather forecasts using this service, which may prove to be more reliable than past services.

### Available Models:
<table>
  <tr><th>Model</th><th>Resolution</th><th>Domain</th></tr>
  <tr><td>GFS</td><td>0.25 deg</td><td>Global</td></tr>
  <tr><td>HIRES Alaska</td><td>~5 km</td><td>Alaska</td></tr>
  <tr><td>HIRES ARW</td><td>~5 km</td><td>CONUS</td></tr>
  <tr><td>HIRES NMM</td><td>~5 km</td><td>CONUS</td></tr>
  <tr><td>NAM Alaska</td><td>11.25 km</td><td>Alaska</td></tr>
  <tr><td>NAM CONUS</td><td>13 km</td><td>CONUS</td></tr>
  <tr><td>NAM North America</td><td>32 km</td><td>North America</td></tr>
  <tr><td>HRRR</td><td>3 km</td><td>CONUS</td></tr>
  <tr><td>RAP CONUS</td><td>13 km</td><td>CONUS</td></tr>
  <tr><td>RAP North America</td><td>32 km</td><td>North America</td></tr>
</table>

NOMADS Dependencies
-------------------

Due to a bug in the vsizip package in GDAL, version 1.11.1 or newer is
required.  See http://trac.osgeo.org/gdal/ticket/5530.

NOMADS Parallel Testing Server
------------------------------

In order to test the new model changes, NOMADS uses a parallel test server.
The git branch para-nomads sets the proper server and script names for the
endpoint.  In order to test the new models, use that branch and test only the
models to be changed, all others will likely fail.  Note that sometimes the NWS
doesn't emulate the grib fetching/subsetting, and that branch won't work.

The para-nomads branch is skipped in travis builds.

NOMADS Configuration
--------------------

### Compile-time options

* `NOMADS_USE_IP` Use the IP address of the NOMADS server instead of the host
   name.  This was to debug a DNS bug in GDAL, not recommended.

* `NOMADS_USE_VSI_READ` Use the GDAL VSIL API to download files.  This allows
   for larger files to be downloaded by chunking them into
   `NOMADS_VSI_BLOCK_SIZE` chunks for download.  Otherwise, the entire file is
   downloaded at once into memory and then written to disk.  Most files are fine
   either way.  If very large weather model files are to be used, set this
   option to `ON`.

* `NOMADS_ENABLE_ASYNC` Allow multiple connections for download.  This can
   provide substantial speed up.  The default implementation attempts to
   download 4 files at a time.  This can be modified using `NOMADS_THREAD_COUNT`
   at runtime.  If this option is disabled, only one file is fetched at a time.

* `NOMADS_ENABLE_3D` Attempt to use three-dimensional initialization data.
  Alpha at best.  Not recommended.

* `NOMADS_RTMA` Enable the CONUS RTMA model.  The RTMA is a "past cast", so the
   duration always defaults to 0.  The most recent forecast hour is always
   used.

* `NOMADS_EXPER_FORECASTS` Enable some forecasts that may not work under the
  current implementation.  NARR and NEST almost fit, but don't with the current
  organization.  Primarily a debugging tool.  Not recommended.

### GFS horizontal grid resolution

GFS has three horizontal grid resolutions, 0.25 (default), 0.5 and 1.0 degrees.

    cmake -DNOMADS_GFS_0P5DEG ...

or

    cmake -DNOMADS_GFS_1P0DEG ...

will override the default and may be useful in some situations, but not
recommended.

Runtime Options
---------------

* `NOMADS_THREAD_COUNT` number of threads to use to download files from
   nomads(default: 4).  You can get good speed up from a ratio of 1:1 threads
   per file to 1:4.

* `NOMADS_MAX_FCST_REWIND` number of forecast runs to go back in time to try
   and get a full forecast(default: 2).

* `NOMADS_VSI_BLOCK_SIZE` If NOMADS_USE_VSI_READ is set to ON in cmake, this is
  the chunk size that is read in each download(default: 512).

* `NOMADS_HTTP_TIMEOUT` The timeout in seconds for a request (default 20)

Known Issues
------------

### GFS cloud cover data

The new (2014-01-14) GFS model omits t000 cloud cover data as it is a
time-averaged variable.  Since no time has passed, it is NULL.  The work around
is to grab the next time step and use it's cloud cover data.

Further Information
-------------------

Model descriptions and other information can be found at
http://nomads.ncep.noaa.gov/

Frequent users would benefit by subscribing to NCEP's list-serv pertaining to
the NOMADS server.  Changes and downtime are reported there:

https://lstsrv.ncep.noaa.gov/mailman/listinfo/ncep.list.nomads-ftpprd

