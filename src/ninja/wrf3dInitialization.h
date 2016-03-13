/******************************************************************************
 *
 * $Id: wrf3dInitialization.h 
 *
 * Project:  WindNinja
 * Purpose:  WRF 3-D Surface Forecast Model Initialization derived class 
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

#ifndef WRF_3D_SURFACE_INITIALIZATION_H 
#define WRF_3D_SURFACE_INITIALIZATION_H

#include "wrfSurfInitialization.h"

/**
 * Class to initialize a WindNinja run from a WRF 3-D forecast file.
 */
class wrf3dInitialization : public wrfSurfInitialization
{
 #ifdef NOMADS_ENABLE_3D
 public:

    wrf3dInitialization();
    virtual ~wrf3dInitialization();

    wrf3dInitialization( wrf3dInitialization const& A );
    wrf3dInitialization& operator= ( wrf3dInitialization const& m );

    virtual bool identify( std::string fileName );
    virtual double getGridResolution();
    virtual std::vector<std::string> get3dVariableList();
    virtual std::string getForecastIdentifier();


 protected:

    virtual void set3dGrids( WindNinjaInputs &input, Mesh const& mesh );
    virtual void setGlobalAttributes(WindNinjaInputs &input);
    virtual void buildWxMeshes(WindNinjaInputs &input, Mesh const& mesh);
    virtual void buildWxScalarFields();
    virtual void allocate(Mesh const& mesh);
    virtual void deallocateTemp();
    
    int mapProj;
    float dx, dy;
    float cenLat, cenLon;
    float moadCenLat, standLon;
    float trueLat1, trueLat2;
    
    
    AsciiGrid<double> airGrid;
    AsciiGrid<double> cloudGrid;
    AsciiGrid<double> uGrid;
    AsciiGrid<double> vGrid;
    AsciiGrid<double> wGrid;
    AsciiGrid<double> phbGrid;  // geopotential
    AsciiGrid<double> phGrid; // perturbation geopotential
    
    wn_3dArray phbArray;  
    wn_3dArray phArray; 
    wn_3dArray airArray;
    wn_3dArray uArray;
    wn_3dArray vArray;
    wn_3dArray wArray;
    wn_3dArray cloudArray;
    
    wn_3dArray zStaggerElevationArray;
    wn_3dArray xStaggerElevationArray;
    wn_3dArray yStaggerElevationArray;
    wn_3dArray cellCenterElevationArray;
    
    
    #endif //STABILITY

};

#endif //WRF_3D_SURFACE_INITIALIZATION_H
