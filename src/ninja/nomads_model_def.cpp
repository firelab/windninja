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

#include "nomads_model_def.h"

/**
 * \file nomads_submodels.h
 *
 * This file implements the various models on NOMADS.  Various grids are used
 * to describe locations and outputs, a brief summary is below.
 *
 * @note See http://www.emc.ncep.noaa.gov/mmb/namgrids/ for grid descriptions
 */

/**
 * \class NamWxModel
 * \briefNorth American Mesoscale Model support
 *
 * Implement support for NAM model output.
 */
NamWxModel::NamWxModel() : NomadsWxModel( "nam", "awphys")
{

}
NamWxModel::NamWxModel( std::string osSubModelShortName ) :
            NomadsWxModel( "nam", osSubModelShortName )
{

}

NamWxModel::~NamWxModel()
{
}

std::vector<int> NamWxModel::GetForecastRunTimes()
{
    std::vector<int> anHours;
    anHours.push_back(0);
    anHours.push_back(6);
    anHours.push_back(12);
    anHours.push_back(18);
    return anHours;
}

std::vector<int> NamWxModel::GetForecastHourSequence()
{
    std::vector<int> anHours;
    for( int i = 0; i <= 36; i++ )
    {
        anHours.push_back( i );
    }
    for( int i = 39; i <= 84; i += 3 )
    {
        anHours.push_back( i );
    }
    return anHours;
}

int NamWxModel::GetLastForecastHour()
{
    return 84;
}

std::vector<std::string> NamWxModel::GetValidSubModelNames()
{
    std::vector<std::string> oNames;
    oNames.push_back( "awphys" );
    return oNames;
}

/**
 * \brief Rapid Refresh Model support
 */
RapWxModel::RapWxModel() : NomadsWxModel()
{

}

RapWxModel::RapWxModel( std::string osSubModelShortName ) :
            NomadsWxModel( "rap", osSubModelShortName )
{
}

RapWxModel::~RapWxModel()
{
}

std::vector<int> RapWxModel::GetForecastRunTimes()
{
    std::vector<int> anHours;
    for( int i = 0; i < 24; i++ )
        anHours.push_back( i );
    return anHours;
}

std::vector<int> RapWxModel::GetForecastHourSequence()
{
    std::vector<int> anHours;
    for( int i = 0; i <= 18; i++ )
        anHours.push_back( i );
    return anHours;
}

int RapWxModel::GetLastForecastHour()
{
    return 23;
}

std::vector<std::string> RapWxModel::GetValidSubModelNames()
{
    std::vector<std::string> oNames;
    oNames.push_back( "awp130bgrbf" );
    oNames.push_back( "awp130pgrbf" );
    return oNames;
}

GfsWxModel::GfsWxModel() : NomadsWxModel()
{

}

GfsWxModel::GfsWxModel( std::string osSubModelShortName ) :
            NomadsWxModel( "gfs", osSubModelShortName )
{

}

GfsWxModel::~GfsWxModel()
{
}

std::vector<int> GfsWxModel::GetForecastRunTimes()
{
    std::vector<int> anHours;
    anHours.push_back( 0 );
    anHours.push_back( 6 );
    anHours.push_back( 12 );
    anHours.push_back( 18 );
    return anHours;
}

std::vector<int> GfsWxModel::GetForecastHourSequence()
{
    std::vector<int> anHours;
    for( int i = 0; i <= 192; i += 3 )
        anHours.push_back( i );
    for( int i = 204; i <= 368; i += 12 )
        anHours.push_back( i );
    return anHours;
}

int GfsWxModel::GetLastForecastHour()
{
    return 192;
}

std::vector<std::string> GfsWxModel::GetValidSubModelNames()
{
    std::vector<std::string> oNames;
    oNames.push_back( "awp130bgrbf" );
    oNames.push_back( "awp130pgrbf" );
    return oNames;
}

std::string GfsWxModel::FormFinalUrl( std::string osUrl, const char *pszLastFolder )
{
    std::string osNewUrl = NomadsWxModel::FormFinalUrl( osUrl, pszLastFolder );
    osNewUrl += "/master/";
    return osNewUrl;
}

std::string GfsWxModel::GetSubModelFilter()
{
    return "mastergrb2f";
}

/**
 * \brief Real Time Mesoscale Analysis (RTMA) support.
 *
 * Implement support for RTMA model output.
 *
 * RTMA description: http://nomads.ncep.noaa.gov/txt_descriptions/RTMA_doc.shtml
 *
 * We support 2.5 km rtma output for CONUS
 */
RtmaWxModel::RtmaWxModel() : NomadsWxModel( "rtma", "rtma2p5")
{

}
RtmaWxModel::RtmaWxModel( std::string osSubModelShortName ) :
            NomadsWxModel( "rtma", osSubModelShortName )
{
}

RtmaWxModel::~RtmaWxModel()
{
}

std::vector<int> RtmaWxModel::GetForecastRunTimes()
{
    std::vector<int> anHours;
    anHours.push_back( 0 );
    return anHours;
}

std::vector<int> RtmaWxModel::GetForecastHourSequence()
{
    std::vector<int> anHours;
    for( int i = 0; i <= 18; i++ )
    {
        anHours.push_back( i );
    }
    return anHours;
}

int RtmaWxModel::GetLastForecastHour()
{
    return 23;
}

std::vector<std::string> RtmaWxModel::GetValidSubModelNames()
{
    std::vector<std::string> oNames;
    oNames.push_back( "2dvaranal_ndfd" );
    return oNames;
}

/**
 * \brief 
 *
 * Implement support for HIRESW model output.
 *
 */
HiResWWxModel::HiResWWxModel() : NomadsWxModel()
{

}
HiResWWxModel::HiResWWxModel( std::string osSubModelShortName ) :
            NomadsWxModel( "hiresw", "" )
{
    char **papszTokens = CSLTokenizeString2( osSubModelShortName.c_str(), "/",
                                             CSLT_STRIPLEADSPACES |
                                             CSLT_STRIPENDSPACES );
    if( CSLCount( papszTokens ) != 2 )
    {
        this->osModelFilter = "westarw";
        this->osSubModelShortName = "awp4km";
    }
    else
    {
        CPLDebug( "NOMADS", "HiRes tokens: %s, %s", papszTokens[0],
                  papszTokens[1] );
        this->osModelFilter = papszTokens[0];
        this->osSubModelShortName = papszTokens[1];
    }
    osInventoryPath = "pub/data/nccf/com/" + this->osModelShortName + "/prod/";
    CSLDestroy( papszTokens );
}

HiResWWxModel::~HiResWWxModel()
{
}

std::vector<int> HiResWWxModel::GetForecastRunTimes()
{
    std::vector<int> anHours;
    anHours.push_back( 0 );
    anHours.push_back( 12 );
    return anHours;
}

std::vector<int> HiResWWxModel::GetForecastHourSequence()
{
    std::vector<int> anHours;
    for( int i = 0; i <= 48; i++ )
    {
        anHours.push_back( i );
    }
    return anHours;
}

int HiResWWxModel::GetLastForecastHour()
{
    return 48;
}

std::vector<std::string> HiResWWxModel::GetValidSubModelNames()
{
    std::vector<std::string> oNames;
    oNames.push_back( "2dvaranal_ndfd" );
    return oNames;
}

#endif /* WITH_NOMADS_SUPPORT */

