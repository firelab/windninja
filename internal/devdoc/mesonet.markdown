---
layout: internal
---

## MesoNet API for WindNinja


## Mesonet API Documentation
http://synopticlabs.org/api/mesonet/

## In WindNinja
Currently WindNinja uses the following variables from a station, wind speed, wind direction, air temperature, solar radiation, cloud layer codes.

Station Fetcher only fetches information from the RAWS and NWS/FAA networks currently. For information on which networks are available via the Mesonet API see [mesonames.csv](https://github.com/firelab/windninja/blob/station-fetch/data/mesonames.csv) (located in the station-fetch branch under data).

station-fetch also interprets NWS/FAA cloud codes as cloud cover through the following table
<table>
<tr><th>NOAA</th><th>Mesonet</th><th>okta</th><th>CloudCover</th></tr>
<tr><td>FEW</td><td>6</td><td>2/8</td><td>25</td></tr>
<tr><td>SCT</td><td>2</td><td>4/8</td><td>50</td></tr>
<tr><td>BKN</td><td>3</td><td>6/8</td><td>75</td></tr>
<tr><td>OVC</td><td>4</td><td>8/8</td><td>100</td></tr>
<tr><td>CLR</td><td>1</td><td>0/8</td><td>0</td></tr>
</table>

For information on how this conversion was done and how it is used in station-fetch,
see this [comment](https://github.com/firelab/windninja/blob/station-fetch/src/ninja/pointInitialization.cpp#L1077)

## Using StationFetch

StationFetch works by building a url to access the Mesonet API. The data is then downloaded and saved as a csv file.
There are different ways of accessing and downloading data from the API. All of these can be specified in[stationFetch.cfg](https://github.com/firelab/windninja/blob/station-fetch/data/stationFetch.cfg).

## New DEM Fetch

Station Fetch currently only works by reading in the bounds from the DEM file provided for a simulation, below information is not outdated, but not currently active.



### For All Fetching Types

All fetching types require a time period. If you want the latest data and a given number of hours back. Set "latest" to "true" and specify the number of hours for the field "forecast_duration". If you want a specific start and end time, set "latest" to "false" and use the "time" fields. "time_1_" is the start time and "time_2_" is the stop time.

### Single Station

If you know a station and its ID, simply set "fetch_type" to "single". and specify the station ID under "fetch_station_name"

this will save the data to a file named "single.csv"

### Multiple Stations  

For multiple stations set "fetch_type" to "multi" and specify the station IDs as a list under "fetch_station_name". For example: "kmso,katl,tr266".

this will save the data to a file named "multi.csv"

### Point and Radius

1. Set "fetch_type" to "point".
2. Specify a station ID as the initial point under "fetch_station_name".
3. Specify a radius under "radius"
4. Limit the number of stations fetched in the radius by setting "station_limit" to a value. Too many stations will generate an error by the Mesonet API.

this will save the data to a file named "point.csv"

### Latitude,Longitude Point and Radius

1. Set "fetch_type" to "latlon".
2. Specify an initial latitude under "point_latitude"
3. Specify an initial longitude under "point_longitude"
4. Specify a radius under "radius"
5. Limit the number of stations fetched in the radius by setting "station_limit" to a value. Too many stations will generate an error by the Mesonet API.  

this will saved the data to a file named "latlon.csv"

### Bounding Box

1. Set "fetch_type" to "box".
2. Specify the lower left latitude and longitude in the fields "box_lower_left_latitude" and "box_lower_left_longitude".
3. Specify the upper right latitude longitude in the fields starting with "box_upper_right_"
Note: Bounding Boxes cannot limit the number of stations it fetches, therefore specifying a very large box may generate an error by the Mesonet API.

this will save the data to a file named "box.csv"

### MesoWest API limits/restrictions

There is currently a hard-coded limit of 100,000 station-hours (number-of-hours × number-of-stations).
