Authors: Natalie Wagenbrenner, Nicholas Kim, Mason Willman

These document tests to run for the C API.

1. Test for a simple domain average wind simulation

gcc -g -Wall -o test_capi_domain_average_wind test_capi_domain_average_wind.c -lninja
./test_capi_domain_average_wind

2. Tests for NinjaFetchDEMBBox, NinjaFetchForecast, NinjaFetchDemPoint, and NinjaFetchStation (currently FetchStation is not working)

gcc -g -Wall -o test_capi_fetching test_capi_fetching.c -lninja
./test_capi_fetching

3. Test for simple weather model intialization run 

gcc -g -Wall -o test_capi_weather_model_initialization_wind test_capi_weather_model_initialization_wind.c -lninja
./test_capi_weather_model_initialization_wind

4. Test for simple point model intialization run 

gcc -g -Wall -o test_capi_point_initialization_wind test_capi_point_initialization_wind.c -lninja
./test_capi_point_initialization_wind