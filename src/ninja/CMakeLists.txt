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

set(NINJA_INCLUDES ${NETCDF_INCLUDES}
                   ${GDAL_INCLUDE_DIR} ${CURL_INCLUDE_DIRS}
                   ${Boost_INCLUDE_DIRS}
                   ${PROJECT_SOURCE_DIR}/src)

set(NINJA_SOURCES addDiurnalFlow.cpp
                  air.cpp
                  Aspect.cpp
                  cellDiurnal.cpp
                  cli.cpp
                  dbfopen.cpp
                  domainAverageInitialization.cpp
                  dust.cpp
                  EasyBMP.cpp
                  EasyBMP_Font.cpp
                  EasyBMP_Geometry.cpp
                  element.cpp
                  Elevation.cpp
                  farsiteAtm.cpp
                  fetch_factory.cpp
                  fluid.cpp
                  frictionVelocity.cpp
                  gdal_util.cpp
                  gdal_fetch.cpp
                  genericSurfInitialization.cpp
                  griddedInitialization.cpp
                  initialize.cpp
                  KmlVector.cpp
                  LineStyle.cpp
                  mesh.cpp
                  landfireclient.cpp
                  ncepDgexSurfInitialization.cpp
                  ncepGfsSurfInitialization.cpp
                  ncepNamAlaskaSurfInitialization.cpp
                  ncepNamSurfInitialization.cpp
                  ncepNamGrib2SurfInitialization.cpp
                  ncepHrrrSurfInitialization.cpp
                  ncepNdfdInitialization.cpp
                  ncepRapSurfInitialization.cpp
                  wrfSurfInitialization.cpp
                  wrf3dInitialization.cpp
                  ninja_conv.cpp
                  ninjaArmy.cpp
                  ninjaCom.cpp
                  ninja.cpp
                  ninjaException.cpp
                  ninja_init.cpp
                  ninjaMathUtility.cpp
                  ninjaUnits.cpp
                  ninja_threaded_exception.cpp
                  omp_guard.cpp
                  OutputWriter.cpp
                  pointInitialization.cpp
                  preconditioner.cpp
                  readInputFile.cpp
                  relief_fetch.cpp
                  Shade.cpp
                  ShapeVector.cpp
                  shpopen.cpp
                  Slope.cpp
                  solar.cpp
                  solpos.cpp
                  stability.cpp
                  startRuns.cpp
                  stl_create.cpp
                  Style.cpp
                  surface_fetch.cpp
                  surfaceVectorField.cpp
                  SurfProperties.cpp
                  volVTK.cpp
                  WindNinjaInputs.cpp
                  windProfile.cpp
                  wn_3dArray.cpp
                  wn_3dScalarField.cpp
                  wn_3dVectorField.cpp
                  wn_Arrow.cpp
                  wxModelInitialization.cpp
                  wxModelInitializationFactory.cpp
                  wxStation.cpp
                  windninja.cpp)
if(NINJAFOAM)
    set(NINJA_SOURCES ${NINJA_SOURCES} 
                    ninjafoam.cpp
                    foamInitialization.cpp)
endif(NINJAFOAM)

if(WITH_NOMADS_SUPPORT)
    set(NINJA_SOURCES ${NINJA_SOURCES}
                      nomads_wx_init.cpp
                      nomads.c
                      nomads_utc.c)
    if(NOT HAVE_TIMEGM)
        message("Using internal timegm")
        set(NINJA_SOURCES ${NINJA_SOURCES} timegm.c)
    endif(NOT HAVE_TIMEGM)
endif(WITH_NOMADS_SUPPORT)

set(LINK_LIBS ${NETCDF_LIBRARIES_C} 
              ${GDAL_LIBRARY} 
              ${CURL_LIBRARIES}
              ${Boost_LIBRARIES})


if(NINJA_QTGUI)
    include_directories(${NINJA_INCLUDES} ${QT_INCLUDES})
    QT4_WRAP_CPP(NINJA_LIB_INCLUDES_MOC ninjaCom.h)
    set(NINJA_SOURCES ${NINJA_SOURCES} ${NINJA_LIB_INCLUDES_MOC})
    set(LINK_LIBS ${LINK_LIBS} ${QT_LIBRARIES})
else(NINJA_QTGUI)
    include_directories(${NINJA_INCLUDES})
endif(NINJA_QTGUI)

if(WIN32)
    add_library(ninja STATIC ${NINJA_SOURCES})
else(WIN32)
    add_library(ninja STATIC SHARED ${NINJA_SOURCES})
endif(WIN32)

target_link_libraries(ninja ${LINK_LIBS})

install(TARGETS ninja DESTINATION lib COMPONENT libs)
install(FILES WindNinjaInputs.h ninja.h windninja.h DESTINATION include COMPONENT includes)

