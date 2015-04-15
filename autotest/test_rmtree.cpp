/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  Test rm tree
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

#include "ninja_conv.h"
#include <boost/test/unit_test.hpp>

void Touch( const char *pszPath )
{
    FILE *f = fopen( pszPath, "w" );
    fclose( f );
}

struct TmpPath
{
    TmpPath()
    {
        pszTmpPath = CPLStrdup( CPLGenerateTempFilename( "NINJA_RMTREE" ) );
        VSIMkdir( pszTmpPath, 0777 );
    }
    ~TmpPath()
    {
        CPLFree( pszTmpPath );
    }
    char *pszTmpPath;
};

BOOST_FIXTURE_TEST_SUITE( rmtree, TmpPath )

BOOST_AUTO_TEST_CASE( rmtree_1 )
{
    int rc;
    const char *pszPath;
    pszPath = CPLFormFilename( pszTmpPath, "a", NULL );
    rc = VSIMkdir( pszPath, 0777 );
    pszPath = CPLFormFilename( pszTmpPath, "b", NULL );
    rc = VSIMkdir( pszPath, 0777 );
    pszPath = CPLFormFilename( pszPath, "c", ".txt" );
    Touch( pszPath );
    rc = NinjaUnlinkTree( pszTmpPath );
    VSIStatBufL sStat;
    BOOST_CHECK( VSIStatExL( pszTmpPath, &sStat, VSI_STAT_EXISTS_FLAG ) != 0 );
}

#ifndef WIN32

/* Use symlink, not cross platform */

BOOST_AUTO_TEST_CASE( rmtree_sym_1 )
{
    int rc;
    const char *pszTarget, *pszPath, *pszSym;
    pszPath = CPLFormFilename( pszTmpPath, "a", NULL );
    pszSym = CPLFormFilename( pszPath, "s", NULL );
    rc = VSIMkdir( pszPath, 0777 );
    pszPath = CPLFormFilename( pszTmpPath, "b", NULL );
    rc = VSIMkdir( pszPath, 0777 );
    pszTarget = CPLFormFilename( pszPath, "c", ".txt" );
    Touch( pszTarget );
    symlink( pszTarget, pszSym );
    rc = NinjaUnlinkTree( pszTmpPath );
    VSIStatBufL sStat;
    BOOST_CHECK( VSIStatExL( pszTmpPath, &sStat, VSI_STAT_EXISTS_FLAG ) != 0 );
}

BOOST_AUTO_TEST_CASE( rmtree_sym_2 )
{
    int rc;
    const char *pszTarget, *pszPath, *pszSym;
    pszPath = CPLFormFilename( pszTmpPath, "a", NULL );
    pszSym = CPLFormFilename( pszPath, "s", NULL );
    rc = VSIMkdir( pszPath, 0777 );
    pszPath = CPLFormFilename( pszTmpPath, "b", NULL );
    rc = VSIMkdir( pszPath, 0777 );
    pszTarget = CPLFormFilename( pszPath, "c", ".txt" );
    Touch( pszTarget );
    symlink( pszTarget, pszSym );
    pszSym = CPLFormFilename( pszTmpPath, "s2", NULL );
    symlink( pszTarget, pszSym );
    rc = NinjaUnlinkTree( pszTmpPath );
    VSIStatBufL sStat;
    BOOST_CHECK( VSIStatExL( pszTmpPath, &sStat, VSI_STAT_EXISTS_FLAG ) != 0 );
}

BOOST_AUTO_TEST_CASE( rmtree_sym_3 )
{
    int rc;
    const char *pszTarget, *pszPath, *pszSym;
    pszPath = CPLFormFilename( pszTmpPath, "a", NULL );
    pszSym = CPLFormFilename( pszPath, "s", NULL );
    rc = VSIMkdir( pszPath, 0777 );
    pszPath = CPLFormFilename( pszTmpPath, "b", NULL );
    rc = VSIMkdir( pszPath, 0777 );
    pszTarget = CPLFormFilename( pszPath, "c", ".txt" );
    Touch( pszTarget );
    symlink( pszTarget, pszSym );
    pszSym = CPLFormFilename( pszPath, "s3", NULL );
    symlink( pszTarget, pszSym );
    pszSym = CPLFormFilename( pszTmpPath, "s2", NULL );
    symlink( pszTarget, pszSym );
    rc = NinjaUnlinkTree( pszTmpPath );
    VSIStatBufL sStat;
    BOOST_CHECK( VSIStatExL( pszTmpPath, &sStat, VSI_STAT_EXISTS_FLAG ) != 0 );
}

#endif

BOOST_AUTO_TEST_SUITE_END()

