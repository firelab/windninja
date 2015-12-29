/******************************************************************************
 *
 * $Id: wxModelInitialization.h 2506 2013-06-20 16:24:32Z nwagenbrenner $
 *
 * Project:  WindNinja
 * Purpose:  NCEP NDFD Initilization Weather Model derived class
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

#ifndef NCEP_NDFD_INITILIZATION_H
#define NCEP_NDFD_INITILIZATION_H 

#include "wxModelInitialization.h"

/**
 * Class to initialize a WindNinja run from an NCEP NDFD forecast file.
 */
class ncepNdfdInitialization : public wxModelInitialization
{
 public:

    ncepNdfdInitialization();
    virtual ~ncepNdfdInitialization();

    ncepNdfdInitialization( ncepNdfdInitialization const& A );
    ncepNdfdInitialization& operator= ( ncepNdfdInitialization const& m );

    virtual bool identify( std::string fileName );
    virtual std::vector<std::string> getVariableList();
    virtual std::string getForecastIdentifier();
    virtual std::vector<blt::local_date_time> getTimeList(blt::time_zone_ptr timeZonePtr);

    virtual double getGridResolution();
    virtual int getStartHour();
    virtual int getEndHour();

    virtual void checkForValidData();
    virtual double Get_Wind_Height();

 protected:
    enum eTempType{
        max,
        min
    };

    virtual std::vector<blt::local_date_time> getTempTimeList( eTempType type, blt::time_zone_ptr timeZonePtr );
    virtual void setSurfaceGrids( WindNinjaInputs &input,
                                  AsciiGrid<double> &airGrid,
                                  AsciiGrid<double> &cloudGrid,
                                  AsciiGrid<double> &uGrid,
                                  AsciiGrid<double> &vGrid,
                                  AsciiGrid<double> &wGrid );

};

#endif //NCEP_NDFD_INITILIZATION_H
