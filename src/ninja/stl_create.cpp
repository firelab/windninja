/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Functions for creating an stl from a gdal dataset
 * Author:   Kyle Shannon <kyle at pobox dot com>
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

#include "stl_create.h"

static StlPosition StlComputeNormal( StlPosition *v1,  StlPosition *v2 )
{
     float norm_factor = 0;
     StlPosition norm;

     norm.x = ( v1->y * v2->z ) - ( v1->z * v2->y );
     norm.y = ( v2->x * v1->z ) - ( v2->z * v1->x );
     norm.z = ( v1->x * v2->y ) - ( v1->y * v2->x );

     norm_factor = sqrt( norm.x * norm.x + norm.y * norm.y + norm.z * norm.z );

     norm.x /= norm_factor;
     norm.y /= norm_factor;
     norm.z /= norm_factor;

     return norm;
}

/**
 * \brief Create an STL representation of an elevation grid.
 *
 * \param pszInput file to read and convert
 * \param pszOutput file to write (stl)
 * \param nBand band to treat as elevation
 * \param eType type of stl file to create, ascii or binary.  Currently only
 *              binary is supported
 * \param pfnProgress a pointer to a progress function
 * \return zero on success, non-zero otherwise
 */
CPLErr NinjaElevationToStl( const char *pszInput,
                            const char *pszOutput,
                            int nBand,
                            NinjaStlType eType,
                            GDALProgressFunc pfnProgress )
{
    GDALDatasetH hDS;
    GDALRasterBandH hBand;
    double adfGeoTransform[6];
    int nXSize, nYSize, nBandCount;
    float *pafScanline;
    unsigned int nTriCount;
    unsigned short nAttrCount;

    VSILFILE *fout;

    VALIDATE_POINTER1( pszInput, "NinjaElevationToStl()", CE_Failure );
    VALIDATE_POINTER1( pszOutput, "NinjaElevationToStl()", CE_Failure );

    if( nBand < 1 )
    {
        CPLError( CE_Warning, CPLE_AppDefined,
                  "Invalid band, using band 1" );
    }
    nBand = nBand < 1 ? 1 : nBand;

    hDS = GDALOpen( pszInput, GA_ReadOnly );
    if( hDS == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to open input dataset" );
        return CE_Failure;
    }

    if( GDALGetGeoTransform( hDS, adfGeoTransform ) != CE_None )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to fetch geotransform from dataset" );
        GDALClose( hDS );
        return CE_Failure;
    }

    nBandCount = GDALGetRasterCount( hDS );
    if( nBand > nBandCount )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Invalid band" );
        GDALClose( hDS );
        return CE_Failure;
    }
    nXSize = GDALGetRasterXSize( hDS );
    nYSize = GDALGetRasterYSize( hDS );

    hBand = GDALGetRasterBand( hDS, nBand );
    if( hBand == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Invalid band" );
        GDALClose( hDS );
        return CE_Failure;
    }

    if( eType == NinjaStlBinary )
    {
        fout = VSIFOpenL( pszOutput, "wb" );
    }
    else
    {
        fout = VSIFOpenL( pszOutput, "w" );
    }
    if( fout == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to open output file" );
        GDALClose( hDS );
        return CE_Failure;
    }

    char nil[80];
    memset( nil, '\0', 80 );

    nTriCount = (nXSize-1) * (nYSize-1) * 2; //cell centers are vertices
    nAttrCount = 0;

    float fXOffset, fYOffset;
    fXOffset = adfGeoTransform[1] * 0.5;
    fYOffset = adfGeoTransform[5] * 0.5;

    pafScanline = (float*)VSIMalloc3( nXSize, 2, sizeof( float ) );
    if( pafScanline == NULL )
    {
        CPLError( CE_Failure, CPLE_OutOfMemory,
                  "Could not allocate buffer" );
        GDALClose( hDS );
        VSIFCloseL( fout );
        return CE_Failure;
    }

    CPLErr eErr = CE_None;
    int i, j;
    StlPosition a, b, c, d;
    StlPosition v1, v2, norm;
    if( eType == NinjaStlBinary )
    {
        VSIFWriteL( nil, 1, 80, fout );
        VSIFWriteL( &nTriCount, sizeof( unsigned int ), 1, fout );
    }
    else
    {
        VSIFPrintfL( fout, "solid NAME\n" );
    }
    if( pfnProgress )
    {
        pfnProgress( 0.0, NULL, NULL );
    }
    for( i = 0 ; i < nYSize - 1; i++ )
    {
        eErr = GDALRasterIO( hBand, GF_Read, 0, i, nXSize, 2,
                             pafScanline, nXSize, 2, GDT_Float32, 0, 0 );
        for( j = 0; j < nXSize - 1; j++ )
        {
            a.x = adfGeoTransform[0] + j * adfGeoTransform[1] + fXOffset;
            a.y = adfGeoTransform[3] + i * adfGeoTransform[5] + fYOffset;
            a.z = pafScanline[j];

            b.x = adfGeoTransform[0] + ( j + 1 ) * adfGeoTransform[1] + fXOffset;
            b.y = a.y;
            b.z = pafScanline[j + 1];

            c.x = a.x;
            c.y = adfGeoTransform[3] + ( i + 1 ) * adfGeoTransform[5] + fYOffset;
            c.z = pafScanline[j + nXSize];

            d.x = b.x;
            d.y = c.y;
            d.z = pafScanline[j + nXSize + 1];

            v1.x = c.x - a.x;
            v1.y = c.y - a.y;
            v1.z = c.z - a.z;

            v2.x = b.x - a.x;
            v2.y = b.y - a.y;
            v2.z = b.z - a.z;

            norm = StlComputeNormal( &v1, &v2 );

            if( eType == NinjaStlBinary )
            {
                VSIFWriteL( &norm.x, sizeof( float ), 1, fout );
                VSIFWriteL( &norm.y, sizeof( float ), 1, fout );
                VSIFWriteL( &norm.z, sizeof( float ), 1, fout );
                VSIFWriteL( &b.x, sizeof( float ), 1, fout );
                VSIFWriteL( &b.y, sizeof( float ), 1, fout );
                VSIFWriteL( &b.z, sizeof( float ), 1, fout );
                VSIFWriteL( &a.x, sizeof( float ), 1, fout );
                VSIFWriteL( &a.y, sizeof( float ), 1, fout );
                VSIFWriteL( &a.z, sizeof( float ), 1, fout );
                VSIFWriteL( &c.x, sizeof( float ), 1, fout );
                VSIFWriteL( &c.y, sizeof( float ), 1, fout );
                VSIFWriteL( &c.z, sizeof( float ), 1, fout );
                VSIFWriteL( &nAttrCount, sizeof( unsigned short ), 1, fout );
            }
            else
            {
                VSIFPrintfL( fout, "facet normal %e %e %e\n",
                             norm.x, norm.y, norm.z );

                VSIFPrintfL( fout, "    outer loop\n" );
                VSIFPrintfL( fout, "        vertex %e %e %e\n", b.x, b.y, b.z );
                VSIFPrintfL( fout, "        vertex %e %e %e\n", a.x, a.y, a.z );
                VSIFPrintfL( fout, "        vertex %e %e %e\n", c.x, c.y, c.z );
                VSIFPrintfL( fout, "    endloop\n" );
                VSIFPrintfL( fout, "endfacet\n" );
            }

            v1.x = b.x - d.x;
            v1.y = b.y - d.y;
            v1.z = b.z - d.z;

            v2.x = c.x - d.x;
            v2.y = c.y - d.y;
            v2.z = c.z - d.z;

            norm = StlComputeNormal( &v1, &v2 );
            if( eType == NinjaStlBinary )
            {
                VSIFWriteL( &norm.x, sizeof( float ), 1, fout );
                VSIFWriteL( &norm.y, sizeof( float ), 1, fout );
                VSIFWriteL( &norm.z, sizeof( float ), 1, fout );
                VSIFWriteL( &d.x, sizeof( float ), 1, fout );
                VSIFWriteL( &d.y, sizeof( float ), 1, fout );
                VSIFWriteL( &d.z, sizeof( float ), 1, fout );
                VSIFWriteL( &b.x, sizeof( float ), 1, fout );
                VSIFWriteL( &b.y, sizeof( float ), 1, fout );
                VSIFWriteL( &b.z, sizeof( float ), 1, fout );
                VSIFWriteL( &c.x, sizeof( float ), 1, fout );
                VSIFWriteL( &c.y, sizeof( float ), 1, fout );
                VSIFWriteL( &c.z, sizeof( float ), 1, fout );
                VSIFWriteL( &nAttrCount, sizeof( unsigned short ), 1, fout );
            }
            else
            {
                VSIFPrintfL( fout, "facet normal %e %e %e\n",
                            norm.x, norm.y, norm.z );

                VSIFPrintfL( fout, "    outer loop\n" );
                VSIFPrintfL( fout, "        vertex %e %e %e\n", d.x, d.y, d.z );
                VSIFPrintfL( fout, "        vertex %e %e %e\n", b.x, b.y, b.z );
                VSIFPrintfL( fout, "        vertex %e %e %e\n", c.x, c.y, c.z );
                VSIFPrintfL( fout, "    endloop\n" );
                VSIFPrintfL( fout, "endfacet\n" );
            }
        }
        if( pfnProgress )
        {
            pfnProgress( double(i) / (double)nYSize, NULL, NULL );
        }
    }
    if( eType == NinjaStlAscii )
    {
        VSIFPrintfL( fout, "endsolid %s\n", CPLGetBasename( pszInput ) );
    }
    if( pfnProgress )
    {
        pfnProgress( 1.0, NULL, NULL );
    }

    VSIFree( pafScanline );
    GDALClose( hDS );
    VSIFCloseL( fout );

    return eErr;
}

