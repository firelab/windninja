/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  utility functions to store and output 3D (u,v,w,h) wind data
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

#ifndef UVWH_OUTPUT
#define UVWH_OUTPUT

#include <string>
#include "gdal_util.h"

//--- the data model for huvw output
GDALDataset* createHuvwDS (const char* filename, const char* descr, const char* prjString, int nCols, int nRows, double xllCorner, double yllCorner, double cellSize);
void setHuvwScanlines(GDALDataset* pDS, int rowIdx, int nCols, float* hRow, float* uRow, float* vRow, float* wRow, float* spdRow);

/////////////// REMOVE
//--- the output generators
void writeHuvwJsonVectors (GDALDataset* pDS, std::string& fname);
void writeHuvwCsvGrid (GDALDataset* pDS, std::string& fname, bool isHuvw0=false);
void writeHuvwCsvVectors (GDALDataset* pDS, std::string& fname, bool isHuvw0=false);
///////////////

#endif //UVWH_OUTPUT