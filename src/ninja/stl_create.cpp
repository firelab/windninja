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
 * \param dfTargetCellSize the absolute resolution for dx/dy in DEM units.  Any
 *        value <= 0.0 is native resolution.
 * \param eType type of stl file to create, ascii or binary.  Currently only
 *              binary is supported
 * \param pfnProgress a pointer to a progress function
 * \return zero on success, non-zero otherwise
 */
CPLErr NinjaElevationToStl( const char *pszInput,
                            const char *pszOutput,
                            int nBand,
                            double dfTargetCellSize,
                            NinjaStlType eType,
                            GDALProgressFunc pfnProgress )
{
    GDALDatasetH hDS;
    GDALRasterBandH hBand;
    double adfGeoTransform[6];
    double dfXRes, dfYRes;
    int nXSize, nYSize, nBandCount;
    float *pafScanline;
    unsigned int nTriCount;
    unsigned short nAttrCount;

    int nOutXSize, nOutYSize;

    FILE *fout;

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
        fout = fopen( pszOutput, "wb" );
    }
    else
    {
        fout = fopen( pszOutput, "w" );
    }
    if( fout == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to open output file" );
        GDALClose( hDS );
        return CE_Failure;
    }
    if( dfTargetCellSize <= 0.0 )
    {
        nOutXSize = nXSize;
        nOutYSize = nYSize;
        dfXRes = adfGeoTransform[1];
        dfYRes = adfGeoTransform[5];
    }
    else
    {
        nOutXSize = nXSize * (adfGeoTransform[1] / dfTargetCellSize);
        nOutYSize = nYSize * (fabs( adfGeoTransform[5] ) / dfTargetCellSize);
        dfXRes = dfTargetCellSize;
        dfYRes = -dfTargetCellSize;
    }
    float fXOffset, fYOffset;
    fXOffset = adfGeoTransform[1] * 0.5;
    fYOffset = adfGeoTransform[5] * 0.5;

    char nil[80];
    memset( nil, '\0', 80 );

    nTriCount = (nOutXSize-1) * (nOutYSize-1) * 2; //cell centers are vertices
    nAttrCount = 0;


    pafScanline = (float*)VSIMalloc3( nXSize, nYSize, sizeof( float ) );
    if( pafScanline == NULL )
    {
        CPLError( CE_Failure, CPLE_OutOfMemory,
                  "Could not allocate buffer" );
        GDALClose( hDS );
        fclose( fout );
        return CE_Failure;
    }

    CPLErr eErr = CE_None;
    int i, j;
    StlPosition a, b, c, d;
    StlPosition v1, v2, norm;
    if( eType == NinjaStlBinary )
    {
        fwrite( nil, 1, 80, fout );
        fwrite( &nTriCount, sizeof( unsigned int ), 1, fout );
    }
    else
    {
        fprintf( fout, "solid NAME\n" );
    }
    if( pfnProgress )
    {
        pfnProgress( 0.0, NULL, NULL );
    }
    eErr = GDALRasterIO( hBand, GF_Read, 0, 0, nXSize, nYSize,
                         pafScanline, nOutXSize, nOutYSize, GDT_Float32, 0, 0 );
    if( eErr != CE_None )
    {
        CPLFree( pafScanline );
        GDALClose( hDS );
        return eErr;
    }
    for( i = 0 ; i < nOutYSize - 1; i++ )
    {
        for( j = 0; j < nOutXSize - 1; j++ )
        {
            a.x = adfGeoTransform[0] + j * dfXRes + fXOffset;
            a.y = adfGeoTransform[3] + i * dfYRes + fYOffset;
            a.z = pafScanline[j+i*nOutXSize];

            b.x = adfGeoTransform[0] + ( j + 1 ) * dfXRes + fXOffset;
            b.y = a.y;
            b.z = pafScanline[(j+i*nOutXSize) + 1];

            c.x = a.x;
            c.y = adfGeoTransform[3] + ( i + 1 ) * dfYRes + fYOffset;
            c.z = pafScanline[(j+i*nOutXSize) + nOutXSize];

            d.x = b.x;
            d.y = c.y;
            d.z = pafScanline[(j+i*nOutXSize) + nOutXSize + 1];

            v1.x = c.x - a.x;
            v1.y = c.y - a.y;
            v1.z = c.z - a.z;

            v2.x = b.x - a.x;
            v2.y = b.y - a.y;
            v2.z = b.z - a.z;

            norm = StlComputeNormal( &v1, &v2 );

            if( eType == NinjaStlBinary )
            {
                fwrite( &norm.x, sizeof( float ), 1, fout );
                fwrite( &norm.y, sizeof( float ), 1, fout );
                fwrite( &norm.z, sizeof( float ), 1, fout );
                fwrite( &b.x, sizeof( float ), 1, fout );
                fwrite( &b.y, sizeof( float ), 1, fout );
                fwrite( &b.z, sizeof( float ), 1, fout );
                fwrite( &a.x, sizeof( float ), 1, fout );
                fwrite( &a.y, sizeof( float ), 1, fout );
                fwrite( &a.z, sizeof( float ), 1, fout );
                fwrite( &c.x, sizeof( float ), 1, fout );
                fwrite( &c.y, sizeof( float ), 1, fout );
                fwrite( &c.z, sizeof( float ), 1, fout );
                fwrite( &nAttrCount, sizeof( unsigned short ), 1, fout );
            }
            else
            {
                fprintf( fout, "facet normal %e %e %e\n",
                             norm.x, norm.y, norm.z );

                fprintf( fout, "    outer loop\n" );
                fprintf( fout, "        vertex %e %e %e\n", b.x, b.y, b.z );
                fprintf( fout, "        vertex %e %e %e\n", a.x, a.y, a.z );
                fprintf( fout, "        vertex %e %e %e\n", c.x, c.y, c.z );
                fprintf( fout, "    endloop\n" );
                fprintf( fout, "endfacet\n" );
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
                fwrite( &norm.x, sizeof( float ), 1, fout );
                fwrite( &norm.y, sizeof( float ), 1, fout );
                fwrite( &norm.z, sizeof( float ), 1, fout );
                fwrite( &d.x, sizeof( float ), 1, fout );
                fwrite( &d.y, sizeof( float ), 1, fout );
                fwrite( &d.z, sizeof( float ), 1, fout );
                fwrite( &b.x, sizeof( float ), 1, fout );
                fwrite( &b.y, sizeof( float ), 1, fout );
                fwrite( &b.z, sizeof( float ), 1, fout );
                fwrite( &c.x, sizeof( float ), 1, fout );
                fwrite( &c.y, sizeof( float ), 1, fout );
                fwrite( &c.z, sizeof( float ), 1, fout );
                fwrite( &nAttrCount, sizeof( unsigned short ), 1, fout );
            }
            else
            {
                fprintf( fout, "facet normal %e %e %e\n",
                            norm.x, norm.y, norm.z );

                fprintf( fout, "    outer loop\n" );
                fprintf( fout, "        vertex %e %e %e\n", d.x, d.y, d.z );
                fprintf( fout, "        vertex %e %e %e\n", b.x, b.y, b.z );
                fprintf( fout, "        vertex %e %e %e\n", c.x, c.y, c.z );
                fprintf( fout, "    endloop\n" );
                fprintf( fout, "endfacet\n" );
            }
        }
        if( pfnProgress )
        {
            pfnProgress( double(i) / (double)nYSize, NULL, NULL );
        }
    }
    if( eType == NinjaStlAscii )
    {
        fprintf( fout, "endsolid %s\n", CPLGetBasename( pszInput ) );
    }
    if( pfnProgress )
    {
        pfnProgress( 1.0, NULL, NULL );
    }

    VSIFree( pafScanline );
    GDALClose( hDS );
    fclose( fout );

    return eErr;
}

