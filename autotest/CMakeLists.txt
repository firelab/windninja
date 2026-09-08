# THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
# MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT
# IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105
# OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT
# PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES
# LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER
# PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY,
# RELIABILITY, OR ANY OTHER CHARACTERISTIC.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

cmake_minimum_required (VERSION 3.16)

include_directories(${PROJECT_SOURCE_DIR}/src/ninja
                    ${NETCDF_INCLUDES}
                    ${GDAL_SYSTEM_INCLUDE} ${GDAL_INCLUDE_DIR}
                    ${CURL_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS})

set(LINK_LIBS ${NETCDF_LIBRARIES_C}
              ${GDAL_LIBRARY}
              ${CURL_LIBRARIES}
              ${Boost_LIBRARIES})

set(TEST_SOURCES test_main.cpp
                 unit/test_contain.cpp
                 unit/test_gdal_util.cpp
                 unit/test_grid_interp.cpp
                 unit/test_array2d.cpp
                 unit/test_timezone.cpp
                 unit/test_init.cpp
                 unit/test_buffer_grid.cpp
                 unit/test_ninjaunits.cpp
                 unit/test_rmtree.cpp
                 unit/test_utm.cpp
                 integration/test_srtm.cpp
                 integration/test_gdal_fetch.cpp
                 integration/test_gdal_output.cpp
                 integration/test_stl.cpp)

if(WITH_LCP_CLIENT)
    set(TEST_SOURCES ${TEST_SOURCES} integration/test_landfireclient.cpp)
endif(WITH_LCP_CLIENT)

if(ENABLE_GMTED)
    set(TEST_SOURCES ${TEST_SOURCES} integration/test_gmted.cpp)
endif(ENABLE_GMTED)

if(WITH_NOMADS_SUPPORT)
    set(TEST_SOURCES ${TEST_SOURCES} unit/test_utctime.cpp integration/test_simplenomadsclient.cpp)
endif(WITH_NOMADS_SUPPORT)

#if(NINJAFOAM)
    #SET(TEST_SOURCES ${TEST_SOURCES} test_foam.cpp)
#endif(NINJAFOAM)

add_executable(test_main ${TEST_SOURCES})
add_dependencies(test_main ninja)
target_link_libraries(test_main ninja ${LINK_LIBS})

#set(TEST_API FALSE)
#if(TEST_API)
    #add_executable( test_api test_capi.c )
    #target_link_libraries(test_api  ninja)
#endif(TEST_API)

enable_testing()

#if(TEST_API)
    #add_test(test_capi test_api)
#endif(TEST_API)

# Unit tests
# contain
add_test(NAME test_contain_point COMMAND $<TARGET_FILE:test_main> --run_test=contain/points)
add_test(NAME test_contain_polygon COMMAND $<TARGET_FILE:test_main> --run_test=contain/polygon)
add_test(NAME test_contain_polygon2 COMMAND $<TARGET_FILE:test_main> --run_test=contain/polygon2)

# grid_interp
add_test(NAME test_grid_interp_order COMMAND $<TARGET_FILE:test_main> --run_test=grid_interp/order)

# array2d
add_test(NAME test_array2d_constructor COMMAND $<TARGET_FILE:test_main> --run_test=array2d/constructor)

# timezone
add_test(NAME test_timezone_boise COMMAND $<TARGET_FILE:test_main> --run_test=timezones/boise)

# init
add_test(NAME test_init_gdal COMMAND $<TARGET_FILE:test_main> --run_test=init/gdal)

# buffer_grid
add_test(NAME test_buffer_grid_init COMMAND $<TARGET_FILE:test_main> --run_test=buffer_grid/init_and_set)

# ninjaUnits
add_test(NAME test_ninja_units COMMAND $<TARGET_FILE:test_main> --run_test=base_units/length_to_base_units)

# gdal_util
add_test(NAME test_gdal_util_center COMMAND $<TARGET_FILE:test_main> --run_test=gdal_util/center)

# landfireclient Test Suite - still experimental
if(WITH_LCP_CLIENT)
    add_test(NAME test_landfireclient_extract COMMAND $<TARGET_FILE:test_main> --run_test=landfireclient/unzip)
endif(WITH_LCP_CLIENT)

# input_points Test Suite
#if(NOT NINJA_GUI)
    # This test doesn't work
    #add_test(NAME test_input_points_mackay COMMAND $<TARGET_FILE:test_main> --run_test=input_points/mackay )
#endif(NOT NINJA_GUI)

if(NINJA_GDAL_OUTPUT)
    add_test(NAME test_gdal_output_shapefile COMMAND $<TARGET_FILE:test_main> --run_test=gdal_output/shapefile)
    add_test(NAME test_gdal_output_write_arrow COMMAND $<TARGET_FILE:test_main> --run_test=gdal_output/write_arrow)
    add_test(NAME test_gdal_output_arrow_west COMMAND $<TARGET_FILE:test_main> --run_test=gdal_output/arrow_west)
    add_test(NAME test_gdal_output_kml COMMAND $<TARGET_FILE:test_main> --run_test=gdal_output/kml)
    add_test(NAME test_gdal_output_kmz COMMAND $<TARGET_FILE:test_main> --run_test=gdal_output/kmz)
    add_test(NAME test_gdal_output_kmz_color COMMAND $<TARGET_FILE:test_main> --run_test=gdal_output/kmz_color)
endif(NINJA_GDAL_OUTPUT)

add_test(NAME test_utm_boise COMMAND $<TARGET_FILE:test_main> --run_test=utm/boise)

add_test(NAME test_utm_adelaide COMMAND $<TARGET_FILE:test_main> --run_test=utm/adelaide)

# ******************************************************************************
# Slow test section
# ******************************************************************************
if(RUN_SLOW_TESTS)
    add_test(NAME test_srtm_us_box COMMAND $<TARGET_FILE:test_main> --run_test=srtm/us_box)
    add_test(NAME test_srtm_world_pt COMMAND $<TARGET_FILE:test_main> --run_test=srtm/world_point)
    # add_test(NAME test_srtm_gdal COMMAND $<TARGET_FILE:test_main> --run_test=srtm/gdal)

    if(ENABLE_GMTED)
        add_test(NAME test_gmted_us COMMAND $<TARGET_FILE:test_main> --run_test=gmted/us)
        add_test(NAME test_gmted_world COMMAND $<TARGET_FILE:test_main> --run_test=gmted/world)
    endif(ENABLE_GMTED)

    add_test(NAME test_gdal_fetch_us_box COMMAND $<TARGET_FILE:test_main> --run_test=gdal_fetch/us_box)

    if(WITH_LCP_CLIENT)
        add_test(NAME test_landfireclient_download_conus COMMAND $<TARGET_FILE:test_main> --run_test=landfireclient/mackay)
        add_test(NAME test_landfireclient_download_alaska COMMAND $<TARGET_FILE:test_main> --run_test=landfireclient/alaska)
        add_test(NAME test_landfireclient_download_alaska_bad_geom COMMAND $<TARGET_FILE:test_main> --run_test=landfireclient/alaska_bad_geom)
        add_test(NAME test_landfireclient_download_alaska_out COMMAND $<TARGET_FILE:test_main> --run_test=landfireclient/alaska_out)
        add_test(NAME test_landfireclient_download_hawaii COMMAND $<TARGET_FILE:test_main> --run_test=landfireclient/hawaii)
    endif(WITH_LCP_CLIENT)
endif(RUN_SLOW_TESTS)

if(WITH_NOMADS_SUPPORT)
    add_test(NAME nomads_nam_conus_3_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay nam_conus 3 4 0 zip)
    add_test(NAME nomads_nam_conus_0_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay nam_conus 0 1 0 zip)
    add_test(NAME nomads_rap_mackay_18_limit_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay rap_conus 18 19 0 zip)
    add_test(NAME nomads_rap_north_america_1_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay rap_north_america 1 2 0 zip)
    add_test(NAME nomads_nam_north_america_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay nam_north_america 1 1 0 zip)
    add_test(NAME nomads_rap_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay rap_conus 1 2 0 zip)
    add_test(NAME nomads_gfs_mackay_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay gfs_global 1 1 0 zip)
    add_test(NAME nomads_gfs_alaska_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 alaska gfs_global 1 1 0 zip)
    add_test(NAME nomads_gfs_africa_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 africa gfs_global 1 1 0 zip)
    add_test(NAME nomads_gfs_south_america_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 south_america gfs_global 1 1 0 zip)
    add_test(NAME nomads_gfs_mackay_large_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay_large gfs_global 12 5 0 zip)
    add_test(NAME nomads_nam_alaska_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 alaska nam_alaska 1 2 0 zip)
    add_test(NAME nomads_hires_arw_conus_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay hires_arw_conus 1 2 0 zip)
    add_test(NAME nomads_hires_fv3_conus_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay hires_fv3_conus 1 2 0 zip)
    add_test(NAME nomads_hires_arw_ak_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay hires_arw_alaska 1 2 0 zip)
    add_test(NAME nomads_hires_fv3_ak_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay hires_fv3_alaska 1 2 0 zip)
    add_test(NAME nomads_hrrr_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay hrrr_conus 1 2 0 zip)
    add_test(NAME nomads_hrrr_ak_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 alaska hrrr_alaska 1 2 0 zip)
    add_test(NAME nomads_hrrr_15_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay hrrr_conus 15 16 0 zip)
    add_test(NAME nomads_hrrr_ext_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay hrrr_conus_ext 1 2 0 zip)
    add_test(NAME nomads_buffer_1 COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 small gfs_global 1 2 0 zip)

    if(NOMADS_EXPER_FORECASTS)
        add_test(NAME nomads_nam_nest_conus_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay nam_nest_conus 1 2 0 zip)
        add_test(NAME nomads_nest_conus_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay nam_nest_conus 1 2 0 zip)
        add_test(NAME nomads_nest_alaska_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay nam_nest_alaska 1 2 0 zip)
        add_test(NAME nomads_nam_narre_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay narre 1 2 0 zip)
    endif(NOMADS_EXPER_FORECASTS)

    if(NOMADS_RTMA)
        add_test(NAME nomads_rtma_conus_1_hour_zip COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/download_1 mackay rtma_conus 1 1 0 zip)
    endif(NOMADS_RTMA)

     # UTC unit tests
    add_test(NAME utc_create_1 COMMAND $<TARGET_FILE:test_main> --run_test=utc/create_1)
    add_test(NAME utc_add_hours_1 COMMAND $<TARGET_FILE:test_main> --run_test=utc/add_hours_1)
    add_test(NAME utc_add_hours_2 COMMAND $<TARGET_FILE:test_main> --run_test=utc/add_hours_2)
    add_test(NAME utc_add_hours_3 COMMAND $<TARGET_FILE:test_main> --run_test=utc/add_hours_3)
    add_test(NAME utc_add_hours_4 COMMAND $<TARGET_FILE:test_main> --run_test=utc/add_hours_4)
    add_test(NAME utc_add_hours_5 COMMAND $<TARGET_FILE:test_main> --run_test=utc/add_hours_5)
    add_test(NAME utc_add_hours_6 COMMAND $<TARGET_FILE:test_main> --run_test=utc/add_hours_6)
    add_test(NAME utc_add_hours_7 COMMAND $<TARGET_FILE:test_main> --run_test=utc/add_hours_7)
    add_test(NAME utc_add_hours_8 COMMAND $<TARGET_FILE:test_main> --run_test=utc/add_hours_8)
    add_test(NAME utc_add_hours_9 COMMAND $<TARGET_FILE:test_main> --run_test=utc/add_hours_9)
    add_test(NAME utc_now_1 COMMAND $<TARGET_FILE:test_main> --run_test=utc/now_1)
    add_test(NAME utc_compare_1 COMMAND $<TARGET_FILE:test_main> --run_test=utc/compare_1)
    add_test(NAME utc_compare_2 COMMAND $<TARGET_FILE:test_main> --run_test=utc/compare_2)
    add_test(NAME utc_compare_3 COMMAND $<TARGET_FILE:test_main> --run_test=utc/compare_3)
    add_test(NAME utc_timet_1 COMMAND $<TARGET_FILE:test_main> --run_test=utc/from_timet_1)
    add_test(NAME utc_iso_1 COMMAND $<TARGET_FILE:test_main> --run_test=utc/from_iso_1)
    add_test(NAME utc_iso_invalid_1 COMMAND $<TARGET_FILE:test_main> --run_test=utc/from_iso_invalid_1)
    add_test(NAME utc_strftime_1 COMMAND $<TARGET_FILE:test_main> --run_test=utc/strftime_1)
    add_test(NAME utc_copy_1 COMMAND $<TARGET_FILE:test_main> --run_test=utc/copy_1)
    add_test(NAME utc_copy_2 COMMAND $<TARGET_FILE:test_main> --run_test=utc/copy_2)
    add_test(NAME form_name_1 COMMAND $<TARGET_FILE:test_main> --run_test=simplenomadsclient/form_name_1)
endif(WITH_NOMADS_SUPPORT)

if(NINJAFOAM)
    # add_test(sample_cloud_1 $<TARGET_FILE:test_main> --run_test=ninjafoam/sample_cloud_1)
    add_test(NAME stl_1 COMMAND $<TARGET_FILE:test_main> --run_test=stl/stl_1)
    add_test(NAME stl_2 COMMAND $<TARGET_FILE:test_main> --run_test=stl/stl_2)
endif(NINJAFOAM)

# rmtree
add_test(NAME rmtree_1 COMMAND $<TARGET_FILE:test_main> --run_test=rmtree/rmtree_1)
add_test(NAME rmtree_sym_1 COMMAND $<TARGET_FILE:test_main> --run_test=rmtree/rmtree_sym_1)
add_test(NAME rmtree_sym_2 COMMAND $<TARGET_FILE:test_main> --run_test=rmtree/rmtree_sym_2)
add_test(NAME rmtree_sym_3 COMMAND $<TARGET_FILE:test_main> --run_test=rmtree/rmtree_sym_3)

if(RUN_CFG_TESTS)
    message(STATUS "Generating cli tests...")
    file(GLOB cfgtests ${PROJECT_SOURCE_DIR}/autotest/config/pass/*.cfg)
    message(STATUS "GLOB: ${cfgtests}")
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/autotest/config)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/autotest/config/pass)
    foreach(cfgtest ${cfgtests})
        get_filename_component(base ${cfgtest} NAME_WE)
        file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/autotest/config/pass/${base})
        file(COPY ${cfgtest} DESTINATION ${PROJECT_BINARY_DIR}/autotest/config/pass/${base})
        message(STATUS "COMMAND ${PROJECT_BINARY_DIR}/autotest/pc ${PROJECT_BINARY_DIR}/autotest/config/pass/${base}/${base}.cfg elevation_file")
        execute_process(WORKING_DIRECTORY . COMMAND python ${PROJECT_SOURCE_DIR}/autotest/config/pef.py ${PROJECT_BINARY_DIR}/autotest/config/pass/${base}/${base}.cfg elevation_file
                                OUTPUT_VARIABLE ELEV_FILE
                                OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(STRIP ${ELEV_FILE} ELEV_FILE)
        file(COPY ${PROJECT_SOURCE_DIR}/data/${ELEV_FILE} DESTINATION
             ${PROJECT_BINARY_DIR}/autotest/config/pass/${base})
        message(STATUS "Copying elevation file: ${ELEV_FILE}")
        add_test(NAME cfg_${base} COMMAND ${PROJECT_BINARY_DIR}/src/cli/WindNinja_cli ${PROJECT_BINARY_DIR}/autotest/config/pass/${base}/${base}.cfg
                 WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/autotest/config/pass/${base})
        set_tests_properties(cfg_${base} PROPERTIES ENVIRONMENT
                             "WINDNINJA_DATA=${PROJECT_SOURCE_DIR}/data")
        message(STATUS "Adding CLI test cfg_${base} for ${cfgtest}")
    endforeach(cfgtest ${cfgtests})
endif(RUN_CFG_TESTS)

