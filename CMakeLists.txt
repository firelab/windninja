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

# Check if the user is using VCPKG on Windows
if(WIN32)
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR NOT CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg.cmake")
        message(FATAL_ERROR
            "WindNinja requires the Visual C++ Package Manager VCPKG.\n"
        )
    endif()
endif(WIN32)

if(DEFINED CMAKE_PROJECT_INCLUDE_BEFORE AND
   NOT EXISTS "${CMAKE_PROJECT_INCLUDE_BEFORE}")
  unset(CMAKE_PROJECT_INCLUDE_BEFORE CACHE)
endif()

project(WindNinja)

# Set C++ version for CMake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# Needed for FindNetCDF and GET_DATE()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

option(FIRELAB_PACKAGE "Build a windows installer with dlls from the firelab machine" OFF)
mark_as_advanced(FIRELAB_PACKAGE)

if(FIRELAB_PACKAGE)
    set(NINJA_GUI ON)
    set(NINJA_CLI ON)
endif(FIRELAB_PACKAGE)

# Generate a current version file derived from VERSION
file(READ ${CMAKE_SOURCE_DIR}/VERSION V)
string(REPLACE "\n" "" V ${V})
if(V)
    string(REPLACE "." ";" VLIST ${V})
    list(LENGTH VLIST VCOUNT)
    if(VCOUNT GREATER 0)
        list(GET VLIST 0 VER_MAJ)
    endif(VCOUNT GREATER 0)
    if(VCOUNT GREATER 1)
        list(GET VLIST 1 VER_MIN)
    endif(VCOUNT GREATER 1)
    if(VCOUNT GREATER 2)
        list(GET VLIST 2 VER_PATCH)
    endif(VCOUNT GREATER 2)
endif(V)
if(NOT GITCOMMAND)
    set(GIT_EXE git)
else(NOT GITCOMMAND)
    set(GIT_EXE ${GITCOMMAND})
endif(NOT GITCOMMAND)
execute_process(COMMAND ${GIT_EXE} "rev-parse" "HEAD"
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                RESULT_VARIABLE GIT_RC
                OUTPUT_VARIABLE SCM_REV)
if(GIT_RC)
    message(STATUS "Could not find git to set the NINJA_SCM_REVISION var, please set it")
    set(SCM_REV "NOT set")
endif(GIT_RC)
string(REPLACE "\n" "" SCM_REV ${SCM_REV})

if(NOT SKIP_GETDATE)
    include(GetDate)
    GET_DATE()
    set(DATE ${GD_YEAR}/${GD_MONTH}/${GD_DAY})
else(NOT SKIP_GETDATE)
    set(DATE "1970/01/01")
endif(NOT SKIP_GETDATE)

set(VER_STR ${VER_MAJ}.${VER_MIN}.${VER_PATCH})
configure_file(${PROJECT_SOURCE_DIR}/src/ninja_version.h.in
               ${PROJECT_BINARY_DIR}/ninja_version.h)
include_directories(${PROJECT_BINARY_DIR})
set(WINDNINJA_VERSION ${VER_STR})
set(WINDNINJA_VERSION_NAME WindNinja-${VER_STR})
message(STATUS "VERSION: ${VER_STR}")
message(STATUS "PKG VERSION: ${WINDNINJA_VERSION_NAME}")

# Find Dependencies: VCPKG for Windows and ATP for Linux
if(MSVC)
    find_package(shapelib CONFIG REQUIRED)
    find_package(GDAL CONFIG REQUIRED)
    find_package(netCDF CONFIG REQUIRED)
    find_package(Boost REQUIRED COMPONENTS date_time program_options unit_test_framework)
    set(GDAL_LIBRARY GDAL::GDAL)
    set(NETCDF_LIBRARIES_C netCDF::netcdf)
    set(Boost_DEBUG OFF)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREAD OFF)
else()
    include(FindBoost)
    set(Boost_DEBUG OFF)
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_MULTITHREAD OFF)
    find_package(Boost 1.41.1 COMPONENTS date_time program_options unit_test_framework REQUIRED)
    include(FindNetCDF)
    include(FindGDAL)
    find_package(NetCDF)
endif(MSVC)

# Ninja executables
option(NINJA_GUI "Build Qt GUI" ON)
option(NINJA_CLI "Build Command Line Interface" ON)

# Weather related options
option(WITH_NOMADS_SUPPORT "Use NOMADS server support" ON)
if(WITH_NOMADS_SUPPORT)
    add_definitions(-DWITH_NOMADS_SUPPORT)
endif(WITH_NOMADS_SUPPORT)

if(WITH_NOMADS_SUPPORT)
    include(CheckFunctionExists)
    CHECK_FUNCTION_EXISTS(timegm HAVE_TIMEGM)
    if(NOT HAVE_TIMEGM)
        add_definitions(-DUSE_INTERNAL_TIME_GM)
    endif(NOT HAVE_TIMEGM)

    option(NOMADS_USE_VSI_READ "Use VSI*L api instead of CPLHTTPFetch" OFF)
    if(NOMADS_USE_VSI_READ)
        add_definitions(-DNOMADS_USE_VSI_READ)
    endif(NOMADS_USE_VSI_READ)

    option(NOMADS_RTMA "Enable the RTMA forecasts" OFF)
    if(NOMADS_RTMA)
        add_definitions(-DNOMADS_RTMA)
    endif(NOMADS_RTMA)

    option(NOMADS_EXPER_FORECASTS "Enable NARRE and NEST forecasts." OFF)
    if(NOMADS_EXPER_FORECASTS)
        add_definitions(-DNOMADS_EXPER_FORECASTS)
    endif(NOMADS_EXPER_FORECASTS)

    option(NOMADS_ENABLE_ASYNC "Enable multi-threading for NOMADS download" ON)
    if(NOMADS_ENABLE_ASYNC)
        add_definitions(-DNOMADS_ENABLE_ASYNC)
    endif(NOMADS_ENABLE_ASYNC)

    option(NOMADS_USE_IP "Use the ip address for the nomads server, not a host" OFF)
    if(NOMADS_USE_IP)
        add_definitions(-DNOMADS_USE_IP)
    endif(NOMADS_USE_IP)

    option(NOMADS_ENABLE_3D "Enable 3d initialization using NOMADS data." OFF)
    if(NOMADS_ENABLE_3D)
        add_definitions(-DNOMADS_ENABLE_3D)
    endif(NOMADS_ENABLE_3D)

    option(NOMADS_INTERNAL_VRT "Use an internal copy of GDALAutoCreateVRT that allows for band subsetting" OFF)
    if(NOMADS_INTERNAL_VRT)
        add_definitions(-DNOMADS_INTERNAL_VRT)
    endif(NOMADS_INTERNAL_VRT)
endif(WITH_NOMADS_SUPPORT)

option(DISABLE_THREDDS_UPDATE "Don't connect to server for thredds updates" OFF)
if(DISABLE_THREDDS_UPDATE)
    add_definitions(-DDISABLE_THREDDS_UPDATE)
endif(DISABLE_THREDDS_UPDATE)

# Surface related options
option(ENABLE_GMTED "Try to use the GMTED data for DEM fetching" ON)
if(ENABLE_GMTED)
    add_definitions(-DHAVE_GMTED)
endif(ENABLE_GMTED)

option(WITH_LCP_CLIENT "Allow LCP downloading" ON)
if(WITH_LCP_CLIENT)
    add_definitions(-DWITH_LCP_CLIENT)
endif(WITH_LCP_CLIENT)

# Disable for exception handling in the GUI and CLI
# option(C_API "Enable WindNinja C API" OFF)
# if(C_API)
#     add_definitions(-DC_API)
# endif(C_API)

# Enable phone home and message server startup
option(PHONE_HOME_QUERIES "Enable Phone Home Queries" ON)
if (PHONE_HOME_QUERIES)
    add_definitions(-DPHONE_HOME_QUERIES_ENABLED)
endif(PHONE_HOME_QUERIES)

# OpenFOAM related options
option(NINJAFOAM "Enable OpenFOAM solver" ON)
if(NINJAFOAM)
    add_compile_definitions(NINJAFOAM)
    if(WIN32 AND FIRELAB_PACKAGE)
        add_compile_definitions(FIRELAB_PACKAGE)
        find_path(FOAM_MINGW_PATH OpenFOAM-2.2.x
                  DOC "Path to x-compiled OpenFOAM directory")

        if(NOT FOAM_MINGW_PATH)
            message(FATAL_ERROR
                "You must provide a path to a cross-compiled OpenFOAM directory for NINJAFOAM support and package building")
        endif()

        string(REGEX REPLACE "\\\\" "/" FOAM_MINGW_PATH "${FOAM_MINGW_PATH}")
    endif()
endif()

# Miscellaneous options
option(NINJA_SPEED_TESTING "Enable initialization speed dampening" OFF)
if(NINJA_SPEED_TESTING)
    add_definitions(-DNINJA_SPEED_TESTING)
endif(NINJA_SPEED_TESTING)

option(FRICTION_VELOCITY "Enable friction velocity calculations" OFF)
if(FRICTION_VELOCITY)
    add_definitions(-DFRICTION_VELOCITY)
endif(FRICTION_VELOCITY)

option(EMISSIONS "Enable emissions model" OFF)
if(EMISSIONS)
    add_definitions(-DEMISSIONS)
    set(FRICTION_VELOCITY ON CACHE BOOL "Enable friction velocity calculations" FORCE)
endif(EMISSIONS)

option(MOBILE_APP "Enable optoins for mobile app" OFF)
if(MOBILE_APP)
    add_definitions(-DMOBILE_APP)
endif(MOBILE_APP)

option(OPENMP_SUPPORT "Build with OpenMP support" ON)
if(OPENMP_SUPPORT)
    include(FindOpenMP)
    FIND_PACKAGE(OpenMP)
    if(OPENMP_FOUND)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    else(OPENMP_FOUND)
        message(STATUS "OpenMP could not be found on your system, disabling support")
    endif(OPENMP_FOUND)
endif(OPENMP_SUPPORT)

option(MAKE_DOCS "Build pdf documents using LaTeX" OFF)
if(MAKE_DOCS)
    add_subdirectory(doc)
endif(MAKE_DOCS)

option(SUPRESS_WARNINGS "Supress known compiler warnings that don't seem to affect the build" OFF)

option(VERBOSE_WARNINGS "Show lots of warnings, supercedes SUPPRESS_WARNINGS" OFF)
if(VERBOSE_WARNINGS)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
    else(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    endif(MSVC)
else(VERBOSE_WARNINGS)
    if(SUPRESS_WARNINGS)
        if(MSVC)
            add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        else(MSVC)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
            set(GDAL_SYSTEM_INCLUDE "SYSTEM")
        endif(MSVC)
    endif(SUPRESS_WARNINGS)
endif(VERBOSE_WARNINGS)
if(NOT MSVC)
    if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
    endif(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
endif(NOT MSVC)

if(NOT WIN32)
    option(ENABLE_PROFILER "Write gprof data on execution" OFF)
    if(ENABLE_PROFILER)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    endif(ENABLE_PROFILER)
endif(NOT WIN32)

# Optional utilities
option(BUILD_FETCH_DEM "Build a standalone command line interface DEM utility" OFF)

option(BUILD_STL_CONVERTER "Build a standalone command line interface for STL file conversions" OFF )

option(BUILD_CONVERT_OUTPUT "Build a standalone command line interface for xyz file conversions" OFF )

option(BUILD_WRF_TO_KMZ "Build a standalone command line interface for converting WRF output to kmz" OFF )
mark_as_advanced(BUILD_WRF_TO_KMZ)

option(BUILD_HRRR_TO_KMZ "Build a standalone command line interface for converting hrrr output runs to kmz, without running full WindNinja" OFF )
mark_as_advanced(BUILD_HRRR_TO_KMZ)

option(BUILD_SLOPE_ASPECT_GRID "Build an application for building slope and aspect grids from a dem" OFF)
mark_as_advanced(BUILD_SLOPE_ASPECT_GRID)

option(BUILD_FLOW_SEPARATION_GRID "Build an application for building flow separation" OFF)
mark_as_advanced(BUILD_FLOW_SEPARATION_GRID)

option(BUILD_SOLAR_GRID "Build a application for building solar grids" OFF)
mark_as_advanced(BUILD_SOLAR_GRID)

option(BUILD_SURFACE_INPUT_NODATA_FILLER "Build an application for filling input surface no data values" ON)
mark_as_advanced(BUILD_SURFACE_INPUT_NODATA_FILLER)

option(NINJA_GDAL_OUTPUT "allow experimental output formats from GDAL" OFF)
mark_as_advanced(NINJA_GDAL_OUTPUT)
if(NINJA_GDAL_OUTPUT)
  add_definitions(-DNINJA_GDAL_OUTPUT)
endif()

# Recurse into subdirectories
add_subdirectory(src)

if(BUILD_TESTING)
    add_definitions(-DNINJA_BUILD_TESTING)
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
endif(BUILD_TESTING)
option(RUN_SLOW_TESTS "Run slower tests, such as srtm fetching" OFF)
option(RUN_CFG_TESTS "Run the cfg file tests in autotest/cfg OFF")
if(BUILD_TESTING)
    add_subdirectory(autotest)
endif(BUILD_TESTING)

# Add the resource files for the .exe icon
if (MSVC)
    target_sources(WindNinja PRIVATE ${PROJECT_SOURCE_DIR}/wn-ms-resource.rc)
endif()

# Run windeployqt6.exe to provide all QT related DLLs
if(MSVC)
    find_program(WINDEPLOYQT_EXE
        NAMES windeployqt6 windeployqt
        HINTS "${Qt6_DIR}/../../../bin"
        REQUIRED
    )

    add_custom_target(deploy_windninja ALL
        COMMAND "${WINDEPLOYQT_EXE}"
                "$<TARGET_FILE:WindNinja>"
        DEPENDS WindNinja
    )
endif()

# Package related options and settings.
option(PACKAGE_DEBUG "Show some information about the package" OFF)
mark_as_advanced(PACKAGE_DEBUG)

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(BUILD_64 TRUE)
    add_definitions(-DNINJA_64BIT=1)
    if(PACKAGE_DEBUG)
        message(STATUS "Building 64-bit installer")
    endif(PACKAGE_DEBUG)
endif(CMAKE_SIZEOF_VOID_P MATCHES "8")

if(NOT CMAKE_BUILD_TYPE AND NOT MSVC_IDE)
    set(CMAKE_BUILD_TYPE "debug")
endif(NOT CMAKE_BUILD_TYPE AND NOT MSVC_IDE)

if(MSVC AND FIRELAB_PACKAGE AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "release")
endif(MSVC AND FIRELAB_PACKAGE AND NOT CMAKE_BUILD_TYPE)
STRING(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

option(VERBOSE_CMAKE "Show system information gathered by cmake, useful for debugging" OFF)
if(VERBOSE_CMAKE)
    include(CMakePrintSystemInformation)
endif(VERBOSE_CMAKE)

# Install DEM and configuration files
set(etc_files
    missoula_valley.tif
    example_lcp.tif
    cli_domainAverage.cfg
    cli_domainAverage_diurnal.cfg
    cli_pointInitialization_diurnal.cfg
    cli_wxModelInitialization_diurnal.cfg
    cli_momentumSolver_diurnal.cfg)
list(TRANSFORM etc_files PREPEND ${PROJECT_SOURCE_DIR}/data/)
install(FILES ${etc_files}
        DESTINATION etc/windninja/example-files
        COMPONENT apps)

# Install wxstation current files (add weather station files to example files)
set(wx_station_current_files
    missoula_valley_stations_4.csv
    KMSO-2018-06-25_1237-0.csv
    PNTM8-2018-06-25_1237-2.csv
    TR266-2018-06-25_1237-3.csv
    TS934-2018-06-25_1237-1.csv)
list(TRANSFORM wx_station_current_files
     PREPEND ${PROJECT_SOURCE_DIR}/data/WXSTATIONS-2018-06-25-1237-missoula_valley/)
install(FILES ${wx_station_current_files}
        DESTINATION etc/windninja/example-files/WXSTATIONS-2018-06-25-1237-missoula_valley
        COMPONENT apps)

# Install wxstation files (add weather station time series files)
set(wx_station_ts_files
    missoula_valley_stations_4.csv
    KMSO-MDT-2018-06-20_2128-2018-06-21_2128-0.csv
    PNTM8-MDT-2018-06-20_2128-2018-06-21_2128-2.csv
    TR266-MDT-2018-06-20_2128-2018-06-21_2128-3.csv
    TS934-MDT-2018-06-20_2128-2018-06-21_2128-1.csv)
list(TRANSFORM wx_station_ts_files
     PREPEND ${PROJECT_SOURCE_DIR}/data/WXSTATIONS-MDT-2018-06-20-2128-2018-06-21-2128-missoula_valley/)
install(FILES ${wx_station_ts_files}
        DESTINATION etc/windninja/example-files/WXSTATIONS-MDT-2018-06-20-2128-2018-06-21-2128-missoula_valley
        COMPONENT apps)

# Install all share files 
set(share_files
    date_time_zonespec.csv
    config_options.csv
    tz_world.zip
    landfire.zip
    map.html
    qt_certs_bundle.pem
    thredds.csv
    surface_data.zip
    srtm_region.geojson
    us_srtm_region.dbf
    us_srtm_region.prj
    us_srtm_region.shp
    us_srtm_region.shx)
list(TRANSFORM share_files PREPEND ${PROJECT_SOURCE_DIR}/data/)
install(FILES ${share_files}
        DESTINATION share/windninja
        COMPONENT apps)

# Install Leaflet data
if(WIN32 AND FIRELAB_PACKAGE)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/leaflet DESTINATION share/windninja)
endif(WIN32 AND FIRELAB_PACKAGE)

# Install all documentation files
set(doc_files
    CLI_instructions.pdf
    displaying_wind_vectors_in_ArcGIS_Pro.pdf
    download_elevation_file.pdf
    fetch_dem_instructions.pdf)
list(TRANSFORM doc_files PREPEND ${PROJECT_SOURCE_DIR}/doc/)
install(FILES ${doc_files}
        DESTINATION share/windninja/doc
        COMPONENT apps)

# Install all tutorial files
set(tutorial_files
    WindNinja_tutorial1.pdf
    WindNinja_tutorial2.pdf
    WindNinja_tutorial3.pdf
    WindNinja_tutorial4.pdf)
list(TRANSFORM tutorial_files PREPEND ${PROJECT_SOURCE_DIR}/doc/tutorials/)
install(FILES ${tutorial_files}
        DESTINATION share/windninja/doc/tutorials
        COMPONENT apps)

# Install the ninjafoam data
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/ninjafoam DESTINATION share/windninja)

# We need the splash screen for pdf output
install(FILES ${PROJECT_SOURCE_DIR}/data/wn-splash.png DESTINATION share/windninja)
install(FILES ${PROJECT_SOURCE_DIR}/data/relief.xml DESTINATION share/windninja)
install(FILES ${PROJECT_SOURCE_DIR}/data/topofire_logo.png DESTINATION share/windninja)

# Installs specific to Windows NSIS installer
if(WIN32)
    # Install DLLS for WindNinja.exe to the bin directory
    install(
    DIRECTORY "${CMAKE_BINARY_DIR}/src/gui/"
    DESTINATION bin
    FILES_MATCHING
        PATTERN "*.exe"
        PATTERN "*.dll"
        PATTERN "plugins/*"
        PATTERN "platforms/*"
        PATTERN "resources/*"
        PATTERN "*.qm")

    # Installs GDAL data from VCPKG to the gdal-data/data directory
    install(
        DIRECTORY "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/share/gdal/"
        DESTINATION share/windninja/gdal-data/data
        COMPONENT apps)

    # Installs PROJ data from VCPKG to the proj-data/data directory
    install(
        DIRECTORY "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/share/proj/"
        DESTINATION share/windninja/proj-data/data
        COMPONENT apps)

    if(NINJAFOAM)
        install(DIRECTORY "${FOAM_MINGW_PATH}/etc" DESTINATION bin)
        install(DIRECTORY "${FOAM_MINGW_PATH}/platforms" DESTINATION bin)
    endif(NINJAFOAM)
endif(WIN32)

set(ZIPFILES "")

if(BUILD_TYPE MATCHES "debug")
    if(PACKAGE_DEBUG)
        message(STATUS "Copying boost date time file into build/src/gui and build/src/cli")
    endif(PACKAGE_DEBUG)
    file(COPY "${PROJECT_SOURCE_DIR}/data/date_time_zonespec.csv" DESTINATION "${PROJECT_BINARY_DIR}/src/gui/qt6")
    file(COPY "${PROJECT_SOURCE_DIR}/data/date_time_zonespec.csv" DESTINATION "${PROJECT_BINARY_DIR}/src/cli")
endif(BUILD_TYPE MATCHES "debug")

# CPack related scripting
include(InstallRequiredSystemLibraries)
set(CPACK_NSIS_PACKAGE_NAME ${WINDNINJA_VERSION_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
# version
set(CPACK_VERSION_MAJOR ${VER_MAJ})
set(CPACK_VERSION_MINOR ${VER_MIN})
set(CPACK_VERSION_PATCH ${VER_PATCH})
set(CPACK_PACKAGE_VERSION ${WINDNINJA_VERSION})

set(CPACK_NSIS_INSTALL_ROOT "C:")
CMAKE_POLICY(PUSH)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "WindNinja\\\\${WINDNINJA_VERSION_NAME}")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "WindNinja\\\\${WINDNINJA_VERSION_NAME}")
CMAKE_POLICY(POP)
set(CPACK_NSIS_DISPLAY_NAME "WindNinja\\\\${WINDNINJA_VERSION_NAME}")
set(CPACK_NSIS_INSTALLED_ICON_NAME "${PROJECT_SOURCE_DIR}/images/icons/wn-desktop.ico")

set(CPACK_CREATE_DESKTOP_LINKS WindNinja)
set(CPACK_NSIS_MODifY_PATH WindNinja)

set(CPACK_PACKAGE_EXECUTABLES "WindNinja" ${WINDNINJA_VERSION_NAME})
set(CPACK_NSIS_CREATE_ICONS_EXTRA "
CreateDirectory \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\"
CreateDirectory \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\\Tutorial 1.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\tutorials\\\\WindNinja_Tutorial1.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\\Tutorial 2.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\tutorials\\\\WindNinja_Tutorial2.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\\Tutorial 3.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\tutorials\\\\WindNinja_Tutorial3.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\\Tutorial 4.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\tutorials\\\\WindNinja_Tutorial4.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\CLI Instructions.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\CLI_instructions.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\ArcMap Instructions.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\Displaying_wind_vectors_in_ArcMap.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\DEM Download Instructions.lnk\\\" \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\download_elevation_file.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Example Files.lnk\\\" \\\"$INSTDIR\\\\etc\\\\windninja\\\\example-files\\\" ")

set(CPACK_NSIS_DELETE_ICONS_EXTRA "
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\\Tutorial 1.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\\Tutorial 2.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\\Tutorial 3.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\\Tutorial 4.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\\CLI Instructions.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\\ArcMap Instructions.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\\DEM Download Instructions.lnk\\\"
Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Example Files.lnk\\\"
RMDir \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\"
RMDir \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\" ")

set(CPACK_COMPONENT_APPS_REQUIRED TRUE)
set(CPACK_COMPONENT_LIBS_HIDDEN TRUE)
set(CPACK_COMPONENT_LIBS_DISABLED TRUE)
set(CPACK_COMPONENT_INCLUDES_HIDDEN TRUE)
set(CPACK_COMPONENT_INCLUDES_DISABLED TRUE)

# CPack call
include(CPack)
include(CTest)

# CPack Groupings
CPACK_ADD_COMPONENT(apps
                    DISPLAY_NAME "WindNinja"
                    DESCRIPTION "WindNinja binaries and documentation"
                    GROUP Application)

CPACK_ADD_COMPONENT(libs
                    DISPLAY_NAME "WindNinja Libraries"
                    DESCRIPTION "Development libraries")

CPACK_ADD_COMPONENT(includes
                    DISPLAY_NAME "WindNinja Headers"
                    DESCRIPTION "Development include files")
