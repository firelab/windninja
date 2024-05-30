/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Client to download SRTM files
 * Author:   Natalie Wagenbrenner <nwagenbrenner@gmail.com>
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
#ifndef SRTM_CLIENT_H_
#define SRTM_CLIENT_H_

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

#define API_KEY "b939a683596989f37b78a930e1199a1c"

#define SRTM_REQUEST_TEMPLATE "https://portal.opentopography.org/" \
                              "API/globaldem?demtype=SRTMGL1&" \
                              "south=%lf&north=%lf&west=%lf&east=%lf&outputFormat=GTiff&API_Key=%s"

/*-----------------------------------------------------------------------------
 *  LandFireClient Class Definition
 *-----------------------------------------------------------------------------*/
class SRTMClient : public SurfaceFetch
{
public:
    SRTMClient();
    ~SRTMClient();
    virtual SURF_FETCH_E FetchBoundingBox(double *bbox, double resolution,
                                          const char *filename, char **options);
private:
    SRTMClient( SRTMClient &oOther ) { (void)oOther; }

    CPLHTTPResult *psResult;
    std::string m_JobId;
};

#endif /* SRTM_CLIENT_H_ */
