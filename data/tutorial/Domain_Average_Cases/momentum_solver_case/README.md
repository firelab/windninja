## WindNinja Momentum Solver Tutorial README

This document provides guidance on initializing WindNinja simulations that use a domain average initialization method, a momentum solver, and diurnal winds. 

## RUNNING THE TEST CASE

**NOTE**: Python 3 or later is required to run the script.

To run this test case, execute the following command in the momentum_solver_case directory:
- `python momentum_solver_case.py`

The output will appear in the momentum_solver_case directory.

## CONFIGURATION FILE DETAILS

The following settings are included in the `cli_momentumSolver_diurnal.cfg` file. A list of required settings for CLI runs can be found here: [CLI Instructions for WindNinja](windninja/doc/CLI_instructions.pdf).

- `num_threads`: Number of threads for parallel processing.
- `elevation_file`: Path to the elevation data file.
- `initialization_method`: Initialization method.
- `momentum_flag`: If 'true', enables the momentum solver.
- `number_of_iterations`: Number of iterations for the simulation.
- `mesh_choice`: Choice of computational mesh.
- `time_zone`: Time zone for the simulation area.
- `input_speed`: Wind input speed.
- `input_direction`: Direction of the input wind.
- `uni_air_temp`: Uniform air temperature across the domain.
- `air_temp_units`: Units for the air temperature.
- `uni_cloud_cover`: Uniform cloud cover percentage.
- `cloud_cover_units`: Units for cloud cover.
- `input_wind_height`: Height at which wind speed is measured.
- `units_input_wind_height`: Units for the input wind height.
- `output_wind_height`: Height at which wind output will be generated.
- `units_output_wind_height`: Units for the output wind height.
- `vegetation`: Type of vegetation.
- `diurnal_winds`: If 'true', enables diurnal winds.
- `year`: Year for the simulation.
- `month`: Month for the simulation.
- `day`: Day for the simulation.
- `hour`: Hour for the simulation.
- `minute`: Minute for the simulation.
- `write_goog_output`: If 'true', generates Google Earth output.
- `write_shapefile_output`: If 'true', generates Shapefile output.
- `write_ascii_output`: If 'true', generates ASCII output.
- `write_farsite_atm`: If 'true', enables writing FARSITE atmosphere files.
