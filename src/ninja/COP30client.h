/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Client to download COP30 files
 * Author:   Sathwik Reddy Chandiri <sathwikreddy56@gmail.com>
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
#ifndef COP30_CLIENT_H_
#define COP30_CLIENT_H_

#include <string>
#include <cstdio>

#include "cpl_http.h"

#include "surface_fetch.h"

/*-----------------------------------------------------------------------------
 *  REST API", string templates
 *-----------------------------------------------------------------------------*/

#define COP30_REQUEST_TEMPLATE "https://portal.opentopography.org/" \
                              "API/globaldem?demtype=COP30&" \
                              "south=%lf&north=%lf&west=%lf&east=%lf&outputFormat=GTiff&API_Key=%s"

/*-----------------------------------------------------------------------------
 *  LandFireClient Class Definition
 *-----------------------------------------------------------------------------*/
class COP30Client : public SurfaceFetch
{
public:
    COP30Client();
    ~COP30Client();
    virtual SURF_FETCH_E FetchBoundingBox(double *bbox, double resolution,
                                          const char *filename, char **options);
private:
    COP30Client( COP30Client &oOther ) { (void)oOther; }

    CPLHTTPResult *psResult;
    std::string m_JobId;
    const char* APIKey;
};

#endif /* COP30_CLIENT_H_ */
