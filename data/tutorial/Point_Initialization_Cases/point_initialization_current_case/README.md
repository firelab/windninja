## WindNinja Point Initialization Tutorial README

This document provides guidance on setting up a single time step point initalization run with local data and diurnal winds. 
## RUNNING THE TEST CASE

**NOTE**: Python 3 or later is required to run the script.

To run this test case, execute the following command in the `cli_point_initialization_current_case` directory:
- `python cli_pointInitialization_current_case.py`

The output will appear in the `cli_point_initialization_current_case` directory.

## CONFIGURATION FILE DETAILS

The following settings are included in the `cli_pointInitialization_current.cfg` file. A list of required settings for CLI runs can be found here: [CLI Instructions for WindNinja](windninja/doc/CLI_instructions.pdf).


- `num_threads`: Number of threads for parallel processing.
- `elevation_file`: Path to the elevation data file.
- `time_zone`: Time zone for the simulation.
- `initialization_method`: Initialization method.
- `match_points`: If 'true', matches points for initialization.
- `wx_station_filename`: Path to the weather station data file.
- `write_wx_station_kml`: If 'true', writes a KML file for weather stations.
- `output_wind_height`: Height at which wind output will be generated.
- `units_output_wind_height`: Units for the output wind height.
- `vegetation`: Type of vegetation.
- `mesh_resolution`: Mesh resolution.
- `units_mesh_resolution`: Units for the mesh resolution.
- `write_goog_output`: If 'true', generates Google Earth output files.
- `write_shapefile_output`: If 'true', generates Shapefile output files.
- `write_ascii_output`: If 'true', generates ASCII output files.
- `write_farsite_atm`: If 'true', enables writing FARSITE atmosphere files.
