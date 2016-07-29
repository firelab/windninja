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

cmake_minimum_required(VERSION 2.6)
project(WindNinja)

# Needed for FindNetCDF and GET_DATE()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

option(FIRELAB_PACKAGE "Build a windows installer with dlls from the firelab machine" OFF)
mark_as_advanced(FIRELAB_PACKAGE)

if(FIRELAB_PACKAGE)
    set(NINJA_QTGUI ON)
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
configure_file(${CMAKE_SOURCE_DIR}/src/ninja_version.h.in
               ${CMAKE_BINARY_DIR}/ninja_version.h)
include_directories(${CMAKE_BINARY_DIR})
set(WINDNINJA_VERSION ${VER_STR})
set(WINDNINJA_VERSION_NAME WindNinja-${VER_STR})
message(STATUS "VERSION: ${VER_STR}")
message(STATUS "PKG VERSION: ${WINDNINJA_VERSION_NAME}")

# Locate third party libs
include(FindBoost)
set(Boost_DEBUG OFF)
if(WIN32)
    set(Boost_USE_STATIC_LIBS ON)
else(WIN32)
    set(Boost_USE_STATIC_LIBS OFF)
endif(WIN32)
set(Boost_USE_MULTITHREAD OFF)
find_package(Boost 1.41.1 COMPONENTS date_time program_options
                                     unit_test_framework REQUIRED)

# All GIS related libs.  Windows uses GIS_INTERNALS_HOME
if(WIN32)
    option(USE_GIS_INTERNALS "Use dependencies (as many as possible) from gisinternals.com" OFF)
    if(USE_GIS_INTERNALS)
        find_path(GIS_INTERNALS_HOME gdal CACHE PATH)
    file(TO_NATIVE_PATH ${GIS_INTERNALS_HOME} GIS_INTERNALS_HOME)
    string(REGEX REPLACE "\\\\" "/" GIS_INTERNALS_HOME "${GIS_INTERNALS_HOME}")
    endif(USE_GIS_INTERNALS)
endif(WIN32)

include(FindNetCDF)
include(FindGDAL)
if(WIN32)
    if(NOT GIS_INTERNALS_HOME)
        find_package(NetCDF REQUIRED)
        find_package(GDAL REQUIRED)
    else(NOT GIS_INTERNALS_HOME)
        find_package(NetCDF)
        find_package(GDAL)
    endif(NOT GIS_INTERNALS_HOME)
else(WIN32)
    find_package(NetCDF REQUIRED)
    find_package(GDAL REQUIRED)
endif(WIN32)

if(WIN32)
    if(NOT NETCDF_INCLUDES-NOTFOUND AND GIS_INTERNALS_HOME)
        set(NETCDF_INCLUDES ${GIS_INTERNALS_HOME}/include CACHE TYPE PATH FORCE)
        set(NETCDF_LIBRARIES NOT_USED CACHE TYPE PATH FORCE)
        set(NETCDF_LIBRARIES_C ${GIS_INTERNALS_HOME}/lib/netcdf.lib CACHE TYPE NAME FORCE)
        set(NETCDF_INCLUDES-FOUND TRUE)
        set(NETCDF_LIBRARIES_C-FOUND TRUE)
   endif(NOT NETCDF_INCLUDES-NOTFOUND AND GIS_INTERNALS_HOME)
endif(WIN32)

if(WIN32)
    if(NOT GDAL_INCLUDE_DIR-NOTFOUND AND GIS_INTERNALS_HOME)
        set(GDAL_INCLUDE_DIR ${GIS_INTERNALS_HOME}/include CACHE TYPE PATH FORCE)
        set(GDAL_LIBRARY ${GIS_INTERNALS_HOME}/lib/gdal_i.lib CACHE TYPE NAME FORCE)
        set(GDAL_INCLUDE_DIR-FOUND TRUE)
        set(GDAL_LIBRARY-FOUND TRUE)
    endif(NOT GDAL_INCLUDE_DIR-NOTFOUND AND GIS_INTERNALS_HOME)
endif(WIN32)

# Ninja executables
option(NINJA_QTGUI "Build Qt GUI" ON)
if(NINJA_QTGUI)
    set(QT_USE_QTMAIN_LIBRARY 1)
    include(FindQt4)
    find_package(Qt4 COMPONENTS QtCore QtGui QtWebKit REQUIRED)
    include(${QT_USE_FILE})
    add_definitions(-DNINJA_GUI)
    if(WIN32)
        option(ENABLE_CONSOLE "Show the console for CLI/GUI runs" OFF)
        option(ENABLE_FIDDLER "Use Fiddler2 proxy to monitor web traffic" OFF)
    endif(WIN32)
    if(ENABLE_FIDDLER)
        add_definitions(-DENABLE_FIDDLER)
    endif(ENABLE_FIDDLER)
endif(NINJA_QTGUI)

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

# OpenFOAM related options
option(NINJAFOAM "Enable OpenFOAM solver" ON)
if(NINJAFOAM)
    add_definitions(-DNINJAFOAM)
    if(WIN32 AND FIRELAB_PACKAGE)
        add_definitions(-DFIRELAB_PACKAGE)
        find_path(FOAM_MINGW_PATH OpenFOAM-2.2.x
                  DOC "Path to x-compiled OpenFOAM directory")
        if(FOAM_MINGW_PATH-NOTFOUND)
            message(FATAL_ERROR "You must provide a path to a cross-compiled OpenFOAM directory for NINJAFOAM support and package building")
        endif(FOAM_MINGW_PATH-NOTFOUND)
    endif(WIN32 AND FIRELAB_PACKAGE)
endif(NINJAFOAM)
string(REGEX REPLACE "\\\\" "/" FOAM_MINGW_PATH "${FOAM_MINGW_PATH}")

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

option(STABILITY "Enable atmospheric stability model" ON)
if(STABILITY)
    add_definitions(-DSTABILITY)
endif(STABILITY)

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
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format -Wno-unused-result")
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
option(BUILD_SOLAR_GRID "Build a application for building solar grids" OFF)
mark_as_advanced(BUILD_SOLAR_GRID)

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

# Package related options and settings.
option(PACKAGE_DEBUG "Show some information about the package" OFF)
mark_as_advanced(PACKAGE_DEBUG)

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(BUILD_64 TRUE)
    add_definitions(-DNINJA_64BIT=1)
    if(PACKAGE_DEBUG)
        message(STATUS "Building 64-bit installer")
    endif(PACKAGE_DEBUG)
elseif(CMAKE_SIZEOF_VOID_P MATCHES "4")
    set(BUILD_64 FALSE)
    add_definitions(-DNINJA_32BIT=1)
    if(PACKAGE_DEBUG)
        message(STATUS "Building 32-bit installer")
    endif(PACKAGE_DEBUG)
else(CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(BUILD_64 FALSE)
    add_definitions(-DNINJA_32BIT=1)
    if(PACKAGE_DEBUG)
        message(STATUS "Cannot determine architecture, using 32-bit by default")
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

# install target related files
set(etc_files mackay.tif
                  example_lcp.lcp
                  example_lcp.prj
                  cli_domainAverage.cfg
                  cli_domainAverage_diurnal.cfg
                  cli_pointInitialization_diurnal.cfg
                  cli_wxModelInitialization_diurnal.cfg
                  cli_momentumSolver_diurnal.cfg
                  mackay_wx_stations.csv)
foreach(f ${etc_files})
    install(FILES ${PROJECT_SOURCE_DIR}/data/${f} DESTINATION
            etc/windninja/example-files COMPONENT apps)
endforeach(f ${etc_files})

set(share_files date_time_zonespec.csv
                tz_world.zip
                landfire.zip
                map.htm
                thredds.csv
                surface_data.zip
                us_srtm_region.dbf
                us_srtm_region.prj
                us_srtm_region.shp
                us_srtm_region.shx)
foreach(f ${share_files})
    install(FILES ${PROJECT_SOURCE_DIR}/data/${f} DESTINATION
            share/windninja COMPONENT apps)
endforeach(f ${share_files})

set(doc_files CLI_instructions.pdf
              displaying_wind_vectors_in_ArcMap.pdf
              displaying_wind_vectors_in_ArcView.pdf
              download_elevation_file.pdf
              fetch_dem_instructions.pdf)
foreach(f ${doc_files})
    install(FILES ${PROJECT_SOURCE_DIR}/doc/${f} DESTINATION
            share/windninja/doc COMPONENT apps)
endforeach(f ${doc_files})

set(tutorials tutorials/WindNinja_tutorial1.pdf
              tutorials/WindNinja_tutorial2.pdf
              tutorials/WindNinja_tutorial3.pdf
              tutorials/WindNinja_tutorial4.pdf)
foreach(f ${tutorials})
    install(FILES ${PROJECT_SOURCE_DIR}/doc/${f} DESTINATION
            share/windninja/doc/tutorials COMPONENT apps)
endforeach(f ${tutorials})

# Install the ninjafoam data
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ninjafoam DESTINATION share/windninja)

# We need the splash screen for pdf output
install(FILES ${CMAKE_SOURCE_DIR}/data/wn-splash.png DESTINATION share/windninja)
install(FILES ${CMAKE_SOURCE_DIR}/data/relief.xml DESTINATION share/windninja)
install(FILES ${CMAKE_SOURCE_DIR}/data/topofire_logo.png DESTINATION share/windninja)

# GDAL data
if(WIN32 AND FIRELAB_PACKAGE AND MSVC AND GIS_INTERNALS_HOME)
    INSTALL(DIRECTORY "${GIS_INTERNALS_HOME}/bin/gdal-data/" DESTINATION share/windninja/gdal-data/data)
endif(WIN32 AND FIRELAB_PACKAGE AND MSVC AND GIS_INTERNALS_HOME)

if(BUILD_TYPE MATCHES "debug")
    if(PACKAGE_DEBUG)
        message(STATUS "Copying boost date time file into build/src/gui and build/src/cli")
    endif(PACKAGE_DEBUG)
    FILE(COPY "${CMAKE_SOURCE_DIR}/data/date_time_zonespec.csv" DESTINATION "${CMAKE_BINARY_DIR}/src/gui")
    FILE(COPY "${CMAKE_SOURCE_DIR}/data/date_time_zonespec.csv" DESTINATION "${CMAKE_BINARY_DIR}/src/cli")
endif(BUILD_TYPE MATCHES "debug")

# CPack related scripting
include(InstallRequiredSystemLibraries)
set(CPACK_NSIS_PACKAGE_NAME ${WINDNINJA_VERSION_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license.txt")
# version
set(CPACK_VERSION_MAJOR ${VER_MAJ})
set(CPACK_VERSION_MINOR ${VER_MIN})
set(CPACK_VERSION_PATCH ${VER_PATCH})
set(CPACK_PACKAGE_VERSION ${WINDNINJA_VERSION})

set(CPACK_NSIS_INSTALL_ROOT "C:")
CMAKE_POLICY(PUSH)
CMAKE_POLICY(SET CMP0010 OLD)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "WindNinja\\\\${WINDNINJA_VERSION_NAME}")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "WindNinja\\\\${WINDNINJA_VERSION_NAME}")
CMAKE_POLICY(POP)
set(CPACK_NSIS_DISPLAY_NAME "WindNinja\\\\${WINDNINJA_VERSION_NAME}")
set(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_SOURCE_DIR}/images/icons/wn-desktop.ico")

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
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\ArcView Instructions.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\Displaying_wind_vectors_in_ArcView.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\DEM Download Instructions.lnk\\\" \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\download_elevation_file.pdf\\\"
CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Example Files.lnk\\\" \\\"$INSTDIR\\\\etc\\\\windninja\\\\example-files\\\" ")

set(CPACK_NSIS_DELETE_ICONS_EXTRA "
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

# Build the nsis installer on the firelab machine(s).  Not for external use.
if(FIRELAB_PACKAGE AND WIN32 AND MSVC AND GIS_INTERNALS_HOME)
    file(GLOB dlls ${GIS_INTERNALS_HOME}/bin/*.dll)
    foreach(dll ${dlls})
        install(PROGRAMS ${dll} DESTINATION bin COMPONENT apps)
    endforeach(dll ${dlls})
    install(PROGRAMS ${GIS_INTERNALS_HOME}/bin/gdal/plugins/gdal_netCDF.dll DESTINATION bin/gdalplugins COMPONENT app)
    get_filename_component(qtbin ${QT_QMAKE_EXECUTABLE} DIRECTORY)
    string(REGEX REPLACE "\\\\" "/" qtbin "${qtbin}")
    if(BUILD_TYPE MATCHES "release")
        install(PROGRAMS "${qtbin}/qtgui4.dll" DESTINATION bin COMPONENT apps)
        install(PROGRAMS "${qtbin}/qtcore4.dll" DESTINATION bin COMPONENT apps)
        install(PROGRAMS "${qtbin}/qtwebkit4.dll" DESTINATION bin COMPONENT apps)
        install(PROGRAMS "${qtbin}/phonon4.dll" DESTINATION bin COMPONENT apps)
        install(PROGRAMS "${qtbin}/qtnetwork4.dll" DESTINATION bin COMPONENT apps)
        install(PROGRAMS "${QT_PLUGINS_DIR}/imageformats/qjpeg4.dll" DESTINATION bin/imageformats)
    else(BUILD_TYPE MATCHES "release")
        install(PROGRAMS "${qtbin}/qtguid4.dll" DESTINATION bin COMPONENT apps)
        install(PROGRAMS "${qtbin}/qtcored4.dll" DESTINATION bin COMPONENT apps)
        install(PROGRAMS "${qtbin}/qtwebkitd4.dll" DESTINATION bin COMPONENT apps)
        install(PROGRAMS "${qtbin}/phonond4.dll" DESTINATION bin COMPONENT apps)
        install(PROGRAMS "${qtbin}/qtnetworkd4.dll" DESTINATION bin COMPONENT apps)
        install(PROGRAMS "${QT_PLUGINS_DIR}/imageformats/qjpegd4.dll" DESTINATION bin/imageformats)
    endif( BUILD_TYPE MATCHES "release")
    # HACK FOR OPENMP
    if(BUILD_64)
        install(PROGRAMS ${MSVC10_REDIST_DIR}/x64/Microsoft.VC100.OPENMP/vcomp100.dll DESTINATION bin COMPONENT apps)
    else(BUILD_64)
        install(PROGRAMS ${MSVC10_REDIST_DIR}/x86/Microsoft.VC100.OPENMP/vcomp100.dll DESTINATION bin COMPONENT apps)
    endif(BUILD_64)
    if(NINJAFOAM)
        install(DIRECTORY "${FOAM_MINGW_PATH}/etc" DESTINATION bin)
        install(DIRECTORY "${FOAM_MINGW_PATH}/platforms" DESTINATION bin)
    endif(NINJAFOAM)
endif(FIRELAB_PACKAGE AND WIN32 AND MSVC AND GIS_INTERNALS_HOME)
set(CPACK_COMPONENT_APPS_REQUIRED TRUE)
set(CPACK_COMPONENT_LIBS_HIDDEN TRUE)
set(CPACK_COMPONENT_LIBS_DISABLED TRUE)
set(CPACK_COMPONENT_INCLUDES_HIDDEN TRUE)
set(CPACK_COMPONENT_INCLUDES_DISABLED TRUE)

# *****************************************************************************
# CPack call
# *****************************************************************************
include(CPack)
include(CTest)

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

