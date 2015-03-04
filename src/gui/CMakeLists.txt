#/*****************************************************************************
# *
# * $Id$
# *
# * Project:  WindNinja
# * Purpose:  CMake script
# * Author:   Kyle Shannon <ksshannon@gmail.com>
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

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

QT4_WRAP_UI(QTUI_H_SRCS WidgetDownloadDEM.ui
                        setconfigdialog.ui)

SET(NINJA_GUI_INCLUDES consoleDockWidget.h
                       diurnalInput.h
                       stabilityInput.h
                       fbOutput.h
                       googleOutput.h
                       latLonWidget.h
                       mainWindow.h
                       metaWindWidget.h
                       nativeSolverInput.h
                       ninjafoamInput.h
                       outputHeightWidget.h
                       outputMetaData.h
                       pointDataModel.h
                       pointInputDelegate.h
                       pointInput.h
                       shapeOutput.h
                       solvePage.h
                       solveThread.h
                       splash.h
                       surfaceInput.h
                       timeZoneWidget.h
                       #vec3f.h
                       vtkOutput.h
                       weatherModel.h
                       WidgetDownloadDEM.h
                       windInput.h
                       windInputTable.h
                       WindNinjaTree.h
                       GoogleMapsInterface.h
                       setconfigdialog.h
                       ${QTUI_H_SRCS})

INCLUDE_DIRECTORIES(${QT_INCLUDES}
                    ${Boost_INCLUDE_DIRS}
                    ${NETCDF_INCLUDES}
                    ${GDAL_INCLUDE_DIR}
                    ${CURL_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/src
                    ${PROJECT_SOURCE_DIR}/src/ninja
                    ${CMAKE_CURRENT_BINARY_DIR})

QT4_WRAP_CPP(NINJA_GUI_INCLUDES_MOC ${NINJA_GUI_INCLUDES})
QT4_ADD_RESOURCES(NINJA_GUI_RESOURCES ${PROJECT_SOURCE_DIR}/wn-resources.qrc)

SET(NINJA_GUI_SOURCES cmake_gui.cpp
                      ${PROJECT_SOURCE_DIR}/src/ninja/cli.cpp
                      consoleDockWidget.cpp
                      diurnalInput.cpp
                      stabilityInput.cpp
                      fbOutput.cpp
                      googleOutput.cpp
                      latLonWidget.cpp
                      mainWindow.cpp
                      metaWindWidget.cpp
                      outputHeightWidget.cpp
                      outputMetaData.cpp
                      nativeSolverInput.cpp
                      ninjafoamInput.cpp
                      pointDataModel.cpp
                      pointInput.cpp
                      pointInputDelegate.cpp
                      shapeOutput.cpp
                      solvePage.cpp
                      solveThread.cpp
                      splash.cpp
                      surfaceInput.cpp
                      timeZoneWidget.cpp
                      vec3f.cpp
                      vtkOutput.cpp
                      weatherModel.cpp
                      windInput.cpp
                      windInputTable.cpp
                      WindNinjaTree.cpp
                      WidgetDownloadDEM.cpp
                      GoogleMapsInterface.cpp
                      setconfigdialog.cpp
                      ${QTUI_H_SRCS})
IF(WIN32)
    SET(NINJA_GUI_SOURCES ${NINJA_GUI_SOURCES} 
                          ${PROJECT_SOURCE_DIR}/wn-ms-resource.rc)
    ADD_DEFINITIONS(-DQT_NO_DEBUG_OUTPUT)
ENDIF(WIN32)

SET(LINK_LIBS ${QT_LIBRARIES}
              ${GDAL_LIBRARY}
              ${Boost_LIBRARIES}
              ${CURL_LIBRARIES}
              ${NETCDF_LIBRARIES_C})

IF(WIN32)
    SET(LINK_LIBS ${LINK_LIBS} ${CMAKE_BINARY_DIR}/src/ninja/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}ninja${CMAKE_STATIC_LIBRARY_SUFFIX})
    IF(NOT ENABLE_CONSOLE)
        SET(LINK_LIBS ${LINK_LIBS} ${QT_QTMAIN_LIBRARY})
        SET(GUI_TYPE WIN32)
    ELSE(NOT ENABLE_CONSOLE)
        SET(GUI_TYPE "")
    ENDIF(NOT ENABLE_CONSOLE)
ELSE(WIN32)
    SET(LINK_LIBS ${LINK_LIBS} ${CMAKE_BINARY_DIR}/src/ninja/${CMAKE_SHARED_LIBRARY_PREFIX}ninja${CMAKE_SHARED_LIBRARY_SUFFIX})
ENDIF(WIN32)

ADD_EXECUTABLE(WindNinja ${GUI_TYPE}
                         ${NINJA_GUI_INCLUDES_MOC} 
                         ${NINJA_GUI_RESOURCES}
                         ${NINJA_GUI_SOURCES})

TARGET_LINK_LIBRARIES(WindNinja ${LINK_LIBS})
ADD_DEPENDENCIES(WindNinja ninja)
INSTALL(TARGETS WindNinja DESTINATION bin COMPONENT apps)

