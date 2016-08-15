/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Convenience functions for general use
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

#ifndef NINJA_CONV_H
#define NINJA_CONV_H

#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>

#include "gdal.h"
#include "ogr_api.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "cpl_vsi.h"

#ifndef SKIP_DOT_AND_DOTDOT
#define SKIP_DOT_AND_DOTDOT(a) if(EQUAL(a,"..")||EQUAL(a,".")) continue
#endif

#ifndef MAX_PATH
#define MAX_PATH 8192
#endif

typedef struct
{
    char **papszFiles;
    int nCount;
    int i;
    char* pszPath;
    char* pszDisplayedPath;
}  VSIReadDirRecursiveTask;

std::string FindNinjaRootDir();
std::string FindNinjaBinDir();
//std::string FindBoostDataBaseFile();
std::string FindDataPath(std::string path);

//#ifdef WITH_NOMADS_SUPPORT
//#ifdef GDAL_COMPUTE_VERSION
    //#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(1,10,0)
        //char **VSIReadDirRecursive( const char *pszPathIn );
        //#define USE_MANUAL_VSIREAD_DIR_RECURSIVE 1
    //#endif /* GDAL_VERSION_NUM >= */
//#else
    //char **VSIReadDirRecursive( const char *pszPathIn );
    //#define USE_MANUAL_VSIREAD_DIR_RECURSIVE 1
//#endif /* GDAL_COMPUTE_VERSION */
//#endif /* WITH_NOMADS_SUPPORT */
char **NinjaVSIReadDirRecursive( const char *pszPathIn );

int NinjaUnlinkTree( const char *pszPath );

void NinjaMalloc( void *hData );
void NinjaFree( void *hData );
std::string NinjaRemoveSpaces( std::string s);

#endif /* NINJA_CONV_H */

