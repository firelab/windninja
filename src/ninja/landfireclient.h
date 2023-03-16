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

/*-----------------------------------------------------------------------------
 *  REST API", string templates
 *-----------------------------------------------------------------------------*/
#define LF_REQUEST_TEMPLATE "https://lfps.usgs.gov/" \
                            "arcgis/rest/services/LandfireProductService/" \
                            "GPServer/LandfireProductService/submitJob?" \
                            "Output_Projection=%d&Layer_List=%s&" \
                            "Area_of_Interest=%lf%%20%lf%%20%lf%%20%lf" \
                            "&f=pjson"

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

    CPLHTTPResult *m_poResult;
    std::string m_JobId;
};

#endif /* LANDFIRE_CLIENT_H_ */
#endif /* WITH_LCP_CLIENT */
