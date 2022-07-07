/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Client to download landscape files
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
#ifdef WITH_LCP_CLIENT

#ifndef LANDFIRE_CLIENT_H_
#define LANDFIRE_CLIENT_H_

#include <string>
#include <cstdio>
#if __cplusplus >= 201103
#include <regex>
#endif

#include "cpl_port.h"
#include "cpl_conv.h"
#include "cpl_http.h"
#include "cpl_string.h"
#include "cpl_minixml.h"
#include "cpl_multiproc.h"


#include "surface_fetch.h"
#include "ninja_conv.h"


/*
** Codes for landfire
**static const char *apszFileList[][] = {
**    {"F1J17HZ", "ak_100", "FBFM40"},
**    {"F1I16HZ", "ak_100", "FBFM13"},
**    {"LCP07HZ", "us_100", "FBFM13"},
**    {"LC409HZ", "us_100", "FBFM40"},
**    {"F3V18HZ", "us_105", "FBFM13"},
**    {"F3W19HZ", "us_105", "FBFM40"},
**    {"F4V20HZ", "us_110", "FBFM13"},
**    {"F4W21HZ", "us_110", "FBFM40"},
**    {"F3722HZ", "hi_105", "FBFM13"},
**    {"F3823HZ", "hi_105", "FBFM40"},
**    {"F4724HZ", "hi_110", "FBFM13"},
**    {"F4825HZ", "hi_110", "FBFM40"},
**    {"F7B28HZ", "ak_110", "FBFM13"},
**    {"F7C29HZ", "ak_110", "FBFM40"},
**    {"F6B26HZ", "ak_105", "FBFM13"},
**    {"F6C27HZ", "ak_105", "FBFM40"},
**    {"F8V30HZ", "us_120", "FBFM13"},
**    {"F8W31HZ", "us_120", "FBFM40"}};
*/

/*-----------------------------------------------------------------------------
 *  REST API", string templates
 *-----------------------------------------------------------------------------*/
#define LF_REQUEST_TEMPLATE "https://lfps.usgs.gov/" \
                            "arcgis/rest/services/LandfireProductService/" \
                            "GPServer/LandfireProductService/submitJob?" \
                            "Layer_List=%s&Area_of_Interest=%lf%%20%lf%%20%lf%%20%lf" \
                            "&f=pjson"

#define LF_REQUEST_RETURN_TEMPLATE  "\n\n\n{\"REQUEST_SERVICE_RESPONSE\":" \
                                    "{\"PIECE\":[{\"THUMBNAIL_URL\":\"none\"," \
                                    "\"DOWNLOAD_URL\":\"%[^'\"']\"}]," \
                                    "\"STATUS\":true}}\n"

#define LF_INIT_RESPONSE_TEMPLATE "<ns:initiateDownloadResponse xmlns:ns=\"http://edc/usgs/gov/xsd\">" \
                                  "<ns:return>%[^'<']</ns:return>" \
                                  "</ns:initiateDownloadResponse>"


#define LF_DOWNLOAD_JOB_TEMPLATE "https://lfps.usgs.gov/arcgis/rest/directories/" \
                                  "arcgisjobs/landfireproductservice_gpserver/%s/scratch/%s.zip"

#define LF_GET_STATUS_TEMPLATE "https://lfps.usgs.gov/arcgis/rest/services/LandfireProductService/" \
                               "GPServer/LandfireProductService/jobs/%s/?f=pjson"
                               

#define LF_DEFAULT_SRS_TOKENS     "&prj=102039,&prj=0"

/* 
 * ===  MACRO  ======================================================================
 *         Name:  checkHttpResult
 *  Description:  Determines if the member attribute m_poResult contains useful information
 *                after a call to CPLHTTPFetch. This method is inlined so checkHttpResult
 *                can cause the calling function scope to exit. This is useful since
 *                this check is commonly used in FetchBoundingBox, but we want
 *                FetchBoundingBox to return an error if we do not get an HTTP result.
 * =====================================================================================
 */
#define CHECK_HTTP_RESULT( error_msg )                      \
    if( m_poResult == NULL || m_poResult->nStatus != 0 )    \
    {                                                       \
        CPLError( CE_Warning, CPLE_AppDefined, error_msg ); \
        CPLHTTPDestroyResult( m_poResult );                 \
        return SURF_FETCH_E_IO_ERR;                         \
    }


/*-----------------------------------------------------------------------------
 *  LandFireClient Class Definition
 *-----------------------------------------------------------------------------*/
class LandfireClient : public SurfaceFetch
{
public:
    LandfireClient();
    ~LandfireClient();
    virtual SURF_FETCH_E FetchBoundingBox(double *bbox, double resolution,
                                          const char *filename, char **options);
private:
    LandfireClient( LandfireClient &oOther ) { (void)oOther; }

    const char * ReplaceSRS( int nEpsgCode, const char *pszUrl );

    CPLHTTPResult *m_poResult;
    std::string m_JobId;
};

#endif /* LANDFIRE_CLIENT_H_ */
#endif /* WITH_LCP_CLIENT */
