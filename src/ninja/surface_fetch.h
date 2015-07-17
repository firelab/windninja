/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Base class for surface fetching
 * Author:   Kyle Shannon <kyle@pobox.com>
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
#ifndef SURFACE_FETCH_H
#define SURFACE_FETCH_H

#include "gdal_priv.h"
#include "gdalwarper.h"
#include "ogr_spatialref.h"

#include "gdal_util.h"
#include "ninjaUnits.h"
#include "ninja_conv.h"

typedef int SURF_FETCH_E;
#define SURF_FETCH_E_NONE          0
#define SURF_FETCH_E_IO_ERR       -1
#define SURF_FETCH_E_BOUNDS_ERR   -2
#define SURF_FETCH_E_WARPER_ERR   -3
#define SURF_FETCH_E_BAD_INPUT    -4
#define SURF_FETCH_E_SIZE_LIMIT   -5
#define SURF_FETCH_E_NO_GDAL_DATA -6
#define SURF_FETCH_E_TIMEOUT      -7

class SurfaceFetch
{
public:
    enum FetchType
    {
        US_SRTM,
        WORLD_SRTM,
        WORLD_GMTED,
        CUSTOM_GDAL
    };
    SurfaceFetch();
    virtual ~SurfaceFetch();

    virtual SURF_FETCH_E FetchBoundingBox(double *bbox, double resolution,
                                          const char *filename,
                                          char **options) = 0;

    virtual SURF_FETCH_E FetchPoint(double *point, double *buffer,
                                    lengthUnits::eLengthUnits units,
                                    double resolution, const char *filename,
                                    char **options);
    virtual SURF_FETCH_E GetCorners(double *northeast, double *southeast,
                                    double *southwest, double *northwest);
    virtual double GetXRes();
    virtual double GetYRes();
    virtual SURF_FETCH_E makeReliefOf( std::string infile, std::string outfile );

#ifdef WITH_LCP_CLIENT
    int ExtractFileFromZip( const char *pszArchive,
                            const char *pszFile,
                            const char *pszDstFile );

    virtual int SelfTest();
    int TestZip();
#endif /* WITH_LCP_CLIENT */

protected:
    virtual SURF_FETCH_E WarpBoundingBox(double *bbox);
    virtual std::string GetPath();
    virtual int BoundingBoxUtm(double *bbox);
    virtual SURF_FETCH_E CreateBoundingBox(double *point, double *buffer, 
                                           lengthUnits::eLengthUnits units,
                                           double *bbox);
    double xRes;
    double yRes;
    double northeast_x;
    double northeast_y;
    double southeast_x;
    double southeast_y;
    double southwest_x;
    double southwest_y;
    double northwest_x;
    double northwest_y;
    std::string path;
};


#endif /* SURFACE_FETCH_H */

