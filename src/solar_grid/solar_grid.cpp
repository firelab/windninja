/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Application for creating a solar intensity grid
 * Author:   Jason Forthofer <jaforthofer@fs.fed.us>
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

#include "cpl_port.h"
#include "ogr_srs_api.h"

#include "Elevation.h"
#include "ascii_grid.h"
#include "solar.h"
#include "Shade.h"
#include "Aspect.h"
#include "Slope.h"
#include "ninja_conv.h"
#include "ninja_init.h"
#include "boost/date_time/local_time/local_time.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp" //no i/o just types
#include "boost/date_time/gregorian/gregorian_types.hpp"    //no i/o just types

void Usage(const char *pszError)
{
    printf("solar_grid [--perc-cloud-cover percent] [--minute minute]\n"
           "           [--hour hour] [--day day] [--month month]\n"
#ifdef _OPENMP
           "           [--year year] [--time-zone zone] [--num-threads n]\n"
#else
           "           [--year year] [--time-zone zone]\n"
#endif
           "           [--output-cell-size size]\n"
           "           input_file output_file\n"
           "\n"
           "Defaults:\n"
           "    --perc-cloud-cover 0\n");
    if(pszError)
    {
        fprintf(stderr, "%s\n", pszError);
    }
    exit(1);
}

int CheckBounds(int value, int low, int high)
{
    return (value < low || value > high) ? FALSE : TRUE;
}

int main(int argc, char *argv[])
{
    NinjaInitialize();
    
    double startTotal, endTotal, startShdAspSlp, endShdAspSlp, startRad, endRad;
    
#ifdef _OPENMP
    startTotal = omp_get_wtime();
#endif

    int nPercCloudCover = 0;
    int nMinute = -1;
    int nHour = -1;
    int nDay = -1;
    int nMonth = -1;
    int nYear = -1;
    const char *pszTimeZone = NULL;
    int nNumThreads = 1;
    double dfCellSize = -1;
    const char *pszInputFile = NULL;
    const char *pszOutputFile = NULL;

    int i = 1;
    while(i < argc)
    {
        if(EQUAL(argv[i], "--perc-cloud-cover") || EQUAL(argv[i], "--p"))
        {
            nPercCloudCover = atoi(argv[++i]);
        }
        else if(EQUAL(argv[i], "--minute") || EQUAL(argv[i], "--m"))
        {
            nMinute = atoi(argv[++i]);
        }
        else if(EQUAL(argv[i], "--hour") || EQUAL(argv[i], "--u"))
        {
            nHour = atoi(argv[++i]);
        }
        else if(EQUAL(argv[i], "--day") || EQUAL(argv[i], "--d"))
        {
            nDay = atoi(argv[++i]);
        }
        else if(EQUAL(argv[i], "--month") || EQUAL(argv[i], "--o"))
        {
            nMonth = atoi(argv[++i]);
        }
        else if(EQUAL(argv[i], "--year") || EQUAL(argv[i], "--y"))
        {
            nYear = atoi(argv[++i]);
        }
        else if(EQUAL(argv[i], "--time-zone") || EQUAL(argv[i], "--z"))
        {
            pszTimeZone = argv[++i];
        }
        else if(EQUAL(argv[i], "--output-cell-size") || EQUAL(argv[i], "--c"))
        {
            dfCellSize = atof(argv[++i]);
        }
        else if(EQUAL(argv[i], "--num-threads") || EQUAL(argv[i], "--n"))
        {
            nNumThreads = atoi(argv[++i]);
        }
        else if(EQUAL(argv[i], "--help") || EQUAL(argv[i], "--h"))
        {
            Usage(NULL);
        }
        else if(pszInputFile == NULL)
        {
            pszInputFile = argv[i];
        }
        else if(pszOutputFile == NULL)
        {
            pszOutputFile = argv[i];
        }
        else
        {
            Usage(NULL);
        }
        i++;
    }
    if(pszInputFile == NULL || pszOutputFile == NULL)
    {
        Usage("Please Enter a valid input and output file");
    }
    if(!CheckBounds(nMinute, 0, 59))
        Usage("Invalid value for minute");
    else if(!CheckBounds(nHour, 0, 23))
        Usage("Invalid value for hour");
    else if(!CheckBounds(nDay, 1, 31))
        Usage("Invalid value for day");
    else if(!CheckBounds(nMonth, 1, 12))
        Usage("Invalid value for month");
    else if(!CheckBounds(nYear, 0, 5000))
        Usage("Invalid value for year");
    if(pszTimeZone == NULL)
    {
        Usage("Please Enter a valid time zone");
    }

    double aspect_temp = 0, slope_temp = 0;
    double sinPsi;
    double a1 = 990.0;
    double a2 = -30.0;
    double b1 = -0.75;
    double b2 = 3.4;
    int j;
    i = 0;

    //Read in elevation
    Elevation elev;
    elev.GDALReadGrid(pszInputFile, 1);
    if(dfCellSize > 0)
        elev.resample_Grid_in_place(dfCellSize, AsciiGrid<double>::order1);

    //Compute lat/lon of domain center
    double latitude, longitude;
    elev.get_gridCenter(&longitude, &latitude);
    OGRSpatialReference oSrcSRS, oDstSRS;
    OGRCoordinateTransformation *poCT;
    const char* pszWKT = VSIStrdup(elev.prjString.c_str());
    const char* startOfMem = pszWKT;
    oSrcSRS.importFromWkt((char**)&pszWKT);
    oDstSRS.importFromEPSG(4326);
    VSIFree((void*)startOfMem);

    poCT = OGRCreateCoordinateTransformation(&oSrcSRS, &oDstSRS);
    poCT->Transform(1, &longitude, &latitude);

    boost::local_time::time_zone_ptr ninjaTimeZone;

    ninjaTimeZone = globaTimeZoneDB.time_zone_from_region(pszTimeZone);
    if(ninjaTimeZone == NULL)
    {
        ostringstream os;
        os << "The time zone string: " << pszTimeZone << " does not match any in "
                << "the time zone database file: date_time_zonespec.csv.";
        throw std::runtime_error(os.str());
    }

    boost::local_time::local_date_time ninjaTime = boost::local_time::local_date_time(boost::gregorian::date(nYear, nMonth, nDay), boost::posix_time::time_duration(nHour,nMinute,0,0), ninjaTimeZone, boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
    if(ninjaTime.is_not_a_date_time())
    {
        throw std::runtime_error("Time could not be properly set in "
                                 "WindNinjaInputs::set_date_time().");
    }
    Solar solar(ninjaTime, latitude, longitude, aspect_temp, slope_temp);

#ifdef _OPENMP
    omp_set_num_threads(nNumThreads);
#endif

#ifdef _OPENMP
    startShdAspSlp = omp_get_wtime();
#endif
    
    //make aspect, slope, and shade grids
    Aspect asp(&elev,nNumThreads);
    Slope slp(&elev,nNumThreads);
    Shade shd(&elev, solar.get_theta(), solar.get_phi(), nNumThreads);
    AsciiGrid<double> CloudCover((AsciiGrid<double>&)elev);
    CloudCover = (double)nPercCloudCover / 100.0;
    AsciiGrid<double> solar_grid(elev);
    
#ifdef _OPENMP
    endShdAspSlp = omp_get_wtime();
#endif
    
#ifdef _OPENMP
    startRad = omp_get_wtime();
#endif

    //Compute incident solar radiation
    #pragma omp parallel for default(none) private(i,j,sinPsi) firstprivate(solar) shared(a1, a2, b1, b2, elev, asp, CloudCover, slp, shd, solar_grid)
    for(i = 0; i < elev.get_nRows(); i++)
    {
        for(j = 0; j < elev.get_nCols(); j++)
        {

            solar.set_aspect(asp(i,j));
            solar.set_slope(slp(i,j));
            solar.call_solPos();

            if(shd(i,j) == true)
                sinPsi = 0.0;
            else
                sinPsi = solar.get_solarIntensity() / 1353.0;
            solar_grid(i,j) = (a1 * sinPsi + a2) * 
                              (1.0 + b1 * std::pow(CloudCover(i,j), b2));
        }
    }
    
#ifdef _OPENMP
    endRad = omp_get_wtime();
#endif

    solar_grid.write_Grid(pszOutputFile, 2);

#ifdef _OPENMP
    endTotal = omp_get_wtime();
#endif
    
    std::cout << "ShdAspSlp time = " << endShdAspSlp - startShdAspSlp << std::endl;
    std::cout << "Rad time = " << endRad - startRad << std::endl;
    std::cout << "Total time = " << endTotal - startTotal << std::endl;
    
    
    return 0;
}

