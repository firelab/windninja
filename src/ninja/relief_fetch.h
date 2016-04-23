/******************************************************************************
 * relief_fetch.h 
 *
 * Project:  WindNinja
 * Purpose:  Downloads relief maps given a bounding box 
 * Author:   Levi Malott <levi.malott@mst.edu>
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

#ifndef RELIEFFETCH_H
#define RELIEFFETCH_H

#include "surface_fetch.h"
#include <string>

/**
 * @brief Downloads relief maps from a tile server
 */
class ReliefFetch : public SurfaceFetch
{
public:
    /**
     * Constructor
     */
    ReliefFetch();
    ReliefFetch( std::string path );
    virtual SURF_FETCH_E FetchBoundingBox(double *bbox, double resolution,
                                          const char *filename, char ** options );


    SURF_FETCH_E makeReliefOf( std::string infile, std::string outfile, int nXSize, int nYSize );
    /**
     * Destructor
     */
    virtual ~ReliefFetch();

protected:
    SURF_FETCH_E Initialize();

};



#endif // RELIEFFETCH_H
