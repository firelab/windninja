## WindNinja Weather Model Diurnal Tutorial README

This document provides guidance on setting up a  weather forecast model initialization run with diurnal winds
## RUNNING THE TEST CASE

**NOTE**: Python 3 or later is required to run the script.

To run this test case, execute the following command in the `wx_model_diurnal_case` directory:
- `wx_model_diurnal_case.py`

The output will appear in the `wx_model_diurnal_case` directory.

## CONFIGURATION FILE DETAILS

The following settings are included in the `cli_wxModelInitialization_diurnal.cfg` file. A list of required settings for CLI runs can be found here: [CLI Instructions for WindNinja](windninja/doc/CLI_instructions.pdf).



- `num_threads`: Number of threads for parallel processing.
- `elevation_file`: Path to the elevation data file (location differs for Windows and Linux).
- `initialization_method`: Initialization method.
- `time_zone`: Time zone for the simulation.
- `wx_model_type`: Type of weather model used.
- `forecast_duration`: Duration of the forecast in hours.
- `output_wind_height`: Height at which wind output will be generated.
- `units_output_wind_height`: Units for the output wind height.
- `vegetation`: Type of vegetation.
- `diurnal_winds`: If 'true', enables diurnal winds.
- `mesh_resolution`: Mesh resolution.
- `units_mesh_resolution`: Units for the mesh resolution.
- `write_goog_output`: If 'true', generates Google Earth output files.
- `write_shapefile_output`: If 'true', generates Shapefile output files.
- `write_ascii_output`: If 'true', generates ASCII output files.
- `write_farsite_atm`: If 'true', enables writing FARSITE atmosphere files.
- `write_wx_model_goog_output`: If 'true', creates Google Files for the weather model data.
- `write_wx_model_shapefile_output`: If 'true', generates Shapefile outputs for the weather model data.
- `write_wx_model_ascii_output`: If 'true', generates ASCII outputs for the weather model data.
