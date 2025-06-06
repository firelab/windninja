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

include_directories(${PROJECT_SOURCE_DIR}/src
                    ${PROJECT_SOURCE_DIR}/src/ninja
                    ${Boost_INCLUDE_DIRS}
                    ${NETCDF_INCLUDES}
                    ${GDAL_SYSTEM_INCLUDE} ${GDAL_INCLUDE_DIR})

set(LINK_LIBS ${Boost_LIBRARIES}
              ${GDAL_LIBRARY}
              ${NETCDF_LIBRARIES_C})

set(SOLAR_GRID_SRC solar_grid.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/ascii_grid.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/ninjaMathUtility.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/Array2D.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/EasyBMP.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/EasyBMP_Geometry.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/EasyBMP_Font.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/solpos.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/solar.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/Elevation.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/Slope.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/Aspect.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/Shade.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/ninja_conv.cpp
                   ${PROJECT_SOURCE_DIR}/src/ninja/ninja_init.cpp)

add_executable(solar_grid ${SOLAR_GRID_SRC})

target_link_libraries(solar_grid ${LINK_LIBS})

install(TARGETS solar_grid DESTINATION bin COMPONENT apps)

