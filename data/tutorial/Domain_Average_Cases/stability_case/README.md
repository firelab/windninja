## WindNinja Stability Tutorial README

This document provides guidance oninitializing WindNinja simulations using a domain average initialization method and non-neutral atmospheric stability effects.

## RUNNING THE TEST CASE

**NOTE**: Python 3 or later is required to run the script.

To run this test case, execute the following command in the stability_case directory:
- `python stability_case.py`

The output will appear in the stability_case directory.

## CONFIGURATION FILE DETAILS

The following settings are included in the `cli_bigbutte_stability.cfg` file. A list of required settings for CLI runs can be found here: [CLI Instructions for WindNinja](windninja/doc/CLI_instructions.pdf).

- `num_threads`: Sets the number of threads (parallel processors) for the simulation.
- `elevation_file`: Path to the elevation data file used in the simulation.
- `initialization_method`: The initialization method for the simulation, currently set to `domainAverageInitialization`.
- `input_speed`: The speed of the input wind.
- `input_speed_units`: Units of the input wind speed.
- `input_direction`: The direction of the input wind.
- `input_wind_height`: The height at which the wind speed is measured.
- `units_input_wind_height`: Units for the input wind height.
- `output_wind_height`: The height at which the wind output will be generated.
- `units_output_wind_height`: Units for the output wind height.
- `vegetation`: Type of vegetation cover for the simulation.
- `mesh_choice`: Determines the resolution of the computational mesh.
- `write_goog_output`: If `true`, generates Google Earth output files.
- `write_shapefile_output`: If `true`, generates Shapefile output.
- `write_ascii_output`: If `true`, generates ASCII output files.
- `ascii_out_resolution`: Resolution for ASCII output.
- `units_ascii_out_resolution`: Units for the ASCII output resolution.
- `non_neutral_stability`: Set to `true` to consider atmospheric stability.
- `alpha_stability`: Used to adjust the atmospheric stability.
