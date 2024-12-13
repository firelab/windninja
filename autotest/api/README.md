Authors: Natalie Wagenbrenner, Nicholas Kim

These document tests to run for the C API.

1. Test for a simple domain average wind simulation

gcc -g -Wall -o test_capi_domain_average_wind test_capi_domain_average_wind.c -lninja
./test_capi_domain_average_wind

2. Tests for NinjaFetchDEMBBox, NinjaFetchForecast, NinjaFetchDemPoint, and NinjaFetchStation

gcc -g -Wall -o test_capi_fetching test_capi_fetching.c -lninja
./test_capi_fetching

3. TODO: create a test for weather model intialization run

4. TODO: create a test for point model intialization run (look inside apiTestPoint.c for starter code)