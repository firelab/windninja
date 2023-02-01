/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for writing volume and surface netcdf files
 * Author:   Loren Atwood <pianotocador@gmail.com>
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

#ifndef VOLNETCDF_H
#define VOLNETCDF_H

#include <string>
#include <vector>

#include "netcdf.h"
#include "gdal.h"
#include "ogr_spatialref.h"

#include "wn_3dArray.h"
#include "wn_3dScalarField.h"

#include "ninja_version.h"


// defined for netcdf c error checking commands
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}


class volNetcdf
{
public:

	volNetcdf();
    volNetcdf(wn_3dScalarField const& u, wn_3dScalarField const& v, wn_3dScalarField const& w, 
              wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
              int nCols, int nRows, int nLayers, std::string filename,  
              std::string prjString, double meshRes, 
              bool convertToTrueLatLong, double dem_xllCorner, double dem_yllCorner);
	~volNetcdf();

    bool writeVolNetcdf(wn_3dScalarField const& u, wn_3dScalarField const& v, wn_3dScalarField const& w, 
                        wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
                        int nCols, int nRows, int nLayers, std::string filename,  
                        std::string prjString, double meshRes, 
                        bool convertToTrueLatLong, double dem_xllCorner, double dem_yllCorner);
    
private:
	
    std::string prjString_latLong;
    
};


#endif	//VOLNETCDF_H
