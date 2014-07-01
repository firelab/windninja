#/*****************************************************************************
# *
# * $Id$
# *
# * Project:  WindNinja
# * Purpose:  CMake script
# * Author:   Levi Malott <lmnn3@mst.edu
# *
# *****************************************************************************
# *
# * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
# * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT
# * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105
# * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT
# * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES
# * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER
# * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY,
# * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
# *
# * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# * DEALINGS IN THE SOFTWARE.
# *
# ****************************************************************************/
# This is set arbitrarily at 2.6.  If there is proof that it works better, 
# set it.  Also set all sub CMakeLists.txt
# *****************************************************************************
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# *****************************************************************************
# Includes
# *****************************************************************************
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/ninja
                    ${NETCDF_INCLUDES}
                    ${GDAL_INCLUDE_DIR}
                    ${CURL_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS})

SET(LINK_LIBS ${NETCDF_LIBRARIES_C}
              ${GDAL_LIBRARY}
              ${CURL_LIBRARIES}
              ${Boost_LIBRARIES})

             
IF(WIN32)
    SET(LINK_LIBS ${LINK_LIBS}
                  ${CMAKE_BINARY_DIR}/src/ninja/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}ninja${CMAKE_STATIC_LIBRARY_SUFFIX})
ELSE(WIN32)
    SET(LINK_LIBS ${LINK_LIBS}
                  ${CMAKE_BINARY_DIR}/src/ninja/${CMAKE_SHARED_LIBRARY_PREFIX}ninja${CMAKE_SHARED_LIBRARY_SUFFIX})
ENDIF(WIN32)

#Copy over surface_data folder, necessary for custom gdal_fetch
FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/surface_data
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


# *****************************************************************************
# Test Compilation
# *****************************************************************************

SET(TEST_SOURCES test_main.cpp 
                 test_contain.cpp 
                 test_srtm.cpp 
                 test_grid_interp.cpp
                 test_array2d.cpp
                 test_timezone.cpp
                 test_init.cpp
                 #test_input_points.cpp
                 test_buffer_grid.cpp
                 test_stl)
IF(WITH_LCP_CLIENT)
    SET(TEST_SOURCES ${TEST_SOURCES} test_landfireclient.cpp)
ENDIF(WITH_LCP_CLIENT)

IF(ENABLE_GMTED)
    SET(TEST_SOURCES ${TEST_SOURCES} test_gmted.cpp)
ENDIF(ENABLE_GMTED)

IF(WITH_NOMADS_SUPPORT)
    SET(TEST_SOURCES ${TEST_SOURCES}
                     test_simplenomadsclient.cpp
                     test_utctime.cpp)
ENDIF(WITH_NOMADS_SUPPORT)

ADD_EXECUTABLE(test_main ${TEST_SOURCES})

# *****************************************************************************
# Add executable dependencies
# *****************************************************************************
ADD_DEPENDENCIES(test_main ninja)
#ADD_DEPENDENCIES(test_api  ninja)


# *****************************************************************************
# Link libraries to executable
# *****************************************************************************
TARGET_LINK_LIBRARIES(test_main ${LINK_LIBS})
#disabled for now
SET(TEST_API FALSE)
IF(TEST_API)
    ADD_EXECUTABLE( test_api test_capi.c )
    TARGET_LINK_LIBRARIES(test_api  ninja)
    #SET_TARGET_PROPERTIES( test_api PROPERTIES LINKER_LANGUAGE CXX )
ENDIF(TEST_API)

ENABLE_TESTING()

# *****************************************************************************
# General Unit Test Section
# *****************************************************************************

# C API testing
IF(TEST_API)
    ADD_TEST(test_capi test_api)
ENDIF(TEST_API)

# contain Test Suite
ADD_TEST(test_contain_point     
         ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=contain/points )
ADD_TEST(test_contain_polygon   
         ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=contain/polygon )
ADD_TEST(test_contain_polygon2  
         ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=contain/polygon2 )

# grid_interp Test Suite
ADD_TEST(test_grid_interp_order 
         ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=grid_interp/order )

# array2d Test Suite
ADD_TEST(test_array2d_constructor 
         ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=array2d/constructor )

# timezone Test Suite
ADD_TEST(test_timezone_boise    
         ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=timezones/boise )

# init Test Suite
ADD_TEST(test_init_gdal         
         ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=init/gdal )

# buffer_grid Test Suite
ADD_TEST(test_buffer_grid_init  
         ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=buffer_grid/init_and_set)

# landfireclient Test Suite - still experimental
IF(WITH_LCP_CLIENT)
    ADD_TEST(test_landfireclient_extract
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=landfireclient/unzip)
ENDIF(WITH_LCP_CLIENT)

# input_points Test Suite
IF(NOT NINJA_QTGUI)
    ADD_TEST(test_input_points_mackay         
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=input_points/mackay )
ENDIF(NOT NINJA_QTGUI)

ADD_TEST(stl_1 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=stl/stl_1)

# ******************************************************************************
# Slow test section
# ******************************************************************************
IF(RUN_SLOW_TESTS)
    # strm Test Suite
    ADD_TEST(test_srtm_us_box       
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=srtm/us_box )
    ADD_TEST(test_srtm_world_pt     
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=srtm/world_point )
    #ADD_TEST(test_srtm_gdal         
    #         ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=srtm/gdal )

    #gmted Test Suite
    IF(ENABLE_GMTED)
        ADD_TEST(test_gmted_us      
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=gmted/us )
        ADD_TEST(test_gmted_world   
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=gmted/world )
    ENDIF(ENABLE_GMTED)

    IF(WITH_LCP_CLIENT)
        ADD_TEST(test_landfireclient_download_conus
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=landfireclient/mackay )
        ADD_TEST(test_landfireclient_download_alaska
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=landfireclient/alaska )
        ADD_TEST(test_landfireclient_download_alaska_bad_geom
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=landfireclient/alaska_bad_geom)
        ADD_TEST(test_landfireclient_download_alaska_out
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=landfireclient/alaska_out)
        ADD_TEST(test_landfireclient_download_hawaii
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=landfireclient/hawaii )
    ENDIF(WITH_LCP_CLIENT)
ENDIF(RUN_SLOW_TESTS)
IF(WITH_NOMADS_SUPPORT)
    OPTION(RUN_NOMADS_ZIP_TESTS "Try to download to zip files" OFF)
    IF( RUN_NOMADS_ZIP_TESTS)
    ADD_TEST(nomads_nam_conus_3_hour_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay nam_conus 3 4 0 zip)
    ADD_TEST(nomads_nam_conus_39_hour_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay nam_conus 39 38 0 zip)
    ADD_TEST(nomads_nam_conus_0_hour_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay nam_conus 0 1 0 zip)
    ADD_TEST(nomads_nam_north_america_39_hour_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay nam_north_america 39 14 0 zip)
    ADD_TEST(nomads_gfs_mackay_39_hour_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay gfs 39 14 0 zip)
    ADD_TEST(nomads_rap_mackay_18_limit_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay rap 18 19 0 zip)
    ADD_TEST(nomads_nam_north_america_1_hour_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay nam_north_america 1 1 0 zip)
    ADD_TEST(nomads_rap_1_hour_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay rap 1 2 0 zip)
    ADD_TEST(nomads_gfs_mackay_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay gfs 1 1 0 zip)
    ADD_TEST(nomads_gfs_alaska_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 alaska gfs 1 1 0 zip)
    ADD_TEST(nomads_gfs_africa_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 africa gfs 1 1 0 zip)
    ADD_TEST(nomads_gfs_south_america_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 south_america gfs 1 1 0 zip)
    ADD_TEST(nomads_gfs_mackay_large_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay_large gfs 12 5 0 zip)
    ADD_TEST(nomads_nam_alaska_1_hour_zip
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 alaska nam_alaska 1 1 0 zip)
    IF(NOMADS_EXPER_FORECASTS)
        ADD_TEST(nomads_nam_narre_1_hour_zip
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay narre 1 2 0 zip)
        ADD_TEST(nomads_rtma_conus_1_hour_zip
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay rtma_conus 1 1 0 zip)
    ENDIF(NOMADS_EXPER_FORECASTS)
ENDIF(RUN_NOMADS_ZIP_TESTS)
    ADD_TEST(nomads_nam_conus_0_hour_path
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay nam_conus 0 1 0 path)
    ADD_TEST(nomads_nam_conus_3_hour_path
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay nam_conus 3 4 0 path)
    IF(RUN_SLOW_TESTS)
        ADD_TEST(nomads_nam_conus_39_hour_path
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay nam_conus 39 38 0 path)
        ADD_TEST(nomads_nam_north_america_39_hour_path
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay nam_north_america 39 14 0 path)
        ADD_TEST(nomads_gfs_mackay_39_hour_path
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay gfs 39 14 0 path)
        ADD_TEST(nomads_rap_mackay_18_limit_path
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay rap 18 19 0 path)
    ENDIF(RUN_SLOW_TESTS)
    ADD_TEST(nomads_nam_north_america_1_hour_path
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay nam_north_america 1 1 0 path)
    ADD_TEST(nomads_rap_1_hour_path
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay rap 1 2 0 path)
    ADD_TEST(nomads_gfs_mackay_path
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay gfs 1 1 0 path)
    ADD_TEST(nomads_gfs_alaska_path
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 alaska gfs 1 1 0 path)
    ADD_TEST(nomads_gfs_africa_path
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 africa gfs 1 1 0 path)
    ADD_TEST(nomads_gfs_south_america_path
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 south_america gfs 1 1 0 path)
    ADD_TEST(nomads_gfs_mackay_large_path
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay_large gfs 12 5 0 path)
    ADD_TEST(nomads_nam_alaska_1_hour_path
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 alaska nam_alaska 1 1 0 path)
    IF(NOMADS_EXPER_FORECASTS)
        ADD_TEST(nomads_nam_narre_1_hour_path
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay narre 1 2 0 path)
        ADD_TEST(nomads_rtma_conus_1_hour_path
                 ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=simplenomadsclient/download_1 mackay rtma_conus 1 1 0 path)
    ENDIF(NOMADS_EXPER_FORECASTS)
    # UTC
    ADD_TEST(utc_create_1
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=utc/create_1)
    ADD_TEST(utc_add_hours_1
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=utc/add_hours_1)
    ADD_TEST(utc_add_hours_2
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=utc/add_hours_2)
    ADD_TEST(utc_add_hours_3
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=utc/add_hours_3)
    ADD_TEST(utc_strptime_1
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=utc/strptime_1)
    ADD_TEST(utc_now_1
             ${EXECUTABLE_OUTPUT_PATH}/test_main --run_test=utc/now_1)
ENDIF(WITH_NOMADS_SUPPORT)

# *****************************************************************************
# Command Line Interface Testing Section
# *****************************************************************************
IF(RUN_CLI_TESTS)
    INCLUDE(FindPythonInterp)
    IF(PYTHONINTERP_FOUND)
        FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/testing.py
             DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	    FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/output
             DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
             
        SET( configs cli_domainAverage
                      cli_domainAverage_diurnal
                      cli_pointInitialization
                      cli_pointInitialization_diurnal
                      cli_wxModelInitialization_local_model
                      cli_wxModelInitialization_diurnal_local_model
           )
             
         FOREACH( cfg_file ${configs} )
             ADD_TEST( ${cfg_file} ${PYTHON_EXECUTABLE} testing.py -c ${cfg_file}.cfg )
         ENDFOREACH( cfg_file )
           
    ENDIF(PYTHONINTERP_FOUND)
ENDIF(RUN_CLI_TESTS)

