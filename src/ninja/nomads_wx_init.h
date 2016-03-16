/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Nomads weather model initialization
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

#ifndef NOMADS_WX_MODEL_H_
#define NOMADS_WX_MODEL_H_

#include "wxModelInitialization.h"

#include "nomads.h"

class NomadsWxModel : public wxModelInitialization
{

public:
    NomadsWxModel();
    NomadsWxModel( std::string filename );
    NomadsWxModel( const char *pszModelKey );
    virtual ~NomadsWxModel();

    virtual std::string fetchForecast( std::string demFile, int nHours );

    virtual std::vector<blt::local_date_time>
        getTimeList(const char *pszVariable, blt::time_zone_ptr timeZonePtr);

    virtual void set3dGrids( WindNinjaInputs &input, Mesh const& mesh );
    virtual bool identify( std::string fileName );
    const char ** FindModelKey( const char *pszFilename );
    virtual std::vector<std::string> getVariableList();
    virtual std::string getForecastIdentifier();
    virtual std::string getForecastReadable( const char bySwapWithSpace=' ' );
    virtual double getGridResolution();
    virtual int getStartHour();
    virtual int getEndHour();
    virtual void checkForValidData();
    virtual double Get_Wind_Height() { return 10; }
    virtual void setSurfaceGrids( WindNinjaInputs &input,
                                  AsciiGrid<double> &airGrid,
                                  AsciiGrid<double> &cloudGrid,
                                  AsciiGrid<double> &uGrid,
                                  AsciiGrid<double> &vGrid,
                                  AsciiGrid<double> &wGrid );

private:
    const char *pszKey;
    const char **ppszModelData;

    int InitializeForecastTimes();
    const char * NomadsFindForecast( const char *pszFilePath, time_t nTime );

    nomads_utc *u;

    wn_3dArray oArray;
    Mesh wxMesh;
    wn_3dScalarField *wxFields[4];
    wn_3dScalarField *fields[4];

    int CheckFileName( const char *pszFile, const char *pszFormat );
    int ClipNoData( GDALRasterBandH hBand, double dfNoData, int *pnRowsToCull,
                    int *pnColsToCull );
    GDALRasterBandH FindBand( GDALDatasetH hDS, const char *pszVar,
                              const char *pszHeight );


};

#endif /* NOMADS_WX_MODEL_H_ */

