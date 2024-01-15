## WindNinja Point Initialization Tutorial README

This document provides guidance on setting up a multi time step point initalization run downloading remote weather data and with diurnal winds. 
## RUNNING THE TEST CASE

**NOTE**: Python 3 or later is required to run the script.

To run this test case, execute the following command in the `cli_point_initialization_diurnal_case` directory:
- `python cli_point_initialization_diurnal_case.py`

The output will appear in the `cli_point_initialization_diurnal_case` directory.

## CONFIGURATION FILE DETAILS

The following settings are included in the `cli_pointInitialization_diurnal.cfg` file. A list of required settings for CLI runs can be found here: [CLI Instructions for WindNinja](windninja/doc/CLI_instructions.pdf).



- `num_threads`: Number of threads for parallel processing.
- `elevation_file`: Path to the elevation data file.
- `initialization_method`: Initialization method.
- `time_zone`: Time zone for the simulation; set to auto-detect.
- `match_points`: If 'true', matches points for initialization.
- `fetch_station`: If 'true', enables fetching of remote weather stations.
- `fetch_type`: Type of data fetching method; set to 'bbox' for a specific geographic bounding box.
- `diurnal_winds`: If 'true', enables diurnal winds.
- `write_wx_station_kml`: If 'true', writes a KML file for weather stations.
- `start_year`: Start year for the simulation.
- `start_month`: Start month for the simulation.
- `start_day`: Start day for the simulation.
- `start_hour`: Start hour for the simulation.
- `start_minute`: Start minute for the simulation.
- `stop_year`: Stop year for the simulation.
- `stop_month`: Stop month for the simulation.
- `stop_day`: Stop day for the simulation.
- `stop_hour`: Stop hour for the simulation.
- `stop_minute`: Stop minute for the simulation.
- `number_time_steps`: Number of time steps for the simulation.
- `station_buffer`: Buffer distance around stations.
- `station_buffer_units`: Units for the station buffer.
- `output_wind_height`: Height at which wind output will be generated.
- `units_output_wind_height`: Units for the output wind height.
- `vegetation`: Type of vegetation.
- `mesh_resolution`: Mesh resolution.
- `units_mesh_resolution`: Units for the mesh resolution.
- `write_goog_output`: If 'true', generates Google Earth output files.
- `write_shapefile_output`: If 'true', generates Shapefile output files.
- `write_ascii_output`: If 'true', generates ASCII output files.
- `write_farsite_atm`: If 'true', enables writing FARSITE atmosphere files.
