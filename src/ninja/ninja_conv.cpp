/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Convenience functions
 * Author:   Kyle Shannon <kyle at pobox dot com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE n17 SECTION 105 
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
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
/**
 * \brief Find the root directory for the installation.
 *
 * Allocate enough space to get the path plus the relative "/../\0" for the 
 * parent directory.
 * \return full path to the directory above the 'bin' folder
 */
std::string FindNinjaRootDir()
{
    char pszFilePath[MAX_PATH];
    CPLGetExecPath(pszFilePath, MAX_PATH - 5);
    const char* pszNinjaPath;
    pszNinjaPath = CPLSPrintf("%s/../", pszFilePath);
    pszNinjaPath = CPLGetPath(pszFilePath);
    return std::string((char*)pszNinjaPath);
}

std::string FindNinjaBinDir()
{
    char pszFilePath[MAX_PATH];
    CPLGetExecPath(pszFilePath, MAX_PATH);
    const char* pszNinjaPath;
    pszNinjaPath = CPLGetPath(pszFilePath);

    return std::string((char*)pszNinjaPath);
}

std::string FindBoostDataBaseFile()
{
    const char* pszBase = "date_time_zonespec";
    const char* pszExt = "csv";
    const char* pszFilename;
    const char* pszNinjaPath;
    const char* pszNinjaSharePath;
    char pszFilePath[MAX_PATH];

    CPLGetExecPath(pszFilePath, MAX_PATH);
    pszNinjaPath = CPLGetPath(pszFilePath);

    pszNinjaSharePath = CPLProjectRelativeFilename(pszNinjaPath, 
        "../share/windninja");

    pszFilename = CPLFormFilename(CPLGetCurrentDir(), pszBase, pszExt);
    if(CPLCheckForFile((char*)pszFilename, NULL)) {
        return std::string((char*)pszFilename);
    }

    pszFilename = CPLFormFilename(pszNinjaPath, pszBase, pszExt);
    if(CPLCheckForFile((char*)pszFilename, NULL)) {
        return std::string((char*)pszFilename);
    }

    pszFilename = CPLFormFilename(pszNinjaSharePath, pszBase, pszExt);
    if(CPLCheckForFile((char*)pszFilename, NULL)) {
        return std::string((char*)pszFilename);
    }
    return std::string();
}

/**
 * \brief Find a file or folder in the WindNinja data path
 *
 * XXX: Refactored by Kyle 20130117
 *
 * For example the date_time_zonespec.csv file is location in data.  If
 * WINDNINJA_DATA is *not* defined, try to find the file relative to the bin
 * path, ie ../share/data, otherwise return WINDNINJA_DATA + filename.
 *
 * \param file file or folder to look for.
 * \return a full path to file
 */
std::string FindDataPath(std::string file)
{
    const char* pszFilename;
    const char* pszNinjaPath;
    const char* pszNinjaDataPath;
    const char* pszCurDir;
    char pszExePath[MAX_PATH];

    /* Check WINDNINJA_DATA */
    VSIStatBufL sStat;
    pszNinjaDataPath = CPLGetConfigOption( "WINDNINJA_DATA", NULL );
    if( pszNinjaDataPath != NULL )
    {
        pszFilename = CPLFormFilename( pszNinjaDataPath, file.c_str(), NULL );
        VSIStatL( pszFilename, &sStat );
        if( VSI_ISREG( sStat.st_mode ) || VSI_ISDIR( sStat.st_mode ) )
        {
            return std::string( pszFilename );
        }
    }

    /* Check 'normal' installation location */
    CPLGetExecPath( pszExePath, MAX_PATH );
    pszNinjaPath = CPLGetPath( pszExePath );

    pszNinjaDataPath = CPLProjectRelativeFilename(pszNinjaPath, 
                                                   "../share/windninja");
    pszFilename = CPLFormFilename( pszNinjaDataPath, file.c_str(), NULL );
    VSIStatL( pszFilename, &sStat );
    if( VSI_ISREG( sStat.st_mode ) || VSI_ISDIR( sStat.st_mode ) )
    {
        return std::string( pszFilename );
    }

    /* Check the current directory */
    pszCurDir = CPLGetCurrentDir();
    pszFilename = CPLFormFilename( pszCurDir, file.c_str(), NULL );
    CPLFree( (void*)pszCurDir );
    if( CPLCheckForFile( (char*)pszFilename, NULL ))
    {
        return std::string( pszFilename );
    }

    return std::string();

}

/*
** If we are older than GDAL 1.10, we don't have VSIReadDirRecursive().
** Manually define it here if that's the case.  It is used in LandfireClient
** and NomadsWxModel.
*/
//#ifdef WITH_NOMADS_SUPPORT
//#ifdef USE_MANUAL_VSIREAD_DIR_RECURSIVE
char **NinjaVSIReadDirRecursive( const char *pszPathIn )
{
    CPLStringList oFiles = NULL;
    char **papszFiles = NULL;
    VSIStatBufL psStatBuf;
    CPLString osTemp1, osTemp2;
    int i = 0;
    int nCount = -1;

    std::vector<VSIReadDirRecursiveTask> aoStack;
    char* pszPath = CPLStrdup(pszPathIn);
    char* pszDisplayedPath = NULL;

    while(TRUE)
    {
        if( nCount < 0 )
        {
            // get listing
            papszFiles = VSIReadDir( pszPath );

            // get files and directories inside listing
            nCount = papszFiles ? CSLCount( papszFiles ) : 0;
            i = 0;
        }

        for ( ; i < nCount; i++ )
        {
			if( EQUAL( ".", papszFiles[i] ) || EQUAL( "..", papszFiles[i] ) )
			{
				continue;
			}
            // build complete file name for stat
            osTemp1.clear();
            osTemp1.append( pszPath );
            osTemp1.append( "/" );
            osTemp1.append( papszFiles[i] );

            // if is file, add it
            if ( VSIStatL( osTemp1.c_str(), &psStatBuf ) != 0 )
                continue;

            if( VSI_ISREG( psStatBuf.st_mode ) )
            {
                if( pszDisplayedPath )
                {
                    osTemp1.clear();
                    osTemp1.append( pszDisplayedPath );
                    osTemp1.append( "/" );
                    osTemp1.append( papszFiles[i] );
                    oFiles.AddString( osTemp1 );
                }
                else
                    oFiles.AddString( papszFiles[i] );
            }
            else if ( VSI_ISDIR( psStatBuf.st_mode ) )
            {
                // add directory entry
                osTemp2.clear();
                if( pszDisplayedPath )
                {
                    osTemp2.append( pszDisplayedPath );
                    osTemp2.append( "/" );
                }
                osTemp2.append( papszFiles[i] );
                osTemp2.append( "/" );
                oFiles.AddString( osTemp2.c_str() );

                VSIReadDirRecursiveTask sTask;
                sTask.papszFiles = papszFiles;
                sTask.nCount = nCount;
                sTask.i = i;
                sTask.pszPath = CPLStrdup(pszPath);
                sTask.pszDisplayedPath = pszDisplayedPath ? CPLStrdup(pszDisplayedPath) : NULL;
                aoStack.push_back(sTask);

                CPLFree(pszPath);
                pszPath = CPLStrdup( osTemp1.c_str() );

                char* pszDisplayedPathNew;
                if( pszDisplayedPath )
                    pszDisplayedPathNew = CPLStrdup( CPLSPrintf("%s/%s", pszDisplayedPath, papszFiles[i]) );
                else
                    pszDisplayedPathNew = CPLStrdup( papszFiles[i] );
                CPLFree(pszDisplayedPath);
                pszDisplayedPath = pszDisplayedPathNew;

                i = 0;
                papszFiles = NULL;
                nCount = -1;

                break;
            }
        }

        if( nCount >= 0 )
        {
            CSLDestroy( papszFiles );

            if( aoStack.size() )
            {
                int iLast = (int)aoStack.size() - 1;
                CPLFree(pszPath);
                CPLFree(pszDisplayedPath);
                nCount = aoStack[iLast].nCount;
                papszFiles = aoStack[iLast].papszFiles;
                i = aoStack[iLast].i + 1;
                pszPath = aoStack[iLast].pszPath;
                pszDisplayedPath = aoStack[iLast].pszDisplayedPath;

                aoStack.resize(iLast);
            }
            else
                break;
        }
    }

    CPLFree(pszPath);
    CPLFree(pszDisplayedPath);

    return oFiles.StealList();
}
//#endif /* USE_MANUAL_VSIREAD_DIR_RECURSIVE */
//#endif /* WITH_NOMADS_SUPPORT */

static int NinjaUnlinkTreeRecurse( const char *pszPath )
{
    int i, n, rc;
    char **papszDirList = NULL;
    char *pszJoinedPath, *pszTmp;
    VSIStatBufL sStat;

    papszDirList = VSIReadDir( pszPath );
    i = 0;
    n = CSLCount( papszDirList );
    for( i=0; i<n; i++ )
    {
        pszTmp = papszDirList[i];
        SKIP_DOT_AND_DOTDOT( pszTmp );
        pszJoinedPath = CPLStrdup( CPLFormFilename( pszPath, pszTmp, NULL ) );
        VSIStatExL( pszJoinedPath, &sStat, 0 );
        if( VSI_ISDIR( sStat.st_mode ) )
        {
            NinjaUnlinkTreeRecurse( pszJoinedPath );
            VSIUnlink( pszJoinedPath );
            VSIRmdir( pszJoinedPath );
        }
        else
        {
            VSIUnlink( pszJoinedPath);
        }
        CPLFree( pszJoinedPath );
    }
    CSLDestroy( papszDirList );
    return 0;
}

int NinjaUnlinkTree( const char *pszPath )
{
    int rc = NinjaUnlinkTreeRecurse( pszPath );
    VSIRmdir( pszPath );
    return rc;
}

void * NinjaMalloc( unsigned int nSize )
{
    return malloc( nSize );
}

void NinjaFree( void *hData )
{
    free( hData );
}

std::string NinjaRemoveSpaces( std::string s )
{
    s.erase( std::remove_if( s.begin(), s.end(), ::isspace ), s.end() );

    return s;
}

