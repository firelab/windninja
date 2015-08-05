/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Implementation of Fetch Factory 
 * Author:   Levi Malott <lmnn3@mst.edu> 
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

#include "fetch_factory.h"

const std::string FetchFactory::US_SRTM_STR     = "us_srtm";
const std::string FetchFactory::WORLD_SRTM_STR  = "world";
const std::string FetchFactory::RELIEF_STR      = "relief";
#ifdef HAVE_GMTED
const std::string FetchFactory::WORLD_GMTED_STR = "gmted";
#endif //HAVE_GMTED




SurfaceFetch* FetchFactory::GetSurfaceFetch(FetchType type, std::string path)
{
    std::string p;
    const char* pszPath;

    if(type == US_SRTM || type == WORLD_SRTM || type == WORLD_GMTED )
    {
        p = FindDataPath("surface_data.zip");
        pszPath = CPLFormFilename("/vsizip/", p.c_str(), NULL);
        if(type == US_SRTM)
            pszPath = CPLFormFilename(pszPath, "us", ".vrt");
        else if(type == WORLD_SRTM)
            pszPath = CPLFormFilename(pszPath, "world", ".vrt");
        else if(type == WORLD_GMTED)
            pszPath = CPLFormFilename(pszPath, "gmted_2", ".vrt");
            //pszPath = CPLFormFilename(pszPath, "gmted/all075", ".vrt");
        return new GDALFetch(std::string(pszPath));
    }
#ifdef WITH_LCP_CLIENT
    else if(type == LCP)
        return new LandfireClient();
#endif
    else if(type == CUSTOM_GDAL)
        return new GDALFetch(path);
    else if( type == RELIEF )
    {
        p = FindDataPath( "relief.xml" );
        return new ReliefFetch( p );
    }
    else
        return NULL;
}

SurfaceFetch* FetchFactory::GetSurfaceFetch( std::string type, std::string path )
{
    std::string p;
    if( type == US_SRTM_STR )
    {
        return GetSurfaceFetch( US_SRTM );
    }
    else if( type == WORLD_SRTM_STR )
    {
        return GetSurfaceFetch( WORLD_SRTM );
    }
#ifdef HAVE_GMTED
    else if( type == WORLD_GMTED_STR )
    {
        return GetSurfaceFetch( WORLD_GMTED );
    }
#endif
    else if( type == RELIEF_STR )
    {
        return GetSurfaceFetch( RELIEF );
    }
    else
    {
        return GetSurfaceFetch( CUSTOM_GDAL, path );
    }
}

