WindNinja WRF Configuration File README

This document provides guidance on the use of a specific WindNinja CLI (Command Line Interface) configuration file. The file is tailored for initializing WindNinja simulations using WRF (Weather Research and Forecasting model) data.


WRF FILE DOWNLOAD

In order to run this test case, you must download the .nc file hosted here: https://ninjastorm.firelab.org/data/ and include the path of the downloaded file in the .cfg file.


RUNNING THE CFG FILE

To run, navigate to the location of the WindNinja_CLI.exe and run the following command:

../../data/tutorial/wrf_file_case/bigbutte_wrf_initialization.cfg

where ../../ represents the appropriate file path to the data directory. 



CONFIGURATION FILE DETAILS

the following is an explanation of the included settings in the .cfg file. 
A list of required settings for CLI runs can be found in windninja/doc/CLI_instructions.pdf

num_threads: Sets the number of threads (parallel processors) to be used for the simulation. Currently set to 1.
--
elevation_file: Path to the elevation data file used by WindNinja for the simulation.
--
initialization_method: Defines the initialization method for the simulation.
--
time_zone: Specifies the time zone for the simulation.
--
forecast_filename: path to the WRF weather model file. To run this test case, the WRF file must first be downloaded. A test file is provided above and in the .cfg file. 
--
forecast_duration: The duration of the forecast in hours. This setting dictates how long the wind simulation will run.
--
units_input_wind_height: The units for the input wind height. This is used in conjunction with weather model data to specify at what height the wind data should be considered.
--
output_wind_height: The height for which the wind output will be generated.
--
units_output_wind_height: Specifies the units for the output wind height.
--
vegetation: Sets the type of vegetation cover for the simulation.
--
mesh_choice: Determines the resolution of the computational mesh. (A 'fine' mesh means more detailed simulations, but requires more computational power.)
--
write_goog_output: If set to true, WindNInja will generate output files that can be viewed in Google Earth.
--
write_shapefile_output: When true, shapefile outputs of the simulation results will be generated.
--
write_ascii_output: When true, ASCII outputs of the simulation results will be generated.
--
ascii_out_resolution: Required when generating ASCII output. The resolution of the output.
--
units_ascii_out_resolution: Required when generating ASCII output. Specifies the units for the output resolution.
--
write_wx_model_goog_output: When true, WindNinja will create Google Files specifically for the weather model initialization data.