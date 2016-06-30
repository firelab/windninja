/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Surface Fetch factory class for instantiating derived classes 
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

#ifndef FETCH_FACTORY_H
#define FETCH_FACTORY_H

#include "nomads_wx_init.h"
#include "surface_fetch.h"
#include "gdal_fetch.h"
#include "landfireclient.h"
#include "relief_fetch.h"
#include <string>

class FetchFactory
{
    public:
        enum FetchType
            {
                US_SRTM,
                WORLD_SRTM,
                WORLD_GMTED,
#ifdef WITH_LCP_CLIENT
                LCP,
#endif
                CUSTOM_GDAL,
                RELIEF
            };
        static const std::string US_SRTM_STR;
        static const std::string WORLD_SRTM_STR;
        static const std::string RELIEF_STR;
#ifdef HAVE_GMTED
        static const std::string WORLD_GMTED_STR;
#endif //HAVE_GMTED

        static SurfaceFetch* GetSurfaceFetch( FetchType, std::string path="" );
        static SurfaceFetch* GetSurfaceFetch( std::string type, std::string path="" );

};

#endif //FETCH_FACTORY_H
