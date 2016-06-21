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

#include "surface_fetch.h"
SurfaceFetch::SurfaceFetch()
{
    xRes = 0.0;
    yRes = 0.0;
    northeast_x = 0.0;
    northeast_y = 0.0;
    southeast_x = 0.0;
    southeast_y = 0.0;
    southwest_x = 0.0;
    southwest_y = 0.0;
    northwest_x = 0.0;
    northwest_y = 0.0;
    path = "";
}

SurfaceFetch::~SurfaceFetch()
{

}

SURF_FETCH_E SurfaceFetch::GetCorners(double *northeast, double *southeast,
                                      double *southwest, double *northwest)
{
    northeast[0] = northeast_x;
    northeast[1] = northeast_y;
    southeast[0] = southeast_x;
    southeast[1] = southeast_y;
    southwest[0] = southwest_x;
    southwest[1] = southwest_y;
    northwest[0] = northwest_x;
    northwest[1] = northwest_y;
    return SURF_FETCH_E_NONE;
}
double SurfaceFetch::GetXRes()
{
    return xRes;
}

double SurfaceFetch::GetYRes()
{
    return yRes;
}

std::string SurfaceFetch::GetPath()
{
    return path;
}

SURF_FETCH_E SurfaceFetch::makeReliefOf( std::string infile, std::string outfile, int nXSize, int nYSize )
{
    return SURF_FETCH_E_NONE;
}

/**
 * \brief Default implementation of FetchPoint
 *
 * This function merely creates a bounding box and passes through to
 * FetchBoundingBox.  It _does not_ check options.  Any class with other
 * options that are specific to the point data fetching should reimplement 
 * this and check for options.
 *
 * @param point x, y point in WGS 84 longitude, latitude
 * @param buffer length of the buffer for x and y
 * @param units buffer units
 * @param resoultion output cell resolution
 * @param filename output filename
 * @param options list of options, not checked in here.
 * @return number of no data values on success, < 0 on error
 */
SURF_FETCH_E SurfaceFetch::FetchPoint(double *point, double *buffer,
                                      lengthUnits::eLengthUnits units,
                                      double resolution, const char *filename,
                                      char **options)
{
    double bbox[4];
    CreateBoundingBox(point, buffer, units, bbox);
    int nNoDataCount = FetchBoundingBox(bbox, resolution, filename,
                                        options);

    return nNoDataCount;
}
/**
 * \brief Create a bounding box from a point and a buffer.
 *
 * @param point a x,y point in WGS 84 longitude, latitude
 * @param buffer length of a buffer in the x and y directions
 * @param units the units for the buffer
 * @param the 4 element array to be filled in by the function, clockwise from
 *        north:
 *        bbox[0] -> north
 *        bbox[1] -> east
 *        bbox[2] -> south
 *        bbox[3] -> west
 * @return zero on success, negative otherwise
 */
SURF_FETCH_E SurfaceFetch::CreateBoundingBox(double *point, double *buffer, 
                                             lengthUnits::eLengthUnits units,
                                             double *bbox)
{
    if(point == NULL || buffer == NULL || bbox == NULL)
    {
        fprintf(stderr, "Invalid input\n");
        return SURF_FETCH_E_BAD_INPUT;
    }
    OGRSpatialReference oSrcSRS, oDstSRS;

    /* We assume the point is in WGS 84, always */
    oSrcSRS.importFromEPSG(4326);

    int zone = GetUTMZoneInEPSG(point[0], point[1]);
    oDstSRS.importFromEPSG(zone);

    OGRCoordinateTransformation *poCT;
    poCT = OGRCreateCoordinateTransformation( &oSrcSRS, &oDstSRS );
    if(poCT == NULL)
    {
        return SURF_FETCH_E_WARPER_ERR;
    }

    poCT->Transform(1, &point[0], &point[1]);
    OGRCoordinateTransformation::DestroyCT(poCT);

    lengthUnits::toBaseUnits(buffer[0], units);
    lengthUnits::toBaseUnits(buffer[1], units);

    bbox[1] = point[0] + buffer[0];
    bbox[3] = point[0] - buffer[0];
    bbox[0] = point[1] + buffer[1];
    bbox[2] = point[1] - buffer[1];

    /* Warp back to WGS 84, this isn't the best way to do this */
    poCT = OGRCreateCoordinateTransformation(&oDstSRS, &oSrcSRS);
    if(poCT == NULL)
    {
        return SURF_FETCH_E_WARPER_ERR;
    }

    poCT->Transform(1, &bbox[1], &bbox[0]);
    poCT->Transform(1, &bbox[3], &bbox[2]);
    OGRCoordinateTransformation::DestroyCT(poCT);

    return SURF_FETCH_E_NONE;
}

SURF_FETCH_E SurfaceFetch::WarpBoundingBox(double *bbox)
{
    int nUtmZone = BoundingBoxUtm(bbox);
    OGRSpatialReference oSrcSRS, oDstSRS;
    oSrcSRS.importFromEPSG(4326);
    oDstSRS.importFromEPSG(nUtmZone);
    OGRCoordinateTransformation *poCT;
    poCT = OGRCreateCoordinateTransformation( &oSrcSRS, &oDstSRS );
    if(poCT == NULL)
    {
        return SURF_FETCH_E_WARPER_ERR;
    }

    poCT->Transform(1, &bbox[3], &bbox[0]);
    poCT->Transform(1, &bbox[1], &bbox[2]);
    OGRCoordinateTransformation::DestroyCT(poCT);
    return SURF_FETCH_E_NONE;
}

int SurfaceFetch::BoundingBoxUtm(double *bbox)
{
    double dfX, dfY;
    dfX = bbox[3] + ((bbox[1] - bbox[3]) / 2);
    dfY = bbox[2] + ((bbox[0] - bbox[2]) / 2);
    return GetUTMZoneInEPSG(dfX, dfY);
}

/**
 * \brief Extract a file from an archive.  If pszFile is null, extract all.
 *
 * \param pszArchive archive to extract files from
 * \param pszDstPath path to write files to.
 * \param pszFile file to extract.  Must be the full path *inside* the archive
 * \return 0 on success
 */
#ifdef WITH_LCP_CLIENT
int SurfaceFetch::ExtractFileFromZip( const char *pszArchive,
                                      const char *pszFile,
                                      const char *pszDstFile )
{

    if( pszArchive == NULL || pszDstFile == NULL || pszFile == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Invalid input" );
        return SURF_FETCH_E_BAD_INPUT;
    }
    const char *pszVSIName = CPLSPrintf( "/vsizip/%s", pszArchive );
    int nBlockSize = atoi( CPLGetConfigOption( "SURF_FETCH_EXTRACT_BLOCK_SIZE",
                                               "1024" ) );
    char **papszFileList = VSIReadDirRecursive( pszVSIName );
    int nCount = CSLCount( papszFileList );
    if( nCount < 1 )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Archive is empty" );
        return SURF_FETCH_E_IO_ERR;
    }
    /*
    ** Check for the file if we are extracting
    */
    int bFound = FALSE;
    int nFileListIndex = 0;
    for( int i = 0; i < nCount; i++ )
    {
        if( EQUAL( papszFileList[i], pszFile ) )
        {
            bFound = TRUE;
            nFileListIndex = i;
            break;
        }
    }
    if( !bFound )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "File could not be found in archive" );
        CSLDestroy( papszFileList );
        return SURF_FETCH_E_BAD_INPUT;
    }
    VSILFILE *fin, *fout;
    int nBytesRead = 0;
    GByte *pabyData = (GByte*)CPLMalloc( nBlockSize );
    memset( pabyData, 0, nBlockSize );
    vsi_l_offset nOffset = 0;
    for( int i = 0; i < nCount; i++ )
    {
        pszVSIName = CPLSPrintf( "/vsizip/%s/%s", pszArchive,
                                 papszFileList[nFileListIndex] );
        fin = VSIFOpenL( pszVSIName, "rb" );
        fout = VSIFOpenL( pszDstFile, "wb" );
        if( !fout || !fin )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Failed to open files" );
            CPLFree( pabyData );
            CSLDestroy( papszFileList );
            return SURF_FETCH_E_IO_ERR;
        }
        do
        {
            nOffset = VSIFReadL( pabyData, 1, nBlockSize, fin );
            VSIFWriteL( pabyData, 1, nOffset, fout );
        }
        while( nOffset > 0 );
        VSIFCloseL( fin );
        VSIFCloseL( fout );
    }

    CPLFree( pabyData );
    CSLDestroy( papszFileList );
    return SURF_FETCH_E_NONE;
}

int SurfaceFetch::SelfTest()
{
    int nError = 0;
    nError = TestZip();
    if( nError )
        return nError;
    return 0;
}

int SurfaceFetch::TestZip()
{
    if( CPLCheckForFile( (char*)"unziptmp.zip", NULL ) )
    {
        VSIUnlink( "unziptmp.zip" );
    }
    GByte *pabyData[5000];
    VSILFILE *tmp;
    tmp = VSIFOpenL( "/vsizip/unziptmp.zip/one.txt", "w" );
    VSIFWriteL( pabyData, 1, 5000, tmp );
    VSIFCloseL( tmp );
    tmp = VSIFOpenL( "/vsizip/unziptmp.zip/two/two.txt", "w" );
    VSIFWriteL( pabyData, 2, 10000, tmp );
    VSIFCloseL( tmp );
    tmp = VSIFOpenL( "/vsizip/unziptmp.zip/three/three/three.txt", "w" );
    VSIFWriteL( pabyData, 3, 15000, tmp );
    VSIFCloseL( tmp );

    ExtractFileFromZip( "unziptmp.zip", "one.txt", "one.txt" );
    if( !CPLCheckForFile( (char*)"one.txt", NULL ) )
    {
        VSIUnlink( "unziptmp.zip" );
        return 1;
    }
    VSIUnlink( "one.txt" );
    ExtractFileFromZip( "unziptmp.zip",  "two/two.txt", "two.txt" );
    if( !CPLCheckForFile( (char*)"two.txt", NULL ) )
    {
        VSIUnlink( "unziptmp.zip" );
        return 1;
    }
    VSIUnlink( "two.txt" );
    ExtractFileFromZip( "unziptmp.zip", "three/three/three.txt", "three.txt" );
    if( !CPLCheckForFile( (char*)"three.txt", NULL ) )
    {
        VSIUnlink( "unziptmp.zip" );
        return 1;
    }
    VSIUnlink( "three.txt" );
    VSIUnlink( "unziptmp.zip" );
    return 0;
}

#endif /* WITH_LCP_CLIENT */

