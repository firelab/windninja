#/*****************************************************************************
# *
# * $Id$
# *
# * Project:  WindNinja
# * Purpose:  CMake script
# * Author:   Kyle Shannon <kyle@pobox.com>
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
PROJECT(WindNinja)

# *****************************************************************************
# Add our cmake directory to the path for netcdf, gdal and proj
# *****************************************************************************
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# *****************************************************************************
# Set this option early so we can default stuff.
# *****************************************************************************
OPTION(FIRELAB_PACKAGE "Build a windows installer with dlls from the firelab machine" OFF)
MARK_AS_ADVANCED(FIRELAB_PACKAGE)

SET(WINDNINJA_VERSION_MAJOR "2")
SET(WINDNINJA_VERSION_MINOR "4")
SET(WINDNINJA_VERSION_PATCH "1")
OPTION(GENERATE_VERSION_INFO "Write version info files." OFF)
IF(GENERATE_VERSION_INFO)
    FILE(READ ${CMAKE_SOURCE_DIR}/VERSION V)
    STRING(REPLACE "\n" "" V ${V})
    IF(V)
        STRING(REPLACE "." ";" VLIST ${V})
        LIST(LENGTH VLIST VCOUNT)
        IF(VCOUNT GREATER 0)
            LIST(GET VLIST 0 WINDNINJA_VERSION_MAJOR)
        ENDIF(VCOUNT GREATER 0)
        IF(VCOUNT GREATER 1)
            LIST(GET VLIST 1 WINDNINJA_VERSION_MINOR)
        ENDIF(VCOUNT GREATER 1)
        IF(VCOUNT GREATER 2)
            LIST(GET VLIST 2 WINDNINJA_VERSION_PATCH)
        ENDIF(VCOUNT GREATER 2)
    ENDIF(V)
    SET(WINDNINJA_VERSION ${WINDNINJA_VERSION_MAJOR}.${WINDNINJA_VERSION_MINOR}.${WINDNINJA_VERSION_PATCH})
    FILE(READ ${CMAKE_SOURCE_DIR}/disclaimer VERSION_HEADER)
    IF(NOT GITCOMMAND)
        SET(GIT_EXE git)
    ELSE(NOT GITCOMMAND)
        SET(GIT_EXE ${GITCOMMAND})
    ENDIF(NOT GITCOMMAND)
    EXECUTE_PROCESS(COMMAND ${GIT_EXE} "rev-parse" "HEAD"
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_RC
                    OUTPUT_VARIABLE GIT_REV)
    IF(GIT_RC)
        SET(GIT_REV "NOT SET")
    ENDIF(GIT_RC)
    STRING(REPLACE "\n" "" GIT_REV ${GIT_REV})

    INCLUDE(GetDate)
    GET_DATE()

    SET(V "${VERSION_HEADER}
#ifndef VERSION_H
#define VERSION_H
#define VERSION \"${WINDNINJA_VERSION}\"
#define SVN_VERSION \"${GIT_REV}\"
#define RELEASE_DATE \"${GD_MONTH}/${GD_DAY}/${GD_YEAR}\"
#endif /* VERSION_H */\n")
    FILE(WRITE "${CMAKE_SOURCE_DIR}/src/version.h" ${V})
ENDIF(GENERATE_VERSION_INFO)
SET(WINDNINJA_VERSION ${WINDNINJA_VERSION_MAJOR}.${WINDNINJA_VERSION_MINOR}.${WINDNINJA_VERSION_PATCH})
SET(WINDNINJA_VERSION_NAME WindNinja-${WINDNINJA_VERSION})

# *****************************************************************************
# Find boost libs, required for all builds.  1.41.1 may be arbitrary.
# Have to check docs for actual support of our features.  We use static libs
# on windows just because it's easier.
# *****************************************************************************
INCLUDE(FindBoost)
SET(Boost_DEBUG ON)
IF(WIN32)
    SET(Boost_USE_STATIC_LIBS ON)
ELSE(WIN32)
    SET(Boost_USE_STATIC_LIBS OFF)
ENDIF(WIN32)
SET(Boost_USE_MULTITHREAD OFF)
FIND_PACKAGE(Boost 1.41.1 COMPONENTS date_time program_options
                                     unit_test_framework REQUIRED)

# *****************************************************************************
# We now use gdals internal zlib
# Find zlib, required for all builds
# *****************************************************************************
#INCLUDE(FindZLIB)
#FIND_PACKAGE(ZLIB REQUIRED)

# *****************************************************************************
# We must now check for curl support in GDAL, don't know how yet.
# Find curl, required for all builds
# *****************************************************************************
#INCLUDE(FindCURL)
#FIND_PACKAGE(CURL REQUIRED)

# *****************************************************************************
# Find netcdf, we use the netcdf api directly to query wx model files.  See
# cmake/Modules/FindNetCDF.cmake for more info.
# *****************************************************************************
INCLUDE(FindNetCDF)
FIND_PACKAGE(NetCDF REQUIRED)

# *****************************************************************************
# Find gdal, version must be 1.8.1+ right now.  Have NOT tested 1.9,
# and significant changes were made in the netcdf driver.  I *think* as long
# as we warp the data, the units issue is okay.  Stick w/1.8.1 for now.
# *****************************************************************************
INCLUDE(FindGDAL)
FIND_PACKAGE(GDAL REQUIRED)

# *****************************************************************************
# We don't link to proj directly, but gdal must have proj support to perform
# warping.  A better test would be to go through gdal and check for proj 
# support.  I am not completely sure how to do this.
# *****************************************************************************
#INCLUDE(FindProj)
#FIND_PACKAGE(Proj)

IF(WITH_NOMADS_SUPPORT )
    INCLUDE(CheckFunctionExists)
    CHECK_FUNCTION_EXISTS(timegm HAVE_TIMEGM)
    IF(NOT HAVE_TIMEGM)
        ADD_DEFINITIONS(-DUSE_INTERNAL_TIME_GM)
    ENDIF(NOT HAVE_TIMEGM)
    OPTION(NOMADS_USE_VSI_READ "Use VSI*L api instead of CPLHTTPFetch" OFF)
    IF(NOMADS_USE_VSI_READ)
        ADD_DEFINITIONS(-DNOMADS_USE_VSI_READ)
    ENDIF(NOMADS_USE_VSI_READ)
    OPTION(NOMADS_RTMA "Enable the RTMA forecasts" ON)
    IF(NOMADS_RTMA)
        ADD_DEFINITIONS(-DNOMADS_RTMA)
    ENDIF(NOMADS_RTMA)
    OPTION(NOMADS_EXPER_FORECASTS "Enable NARRE and NEST forecasts." OFF)
    IF(NOMADS_EXPER_FORECASTS)
        ADD_DEFINITIONS(-DNOMADS_EXPER_FORECASTS)
    ENDIF(NOMADS_EXPER_FORECASTS)
    OPTION(NOMADS_ENABLE_ASYNC "Enable multi-threading for NOMADS download" ON)
    IF(NOMADS_ENABLE_ASYNC)
        ADD_DEFINITIONS(-DNOMADS_ENABLE_ASYNC)
    ENDIF(NOMADS_ENABLE_ASYNC)
    OPTION(NOMADS_USE_IP "Use the ip address for the nomads server, not a host" OFF)
    IF(NOMADS_USE_IP)
        ADD_DEFINITIONS(-DNOMADS_USE_IP)
    ENDIF(NOMADS_USE_IP)
    OPTION(NOMADS_ENABLE_3D "Enable 3d initialization using NOMADS data." OFF)
    IF(NOMADS_ENABLE_3D)
        ADD_DEFINITIONS(-DNOMADS_ENABLE_3D)
    ENDIF(NOMADS_ENABLE_3D)
    OPTION(NOMADS_TEST_PARA "Use the nomads pre-release parallel test server." OFF)
    IF(NOMADS_TEST_PARA)
        ADD_DEFINITIONS(-DNOMADS_TEST_PARA)
    ENDIF(NOMADS_TEST_PARA)
ENDIF(WITH_NOMADS_SUPPORT)

OPTION(NINJAFOAM "Enable OpenFOAM solver" OFF)
IF(NINJAFOAM)
    ADD_DEFINITIONS(-DNINJAFOAM)
ENDIF(NINJAFOAM)

OPTION(NINJA_SPEED_TESTING "Enable initialization speed dampening" OFF)
IF(NINJA_SPEED_TESTING)
    ADD_DEFINITIONS(-DNINJA_SPEED_TESTING)
ENDIF(NINJA_SPEED_TESTING)

OPTION(FRICTION_VELOCITY "Enable friction velocity calculations" OFF)
IF(FRICTION_VELOCITY)
    ADD_DEFINITIONS(-DFRICTION_VELOCITY)
ENDIF(FRICTION_VELOCITY)

OPTION(EMISSIONS "Enable emissions model" OFF)
IF(EMISSIONS)
    ADD_DEFINITIONS(-DEMISSIONS)
    SET(FRICTION_VELOCITY ON CACHE BOOL "Enable friction velocity calculations" FORCE)
ENDIF(EMISSIONS)

OPTION(STABILITY "Enable atmospheric stability model" OFF)
IF(STABILITY)
    ADD_DEFINITIONS(-DSTABILITY)
ENDIF(STABILITY)

OPTION(MOBILE_APP "Enable optoins for mobile app" OFF)
IF(MOBILE_APP)
    ADD_DEFINITIONS(-DMOBILE_APP)
ENDIF(MOBILE_APP)

# *****************************************************************************
# Allow to permanently disable thredds updating in the build
# *****************************************************************************
OPTION(DISABLE_THREDDS_UPDATE "Don't connect to server for thredds updates" OFF)
IF(DISABLE_THREDDS_UPDATE)
    ADD_DEFINITIONS(-DDISABLE_THREDDS_UPDATE)
ENDIF(DISABLE_THREDDS_UPDATE)

# *****************************************************************************
# If the gui is to be built, check for Qt.  Add the definition to force 
# ninjaCom to compile with Qt support
# *****************************************************************************
OPTION(NINJA_QTGUI "Build Qt GUI" OFF)
IF(NINJA_QTGUI)
    SET(QT_USE_QTMAIN_LIBRARY 1)
    INCLUDE(FindQt4)
    FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtWebKit REQUIRED)
    INCLUDE(${QT_USE_FILE})
    ADD_DEFINITIONS(-DNINJA_GUI)
# *****************************************************************************
# Enable the command prompt for cli builds
# *****************************************************************************
    IF(WIN32)
        OPTION(ENABLE_CONSOLE "Show the console for CLI/GUI runs" OFF)
        OPTION(ENABLE_FIDDLER "Use Fiddler2 proxy to monitor web traffic" OFF)
    ENDIF(WIN32)
    IF(ENABLE_FIDDLER)
        ADD_DEFINITIONS(-DENABLE_FIDDLER)
    ENDIF(ENABLE_FIDDLER)
ENDIF(NINJA_QTGUI)

# *****************************************************************************
# Build the cli executable
# *****************************************************************************
OPTION(NINJA_CLI "Build Command Line Interface" OFF)

# *****************************************************************************
# Enable support for openmp.
# *****************************************************************************
OPTION(OPENMP_SUPPORT "Build with OpenMP support" ON)
IF(OPENMP_SUPPORT)
    INCLUDE(FindOpenMP)
    FIND_PACKAGE(OpenMP)
    IF(OPENMP_FOUND)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    ELSE(OPENMP_FOUND)
        MESSAGE("OpenMP could not be found on your system, disabling support")
    ENDIF(OPENMP_FOUND)
ENDIF(OPENMP_SUPPORT)

OPTION(MAKE_DOCS "Build pdf documents using LaTeX" OFF)
IF(MAKE_DOCS)
    ADD_SUBDIRECTORY(doc)
ENDIF(MAKE_DOCS)

# *****************************************************************************
# Suppress warnings, cleaner
# *****************************************************************************
OPTION(SUPRESS_WARNINGS "Supress known compiler warnings that don't seem to affect the build" OFF)

# *****************************************************************************
# Show all warnings, update for other compilers.
# *****************************************************************************
OPTION(VERBOSE_WARNINGS "Show lots of warnings, supercedes SUPPRESS_WARNINGS" OFF)
IF(VERBOSE_WARNINGS)
    IF(MSVC)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
    ELSE(MSVC)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    ENDIF(MSVC)
ELSE(VERBOSE_WARNINGS)
    IF(SUPRESS_WARNINGS)
        IF(MSVC)
            ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
        ELSE(MSVC)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format -Wno-unused-result")
        ENDIF(MSVC)
    ENDIF(SUPRESS_WARNINGS)
ENDIF(VERBOSE_WARNINGS)
IF(NOT MSVC)
    IF(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
    ENDIF(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
ENDIF(NOT MSVC)

OPTION(ENABLE_GMTED "Try to use the GMTED data for DEM fetching.  It has proven
unreliable and may need to be disabled" ON)
IF(ENABLE_GMTED)
    ADD_DEFINITIONS(-DHAVE_GMTED)
ENDIF(ENABLE_GMTED)

OPTION(WITH_LCP_CLIENT "Allow LCP downloading" OFF)
IF(WITH_LCP_CLIENT)
    ADD_DEFINITIONS(-DWITH_LCP_CLIENT)
ENDIF(WITH_LCP_CLIENT)

OPTION(WITH_NOMADS_SUPPORT "Use NOMADS server support" ON)
IF(WITH_NOMADS_SUPPORT)
    ADD_DEFINITIONS(-DWITH_NOMADS_SUPPORT)
ENDIF(WITH_NOMADS_SUPPORT)

OPTION(BUILD_SOLAR_GRID "Build a application for building solar grids" OFF)
MARK_AS_ADVANCED(BUILD_SOLAR_GRID)

IF(NOT WIN32)
    OPTION(ENABLE_PROFILER "Write gprof data on execution" OFF)
    IF(ENABLE_PROFILER)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    ENDIF(ENABLE_PROFILER)
ENDIF(NOT WIN32)

SET(GRID_INTERP_TIMEOUT 1 CACHE INTEGER "Timeout for grid resampling tests")
MARK_AS_ADVANCED(GRID_INTERP_TIMEOUT)

# *****************************************************************************
# *****************************************************************************
# END OF OPTIONS
# *****************************************************************************
# *****************************************************************************

# *****************************************************************************
# Show some information related to the package being built.
# *****************************************************************************
OPTION(PACKAGE_DEBUG "Show some information about the package" OFF)
MARK_AS_ADVANCED(PACKAGE_DEBUG)

# *****************************************************************************
# Check installer version.
# *****************************************************************************
IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(BUILD_64 TRUE)
    ADD_DEFINITIONS(-DNINJA_64BIT=1)
    IF(PACKAGE_DEBUG)
        MESSAGE("Building 64-bit installer")
    ENDIF(PACKAGE_DEBUG)
ELSEIF(CMAKE_SIZEOF_VOID_P MATCHES "4")
    SET(BUILD_64 FALSE)
    ADD_DEFINITIONS(-DNINJA_32BIT=1)
    IF(PACKAGE_DEBUG)
        MESSAGE("Building 32-bit installer")
    ENDIF(PACKAGE_DEBUG)
ELSE(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(BUILD_64 FALSE)
    ADD_DEFINITIONS(-DNINJA_32BIT=1)
    IF(PACKAGE_DEBUG)
        MESSAGE("Cannot determine architecture, using 32-bit by default")
    ENDIF(PACKAGE_DEBUG)
ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "8")

SET(FIRELAB_DEPEND_ROOT "C:/Users/ksshannon/Desktop/FS_SOFTWARE_BUILD")
SET(FIRELAB_DEPEND_ROOT_QT "${FIRELAB_DEPEND_ROOT}/qt-everywhere-opensource-src-4.8.5")
IF(BUILD_64)
    SET(CPACK_PACKAGE_FILE_NAME ${WINDNINJA_VERSION_NAME}-win64-install)
    SET(FIRELAB_DEPEND_ROOT "${FIRELAB_DEPEND_ROOT}/x64")
ELSE(BUILD_64)
    SET(CPACK_PACKAGE_FILE_NAME ${WINDNINJA_VERSION_NAME}-win32-install)
    SET(FIRELAB_DEPEND_ROOT "${FIRELAB_DEPEND_ROOT}/x86")
ENDIF(BUILD_64)

IF(WIN32 AND FIRELAB_PACKAGE AND MSVC)
    IF(BUILD_64)
        SET(DLL_HOME "c:/src/x64")
    ELSE(BUILD_64)
        SET(DLL_HOME "c:/src/x86")
    ENDIF(BUILD_64)
    SET(FIRELAB_DEPEND_ROOT ${DLL_HOME})
    SET(FIRELAB_DEPEND_ROOT_QT ${DLL_HOME})
    SET(FIRELAB_DEPEND_ROOT_QT_PLUGIN "${DLL_HOME}/plugins/imageformats")
ENDIF(WIN32 AND FIRELAB_PACKAGE AND MSVC)

# *****************************************************************************
# Option for creating the installer with all the dlls from the firelab machine.
# If build type not explicity set, then we set it to release for dll builds
# *****************************************************************************
IF(FIRELAB_PACKAGE)
    SET(NINJA_QTGUI ON)
    SET(NINJA_CLI ON)
ENDIF(FIRELAB_PACKAGE)

IF(NOT CMAKE_BUILD_TYPE AND NOT MSVC_IDE)
    SET(CMAKE_BUILD_TYPE "debug")
ENDIF(NOT CMAKE_BUILD_TYPE AND NOT MSVC_IDE)

IF(MSVC AND FIRELAB_PACKAGE AND NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "release")
ENDIF(MSVC AND FIRELAB_PACKAGE AND NOT CMAKE_BUILD_TYPE)
IF(NOT MSVC_IDE)
    STRING(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
ENDIF(NOT MSVC_IDE)

# *****************************************************************************
# For debugging the cmake script
# *****************************************************************************
OPTION(VERBOSE_CMAKE "Show system information gathered by cmake, useful for debugging" OFF)
IF(VERBOSE_CMAKE)
    INCLUDE(CMakePrintSystemInformation)
ENDIF(VERBOSE_CMAKE)

# *****************************************************************************
# Build the DEM fetching cli tool
# *****************************************************************************
OPTION(BUILD_FETCH_DEM "Build a standalone command line interface DEM utility" OFF)

# *****************************************************************************
# Build the STL file converter
# *****************************************************************************
OPTION(BUILD_STL_CONVERTER "Build a standalone command line interface for STL file conversions" OFF )

# *****************************************************************************
# Build the output file converter
# *****************************************************************************
OPTION(BUILD_CONVERT_OUTPUT "Build a standalone command line interface for xyz file conversions" OFF )

# *****************************************************************************
# Start recursing through directories
# *****************************************************************************

IF(BUILD_TESTING)
    ADD_DEFINITIONS(-DNINJA_BUILD_TESTING)
    ADD_CUSTOM_TARGET(check COMMAND ${CMAKE_CTEST_COMMAND})
ENDIF(BUILD_TESTING)
ADD_SUBDIRECTORY(src)

OPTION(RUN_SLOW_TESTS "Run slower tests, such as srtm fetching" OFF)
OPTION(RUN_CLI_TESTS "Enable Command Line Interface Tests" OFF )

IF(BUILD_TESTING)
    ADD_SUBDIRECTORY(autotest)
ENDIF(BUILD_TESTING)

# Example files
# Surface files
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/mackay.tif DESTINATION etc/windninja/example-files COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/example_lcp.lcp DESTINATION etc/windninja/example-files COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/example_lcp.prj DESTINATION etc/windninja/example-files COMPONENT apps)
# cfg files for the command line
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/cli_domainAverage.cfg DESTINATION etc/windninja/example-files COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/cli_domainAverage_diurnal.cfg DESTINATION etc/windninja/example-files COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/cli_pointInitialization_diurnal.cfg DESTINATION etc/windninja/example-files COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/cli_wxModelInitialization_diurnal.cfg DESTINATION etc/windninja/example-files COMPONENT apps)
# wx station file and atm file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/mackay_wx_stations.csv DESTINATION etc/windninja/example-files COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/doc/example_weather.atm DESTINATION etc/windninja/example-files COMPONENT apps)
# boost
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/date_time_zonespec.csv DESTINATION bin COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/date_time_zonespec.csv DESTINATION share/windninja)
# Timezone shapefile
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/tz_world.zip DESTINATION share/windninja)
# landfire boundary
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/landfire.zip DESTINATION share/windninja)
# Google Maps
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/map.htm DESTINATION share/windninja)
# THREDDS File
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/thredds.csv DESTINATION share/windninja)
# Surface zip data
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/surface_data.zip DESTINATION share/windninja)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/us_srtm_region.shp DESTINATION share/windninja)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/us_srtm_region.dbf DESTINATION share/windninja)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/us_srtm_region.prj DESTINATION share/windninja)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/us_srtm_region.shx DESTINATION share/windninja)
#docs
INSTALL(FILES ${CMAKE_SOURCE_DIR}/doc/CLI_instructions.pdf DESTINATION share/windninja/doc COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/doc/displaying_wind_vectors_in_ArcMap.pdf DESTINATION share/windninja/doc COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/doc/displaying_wind_vectors_in_ArcView.pdf DESTINATION share/windninja/doc COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/doc/download_elevation_file.pdf DESTINATION share/windninja/doc COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/doc/fetch_dem_instructions.pdf DESTINATION share/windninja/doc COMPONENT apps)
#tutorials
INSTALL(FILES ${CMAKE_SOURCE_DIR}/doc/tutorials/WindNinja_tutorial1.pdf DESTINATION share/windninja/doc/tutorials COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/doc/tutorials/WindNinja_tutorial2.pdf DESTINATION share/windninja/doc/tutorials COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/doc/tutorials/WindNinja_tutorial3.pdf DESTINATION share/windninja/doc/tutorials COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/doc/tutorials/WindNinja_tutorial4.pdf DESTINATION share/windninja/doc/tutorials COMPONENT apps)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION share/windninja/doc)

# data folder
#INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/data DESTINATION share/windninja)
IF(WIN32 AND FIRELAB_PACKAGE AND MSVC)
    INSTALL(DIRECTORY "${FIRELAB_DEPEND_ROOT}/data/" DESTINATION share/windninja/gdal-data/data)
ENDIF(WIN32 AND FIRELAB_PACKAGE AND MSVC)

IF(BUILD_TYPE MATCHES "debug")
    IF(PACKAGE_DEBUG)
        MESSAGE("Copying boost date time file into build/src/gui and build/src/cli")
    ENDIF(PACKAGE_DEBUG)
    FILE(COPY "${CMAKE_SOURCE_DIR}/data/date_time_zonespec.csv" DESTINATION "${CMAKE_BINARY_DIR}/src/gui")
    FILE(COPY "${CMAKE_SOURCE_DIR}/data/date_time_zonespec.csv" DESTINATION "${CMAKE_BINARY_DIR}/src/cli")
ENDIF(BUILD_TYPE MATCHES "debug")
# *****************************************************************************
# CPack
# *****************************************************************************
INCLUDE(InstallRequiredSystemLibraries)
# *****************************************************************************
# CPack Debian
# *****************************************************************************
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kyle Shannon kyle@pobox.com")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS libnetcdf libgdal libcurl
                                 libboost_program_options libboost_date_time)
IF(NINJA_QTGUI)
    SET(CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS} 
                                     libqtgui4 libqtcore4)
ENDIF(NINJA_QTGUI)

# *****************************************************************************
# CPack variables
# *****************************************************************************
SET(CPACK_NSIS_PACKAGE_NAME ${WINDNINJA_VERSION_NAME})
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license.txt")
# version
SET(CPACK_VERSION_MAJOR ${WINDNINJA_VERSION_MAJOR})
SET(CPACK_VERSION_MINOR ${WINDNINJA_VERSION_MINOR})
SET(CPACK_VERSION_PATCH ${WINDNINJA_VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION ${WIND_NINJA_VERSION})

SET(CPACK_NSIS_INSTALL_ROOT "C:")
CMAKE_POLICY(PUSH)
CMAKE_POLICY(SET CMP0010 OLD)
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "WindNinja\\\\${WINDNINJA_VERSION_NAME}")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "WindNinja\\\\${WINDNINJA_VERSION_NAME}")
CMAKE_POLICY(POP)
# *****************************************************************************
# Add/Remove Programs support
# *****************************************************************************
#SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\WindNinja.exe")
#SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\windninja.firemodels.org")
# *****************************************************************************
# NSIS Specific stuff
# *****************************************************************************
SET(CPACK_NSIS_DISPLAY_NAME "WindNinja\\\\${WINDNINJA_VERSION_NAME}")
SET(CPACK_NSIS_INSTALLED_ICON_NAME 
    "${CMAKE_SOURCE_DIR}/images/icons/wn-desktop.ico")

SET(CPACK_CREATE_DESKTOP_LINKS WindNinja)
SET(CPACK_NSIS_MODIFY_PATH WindNinja)

SET(CPACK_PACKAGE_EXECUTABLES "WindNinja" ${WINDNINJA_VERSION_NAME})
SET(CPACK_NSIS_CREATE_ICONS_EXTRA "
CreateDirectory \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\"
CreateDirectory \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\\Tutorial 1.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\tutorials\\\\WindNinja_Tutorial1.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\\Tutorial 2.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\tutorials\\\\WindNinja_Tutorial2.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\\Tutorial 3.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\tutorials\\\\WindNinja_Tutorial3.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\\Tutorial 4.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\tutorials\\\\WindNinja_Tutorial4.pdf\\\" 
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\CLI Instructions.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\CLI_instructions.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\ArcMap Instructions.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\Displaying_wind_vectors_in_ArcMap.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\ArcView Instructions.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\Displaying_wind_vectors_in_ArcView.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\DEM Download Instructions.lnk\\\" \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\download_elevation_file.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Example Files.lnk\\\" \\\"$INSTDIR\\\\etc\\\\windninja\\\\example-files\\\" ")

SET(CPACK_NSIS_DELETE_ICONS_EXTRA "
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\\Tutorial 1.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\\Tutorial 2.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\\Tutorial 3.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\\Tutorial 4.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\\CLI Instructions.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\\ArcMap Instructions.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\\ArcView Instructions.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\\DEM Download Instructions.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Example Files.lnk\\\"
RMDir \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\"
RMDir \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\" ")

# *****************************************************************************
# Build the nsis installer on the firelab machine(s).  Not for external use.
# *****************************************************************************
IF(FIRELAB_PACKAGE AND WIN32 AND MSVC)
    INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT}/bin/gdal111.dll" DESTINATION bin COMPONENT apps)
    INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT}/bin/proj.dll" DESTINATION bin COMPONENT apps)
    INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT}/bin/netcdf.dll" DESTINATION bin COMPONENT apps)
    INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT}/bin/geos_c.dll" DESTINATION bin COMPONENT apps)
    INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT}/bin/geos.dll" DESTINATION bin COMPONENT apps)
    INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT}/bin/libcurl.dll" DESTINATION bin COMPONENT apps)
    INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT}/bin/zlib.dll" DESTINATION bin COMPONENT apps)
    IF(BUILD_TYPE MATCHES "release")
        MESSAGE("IN RELEASE PACKAGE MODE")
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT}/bin/qtgui4.dll" DESTINATION bin COMPONENT apps)
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT}/bin/qtcore4.dll" DESTINATION bin COMPONENT apps)
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT}/bin/qtnetwork4.dll" DESTINATION bin COMPONENT apps)
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT}/bin/qtwebkit4.dll" DESTINATION bin COMPONENT apps)
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT}/bin/phonon4.dll" DESTINATION bin COMPONENT apps)
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT}/bin/qtnetwork4.dll" DESTINATION bin COMPONENT apps)
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT_PLUGIN}/qjpeg4.dll" DESTINATION bin/imageformats)
    ELSE(BUILD_TYPE MATCHES "release")
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT}/bin/qtguid4.dll" DESTINATION bin COMPONENT apps)
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT}/bin/qtcored4.dll" DESTINATION bin COMPONENT apps)
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT}/bin/qtwebkitd4.dll" DESTINATION bin COMPONENT apps)
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT}/bin/phonond4.dll" DESTINATION bin COMPONENT apps)
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT}/bin/qtnetworkd4.dll" DESTINATION bin COMPONENT apps)
        INSTALL(PROGRAMS "${FIRELAB_DEPEND_ROOT_QT_PLUGIN}/qjpegd4.dll" DESTINATION bin/imageformats)
    ENDIF(BUILD_TYPE MATCHES "release")
    # HACK FOR OPENMP
    IF(BUILD_64)
        INSTALL(PROGRAMS ${MSVC10_REDIST_DIR}/x64/Microsoft.VC100.OPENMP/vcomp100.dll DESTINATION bin COMPONENT apps)
    ELSE(BUILD_64)
        INSTALL(PROGRAMS ${MSVC10_REDIST_DIR}/x86/Microsoft.VC100.OPENMP/vcomp100.dll DESTINATION bin COMPONENT apps)
    ENDIF(BUILD_64)
ENDIF(FIRELAB_PACKAGE AND WIN32 AND MSVC)
SET(CPACK_COMPONENT_APPS_REQUIRED TRUE)
SET(CPACK_COMPONENT_LIBS_HIDDEN TRUE)
SET(CPACK_COMPONENT_LIBS_DISABLED TRUE)
SET(CPACK_COMPONENT_INCLUDES_HIDDEN TRUE)
SET(CPACK_COMPONENT_INCLUDES_DISABLED TRUE)

# *****************************************************************************
# CPack call
# *****************************************************************************
INCLUDE(CPack)
INCLUDE(CTest)

# *****************************************************************************
# CPack groupings
# *****************************************************************************
CPACK_ADD_COMPONENT(apps 
                    DISPLAY_NAME "WindNinja"
                    DESCRIPTION "WindNinja binaries and documentation"
                    GROUP Application
                    )
CPACK_ADD_COMPONENT(libs
                    DISPLAY_NAME "WindNinja Libraries"
                    DESCRIPTION "Development libraries"
                    #GROUP Development
                    )
CPACK_ADD_COMPONENT(includes
                    DISPLAY_NAME "WindNinja Headers"
                    DESCRIPTION "Development include files"
                    #GROUP Development
                    )

