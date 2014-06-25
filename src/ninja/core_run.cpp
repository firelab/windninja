/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Core run for debugging
 * Author:   Kyle Shannon <ksshannon@gmail.com>
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

#include "core_run.h"

int coreMain(int argc, char *argv[])
{
    unsigned int i_;
    double start = 0, end = 0;
    int n_runs=1;
    int num_threads=8;

    ninjaArmy windsim(n_runs);

    //std::string dem = "/home/kyle/src/windninja/legacy/windninjalegacy/trunk/test_data/mackay_asc_dem.asc";
    //std::string dem = "/home/jforthofer/windninja/trunk/test_data/mackay_gtf_dem.tif";
    std::string dem = "/home/jforthofer/windninja/trunk/test_data/denali.asc";
    //std::string timeZone("America/New_York");
    //std::string timeZone("America/Chicago");
    //std::string timeZone("America/Denver");
    //std::string timeZone("Pacific/Marquesas");    //not integer offset from UTC
    //std::string timeZone("America/Phoenix");  //no daylight savings...
    //std::string timeZone("America/Los_Angeles");
    std::string timeZone("America/Anchorage");

    //Download forecast file, make ninjas and set their times.
    //windsim.makeArmy(windsim.fetch_wxForecast(ninjaArmy::ncepNdfd, 24, dem), timeZone);
    //windsim.makeArmy(windsim.fetch_wxForecast(ninjaArmy::ncepNamSurf, 3600, dem), timeZone);
    //windsim.makeArmy(windsim.fetch_wxForecast(ninjaArmy::ncepRapSurf, 24, dem), timeZone);
    //windsim.makeArmy(windsim.fetch_wxForecast(ninjaArmy::ncepDgexSurf, 240, dem), timeZone);
    windsim.makeArmy(windsim.fetch_wxForecast(ninjaArmy::ncepNamAlaskaSurf, 100, dem), timeZone);

    //Already downloaded forecast
    //windsim.makeArmy("/home/jforthofer/windninja/trunk/test_data/NCEP-RUC-13km-SURFACE-mackay_gtf_dem.tif/20110425T1600/20110425T1600.nc", timeZone);



    for(i_ = 0; i_ < windsim.getSize(); i_++)
    {
        //Set ninja communication----------------------------------------------------------
        windsim.setNinjaCommunication(i_, i_, ninjaComClass::ninjaDefaultCom);
        //windsim.ninjas[i_].set_ninjaCommunication(windsim.ninjas[i_].WFDSSCom);

        //DEM input-------------------------------------------------------------------------
        //windsim.ninjas[i_].readInputFile("C:\\01_JASON_L14\\WindNinja\\Source\\trunk\\source\\glensaugh6km.asc");
        //windsim.ninjas[i_].readInputFile("C:\\01_JASON_L14\\WindNinja\\Source\\trunk\\source\\bell.asc");
        //windsim.ninjas[i_].readInputFile("C:\\01_JASON_L14\\WindNinja\\Source\\trunk\\source\\kootenai.asc");

        //windsim.ninjas[i_].readInputFile("C:\\01_JASON_L14\\WindNinja\\Source\\trunk\\source\\proj2_big.tif");

        //windsim.ninjas[i_].readInputFile("C:\\01_JASON_L14\\WindNinja\\Source\\trunk\\source\\macpass_ww.asc");
        //windsim.ninjas[i_].readInputFile("C:\\01_JASON_L14\\WindNinja\\Source\\trunk\\source\\askervein.asc");
        //windsim.ninjas[i_].readInputFile("C:\\01_JASON_L14\\WindNinja\\Source\\trunk\\source\\snake_elevation.asc");
        //windsim.ninjas[i_].readInputFile("C:\\01_JASON_L14\\WindNinja_legacy\\trunk\\source\\mackay_asc_dem.asc");
        windsim.readInputFile( i_, dem );        

        //windsim.ninjas[i_].readInputFile("C:\\01_JASON_L14\\WindNinja\\Source\\trunk\\source\\columbia_dem.asc");
        //windsim.ninjas[i_].readInputFile("C:\\01_JASON_L14\\WindNinja\\Source\\trunk\\source\\Copy of denali.asc");
        //windsim.ninjas[i_].set_DEM("garceau.asc", windsim.ninjas[i_].dem.meters);

        //windsim.ninjas[i_].set_PrjFromPrjFile("small1.prj"); //do after set_DEM()

        //input windspeed/direction---------------------------------------------------------

        //windsim.ninjas[i_].set_inputSpeed((double)(i_+1) * 2.0, velocityUnits::milesPerHour);

        //For single speed and direction run------------------------------------------------
        //windsim.ninjas[i_].set_initializationMethod(WindNinjaInputs::domainAverageInitializationFlag);
        //windsim.ninjas[i_].set_inputSpeed(3.0, velocityUnits::milesPerHour);
        //windsim.ninjas[i_].set_inputDirection(270.0);
        //windsim.ninjas[i_].set_uniAirTemp(80, temperatureUnits::F); //for average speed and direction initialization
        //windsim.ninjas[i_].set_uniCloudCover(50.0, coverUnits::percent);

        //For point initialization run------------------------------------------------------
        //windsim.ninjas[i_].set_initializationMethod(WindNinjaInputs::pointInitialization, true);
        //windsim.ninjas[i_].set_wxStationFilename("C:\\01_JASON_L14\\WindNinja_legacy\\trunk\\source\\mackay_wx_stations.csv");
        //windsim.ninjas[i_].set_wxStationFilename("/home/jforthofer/windninja/trunk/test_data/mackay_wx_stations.csv");
        //if(windsim.ninjas[i_].initializationMethod == WindNinjaInputs::pointInitialization)
        //    wxStation::writeKmlFile(windsim.ninjas[i_].stations,"/home/jforthofer/windninja/trunk/test_data/mackay_wx_stations.kml");

        //For wxModel run-------------------------------------------------------------------
        //                        windsim.ninjas[i_].set_initializationMethod(WindNinjaInputs::wxModelInitializationFlag);
        //                        windsim.ninjas[i_].set_wxModelFilename("NCEP-NDFD-5km/02-14-2011_1312/02-14-2011_1312.nc");

        windsim.setInputWindHeight( i_, 20.0,lengthUnits::feet );
        windsim.setOutputWindHeight( i_, 20.0, lengthUnits::feet);

        //surface properties----------------------------------------------------------------
        windsim.setUniVegetation( i_, WindNinjaInputs::trees );


        // windsim.ninjas[i_].set_uniRoughness(0.01, lengthUnits::meters); //do after set_DEM()
        // windsim.ninjas[i_].set_uniRoughH(0.0, lengthUnits::meters);
        // windsim.ninjas[i_].set_uniRoughD(0.0, lengthUnits::meters);
        // windsim.ninjas[i_].set_uniAlbedo(0.25);
        // windsim.ninjas[i_].set_uniBowen(1.0);
        // windsim.ninjas[i_].set_uniCg(0.15);
        // windsim.ninjas[i_].set_uniAnthropogenic(0.0);
        //diurnal inputs---------------------------------------------------------------------
        windsim.setDiurnalWinds( i_, true );

        if( windsim.getDiurnalWindFlag( i_ ) == true )
        {
            if( windsim.getInitializationMethod( i_ ) !=
                    WindNinjaInputs::wxModelInitializationFlag)   //if wxModel run, time is already set from wxModel file
                windsim.setDateTime( i_, 2011, 11, 21, 14, 0, 0, timeZone);
            //                         if(i_==0)
            //                          windsim.ninjas[i_].set_date_time(2011, 11, 1, 14, 0, 0, timeZone);
            //                         else if(i_==1)
            //                          windsim.ninjas[i_].set_date_time(2011, 11, 1, 13, 20, 0, timeZone);
            //                         else if(i_==2)
            //                          windsim.ninjas[i_].set_date_time(2011, 11, 1, 13, 40, 0, timeZone);
            //                         else
            //                          windsim.ninjas[i_].set_date_time(2011, 11, 21, 14, 0, 0, timeZone);



            //windsim.ninjas[i_].set_date(18, 9, 2009); // (d, m, y)
            //windsim.ninjas[i_].set_time(0, 0, 2, -6); // (s, m, h, timezone)  //MDT is -6
        }

        // windsim.ninjas[i_].set_airTemp(70, windsim.ninjas[i_].F); //for average speed and direction initialization
        // windsim.ninjas[i_].set_cloudCover(0.0, windsim.ninjas[i_].percent);

        // windsim.ninjas[i_].set_position(48.199316, -114.315640);  //Kalispell - AMS conference
        // windsim.ninjas[i_].set_position(46.891224, -113.983378);  //Waterworks
        // windsim.ninjas[i_].set_position(47,37,51.27,-114,26,17.31); //Garceau
        // windsim.ninjas[i_].set_position(45,8,6,-114,50,59);   //small.lcp
        // windsim.ninjas[i_].set_position(63.0468300822, -150.9988326239);  //Denali
        // windsim.ninjas[i_].set_position(43.9041610661, -113.6060918139);  //Mackay
        // windsim.ninjas[i_].set_position(47.3595169454, -120.1776530798);  //Columbia
        // windsim.ninjas[i_].set_position( 34.223568, -118.061457);  //Station Fire California
        windsim.setPosition( i_ ); //get position from DEM file
        //mesh-------------------------------------------------------------------------------
        // windsim.ninjas[i_].mesh.set_targetNumHorizCells(40000);
        windsim.setMeshResolutionChoice( i_, Mesh::coarse );
        // windsim.ninjas[i_].mesh.set_meshResolution(1000, lengthUnits::meters);
        windsim.setNumVertLayers( i_, 20 );
        // windsim.ninjas[i_].mesh.set_meshResolution(250, lengthUnits::meters);
        //windsim.ninjas[i_].mesh.compute_domain_height();
        // windsim.ninjas[i_].mesh.set_domainHeight(3500, lengthUnits::meters);
        //CPUs--------------------------------------------------------------------------------
        windsim.setNumberCPUs( i_, num_threads );
        //output------------------------------------------------------------------------------
        windsim.setOutputBufferClipping( i_, 0.0 );
        windsim.setWxModelGoogOutFlag( i_, true );
        windsim.setGoogOutFlag( i_, true );
        //windsim.ninjas[i_].set_googResolution(100, windsim.ninjas[i_].meters);
        windsim.setGoogResolution( i_, -1, lengthUnits::meters );
        //windsim.ninjas[i_].set_googSpeedScaling(windsim.ninjas[i_].equal_interval);
        windsim.setGoogSpeedScaling( i_, KmlVector::equal_interval );
        windsim.setGoogLineWidth( i_, 1.0 );
        windsim.setWxModelShpOutFlag( i_, false );
        windsim.setShpOutFlag( i_, false );
        windsim.setShpResolution( i_, 150, lengthUnits::meters );
        windsim.setWxModelAsciiOutFlag( i_, false );
        windsim.setAsciiOutFlag( i_, false );
        windsim.setAsciiResolution( i_, -1, lengthUnits::meters);
        windsim.setTxtOutFlag( i_, false );
        windsim.setVtkOutFlag( i_, false );

        //windsim.ninjas[i_].set_outputFilenames(); //be sure to do this after necessary values are filled in (DEM, windspeed, wind direction, mesh resolution, output mesh resolutions, NDFD, etc.)
    }

    windsim.set_writeFarsiteAtmFile(false);

    //start timer
#ifdef _OPENMP
    start = omp_get_wtime();
#endif


    //run the simulations
    if(!windsim.startRuns(num_threads))
        return false;
// if(!windsim.startFirstRun())
//     return false;



    //catch(bad_alloc &memAllocException)
    // {
    //  Com->ninjaCom(ninjaComClass::ninjaFailure, "\n\nCOULD NOT ALLOCATE MEMORY!!\n\n");
    //     return false;
    // }


    //end timer
#ifdef _OPENMP
    end = omp_get_wtime();
#endif


    //print times
#ifdef _OPENMP
    printf("From main():  Computation time was %f sec. time.\n", end-start);
#endif


    //printf("Press any key to exit...");

    //getch();
    return true;
}
