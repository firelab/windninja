/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Client to download weather data from nomads
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

#ifdef WITH_NOMADS_SUPPORT

#ifndef NOMADS_SUBMODELS_H_
#define NOMADS_SUBMODELS_H_

#include "nomads_wxmodel.h"

class NamWxModel : public NomadsWxModel
{

public:
    NamWxModel();
    NamWxModel( std::string osSubModelName );
    virtual ~NamWxModel();

    virtual int GetLastForecastHour();
    virtual std::vector<int> GetForecastHourSequence();
    virtual std::vector<int> GetForecastRunTimes();
    virtual std::vector<std::string> GetValidSubModelNames();
};

class RapWxModel : public NomadsWxModel
{

public:
    RapWxModel();
    RapWxModel( std::string osSubModelName );
    virtual ~RapWxModel();

    virtual int GetLastForecastHour();
    virtual std::vector<int> GetForecastHourSequence();
    virtual std::vector<int> GetForecastRunTimes();
    virtual std::vector<std::string> GetValidSubModelNames();
};

class GfsWxModel : public NomadsWxModel
{
public:
    GfsWxModel();
    GfsWxModel( std::string osSubModelName );
    virtual ~GfsWxModel();

    virtual int GetLastForecastHour();
    virtual std::vector<int> GetForecastHourSequence();
    virtual std::vector<int> GetForecastRunTimes();
    virtual std::vector<std::string> GetValidSubModelNames();
    virtual std::string FormFinalUrl( std::string osUrl, const char *pszLastFolder );
    virtual std::string GetSubModelFilter();
};

class RtmaWxModel : public NomadsWxModel
{
public:
    RtmaWxModel();
    RtmaWxModel( std::string osSubModelName );
    virtual ~RtmaWxModel();

    virtual int GetLastForecastHour();
    virtual std::vector<int> GetForecastHourSequence();
    virtual std::vector<int> GetForecastRunTimes();
    virtual std::vector<std::string> GetValidSubModelNames();
    virtual int IsDynamicTime() { return TRUE; }
};

class HiResWWxModel : public NomadsWxModel
{
public:
    HiResWWxModel();
    HiResWWxModel( std::string osSubModelName );
    virtual ~HiResWWxModel();

    virtual int GetLastForecastHour();
    virtual std::vector<int> GetForecastHourSequence();
    virtual std::vector<int> GetForecastRunTimes();
    virtual std::vector<std::string> GetValidSubModelNames();
    virtual int IsDynamicTime() { return TRUE; }
    virtual std::string GetModelFilter() { return osModelFilter; }
private:
    std::string osModelFilter;
};

#endif /* NOMADS_SUBMODELS_H_ */

#endif /* WITH_NOMADS_SUPPORT */

