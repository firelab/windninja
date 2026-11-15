## WindNinja Domain Average Tutorial README
This document serves as a guide for initiating WindNinja simulations with a domain average initialization method.

## RUNNING THE TEST CASE
**NOTE**: Python 3 or a later version is required to run the script.

To execute this test case, run the following command in the domain_average_case directory:
- `python domain_average_case.py`

The results will be generated in the stability_case directory.

## CONFIGURATION FILE DETAILS
The provided `cli_domainAverage.cfg` file includes the following settings. For a comprehensive list of required settings, refer to [CLI Instructions for WindNinja](https://github.com/firelab/windninja/tree/master/doc/CLI_instructions.pdf)


- `num_threads`: Number of threads for parallel processing in the simulation.
- `elevation_file`: Location of the elevation data file.
- `initialization_method`: Set to domainAverageInitialization.
- `input_speed`: Wind speed for input.
- `input_direction`: Direction of the input wind.
- `input_wind_height`: Height for wind speed measurement.
- `output_wind_height`: Height for wind output generation.
- `vegetation`: Vegetation cover type.
- `mesh_choice`: Determines the resolution of the computational mesh.
- `write_goog_output`:Boolean to generate Google Earth output.
- `write_shapefile_output`: Boolean to generate Shapefile output.
- `write_ascii_output`: Boolean to generate ASCII output.
- `ascii_out_resolution`: ASCII output resolution.
- `units_ascii_out_resolution`: Units for the ASCII output resolution.
- `write_farsite_atm`: Set to true to enable writing FARSITE atmosphere files.
