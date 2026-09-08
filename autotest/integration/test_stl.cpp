/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja
 * Purpose:  Test AsciiGrid buffer
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

#include <boost/test/unit_test.hpp>

#include "stl_create.h"

BOOST_AUTO_TEST_SUITE( stl )

BOOST_AUTO_TEST_CASE( stl_1 )
{
    GDALAllRegister();
    int rc;
    const char *pszPath = CPLGetConfigOption( "WINDNINJA_DATA", NULL );
    BOOST_REQUIRE( pszPath );
    const char *pszFilename = CPLFormFilename( pszPath, "mackay", ".tif" );
    const char *pszTmpFile = CPLFormFilename( NULL, CPLGenerateTempFilename( NULL ), ".stl" );
    rc = NinjaElevationToStl( pszFilename, pszTmpFile, 1, -1.0, NinjaStlBinary, 0.0, NULL );
    BOOST_CHECK( !rc );
    if( CPLCheckForFile( (char*)pszTmpFile, NULL ) )
        VSIUnlink( pszTmpFile );
}

BOOST_AUTO_TEST_CASE( stl_2 )
{
    GDALAllRegister();
    int rc;
    const char *pszPath = CPLGetConfigOption( "WINDNINJA_DATA", NULL );
    BOOST_REQUIRE( pszPath );
    const char *pszFilename = CPLFormFilename( pszPath, "mackay", ".tif" );
    const char *pszTmpFile = CPLFormFilename( NULL, CPLGenerateTempFilename( NULL ), ".stl" );
    rc = NinjaElevationToStl( pszFilename, pszTmpFile, 1, -1.0, NinjaStlBinary, 0.0, NULL );
    BOOST_CHECK( !rc );
    VSILFILE *fin = VSIFOpenL( pszTmpFile, "rb" );
    BOOST_REQUIRE( fin );
    float adfTri[12];
    double dfEdge = 0.0;
    int nTris = 0;
    VSIFSeekL( fin, 80, SEEK_SET );
    VSIFReadL( &nTris, sizeof( int ), 1, fin );
    VSIFReadL( adfTri, sizeof( adfTri ), 1, fin );

    // x coords are 3, 6, and 9
    dfEdge = fabs( adfTri[3] - adfTri[6] );
    if( fabs( adfTri[6] - adfTri[9] ) < dfEdge && adfTri[6] != adfTri[9] )
    {
        dfEdge = fabs( adfTri[6] - adfTri[9] );
    }
    BOOST_CHECK( CPLIsEqual( dfEdge, 30.0 ) );
    VSIFCloseL( fin );

    if( CPLCheckForFile( (char*)pszTmpFile, NULL ) )
        VSIUnlink( "test.stl" );

    pszFilename = CPLFormFilename( pszPath, "mackay", ".tif" );
    rc = NinjaElevationToStl( pszFilename, pszTmpFile, 1, 90.0, NinjaStlBinary, 0.0, NULL );
    BOOST_CHECK( !rc );
    fin = VSIFOpenL( pszTmpFile, "rb" );
    BOOST_REQUIRE( fin );
    float adfCoarseTri[12];
    double dfCoarseEdge = 0.0;
    int nCoarseTris = 0;
    VSIFSeekL( fin, 80, SEEK_SET );
    VSIFReadL( &nCoarseTris, sizeof( int ), 1, fin );
    VSIFReadL( adfCoarseTri, sizeof( adfCoarseTri ), 1, fin );

    // x coords are 3, 6, and 9
    dfCoarseEdge = fabs( adfCoarseTri[3] - adfCoarseTri[6] );
    if( fabs( adfCoarseTri[6] - adfCoarseTri[9] ) < dfCoarseEdge && adfTri[6] != adfTri[9] )
    {
        dfCoarseEdge = fabs( adfCoarseTri[6] - adfCoarseTri[9] );
    }

    BOOST_CHECK( CPLIsEqual( dfCoarseEdge, 90.0 ) );
    BOOST_CHECK( nTris > nCoarseTris );
    BOOST_CHECK( fabs( adfCoarseTri[3] - adfTri[3] ) <= 90.0 );
    VSIFCloseL( fin );

    if( CPLCheckForFile( (char*)pszTmpFile, NULL ) )
        VSIUnlink( "test.stl" );
}


BOOST_AUTO_TEST_SUITE_END()

