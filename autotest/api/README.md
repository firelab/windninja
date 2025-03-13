# C API for WindNinja

Authors: Natalie Wagenbrenner, Nicholas Kim, Mason Willman

Below are test cases to showcase C API WindNinja functionality. This is currently in development and subject to change. 

## Environment Setup 
The file libninja.so is required to be installed on the system. To install this file, follow the build steps for your OS found in the [WindNinja README](https://github.com/firelab/windninja). After the installation, you should see ```Installing: /usr/local/lib/libninja.so``` or ```Up-to-date: /usr/local/lib/libninja.so``` (Ubuntu).

## 1. Test for a simple domain average wind simulation

```bash
gcc -g -Wall -o test_capi_domain_average_wind test_capi_domain_average_wind.c -lninja
./test_capi_domain_average_wind
```

## 2. Tests for NinjaFetchDEMBBox, NinjaFetchForecast, NinjaFetchDemPoint, and NinjaFetchStation

```bash
gcc -g -Wall -o test_capi_fetching test_capi_fetching.c -lninja
./test_capi_fetching
```

## 3. Test for simple weather model intialization run (Still WIP)

```bash
gcc -g -Wall -o test_capi_weather_model_initialization_wind test_capi_weather_model_initialization_wind.c -lninja
./test_capi_weather_model_initialization_wind
```

## 4. Test for simple point model intialization run (Still WIP)

```bash
gcc -g -Wall -o test_capi_point_initialization_wind test_capi_point_initialization_wind.c -lninja
./test_capi_point_initialization_wind
```