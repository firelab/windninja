/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  utility functions for gdaldatasets
 * Author:   Kyle Shannon <ksshannon@gmail.com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY,
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef GDAL_UTIL_H
#define GDAL_UTIL_H

#include <string>
#include <iostream>

#include "gdal.h"
#include "gdal_alg.h"
#include "gdal_priv.h"
#include "ogr_geometry.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"

#include "ascii_grid.h"

double GDALGetMax( GDALDataset *poDS );
double GDALGetMin( GDALDataset *poDS );
bool GDALGetCenter( GDALDataset *poDS, double *centerLonLat );
bool GDALGetBounds( GDALDataset *poDS, double *boundsLonLat );
bool GDALTestSRS( GDALDataset *poDS );
bool GDALHasNoData( GDALDataset *poDS, int band );
bool GDAL2AsciiGrid( GDALDataset *poDS, int band, AsciiGrid<double> &grid );
bool GDALPointFromLatLon( double &x, double &y, GDALDataset *poDstDS,
                          const char *datum );
bool GDALPointToLatLon( double &x, double &y, GDALDataset *poSrcDS,
                        const char *datum );
bool OGRPointToLatLon( double &x, double &y, OGRDataSourceH hDS,
				const char *datum );
int GDALGetUtmZone( GDALDataset *poDS );
int GDALFillBandNoData( GDALDataset *poDS, int nBand, int nSearchPixels );
int GetUTMZoneInEPSG( double lon, double lat );
int GDALGetCorners( GDALDataset *poDS, double *corners );
std::string FetchTimeZone( double dfX, double dfY, const char *pszWkt );
int NinjaOGRContain(const char *pszWkt, const char *pszFile,
                    const char *pszLayer);
#endif /* GDAL_UTIL_H */
