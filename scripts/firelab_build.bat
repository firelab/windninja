REM ****************************************************************************
REM *
REM * $Id$
REM *
REM * Project:  WindNinja
REM * Purpose:  Build installer for firelab
REM * Author:   Kyle Shannon <kyle@pobox.com>
REM *
REM **************************************************************************
REM *
REM * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
REM * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
REM * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
REM * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
REM * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
REM * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER 
REM * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
REM * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
REM *
REM * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
REM * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
REM * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
REM * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
REM * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
REM * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
REM * DEALINGS IN THE SOFTWARE.
REM *
REM **************************************************************************
REM ftp -A -s:c:/src/windninja/trunk/ninjaftp2.ftp ftp2.fs.fed.us
REM -DGDAL_INCLUDE_DIR=c:/src/gdal/gdal-1.9.1/include ^
REM -DGDAL_INCLUDE_DIR=c:/src/gdal/gdal-1.10.0/include ^
REM -DGDAL_LIBRARY=c:/src/gdal/gdal-1.9.1/lib/gdal_i.lib ^
REM -DGDAL_LIBRARY=c:/src/gdal/gdal-1.10.0/lib/gdal_i.lib ^

cmake c:/src/windninja/trunk ^
      -G "NMake Makefiles JOM" ^
      -DFIRELAB_PACKAGE=ON ^
      -DNINJA_GUI=ON ^
      -DNINJA_CLI=ON ^
      -DENABLE_CONSOLE=FALSE ^
      -DOPENMP_SUPPORT=ON ^
      -DPACKAGE_DEBUG=OFF ^
      -DVERBOSE_WARNINGS=OFF ^
      -DSUPRESS_WARNINGS=ON ^
      -DENABLE_GMTED=ON ^
      -DBUILD_FETCH_DEM=ON ^
      -DBUILD_SOLAR_GRID=ON ^
      -DSTABILITY=ON ^
      -DBUILD_TESTING=OFF ^
      -DCMAKE_BUILD_TYPE=release ^
      -DBOOST_ROOT=c:/src/boost/boost_1_46_1 ^
      -DCURL_INCLUDE_DIR=c:/src/libcurl/curl-7.21.4/include ^
      -DCURL_LIBRARY=c:/src/libcurl/curl-7.21.4/lib/dll-release/libcurl_imp.lib ^
      -DGDAL_INCLUDE_DIR=c:/src/gdal/gdal-1.9.1/include ^
      -DGDAL_LIBRARY=c:/src/gdal/gdal-1.9.1/lib/gdal_i.lib ^
      -DNETCDF_INCLUDES=c:/src/netcdf/netcdf-4.1.1/libsrc ^
      -DNETCDF_LIBRARIES=NOT_USED ^
      -DNETCDF_LIBRARIES_C=c:/src/netcdf/netcdf-4.1.1/win32/NET/Release/netcdf.lib ^
      -DQT_QMAKE_EXECUTABLE=c:/src/qt/qt-everywhere-opensource-src-4.8.4/bin/qmake.exe ^
      -DCPACK_BINARY_NSIS=ON ^
      -DCPACK_SOURCE_ZIP=OFF &
jom -j 2 package &
ftp -A -s:c:/src/windninja/trunk/scripts/ninjaftp2.ftp ftp2.fs.fed.us

