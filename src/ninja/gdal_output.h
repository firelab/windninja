/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  Arbitrary output writer for any GDAL driver
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

#ifndef NINJA_GDAL_OUTPUT_H_
#define NINJA_GDAL_OUTPUT_H_

#include "gdal.h"
#include "ogr_api.h"
#include "ogr_srs_api.h"

#include "ascii_grid.h"

#define NINJA_OUTPUT_VECTOR 1 << 0
#define NINJA_OUTPUT_ARROWS 1 << 1
#define NINJA_OUTPUT_STYLED 1 << 2
#define NINJA_OUTPUT_PALLET 1 << 3

/*
** NinjaGDALOutput writes wind ninja output to an arbitrary driver.
*/
int NinjaGDALVectorOutput(const char *pszDriver,
                          const char *pszFilename,
                          int nFlags,
                          AsciiGrid<double> &spd,
                          AsciiGrid<double> &dir,
                          char **papszOptions);
#endif /* NINJA_GDAL_OUTPUT_H_ */
