/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Functions for creating an stl from a gdal dataset
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

#ifndef NINJA_STL_CONVERT_H_
#define NINJA_STL_CONVERT_H_

#include "cpl_conv.h"
#ifdef GDAL_COMPUTE_VERSION
#include "cpl_progress.h"
#endif
#include "cpl_vsi.h"
#include "gdal.h"

typedef struct _StlPosition
{
    float x;
    float y;
    float z;
} StlPosition;


typedef enum
{
    NinjaStlAscii,
    NinjaStlBinary
} NinjaStlType;

//StlPosition ComputeNormal( StlPosition, S

CPLErr NinjaElevationToStl( const char *pszInput,
                            const char *pszOutput,
                            int nBand,
                            double dfTargetCellSize,
                            NinjaStlType eType,
                            double dfOffset,
                            GDALProgressFunc pfnProgress );

#endif /* NINJA_STL_CONVERT_H_ */

