/******************************************************************************
 *
 * $Id: ncepNamGrib2SurfInitialization.h 
 *
 * Project:  WindNinja
 * Purpose:  NCEP NAM Surface Forecast Model Initialization derived class 
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

#ifndef NCEP_NAM_GRIB2SURF_INITIALIZATION_H
#define NCEP_NAM_GRIB2SURF_INITIALIZATION_H

#include "wxModelInitialization.h"

/**
 * Class to initialize a WindNinja run from an NCEP NAM Surface forecast file
 * in GRIB2 format. Archived forecasts for example.
 */
class ncepNamGrib2SurfInitialization : public wxModelInitialization
{
 public:

    ncepNamGrib2SurfInitialization();
    virtual ~ncepNamGrib2SurfInitialization();

    ncepNamGrib2SurfInitialization( ncepNamGrib2SurfInitialization const& A );
    ncepNamGrib2SurfInitialization& operator= ( ncepNamGrib2SurfInitialization const& m );

    virtual bool identify( std::string fileName );
    virtual std::vector<std::string> getVariableList();
    virtual std::string getForecastIdentifier();

    virtual std::string getPath();
    virtual double getGridResolution();
    virtual int getStartHour();
    virtual int getEndHour();

    virtual void checkForValidData();
    virtual double Get_Wind_Height();

 protected:
    virtual void setSurfaceGrids( WindNinjaInputs &input,
                                  AsciiGrid<double> &airGrid,
                                  AsciiGrid<double> &cloudGrid,
                                  AsciiGrid<double> &uGrid,
                                  AsciiGrid<double> &vGrid,
                                  AsciiGrid<double> &wGrid );

};



#endif //NCEP_NAM_GRIB2SURF_INITIALIZATION_H



