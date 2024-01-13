## WindNinja Point Initialization Tutorial README

This document provides guidance on setting up WindNinja simulations using WRF (Weather Research and Forecasting model) data.

## RUNNING THE TEST CASE

**NOTE**: Python 3 or later is required to run the script.

To run this test case, execute the following command in the `wrf_file_case` directory:
- `python wrf_case.py`

The output will appear in the `wrf_file_case` directory.

## CONFIGURATION FILE DETAILS

The following settings are included in the `bigbutte_wrf_initialization.cfg` file. A list of required settings for CLI runs can be found here: [CLI Instructions for WindNinja](windninja/doc/CLI_instructions.pdf).


- `num_threads`: Sets the number of threads (parallel processors) for the simulation.
- `elevation_file`: Path to the elevation data file used by WindNinja for the simulation.
- `initialization_method`: Defines the initialization method for the simulation.
- `time_zone`: Specifies the time zone for the simulation.
- `forecast_filename`: Path to the WRF weather model file. 
- `forecast_duration`: The duration of the forecast in hours
- `units_input_wind_height`: Units for the input wind height, used with weather model data.
- `output_wind_height`: The height for which the wind output will be generated.
- `units_output_wind_height`: Units for the output wind height.
- `vegetation`: Type of vegetation cover for the simulation.
- `mesh_choice`: Determines the resolution of the computational mesh.
- `write_goog_output`: If 'true', generates Google Earth output files.
- `write_shapefile_output`: If 'true', generates shapefile outputs of the simulation results.
- `write_ascii_output`: If 'true', generates ASCII outputs of the simulation results.
- `ascii_out_resolution`: Resolution for ASCII output.
- `units_ascii_out_resolution`: Units for the ASCII output resolution.
- `write_wx_model_goog_output`: If 'true', creates Google Files for the weather model initialization data.
