/******************************************************************************
*
* $Id$
*
* Project:  WindNinja
* Purpose:  A class for doing multiple ninja runs.
* Author:   Jason Forthofer <jforthofer@gmail.com>
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

#include "ninjaArmy.h"
/**
* @brief Default constructor.
*
*/
ninjaArmy::ninjaArmy()
{
    Com = new ninjaComClass();
    Com->runNumber = 9999;
    Com->printRunNumber = false;

//    ninjas.push_back(new ninja());
    initLocalData();
}

/**
* @brief Copy constructor.
*
* @param A Object to copy.
*/
ninjaArmy::ninjaArmy(const ninjaArmy& A)
{
    Com = new ninjaComClass(*A.Com);

    ninjas = A.ninjas;
    copyLocalData( A );
}

/**
* @brief Destructor.
*
*/
ninjaArmy::~ninjaArmy()
{
    if(ninjas.size() >= 1)
    {
        delete ninjas[0];
    }
    destoryLocalData();
    delete Com;
}

/**
* @brief Equals operator.
*
* @param A Right-hand side.
* @return A wxModelInitialization equal to the one on the right-hand side;
*/
ninjaArmy& ninjaArmy::operator= (ninjaArmy const& A)
{
    if(&A != this)
    {
        delete Com;
        Com = new ninjaComClass();
        *Com = *A.Com;

        ninjas = A.ninjas;
        copyLocalData( A );
    }
    return *this;
}

/**
 * \brief Return the number of ninjas in the army
 *
 * \return num_ninjas the number of ninjas in the army
 */
int ninjaArmy::getSize()
{
    return ninjas.size();
}

void ninjaArmy::makeDomainAverageArmy( int nSize, bool momentumFlag )
{
    if( nSize < 1 )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Invalid input numNinjas '%d' in ninjaArmy::makeDomainAverageArmy()", nSize);
        throw std::runtime_error(CPLSPrintf("Invalid input numNinjas '%d' in ninjaArmy::makeDomainAverageArmy()", nSize));
    }

    int i;
    for( i=0; i < ninjas.size();i ++)
        delete ninjas[i];
    ninjas.resize( nSize );
    for( i = 0; i < nSize; i++ ){
#ifdef NINJAFOAM
        if(momentumFlag){
            ninjas[i] = new NinjaFoam();
        }else{
            ninjas[i] = new ninja();
        }
#else
        ninjas[i] = new ninja();
#endif //NINJAFOAM

        setNinjaCommunication( i, i );
    }
}

/**
 * @brief ninjaArmy::makePointArmy Makes an army (array) of ninjas for a Point Initialization run.
 * @param timeList vector of simulation times
 * @param timeZone
 * @param stationFileName
 * @param demFile
 */
void ninjaArmy::makePointArmy(std::vector<boost::posix_time::ptime> timeList,
                             string timeZone, string stationFileName,
                             string demFile, bool matchPoints, bool momentumFlag)
{
    if( timeList.size() == 0 )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Invalid 'empty' input timeList in ninjaArmy::makePointArmy()");
        throw std::runtime_error("Invalid 'empty' input timeList in ninjaArmy::makePointArmy()");
    }

    vector<wxStation> stationList;
    boost::posix_time::ptime noTime;
    //interpolate raw data to actual time steps

    int stationFormat = wxStation::GetHeaderVersion(stationFileName.c_str());

    if (stationFormat==1) //This is if it is the old format->1 step, no time knowledge
    {
        stationList = pointInitialization::readWxStations(demFile,timeZone);
    }
    else //New Format (station fetch or multiple time steps)
    {
        stationList = pointInitialization::interpolateFromDisk(demFile, timeList, timeZone);
    }

    ninjas.resize(timeList.size());

    for(unsigned int i=0; i<timeList.size(); i++)
    {
        ninjas[i] = new ninja();

        setNinjaCommunication( i, i );
    }

    boost::local_time::tz_database tz_db;
    tz_db.load_from_file( FindDataPath("date_time_zonespec.csv") );
    boost::local_time::time_zone_ptr timeZonePtr;
    timeZonePtr = tz_db.time_zone_from_region(timeZone);

    boost::posix_time::ptime standard = boost::posix_time::second_clock::universal_time();
    boost::local_time::local_date_time localStandard(standard, timeZonePtr);
    vector<boost::local_time::local_date_time> localTimeList;
    CPLDebug("STATION_FETCH","\n!!\nList of steps generated with Local and UTC times\n!!:\n");
    for(unsigned int i = 0; i<timeList.size(); i++)//Take the UTC timelist and covert it to local time (again)
    {
        boost::posix_time::ptime aGlobal = timeList[i];
        boost::local_time::local_date_time aLocal(aGlobal, timeZonePtr);
        localTimeList.push_back(aLocal);
        //This is a bit convoluted and obfuscated but all it does
        //is get the times to print correctly for debugging of timelist
        //in case that is necessary
        CPLDebug("STATION_FETCH","STEP NUM:%i",i);
        CPLDebug("STATION_FETCH","UTC: %s",boost::posix_time::to_iso_extended_string(timeList[i]).c_str());
        CPLDebug("STATION_FETCH","LOCAL: %s",boost::lexical_cast<std::string>(localTimeList[i]).c_str());
        CPLDebug("STATION_FETCH","----");
    }
    //handle old wxStation format
//    if (timeList.size() == 1 && timeList[0] == noTime)
    if(timeList.size()==1)
    {
        CPLDebug("STATION_FETCH","Single Step Run Detected!");
        if (timeList[0]!=noTime)
        {
            CPLDebug("STATION_FETCH","Date Time info available for 1 step!");
            timeList.assign(1, timeList[0]);
            localTimeList.assign(1, boost::local_time::local_date_time(timeList[0],timeZonePtr));
        }
        if (timeList[0]==noTime)
        {
            CPLDebug("STATION_FETCH","No date time data available, assigning simulation time to now!");
            timeList.assign(1, standard);
            localTimeList.assign(1, localStandard);
        }
    }
    for(unsigned int k=0; k<stationList.size(); k++)
    {
        for (unsigned int i=0; i<timeList.size(); i++)
        {
            boost::posix_time::ptime aGlobal=timeList[i];
            boost::local_time::local_date_time aLocal(aGlobal, timeZonePtr);
            stationList[k].set_localDateTime(aLocal);
        }
    }

    for(unsigned int i = 0; i<timeList.size(); i++)
    {
        ninjas[i]->set_stationFetchFlag(true);
        ninjas[i]->set_date_time(localTimeList[i]);
        for(int k=0; k<stationList.size(); k++)
        {
            stationList[k].set_currentTimeStep(ninjas[i]->get_date_time());
        }
        ninjas[i]->set_wxStations(stationList);
        ninjas[i]->set_wxStationFilename(stationFileName);
        //Setting the filename also implicitly sets the stations, set above
        //in set_wxStations. Also it gets the units from the first station
        //The function name is a bit misleading as to what it really does.
        ninjas[i]->set_initializationMethod(WindNinjaInputs::pointInitializationFlag, matchPoints);
   }

    // need to always clear these static values out before adding new ones,
    // between each download/run, or they get kept across downloads/runs.
    wxStation::stationKmlNames.clear();
}

/**
 * @brief Makes an army (array) of ninjas for a weather forecast run.
 *
 * @param forecastFilename Name of forecast file.
 * @param timeZone String identifying time zone (must match strings in the file "date_time_zonespec.csv".
 */
void ninjaArmy::makeWeatherModelArmy(std::string forecastFilename, std::string timeZone, bool momentumFlag)
{
  return makeWeatherModelArmy(forecastFilename, timeZone, std::vector<blt::local_date_time>(), momentumFlag);
}

/**
 * @brief Makes an army (array) of ninjas for a weather forecast run.
 *
 * @param forecastFilename Name of forecast file.
 * @param timeZone String identifying time zone (must match strings in the file "date_time_zonespec.csv".
 * @param times a vector of times to run from the forecast.  If the vector is
 *        empty, run all of the times in the forecast
 */
void ninjaArmy::makeWeatherModelArmy(std::string forecastFilename, std::string timeZone, std::vector<blt::local_date_time> times, bool momentumFlag)
{
    wxModelInitialization* model;

    model = wxModelInitializationFactory::makeWxInitialization(forecastFilename);

    try
    {
        model->checkForValidData(timeZone);
    }
    catch(armyException &e)
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Bad forecast file, exiting");
        throw;
    }
    std::vector<boost::local_time::local_date_time> timeList = model->getTimeList(timeZone);
    if(times.size() > 0) {
      timeList = times;
    }
    ninjas.resize(timeList.size());
    //reallocate ninjas after resizing
    for(unsigned int i = 0; i < timeList.size(); i++)
    {
#ifdef NINJAFOAM
        if(momentumFlag == true){
            ninjas[i] = new NinjaFoam();
        }
        else{
             ninjas[i] = new ninja();
        }
#else
        ninjas[i] = new ninja();
#endif

        setNinjaCommunication( i, i );
    }

    for(unsigned int i = 0; i < timeList.size(); i++)
    //int i = 0;
    //FOR_EVERY( iter_ninja, ninjas )
    {
        ninjas[i]->set_date_time(timeList[i]);
        ninjas[i]->set_wxModelFilename(forecastFilename);
        ninjas[i]->set_initializationMethod(WindNinjaInputs::wxModelInitializationFlag);
        ninjas[i]->set_inputWindHeight( (*model).Get_Wind_Height() );

        /*iter_ninja->set_date_time( timeList[i] );
        iter_ninja->set_wxModelFilename( forecastFilename );
        iter_ninja->set_initializationMethod( WindNinjaInputs::wxModelInitializationFlag );
        iter_ninja->set_inputWindHeight( (*model).Get_Wind_Height() );
        i++;*/
    }

    delete model;
}

/**
* @brief Function to start WindNinja core runs using multiple threads.
*
* @param numProcessors Number of processors to use.
* @return True if runs complete properly.
*/
bool ninjaArmy::startRuns(int numProcessors)
{
    int j;
    bool status = true;

    if(ninjas.size()<1 || numProcessors<1)
        return false;

    //check for duplicate runs before we start the simulations
    //this is mostly for batch domain avg runs in the GUI and the API
    if(ninjas.size() > 1)
    {
        for(unsigned int i=0; i<ninjas.size()-1; i++)
        {
            for(unsigned int j=i+1; j<ninjas.size(); j++)
            {
                if(ninjas[i]->input == ninjas[j]->input && ninjas[i]->get_initializationMethod() == WindNinjaInputs::domainAverageInitializationFlag)
                {
                    ninjas[j]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Multiple runs were requested with the same input parameters.");
                    status = false;
                    throw std::runtime_error("Multiple runs were requested with the same input parameters.");
                }
            }
        }
    }

#ifdef NINJAFOAM
    //if it's a ninjafoam run and the user specified an existing case dir, set it here
    if(ninjas[0]->identify() == "ninjafoam" & ninjas[0]->input.existingCaseDirectory != "!set"){
        NinjaFoam::SetFoamPath(ninjas[0]->input.existingCaseDirectory.c_str());
    }
    //if it's a ninjafoam run and the case is not set by the user, generate the ninjafoam dir
    if(ninjas[0]->identify() == "ninjafoam" & ninjas[0]->input.existingCaseDirectory == "!set"){
        //force temp dir to DEM location
        CPLSetConfigOption("CPL_TMPDIR", CPLGetDirname(ninjas[0]->input.dem.fileName.c_str()));
        CPLSetConfigOption("CPLTMPDIR", CPLGetDirname(ninjas[0]->input.dem.fileName.c_str()));
        CPLSetConfigOption("TEMP", CPLGetDirname(ninjas[0]->input.dem.fileName.c_str()));
        int status = NinjaFoam::GenerateFoamDirectory(ninjas[0]->input.dem.fileName);
        if(status != 0){
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Error generating the NINJAFOAM directory.");
            throw std::runtime_error("Error generating the NINJAFOAM directory.");
        }
    }
#endif //NINJAFOAM

#ifdef _OPENMP
    omp_set_nested(false);
    //omp_set_dynamic(true);
#endif

    //TODO: move common parameters (resolutions, input filenames, output arguments) to ninjaArmy or change storage class specifier to static

    /*
    ** Download a color relief file as the temp file allocated in
    ** initLocalData().  If we fail, clean up properly so we can save a
    ** hillshade file at that location.
    */
    if(ninjas[0]->input.pdfOutFlag == true)
    {
        GDALDatasetH hDS = NULL;
        GDALRasterBandH hBand = NULL;

        hDS = GDALOpen( ninjas[0]->input.dem.fileName.c_str(), GA_ReadOnly );
        assert( hDS );
        hBand = GDALGetRasterBand( hDS, 1 );
        assert( hBand );

        int nXSize = GDALGetRasterXSize( hDS );
        int nYSize = GDALGetRasterYSize( hDS );
        /*
        ** Figure out How big we need to make our raster, given a width,
        ** height and dpi.
        */
        double dfWidth, dfHeight;
        unsigned short nDPI;
        dfHeight = ninjas[0]->input.pdfHeight - OutputWriter::TOP_MARGIN - OutputWriter::BOTTOM_MARGIN;
        dfWidth = ninjas[0]->input.pdfWidth - 2.0*OutputWriter::SIDE_MARGIN;
        nDPI = ninjas[0]->input.pdfDPI;
        double dfRatio, dfRatioH, dfRatioW;

        dfRatioH = dfHeight * nDPI / nYSize;
        dfRatioW = dfWidth * nDPI / nXSize;
        dfRatio = MIN( dfRatioH, dfRatioW );

        int nNewXSize = nXSize * dfRatio;
        int nNewYSize = nYSize * dfRatio;

        CPLSetConfigOption( "GDAL_PAM_ENABLED", "OFF" );

        SURF_FETCH_E retval = SURF_FETCH_E_NONE;
        if( ninjas[0]->input.pdfBaseType == WindNinjaInputs::TOPOFIRE )
        {
            SurfaceFetch * fetcher = FetchFactory::GetSurfaceFetch( "relief" );
            retval = fetcher->makeReliefOf( ninjas[0]->input.dem.fileName,
                                            pszTmpColorRelief, nNewXSize, nNewYSize );
            delete fetcher;
        }
        /*
        ** If we fail, or the user wants a hillshade, copy the dem into the
        ** file as an 8 bit GeoTiff
        */
        if( ninjas[0]->input.pdfBaseType == WindNinjaInputs::HILLSHADE ||
            retval != SURF_FETCH_E_NONE )
        {
            CPLDebug( "NINJA", "Failed to download relief, creating hillshade" );
            GDALDriverH hDrv = NULL;
            hDrv = GDALGetDriverByName( "GTiff" );
            assert( hDrv );
            CPLSetErrorHandler( CPLQuietErrorHandler );
            GDALDeleteDataset( hDrv, pszTmpColorRelief );
            CPLPopErrorHandler();

            GDALDatasetH h8bit = GDALCreate( hDrv, pszTmpColorRelief, nNewXSize,
                                             nNewYSize, 1, GDT_Byte, NULL );
            CPLErr eErr = CE_None;
            double adfGeoTransform[6];
            eErr = GDALGetGeoTransform( hDS, adfGeoTransform );
            assert( eErr == CE_None );
            adfGeoTransform[1] /= dfRatio;
            adfGeoTransform[5] /= dfRatio;
            GDALSetGeoTransform( h8bit, adfGeoTransform );

            GDALSetProjection( h8bit, GDALGetProjectionRef( hDS ) );

            GDALRasterBandH h8bitBand = GDALGetRasterBand( h8bit, 1 );
            float *padfData = NULL;
            padfData = (float*)CPLMalloc( nNewXSize * nNewYSize * sizeof( float ) );
            unsigned char *pabyData = NULL;
            pabyData = (unsigned char*)CPLMalloc( nNewXSize * nNewYSize * sizeof( unsigned char* ) );
            double adfMinMax[2];
            int bSuccess = TRUE;
            double dfMin, dfMax, dfMean, dfStdDev;
            GDALComputeRasterStatistics( hBand, FALSE, &dfMin, &dfMax, &dfMean, &dfStdDev, NULL, NULL );

            eErr = GDALRasterIO( hBand, GF_Read, 0, 0, nXSize, nYSize,
                                 padfData, nNewXSize, nNewYSize,
                                 GDT_Float32, 0, 0 );
            assert( eErr == CE_None );
            for( int i = 0; i < nNewXSize * nNewYSize; i++ )
            {
                /*
                ** Figure out what is going on here and document it.  It makes a
                ** potentially useful map whern dfMax=BIG and dfMin=-BIG.
                */
                //double dfMin = GDALGetRasterMinimum( hBand, NULL );
                //double dfMax = GDALGetRasterMaximum( hBand, NULL );
                //pabyData[j] = (unsigned char)(padfData[j] * (dfMax - dfMin) / (dfMax - dfMin)) * 255;

                /* Normal */
                pabyData[i] = ((padfData[i] - dfMin) / (dfMax - dfMin)) * 255;
            }
            eErr = GDALRasterIO( h8bitBand, GF_Write, 0, 0, nNewXSize,
                                 nNewYSize, pabyData, nNewXSize, nNewYSize,
                                 GDT_Byte, 0, 0 );
            assert( eErr == CE_None );
            CPLFree( (void*)padfData );
            CPLFree( (void*)pabyData );
            GDALFlushCache( h8bit );
            GDALClose( hDS );
            GDALClose( h8bit );

            /* delete stats file */
            if( CPLCheckForFile( (char*)CPLSPrintf("%s.aux.xml", ninjas[0]->input.dem.fileName.c_str()), NULL ) ){
                VSIUnlink( CPLSPrintf("%s.aux.xml", ninjas[0]->input.dem.fileName.c_str()) );
            }
        }
        /* Make sure all runs point to the proper DEM file */
        for(unsigned int i = 0; i < ninjas.size(); i++)
        {
            ninjas[i]->input.pdfDEMFileName = pszTmpColorRelief;
        }
        CPLSetConfigOption( "GDAL_PAM_ENABLED", "ON" );
    }

    // prep/reset the stored atmosphere file data, to be filled before ninjas[i] gets deleted after each run
    // also used to set the size of the outputs, to better handle multi-threading
    atmosphere.reset(ninjas.size());

    // prep a clean set of map visualization output filenames, to be filled before ninjas[i] gets deleted after each run
    fgbzFilenames.resize(ninjas.size());
    stationKmlFilenames.resize(ninjas.size());
    wxModelFgbzFilenames.resize(ninjas.size());

    if(ninjas.size() == 1)
    {
        //set number of threads for the run
        ninjas[0]->set_numberCPUs(numProcessors);
        try{
            if ((ninjas[0]->identify() == "ninjafoam") && ninjas[0]->input.diurnalWinds)
            {
                //Set the ninjafoam solver progress bar to stop at 80% so that
                //the diurnal solver can contribute too
                ninjas[0]->set_progressWeight(0.80);
            }

            //start the run
            if(!ninjas[0]->simulate_wind())
               printf("Return of false from simulate_wind()\n");
#ifdef NINJAFOAM
            //if it's a ninjafoam run and diurnal is turned on, link the ninjafoam with
            //a ninja run to add diurnal flow after the cfd solution is computed
            if ((ninjas[0]->identify() == "ninjafoam") & ninjas[0]->input.diurnalWinds){
                CPLDebug("NINJA", "Starting a ninja to add diurnal to ninjafoam output.");
                ninja* diurnal_ninja = new ninja(*ninjas[0]);
                //Set the diurnal ninja to have the same com object,
                //so that it can update the progress of the original ninja
                diurnal_ninja->input.Com = ninjas[0]->input.Com;
                diurnal_ninja->set_foamVelocityGrid(ninjas[0]->VelocityGrid);
                diurnal_ninja->set_foamAngleGrid(ninjas[0]->AngleGrid);
                diurnal_ninja->set_writeTurbulenceFlag(ninjas[0]->input.writeTurbulence);
                if(ninjas[0]->input.initializationMethod == WindNinjaInputs::domainAverageInitializationFlag){
                    diurnal_ninja->input.initializationMethod = WindNinjaInputs::foamDomainAverageInitializationFlag;
                }
                else if(ninjas[0]->input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag){
                    diurnal_ninja->input.initializationMethod = WindNinjaInputs::foamWxModelInitializationFlag;
                }
                else if(ninjas[0]->input.initializationMethod == WindNinjaInputs::griddedInitializationFlag){
                    diurnal_ninja->input.initializationMethod = WindNinjaInputs::foamGriddedInitializationFlag;
                }
                else{
                    throw std::runtime_error("ninjaArmy: Initialization method not set properly.");
                }
                diurnal_ninja->input.inputWindHeight = ninjas[0]->input.outputWindHeight;
                //if case is re-used resolution may not be set, set mesh resolution based on ninjas[0]
                diurnal_ninja->set_meshResolution(ninjas[0]->get_meshResolution(), lengthUnits::getUnit("m"));
                //need to set some wxModelInit metadata from init-> to help with wxModel legend output
                if(ninjas[0]->input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag)
                {
                    diurnal_ninja->input.foamWxTimeList = ninjas[0]->init->getTimeList(ninjas[0]->input.ninjaTimeZone);
                    diurnal_ninja->input.foamWxForecastIdentifier = ninjas[0]->init->getForecastIdentifier();
                }
                if(!diurnal_ninja->simulate_wind()){
                    printf("Return of false from simulate_wind()\n");
                }
                //set output path on original ninja for the GUI
                ninjas[0]->input.outputPath = diurnal_ninja->input.outputPath;

                //set filenames for atm file writing
                ninjas[0]->input.velFile = diurnal_ninja->get_VelFileName();
                ninjas[0]->input.angFile = diurnal_ninja->get_AngFileName();
                ninjas[0]->input.geoTiffFile = diurnal_ninja->input.geoTiffFile;

                //set fgbzFile for setCurrentMapVisualization(), for the GUI
                ninjas[0]->input.fgbzFile = diurnal_ninja->input.fgbzFile;

                //set other filenames needed for consistentColorScale outputs
                ninjas[0]->input.kmlFile = diurnal_ninja->input.kmlFile;
                ninjas[0]->input.kmzFile = diurnal_ninja->input.kmzFile;
                ninjas[0]->input.legFile = diurnal_ninja->input.legFile;
                ninjas[0]->input.dateTimeLegFile = diurnal_ninja->input.dateTimeLegFile;

                //also need to transfer the 2D output grids back to the original ninja, for consistentColorScale outputs
                //note the TurbulenceGrid and colMaxGrid are NOT altered by the mass solver, so can skip those grids
                ninjas[0]->VelocityGrid = diurnal_ninja->VelocityGrid;
                ninjas[0]->AngleGrid = diurnal_ninja->AngleGrid;
                ninjas[0]->CloudGrid = diurnal_ninja->CloudGrid;
                //#ifdef NINJAFOAM
                //ninjas[0]->TurbulenceGrid = diurnal_ninja->TurbulenceGrid;
                //ninjas[0]->colMaxGrid = diurnal_ninja->colMaxGrid;
                //#endif
                #ifdef FRICTION_VELOCITY
                ninjas[0]->UstarGrid = diurnal_ninja->UstarGrid;
                #endif
                #ifdef EMISSIONS
                ninjas[0]->DustGrid = diurnal_ninja->DustGrid;
                #endif
            }

#endif //NINJAFOAM

            //store data for atmosphere file
            if(ninjas[0]->input.atmOutFlag)
            {
                if(ninjas[0]->input.geoTiffOutFlag == true)
                {
                    std::string velGeoTiffFile = ninjas[0]->input.geoTiffFile;
                    std::string angGeoTiffFile = ninjas[0]->input.geoTiffFile;
                    velGeoTiffFile.insert(velGeoTiffFile.find(".tif"), "_vel");
                    angGeoTiffFile.insert(angGeoTiffFile.find(".tif"), "_ang");
                    atmosphere.push(0, ninjas[0]->get_date_time(), velGeoTiffFile, angGeoTiffFile);
                }
                else
                {
                    atmosphere.push(0, ninjas[0]->get_date_time(), ninjas[0]->get_VelFileName(), ninjas[0]->get_AngFileName());
                }
            }

            //write farsite atmosphere file
            if(ninjas[0]->input.atmOutFlag)
            {
                writeFarsiteAtmosphereFile();
            }

            //setup the run map visualization filenames, for C-API calls
            setCurrentMapVisualizationFilenames(0);

        }catch (bad_alloc& e)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception bad_alloc caught: %s\nWindNinja appears to have run out of memory.", e.what());
            status = false;
            throw;
        }catch (cancelledByUser& e)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaNone, "Exception caught: %s", e.what());
            status = false;
            throw;
        }catch (exception& e)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
            status = false;
            throw;
        }catch (...)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
            status = false;
            throw;
        }
    }
#ifdef NINJAFOAM
    else if ((ninjas.size() > 1) && (ninjas[0]->identify() =="ninjafoam"))
    {
#ifdef _OPENMP
        omp_set_num_threads(numProcessors);
#endif
        for(unsigned int i = 0; i < ninjas.size(); i++)
        {
            try{
                //set number of threads for the run
                ninjas[i]->set_numberCPUs( numProcessors );

                if((ninjas[i]->identify() == "ninjafoam") && ninjas[0]->input.diurnalWinds)
                {
                    //Set the ninjafoam solver progress bar to stop at 80% so that
                    //the diurnal solver can contribute too
                    ninjas[i]->set_progressWeight(0.80);
                }
                //start the run
                if(!ninjas[i]->simulate_wind()){
                    throw std::runtime_error("ninjaArmy: Error in NinjaFoam::simulate_wind().");
                }
                //if it's a ninjafoam run and diurnal is turned on, link the ninjafoam with
                //a ninja run to add diurnal flow after the cfd solution is computed
                if((ninjas[i]->identify() == "ninjafoam") && ninjas[i]->input.diurnalWinds){
                    CPLDebug("NINJA", "Starting a ninja to add diurnal to ninjafoam output.");
                    ninja* diurnal_ninja = new ninja(*ninjas[i]);
                    //Set the diurnal ninja to have the same com object,
                    //so that it can update the progress of the original ninja
                    diurnal_ninja->input.Com = ninjas[i]->input.Com;
                    diurnal_ninja->set_foamVelocityGrid(ninjas[i]->VelocityGrid);
                    diurnal_ninja->set_foamAngleGrid(ninjas[i]->AngleGrid);
                    diurnal_ninja->set_writeTurbulenceFlag(ninjas[0]->input.writeTurbulence);
                    if(ninjas[i]->input.initializationMethod == WindNinjaInputs::domainAverageInitializationFlag){
                        diurnal_ninja->input.initializationMethod = WindNinjaInputs::foamDomainAverageInitializationFlag;
                    }
                    else if(ninjas[i]->input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag){
                        diurnal_ninja->input.initializationMethod = WindNinjaInputs::foamWxModelInitializationFlag;
                    }
                    else if(ninjas[i]->input.initializationMethod == WindNinjaInputs::griddedInitializationFlag){
                        diurnal_ninja->input.initializationMethod = WindNinjaInputs::foamGriddedInitializationFlag;
                    }
                    else{
                        throw std::runtime_error("ninjaArmy: Initialization method not set properly.");
                    }
                    diurnal_ninja->input.inputWindHeight = ninjas[i]->input.outputWindHeight;
                    //if case is re-used resolution may not be set, set mesh resolution based on ninjas[0]
                    diurnal_ninja->set_meshResolution(ninjas[0]->get_meshResolution(), lengthUnits::getUnit("m"));
                    //need to set some wxModelInit metadata from init-> to help with wxModel legend output
                    if(ninjas[i]->input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag)
                    {
                        diurnal_ninja->input.foamWxTimeList = ninjas[i]->init->getTimeList(ninjas[i]->input.ninjaTimeZone);
                        diurnal_ninja->input.foamWxForecastIdentifier = ninjas[i]->init->getForecastIdentifier();
                    }
                    if(!diurnal_ninja->simulate_wind()){
                        throw std::runtime_error("ninjaArmy: Error in ninja::simulate_wind().");
                    }
                    //set output path on original ninja for the GUI
                    ninjas[i]->input.outputPath = diurnal_ninja->input.outputPath;

                    //set filenames for atm file writing
                    ninjas[i]->input.velFile = diurnal_ninja->get_VelFileName();
                    ninjas[i]->input.angFile = diurnal_ninja->get_AngFileName();
                    ninjas[i]->input.geoTiffFile = diurnal_ninja->input.geoTiffFile;

                    //set fgbzFile for setCurrentMapVisualizationFilenames(), for the GUI
                    ninjas[i]->input.fgbzFile = diurnal_ninja->input.fgbzFile;

                    //set other filenames needed for consistentColorScale outputs
                    ninjas[i]->input.kmlFile = diurnal_ninja->input.kmlFile;
                    ninjas[i]->input.kmzFile = diurnal_ninja->input.kmzFile;
                    ninjas[i]->input.legFile = diurnal_ninja->input.legFile;
                    ninjas[i]->input.dateTimeLegFile = diurnal_ninja->input.dateTimeLegFile;

                    //also need to transfer the 2D output grids back to the original ninja, for consistentColorScale outputs
                    //note the TurbulenceGrid and colMaxGrid are NOT altered by the mass solver, so can skip those grids
                    ninjas[i]->VelocityGrid = diurnal_ninja->VelocityGrid;
                    ninjas[i]->AngleGrid = diurnal_ninja->AngleGrid;
                    ninjas[i]->CloudGrid = diurnal_ninja->CloudGrid;
                    //#ifdef NINJAFOAM
                    //ninjas[i]->TurbulenceGrid = diurnal_ninja->TurbulenceGrid;
                    //ninjas[i]->colMaxGrid = diurnal_ninja->colMaxGrid;
                    //#endif
                    #ifdef FRICTION_VELOCITY
                    ninjas[i]->UstarGrid = diurnal_ninja->UstarGrid;
                    #endif
                    #ifdef EMISSIONS
                    ninjas[i]->DustGrid = diurnal_ninja->DustGrid;
                    #endif
                }

                //store data for atmosphere file
                if(ninjas[i]->input.atmOutFlag)
                {
                    if(ninjas[i]->input.geoTiffOutFlag == true)
                    {
                        std::string velGeoTiffFile = ninjas[i]->input.geoTiffFile;
                        std::string angGeoTiffFile = ninjas[i]->input.geoTiffFile;
                        velGeoTiffFile.insert(velGeoTiffFile.find(".tif"), "_vel");
                        angGeoTiffFile.insert(angGeoTiffFile.find(".tif"), "_ang");
                        atmosphere.push(i, ninjas[i]->get_date_time(), velGeoTiffFile, angGeoTiffFile);
                    }
                    else
                    {
                        atmosphere.push(i, ninjas[i]->get_date_time(), ninjas[i]->get_VelFileName(), ninjas[i]->get_AngFileName());
                    }
                }

                //setup the run map visualization filenames, for C-API calls
                setCurrentMapVisualizationFilenames(i);

                //delete all but ninjas[0] (ninjas[0] is used to set the output path in the GUI)
                //need to keep the ninjas for now, if doing a consistent color scale set of outputs
                if( i != 0 && ninjas[0]->input.googUseConsistentColorScale == false && ninjas[0]->input.fgbzUseConsistentColorScale == false )
                {
                    delete ninjas[i];
                    ninjas[i] = NULL;
                }

            }catch (bad_alloc& e)
            {
                ninjas[i]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception bad_alloc caught: %s\nWindNinja appears to have run out of memory.", e.what());
                status = false;
                throw;
            }catch (cancelledByUser& e)
            {
                ninjas[i]->input.Com->ninjaCom(ninjaComClass::ninjaNone, "Exception caught: %s", e.what());
                status = false;
                throw;
            }catch (exception& e)
            {
                ninjas[i]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
                status = false;
                throw;
            }catch (...)
            {
                ninjas[i]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
                status = false;
                throw;
            }
        }
        try{
            //write farsite atmosphere file
            if(ninjas[0]->input.atmOutFlag)
            {
                writeFarsiteAtmosphereFile();
            }
        }catch (bad_alloc& e)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception bad_alloc caught: %s\nWindNinja appears to have run out of memory.", e.what());
            status = false;
            throw;
        }catch (cancelledByUser& e)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaNone, "Exception caught: %s", e.what());
            status = false;
            throw;
        }catch (exception& e)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
            status = false;
            throw;
        }catch (...)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
            status = false;
            throw;
        }
    }
#endif //NINJAFOAM
    else
    {
        for(unsigned int i = 0; i < ninjas.size(); i++)
        {
            ninjas[i]->set_numberCPUs(1);
        }

        /*FOR_EVERY(iter_ninja, ninjas)
        {
            iter_ninja->set_numberCPUs(1);
        }*/
#ifdef _OPENMP
        omp_set_num_threads(numProcessors);
#endif
        std::vector<int> anErrors( numProcessors);
        std::vector<std::string>asMessages( numProcessors );

        std::vector<boost::local_time::local_date_time> timeList;

    #pragma omp parallel for //spread runs on single threads
        //FOR_EVERY(iter_ninja, ninjas) //Doesn't work with omp
        for( int i = 0; i < ninjas.size(); i++ )
        {
            try
            {
                //start the run
                ninjas[i]->simulate_wind();  //runs are done on 1 thread each since omp_set_nested(false)

                //store data for atmosphere file
                if(ninjas[i]->input.atmOutFlag)
                {
                    if(ninjas[i]->input.geoTiffOutFlag == true)
                    {
                        std::string velGeoTiffFile = ninjas[i]->input.geoTiffFile;
                        std::string angGeoTiffFile = ninjas[i]->input.geoTiffFile;
                        velGeoTiffFile.insert(velGeoTiffFile.find(".tif"), "_vel");
                        angGeoTiffFile.insert(angGeoTiffFile.find(".tif"), "_ang");
                        atmosphere.push(i, ninjas[i]->get_date_time(), velGeoTiffFile, angGeoTiffFile);
                    }
                    else
                    {
                        atmosphere.push(i, ninjas[i]->get_date_time(), ninjas[i]->get_VelFileName(), ninjas[i]->get_AngFileName());
                    }
                }

                //setup the run map visualization filenames, for C-API calls
                setCurrentMapVisualizationFilenames(i);

                //delete all but ninjas[0] (ninjas[0] is used to set the output path in the GUI)
                //need to keep the ninjas for now, if doing a consistent color scale set of outputs
                if( i != 0 && ninjas[0]->input.googUseConsistentColorScale == false && ninjas[0]->input.fgbzUseConsistentColorScale == false )
                {
                    delete ninjas[i];
                    ninjas[i] = NULL;
                }

            }catch (bad_alloc& e)
            {
                ninjas[i]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception bad_alloc caught: %s\nWindNinja appears to have run out of memory.", e.what());
#ifdef _OPENMP
                anErrors[omp_get_thread_num()] = STD_BAD_ALLOC_EXC;
                asMessages[omp_get_thread_num()] = "Exception bad_alloc caught:";
                asMessages[omp_get_thread_num()] += e.what();
                asMessages[omp_get_thread_num()] += "\n";
                status = false;
#else
                throw;
#endif
            }catch (logic_error& e)
            {
                ninjas[i]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception logic_error caught: %s", e.what());
#ifdef _OPENMP
                anErrors[omp_get_thread_num()] = STD_LOGIC_EXC;
                asMessages[omp_get_thread_num()] = "Exception logic_error caught:";
                asMessages[omp_get_thread_num()] += e.what();
                asMessages[omp_get_thread_num()] += "\n";
                status = false;
#else
                throw;
#endif
             }catch (cancelledByUser& e)
            {
                ninjas[i]->input.Com->ninjaCom(ninjaComClass::ninjaNone, "Exception canceled by user caught: %s", e.what());
#ifdef _OPENMP
                anErrors[omp_get_thread_num()] = NINJA_CANCEL_USER_EXC;
                asMessages[omp_get_thread_num()] = "Exception canceled by user caught:";
                asMessages[omp_get_thread_num()] + e.what();
                asMessages[omp_get_thread_num()] += "\n";
                status = false;
#else
                throw;
#endif
            }catch (badForecastFile& e)
            {
                ninjas[i]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception badForecastFile caught: %s", e.what());
#ifdef _OPENMP
                anErrors[omp_get_thread_num()] = NINJA_BAD_FORECAST_EXC;
                asMessages[omp_get_thread_num()] = "Exception badForecastFile caught:";
                asMessages[omp_get_thread_num()] + e.what();
                asMessages[omp_get_thread_num()] += "\n";
                status = false;
#else
                throw;
#endif
            }catch (exception& e)
            {
                ninjas[i]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
#ifdef _OPENMP
                anErrors[omp_get_thread_num()] = STD_EXC;
                asMessages[omp_get_thread_num()] = "Exception caught:";
                asMessages[omp_get_thread_num()] + e.what();
                asMessages[omp_get_thread_num()] += "\n";
                status = false;
#else
                throw;
#endif
            }catch (...)
            {
                ninjas[i]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
#ifdef _OPENMP
                anErrors[omp_get_thread_num()] = STD_UNKNOWN_EXC;
                asMessages[omp_get_thread_num()] = "Unknown Exception caught:";
                asMessages[omp_get_thread_num()] += "\n";
                status = false;
#else
                throw;
#endif
            }
        }
#ifdef _OPENMP
        NinjaRethrowThreadedException( anErrors, asMessages, numProcessors );
#endif
        try{
            //write farsite atmosphere file
            if(ninjas[0]->input.atmOutFlag)
            {
                writeFarsiteAtmosphereFile();
            }
        }catch (bad_alloc& e)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception bad_alloc caught: %s\nWindNinja appears to have run out of memory.", e.what());
            status = false;
            throw;
        }catch (cancelledByUser& e)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaNone, "Exception caught: %s", e.what());
            status = false;
            throw;
        }catch (exception& e)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
            status = false;
            throw;
        }catch (...)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
            status = false;
            throw;
        }
    }

    try{
        //write consistent color scale outputs
        if(ninjas.size() > 1 && (ninjas[0]->input.googUseConsistentColorScale == true || ninjas[0]->input.fgbzUseConsistentColorScale == true))
        {
            writeConsistentColorScaleOutputs();

            //cleanup at the end
            for( int i = 0; i < ninjas.size(); i++ )
            {
                //delete all but ninjas[0] (ninjas[0] is used to set the output path in the GUI)
                if( i != 0 )
                {
                    delete ninjas[i];
                    ninjas[i] = NULL;
                }
            }
        }
    }catch (bad_alloc& e)
    {
        ninjas[ninjas.size()-1]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception bad_alloc caught: %s\nWindNinja appears to have run out of memory.", e.what());
        status = false;
        throw;
    }catch (cancelledByUser& e)
    {
        ninjas[ninjas.size()-1]->input.Com->ninjaCom(ninjaComClass::ninjaNone, "Exception caught: %s", e.what());
        status = false;
        throw;
    }catch (exception& e)
    {
        ninjas[ninjas.size()-1]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        status = false;
        throw;
    }catch (...)
    {
        ninjas[ninjas.size()-1]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        status = false;
        throw;
    }

    return status;
}

/**
 *  @brief Function to start the first ninja run using 1 thread.
 *
 *  Primarily used for debugging purposes, such as wx forecast runs.
 *
 *  @return True if runs complete properly.
 */
bool ninjaArmy::startFirstRun()
{
    bool status = true;

    //set number of threads for the run
    ninjas[0]->set_numberCPUs(1);
    try
    {
        //start the run
        if(!ninjas[0]->simulate_wind())
            printf("Return of false from simulate_wind()\n");

        //write farsite atmosphere file
        writeFarsiteAtmosphereFile();

    }
    catch (bad_alloc& e)
    {
        ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception bad_alloc caught: %s\nWindNinja appears to have run out of memory.", e.what());
        status = false;
        throw;
    }
    catch (cancelledByUser& e)
    {
        ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaNone, "Exception caught: %s", e.what());
        status = false;
        throw;
    }
    catch (exception& e)
    {
        ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        status = false;
        throw;
    }
    catch (...)
    {
        ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        status = false;
        throw;
    }
    return status;
}

/**
 * @brief write the atm file
 *
 * if wxModelInitialization or pointInitialization, make one .atm with all runs (times) listed,
 * if domainAverageInitialization, make a separate .atm file for each and every single run.
 */
void ninjaArmy::writeFarsiteAtmosphereFile()
{
    if(ninjas[0]->input.atmOutFlag)
    {
        if(ninjas[0]->get_initializationMethod() == WindNinjaInputs::wxModelInitializationFlag ||
           ninjas[0]->get_initializationMethod() == WindNinjaInputs::pointInitializationFlag)
        {
            bool writeSeparateAtmFiles = false;
            atmosphere.writeAtmFile(writeSeparateAtmFiles, ninjas[0]->get_outputSpeedUnits(), ninjas[0]->get_outputWindHeight());
        }
        else
        {
            bool writeSeparateAtmFiles = true;
            atmosphere.writeAtmFile(writeSeparateAtmFiles, ninjas[0]->get_outputSpeedUnits(), ninjas[0]->get_outputWindHeight());
        }
    }
}

/*-----------------------------------------------------------------------------
 *  C-API makeArmy function calls
 *-----------------------------------------------------------------------------*/

int ninjaArmy::NinjaMakeDomainAverageArmyThermalParameterization( int numNinjas, bool momentumFlag, const double * speedList, const char * speedUnits, const double * directionList, const int * yearList, const int * monthList, const int * dayList, const int * hourList, const int * minuteList, const char * timeZone, const double * airTempList, const char * airTempUnits, const double * cloudCoverList, const char * cloudCoverUnits, char ** papszOptions )
{
    try
    {

#ifndef NINJAFOAM
        if(momentumFlag == true)
        {
            throw std::runtime_error("momentumFlag cannot be set to true. WindNinja was not compiled with mass and momentum support.");
        }
#endif

        if( numNinjas < 1 )
        {
            throw std::runtime_error(CPLSPrintf("Invalid input numNinjas '%d' in ninjaArmy::NinjaMakeDomainAverageArmy()", numNinjas));
        }

        //Get the number of elements in the arrays
/*        size_t length1 = sizeof(speedList) / sizeof(speedList[0]);
        size_t length2 = sizeof(directionList) / sizeof(directionList[0]); */
//        size_t length1 = sizeof(yearList) / sizeof(yearList[0]);
//        size_t length2 = sizeof(monthList) / sizeof(monthList[0]);
//        size_t length3 = sizeof(dayList) / sizeof(dayList[0]);
//        size_t length4 = sizeof(hourList) / sizeof(hourList[0]);
//        size_t length5 = sizeof(minuteList) / sizeof(minuteList[0]);
//        size_t length6 = sizeof(airTempList) / sizeof(airTempList[0]);
//        size_t length7 = sizeof(cloudCoverList) / sizeof(cloudCoverList[0]);
//
//        if(!(length1 == length2 == length3 == length4 == length5 == length6 == length7))
//        {
//            throw std::runtime_error("yearList, monthList, dayList, hourList, minuteList, airTempList, and cloudCoverList must be the same length!");
//

        makeDomainAverageArmy( numNinjas, momentumFlag );

        int retval = NINJA_E_INVALID;
        for(int i=0; i<getSize(); i++)
        {
            retval = setInputSpeed( i, speedList[i], std::string( speedUnits ) );
            if( retval != NINJA_SUCCESS )
            {
                Com->ninjaCom(ninjaComClass::ninjaFailure, "ninjaArmy::setInputSpeed() called in ninjaArmy::NinjaMakeDomainAverageArmy() for ninja '%d' failed.", i);
                return retval;
            }

            retval = setInputDirection( i, directionList[i] );
            if( retval != NINJA_SUCCESS )
            {
                Com->ninjaCom(ninjaComClass::ninjaFailure, "ninjaArmy::setInputDirection() called in ninjaArmy::NinjaMakeDomainAverageArmy() for ninja '%d' failed.", i);
                return retval;
            }

            retval = setDateTime( i, yearList[i], monthList[i], dayList[i], hourList[i], minuteList[i], 0, timeZone );
            if( retval != NINJA_SUCCESS )
            {
                Com->ninjaCom(ninjaComClass::ninjaFailure, "ninjaArmy::setDateTime() called in ninjaArmy::NinjaMakeDomainAverageArmy() for ninja '%d' failed.", i);
                return retval;
            }

            retval = setUniAirTemp( i, airTempList[i], std::string( airTempUnits ) );
            if( retval != NINJA_SUCCESS )
            {
                Com->ninjaCom(ninjaComClass::ninjaFailure, "ninjaArmy::setUniAirTemp() called in ninjaArmy::NinjaMakeDomainAverageArmy() for ninja '%d' failed.", i);
                return retval;
            }

            retval = setUniCloudCover( i, cloudCoverList[i], std::string( cloudCoverUnits ) );
            if( retval != NINJA_SUCCESS )
            {
                Com->ninjaCom(ninjaComClass::ninjaFailure, "ninjaArmy::setUniCloudCover() called in ninjaArmy::NinjaMakeDomainAverageArmy() for ninja '%d' failed.", i);
                return retval;
            }
        }
    }
    catch( armyException & e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( exception & e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( ... )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        return NINJA_E_INVALID;
    }

    return NINJA_SUCCESS;
}


int ninjaArmy::NinjaMakeDomainAverageArmy( int numNinjas, bool momentumFlag, const double * speedList, const char * speedUnits, const double * directionList, char ** papszOptions )
{
    try
    {

#ifndef NINJAFOAM
        if(momentumFlag == true)
        {
            throw std::runtime_error("momentumFlag cannot be set to true. WindNinja was not compiled with mass and momentum support.");
        }
#endif

        if( numNinjas < 1 )
        {
            throw std::runtime_error(CPLSPrintf("Invalid input numNinjas '%d' in ninjaArmy::NinjaMakeDomainAverageArmy()", numNinjas));
        }

        //Get the number of elements in the arrays
        /*        size_t length1 = sizeof(speedList) / sizeof(speedList[0]);
        size_t length2 = sizeof(directionList) / sizeof(directionList[0]); */
        //        size_t length1 = sizeof(yearList) / sizeof(yearList[0]);
        //        size_t length2 = sizeof(monthList) / sizeof(monthList[0]);
        //        size_t length3 = sizeof(dayList) / sizeof(dayList[0]);
        //        size_t length4 = sizeof(hourList) / sizeof(hourList[0]);
        //        size_t length5 = sizeof(minuteList) / sizeof(minuteList[0]);
        //        size_t length6 = sizeof(airTempList) / sizeof(airTempList[0]);
        //        size_t length7 = sizeof(cloudCoverList) / sizeof(cloudCoverList[0]);
        //
        //        if(!(length1 == length2 == length3 == length4 == length5 == length6 == length7))
        //        {
        //            throw std::runtime_error("yearList, monthList, dayList, hourList, minuteList, airTempList, and cloudCoverList must be the same length!");
        //

        makeDomainAverageArmy( numNinjas, momentumFlag );

        int retval = NINJA_E_INVALID;
        for(int i=0; i<getSize(); i++)
        {
            retval = setInputSpeed( i, speedList[i], std::string( speedUnits ) );
            if( retval != NINJA_SUCCESS )
            {
                Com->ninjaCom(ninjaComClass::ninjaFailure, "ninjaArmy::setInputSpeed() called in ninjaArmy::NinjaMakeDomainAverageArmy() for ninja '%d' failed.", i);
                return retval;
            }

            retval = setInputDirection( i, directionList[i] );
            if( retval != NINJA_SUCCESS )
            {
                Com->ninjaCom(ninjaComClass::ninjaFailure, "ninjaArmy::setInputDirection() called in ninjaArmy::NinjaMakeDomainAverageArmy() for ninja '%d' failed.", i);
                return retval;
            }
        }
    }
    catch( armyException & e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( exception & e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( ... )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        return NINJA_E_INVALID;
    }

    return NINJA_SUCCESS;
}

int ninjaArmy::NinjaMakePointArmy( int * yearList, int * monthList, int * dayList, int * hourList, int * minuteList, int timeListSize, char * timeZone, const char ** stationFileNames, int numStationFiles, char * elevationFile, bool matchPointsFlag, bool momentumFlag, char ** papzOptions )
{
    try
    {
        if(momentumFlag == true)
        {
            throw std::runtime_error("The momentum solver is not available for use with Point Initialization runs.");
        }

        if( timeListSize < 1 )
        {
            throw std::runtime_error(CPLSPrintf("Invalid input timeListSize '%d' in ninjaArmy::NinjaMakePointArmy()", timeListSize));
        }
        if( numStationFiles < 1 )
        {
            throw std::runtime_error(CPLSPrintf("Invalid input numStationFiles '%d' in ninjaArmy::NinjaMakePointArmy()", numStationFiles));
        }

        wxStation::SetStationFormat(wxStation::newFormat);

        std::vector <boost::posix_time::ptime> timeList;
        for(size_t i=0; i < timeListSize; i++)
        {
            timeList.push_back(boost::posix_time::ptime(boost::gregorian::date(yearList[i], monthList[i], dayList[i]), boost::posix_time::time_duration(hourList[i],minuteList[i],0,0)));
        }

        std::vector<std::string> sFiles;
        for (int i = 0; i < numStationFiles; i++)
        {
            sFiles.emplace_back(stationFileNames[i]);
        }
        pointInitialization::storeFileNames(sFiles);
        CPLDebug("STATION_FETCH", "FILES STORED...");

        makePointArmy( timeList, std::string(timeZone), sFiles[0], std::string(elevationFile), matchPointsFlag, momentumFlag );
    }
    catch( armyException & e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( exception & e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( ... )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        return NINJA_E_INVALID;
    }

    return NINJA_SUCCESS;
}

int ninjaArmy::NinjaMakeWeatherModelArmy( const char * forecastFilename, const char * timeZone, const char** inputTimeList, int size, bool momentumFlag, char ** papszOptions )
{
    try
    {

#ifndef NINJAFOAM
        if(momentumFlag == true)
        {
            throw std::runtime_error("momentumFlag cannot be set to true. WindNinja was not compiled with mass and momentum support.");
        }
#endif

        // TODO: need to make a better error check for this than to disallow an empty inputTimeList
        // Need to setup a check to compare inputTimeList size to the input timeListSize
        if( size < 1 )
        {
            throw std::runtime_error(CPLSPrintf("Invalid input size '%d' in ninjaArmy::NinjaMakeWeatherModelArmy()", size));
        }

        wxModelInitialization *model = wxModelInitializationFactory::makeWxInitialization(std::string(forecastFilename));
        std::vector<blt::local_date_time> fullTimeList = model->getTimeList(std::string(timeZone));
        std::vector<blt::local_date_time> timeList;

        for(int i = 0; i < fullTimeList.size(); i++)
        {
            for(int j = 0; j < size; j++)
            {
                std::string time1 = fullTimeList[i].to_string();
                std::string time2(inputTimeList[j]);
                if(time1 == time2)
                {
                    timeList.push_back(fullTimeList[i]);
                }
            }
        }

        makeWeatherModelArmy( std::string( forecastFilename ), std::string( timeZone ), timeList, momentumFlag );
    }
    catch( armyException & e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( exception & e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( ... )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        return NINJA_E_INVALID;
    }

    return NINJA_SUCCESS;
}

/*-----------------------------------------------------------------------------
 *  Ninja Communication Methods
 *-----------------------------------------------------------------------------*/

int ninjaArmy::setNinjaComMessageHandler( ninjaComMessageHandler pMsgHandler, void *pUser,
                                          char ** papszOptions )
{
    try
    {
        Com->set_messageHandler(pMsgHandler, pUser);
    }
    catch( ... )
    {
        std::cerr << "CRITICAL: ninjaArmy level ninjaComMessageHandler not set. Messages will NOT be delivered." << std::endl;
        return NINJA_E_INVALID;
    }
    return NINJA_SUCCESS;
}

int ninjaArmy::setNinjaMultiComStream( FILE* stream,
                                       char ** papszOptions )
{
    try
    {
        Com->multiStream = stream;
    }
    catch( ... )
    {
        std::cerr << "ERROR: ninjaArmy level ninjaCom multiStream FILE pointer not set." << std::endl;
        return NINJA_E_INVALID;
    }
    return NINJA_SUCCESS;
}

int ninjaArmy::setNinjaCommunication( const int nIndex, const int RunNumber,
                                      char ** papszOptions )
{
    int retval = NINJA_E_INVALID;
    IF_VALID_INDEX( nIndex, ninjas )
    {
        try
        {
            ninjas[ nIndex ]->set_ninjaCommunication( Com );
            ninjas[ nIndex ]->set_ninjaComRunNumber( RunNumber );
            retval = NINJA_SUCCESS;
        }
        catch( std::exception &e )
        {
            Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s, Failed to set ninjas[%d] level ninjaCom", e.what(), RunNumber);
            retval = NINJA_E_INVALID;
        }
        catch( ... )
        {
            Com->ninjaCom(ninjaComClass::ninjaFailure, "Failed to set ninjas[%d] level ninjaCom", RunNumber);
            retval = NINJA_E_INVALID;
        }
    }
    return retval;
}

/*-----------------------------------------------------------------------------
 *  Ninja Speed Testing Methods
 *-----------------------------------------------------------------------------*/
#ifdef NINJA_SPEED_TESTING
int ninjaArmy::setSpeedDampeningRatio( const int nIndex, const double r,
                            char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_speedDampeningRatio( r ) );
}

int ninjaArmy::setDownDragCoeff( const int nIndex, const double coeff,
                            char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_downDragCoeff( coeff ) );
}

int ninjaArmy::setDownEntrainmentCoeff( const int nIndex, const double coeff,
                            char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_downEntrainmentCoeff( coeff ) );
}

int ninjaArmy::setUpDragCoeff( const int nIndex, const double coeff,
                            char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_upDragCoeff( coeff ) );
}

int ninjaArmy::setUpEntrainmentCoeff( const int nIndex, const double coeff,
                            char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_upEntrainmentCoeff( coeff ) );
}
#endif


/*-----------------------------------------------------------------------------
 *  Friciton Velocity Methods
 *-----------------------------------------------------------------------------*/
#ifdef FRICTION_VELOCITY
int ninjaArmy::setFrictionVelocityFlag( const int nIndex, const bool flag,
                            char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_frictionVelocityFlag( flag ) );
}

int ninjaArmy::setFrictionVelocityCalculationMethod( const int nIndex,
                                                    const std::string calcMethod,
                                                    char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_frictionVelocityCalculationMethod( calcMethod ) );
}
#endif //FRICTION_VELOCITY

/*-----------------------------------------------------------------------------
 *  Dust Methods
 *-----------------------------------------------------------------------------*/
#ifdef EMISSIONS
int ninjaArmy::setDustFilename( const int nIndex, const std::string filename,
                                char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_dustFilename( filename ) );
}

int ninjaArmy::setDustFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_dustFlag( flag ) );
}
#endif //EMISSIONS

#ifdef NINJAFOAM
/*-----------------------------------------------------------------------------
 *  NinjaFOAM Methods
 *-----------------------------------------------------------------------------*/
int ninjaArmy::setNumberOfIterations( const int nIndex, const int nIterations, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_NumberOfIterations( nIterations ) );
}
int ninjaArmy::setMeshCount( const int nIndex, const int meshCount, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_MeshCount( meshCount ) );
}
int ninjaArmy::setMeshCount( const int nIndex,
                             const WindNinjaInputs::eNinjafoamMeshChoice meshChoice,
                             char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_MeshCount( meshChoice ) );
}
int ninjaArmy::setExistingCaseDirectory( const int nIndex, const std::string directory, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_ExistingCaseDirectory( directory ) );
}
int ninjaArmy::setWriteTurbulenceFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_writeTurbulenceFlag( flag ) );
}
#endif
/*-----------------------------------------------------------------------------
 *  Forecast Model Methods
 *-----------------------------------------------------------------------------*/
int ninjaArmy::setWxModelFilename(const int nIndex, const std::string wx_filename, char ** papszOptions)
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_wxModelFilename( wx_filename ) );
}

/*-----------------------------------------------------------------------------
 *  Point Initializaiton Methods
 *-----------------------------------------------------------------------------*/
int ninjaArmy::setInputPointsFilename( const int nIndex, const std::string filename, char ** papszOptions)
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_inputPointsFilename( filename ) );
}

int ninjaArmy::setOutputPointsFilename( const int nIndex, const std::string filename, char **papszOptions)
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_outputPointsFilename( filename ) );
}

int ninjaArmy::readInputFile( const int nIndex, const std::string filename, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->readInputFile( filename ) ) ;
}

int ninjaArmy::readInputFile( const int nIndex, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->readInputFile() );
}

/*-----------------------------------------------------------------------------
 * Station Fetch Methods
 *-----------------------------------------------------------------------------*/
int ninjaArmy::setStationFetchFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_stationFetchFlag( flag ) );
}

/*-----------------------------------------------------------------------------
 *  Simulation Parameter Methods
 *-----------------------------------------------------------------------------*/
int ninjaArmy::ninjaInitialize()
{
    int retval = NINJA_E_INVALID;

    retval = NinjaInitialize();

    return retval;
}

int ninjaArmy::setDEM( const int nIndex, const std::string dem_filename, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_DEM( dem_filename ) );
}

int ninjaArmy::setDEM( const int nIndex, const double* demValues, const int nXSize,
                       const int nYSize, const double* geoRef, std::string prj, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_DEM( demValues, nXSize, nYSize,
                                                                   geoRef, prj ) );
}

int ninjaArmy::setPosition( const int nIndex, const double lat_degrees, const double lon_degrees,
                 char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_position( lat_degrees, lon_degrees ) );
}

int ninjaArmy::setPosition( const int nIndex, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_position() );
}

int ninjaArmy::setNumberCPUs( const int nIndex, const int nCPUs, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_numberCPUs( nCPUs ) );
}

int ninjaArmy::setSpeedInitGrid( const int nIndex, const std::string speedFile,
                                 const velocityUnits::eVelocityUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_speedFile( speedFile, units ) );
}

int ninjaArmy::setDirInitGrid( const int nIndex, const std::string dirFile, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_dirFile( dirFile ) );
}

int ninjaArmy::setInitializationMethod( const int nIndex,
                                        const WindNinjaInputs::eInitializationMethod  method,
                                        const bool matchPoints, char ** papszOptions )
{
    bool bMatch = false;
    if( method == WindNinjaInputs::pointInitializationFlag && matchPoints )
    {
        bMatch = true;
    }
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_initializationMethod( method, bMatch ) );
}

int ninjaArmy::setInitializationMethod( const int nIndex,
                                        std::string method,
                                        const bool matchPoints, char ** papszOptions )
{
    int retval = NINJA_E_INVALID;
    IF_VALID_INDEX( nIndex, ninjas )
    {
        std::transform( method.begin(), method.end(), method.begin(), ::tolower );
        if( method == "domain_average" || method == "domainAverage" ||
            method == "domainaverageinitializationflag" || method == "domain" )
        {
            ninjas[ nIndex ]->set_initializationMethod
                ( WindNinjaInputs::domainAverageInitializationFlag, matchPoints );
            retval = NINJA_SUCCESS;
        }
        else if( method == "point" || method == "pointinitializationflag" )
        {
            ninjas[ nIndex ]->set_initializationMethod
                ( WindNinjaInputs::pointInitializationFlag, matchPoints );
            retval = NINJA_SUCCESS;

        }
        else if( method == "wxmodel" || method == "wxmodelinitializationflag" )
        {
            ninjas[ nIndex ]->set_initializationMethod
                ( WindNinjaInputs::wxModelInitializationFlag, matchPoints );
            retval = NINJA_SUCCESS;
        }
        else if( method == "griddedInitialization" )
        {
            ninjas[ nIndex ]->set_initializationMethod
                ( WindNinjaInputs::griddedInitializationFlag, matchPoints );
            retval = NINJA_SUCCESS;
        }
#ifdef NINJAFOAM
        else if( method == "foamDomainAverageInitialization" )
        {
            ninjas[ nIndex ]->set_initializationMethod
                ( WindNinjaInputs::foamDomainAverageInitializationFlag, matchPoints );
            retval = NINJA_SUCCESS;
        }
#endif
        else
        {
#ifdef NINJAFOAM
            ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Invalid input initialization_method '%s' in ninjaArmy::setInitializationMethod()\nchoices are: 'domain_average', 'domainAverage', 'domainaverageinitializationflag', 'domain',\n'point', 'pointinitializationflag', 'wxmodel', 'wxmodelinitializationflag', 'griddedInitialization'", method.c_str());
#else
            ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Invalid input initialization_method '%s' in ninjaArmy::setInitializationMethod()\nchoices are: 'domain_average', 'domainAverage', 'domainaverageinitializationflag', 'domain',\n'point', 'pointinitializationflag', 'wxmodel', 'wxmodelinitializationflag', 'griddedInitialization', 'foamDomainAverageInitialization'", method.c_str());
#endif
            retval = NINJA_E_INVALID;
        }
    }
    return retval;
}
int ninjaArmy::setInputSpeed( const int nIndex, const double speed,
                              const velocityUnits::eVelocityUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_inputSpeed( speed, units ) );
}

int ninjaArmy::setInputSpeed( const int nIndex, const double speed,
                              std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       try
       {
           ninjas[ nIndex ]->set_inputSpeed( speed, velocityUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       catch( std::logic_error &e )
       {
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}

int ninjaArmy::setInputDirection( const int nIndex, const double direction, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_inputDirection( direction ) );
}
int ninjaArmy::setInputWindHeight( const int nIndex, const double height,
                        const lengthUnits::eLengthUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_inputWindHeight( height, units ) );
}

int ninjaArmy::setInputWindHeight( const int nIndex, const double height,
                                   std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       //Parse units so it contains only lowercase letters
       std::transform( units.begin(), units.end(), units.begin(), ::tolower );
       try
       {
           ninjas[ nIndex ]->set_inputWindHeight( height, lengthUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       /*catch( std::range_error &e )
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
           retval = NINJA_E_INVALID;
       }*/
       //catch( std::logic_error &e )
       catch( std::exception &e )
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}

int ninjaArmy::setInputWindHeight( const int nIndex, const double height, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_inputWindHeight( height ) );
}

int ninjaArmy::setOutputWindHeight( const int nIndex, const double height,
                         const lengthUnits::eLengthUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_outputWindHeight( height, units ) );
}

int ninjaArmy::setOutputWindHeight( const int nIndex, const double height,
                                               std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       //Parse units so it contains only lowercase letters
       std::transform( units.begin(), units.end(), units.begin(), ::tolower );
       try
       {
           ninjas[ nIndex ]->set_outputWindHeight( height, lengthUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       /*catch( std::range_error &e )
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
           retval = NINJA_E_INVALID;
       }*/
       //catch( std::logic_error &e )
       catch( std::exception &e )
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}

//int ninjaArmy::setOutputWindHeight( const int nIndex, const double height,
//                                    std::string units, char ** papszOptions )
//{
//    //Parse units so it contains only lowercase letters
//    std::transform( units.begin(), units.end(), units.begin(), ::tolower );
//
//    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_outputWindHeight( height, lengthUnits::getUnit( units ) ) );
//}

int ninjaArmy::setOutputSpeedUnits( const int nIndex, const velocityUnits::eVelocityUnits units,
                             char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_outputSpeedUnits( units ) );
}

int ninjaArmy::setOutputSpeedUnits( const int nIndex, std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       //Parse units so it contains only lowercase letters
       std::transform( units.begin(), units.end(), units.begin(), ::tolower );
       try
       {
           ninjas[ nIndex ]->set_outputSpeedUnits( velocityUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       catch( std::logic_error &e )
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}



int ninjaArmy::setDiurnalWinds( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_diurnalWinds( flag ) );
}

int ninjaArmy::setUniAirTemp( const int nIndex, const double temp,
                   const temperatureUnits::eTempUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_uniAirTemp( temp, units ) );
}

int ninjaArmy::setUniAirTemp( const int nIndex, const double temp,
                              std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       try
       {
           ninjas[ nIndex ]->set_uniAirTemp( temp, temperatureUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       catch( std::logic_error &e )
       {
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}

int ninjaArmy::setUniCloudCover( const int nIndex, const double cloud_cover,
                      const coverUnits::eCoverUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_uniCloudCover( cloud_cover, units ) );
}

int ninjaArmy::setUniCloudCover( const int nIndex, const double cloud_cover,
                                 std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       //Parse units so it contains only lowercase letters
       try
       {
           ninjas[ nIndex ]->set_uniCloudCover( cloud_cover, coverUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       catch( std::logic_error &e )
       {
           retval = NINJA_E_INVALID;
       }
       catch( std::range_error &e )
       {
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}

int ninjaArmy::setDateTime( const int nIndex, int const &yr, int const &mo, int const &day,
                 int const &hr, int const &min, int const &sec,
                 std::string const &timeZoneString, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_date_time( yr, mo, day, hr, min, sec, timeZoneString ) );
}

int ninjaArmy::setWxStationFilename( const int nIndex, const std::string station_filename,
                          char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_wxStationFilename( station_filename ) );
}

std::vector<wxStation> ninjaArmy::getWxStations( const int nIndex, char ** papszOptions )
{
    IF_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_wxStations();
    }
    std::vector<wxStation> none;
    return none; //if invalid index
}

int ninjaArmy::setUniVegetation( const int nIndex,
                                 const WindNinjaInputs::eVegetation vegetation_,
                                 char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_uniVegetation( vegetation_ ) );
}

int ninjaArmy::setUniVegetation( const int nIndex, std::string vegetation,
                                 char ** papszOptions )
{
    int retval = NINJA_E_INVALID;
    IF_VALID_INDEX( nIndex, ninjas )
    {
        std::transform( vegetation.begin(), vegetation.end(), vegetation.begin(), ::tolower );
        if( vegetation == "grass" || vegetation == "g" )
        {
            ninjas[ nIndex ]->set_uniVegetation( WindNinjaInputs::grass );
            retval = NINJA_SUCCESS;
        }
        else if( vegetation == "brush" || vegetation == "b" )
        {
            ninjas[ nIndex ]->set_uniVegetation( WindNinjaInputs::brush );
            retval = NINJA_SUCCESS;
        }
        else if( vegetation == "trees" || vegetation == "t" )
        {
            ninjas[ nIndex ]->set_uniVegetation( WindNinjaInputs::trees );
            retval = NINJA_SUCCESS;
        }
        else
        {
            ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Invalid input vegation '%s' in ninjaArmy::setUniVegetation()\nchoices are: 'grass', 'g', 'brush', 'b', 'trees', 't'", vegetation.c_str());
            retval = NINJA_E_INVALID;
        }
    }
    return retval;

}

int ninjaArmy::setUniVegetation( const int nIndex, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_uniVegetation() );
}
int ninjaArmy::setMeshResolutionChoice( const int nIndex, const std::string choice,
                                        char ** papszOptions )
{
#ifndef NINJAFOAM
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_meshResChoice( choice ) );
#else
    int retval = NINJA_E_INVALID;
    IF_VALID_INDEX( nIndex, ninjas )
    {
        try
        {
            if( ninjas[ nIndex ]->identify() == "ninja" )
            {
                ninjas[ nIndex ]->set_meshResChoice( choice );
                retval = NINJA_SUCCESS;
            } else if( ninjas[ nIndex ]->identify() == "ninjafoam" )
            {
                ninjas[ nIndex ]->set_MeshCount( ninja::get_eNinjafoamMeshChoice(choice) );
                retval = NINJA_SUCCESS;
            }
            else
            {
                throw std::invalid_argument( "invalid ninja->identify() '" + choice +
                                             "' in ninjaArmy::setMeshResolutionChoice()" +
                                             "\nshould be: 'ninja' or 'ninjafoam'" );
            }
        }
        catch( std::logic_error &e )
        {
            ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
            retval = NINJA_E_INVALID;
        }
    }
    return retval;
#endif
}

int ninjaArmy::setMeshResolutionChoice( const int nIndex, const Mesh::eMeshChoice choice,
                                        char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_meshResChoice( choice ) );
}

int ninjaArmy::setMeshResolution( const int nIndex, const double resolution,
                                   const lengthUnits::eLengthUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_meshResolution( resolution, units ) );
}

int ninjaArmy::setMeshResolution( const int nIndex, const double resolution,
                                  std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       //Parse units so it contains only lowercase letters
       std::transform( units.begin(), units.end(), units.begin(), ::tolower );
       try
       {
           ninjas[ nIndex ]->set_meshResolution( resolution, lengthUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       catch( std::logic_error &e )
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}

int ninjaArmy::setNumVertLayers( const int nIndex, const int nLayers, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_numVertLayers( nLayers ) );
}
/*  Accesors  */

bool ninjaArmy::getDiurnalWindFlag( const int nIndex, char ** papszOptions )
{
    IF_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_diurnalWindFlag();
    }
    return false; //if not a valid index
}

WindNinjaInputs::eInitializationMethod ninjaArmy::getInitializationMethod
    ( const int nIndex, char ** papszOptions )
{
    IF_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_initializationMethod();
    }
    return WindNinjaInputs::noInitializationFlag; //if not a valid index
}

std::string ninjaArmy::getInitializationMethodString( const int nIndex,
                                                      char ** papszOptions )
{
    std::string retstr = "";
    IF_VALID_INDEX( nIndex, ninjas )
    {
        WindNinjaInputs::eInitializationMethod method =
            ninjas[ nIndex ]->get_initializationMethod();
        if( method == WindNinjaInputs::noInitializationFlag )
        {
           retstr = "noInitializationFlag";
        }
        else if( method == WindNinjaInputs::domainAverageInitializationFlag )
        {
           retstr = "domainAverageInitializationFlag";
        }
        else if( method == WindNinjaInputs::pointInitializationFlag )
        {
           retstr = "pointInitializationFlag";
        }
        else if( method == WindNinjaInputs::wxModelInitializationFlag )
        {
            retstr = "wxModelInitializationFlag";
        }
    }
    return retstr;
}

/*-----------------------------------------------------------------------------
 *  STABILITY section
 *-----------------------------------------------------------------------------*/
int ninjaArmy::setStabilityFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_stabilityFlag( flag ) );
}
int ninjaArmy::setAlphaStability( const int nIndex, const double stability_,
                                  char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_alphaStability( stability_ ) );
}
/*-----------------------------------------------------------------------------
 *  Output Parameter Methods
 *-----------------------------------------------------------------------------*/
int ninjaArmy::setOutputPath( const int nIndex, std::string path,
                                 char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_outputPath( path ) );
}
int ninjaArmy::setOutputSpeedGridResolution( const int nIndex, const double resolution,
                                            const lengthUnits::eLengthUnits units,
                                            char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_outputSpeedGridResolution( resolution, units ) );
}

int ninjaArmy::setOutputSpeedGridResolution( const int nIndex, const double resolution,
                                  std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       //Parse units so it contains only lowercase letters
       std::transform( units.begin(), units.end(), units.begin(), ::tolower );
       try
       {
           ninjas[ nIndex ]->set_outputSpeedGridResolution( resolution, lengthUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       catch( std::logic_error &e )
       {
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}

int ninjaArmy::setOutputDirectionGridResolution( const int nIndex, const double resolution,
                                                 const lengthUnits::eLengthUnits units,
                                                 char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_outputDirectionGridResolution( resolution, units ) );
}

int ninjaArmy::setOutputDirectionGridResolution( const int nIndex, const double resolution,
                                                 std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       //Parse units so it contains only lowercase letters
       std::transform( units.begin(), units.end(), units.begin(), ::tolower );
       try
       {
           ninjas[ nIndex ]->set_outputDirectionGridResolution( resolution, lengthUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       catch( std::logic_error &e )
       {
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}


const double* ninjaArmy::getOutputSpeedGrid( const int nIndex, char** papszOptions)
{
    CHECK_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_outputSpeedGrid( );
        return ninjas[ nIndex ]->get_outputSpeedGrid( );
    }
}

const double* ninjaArmy::getOutputDirectionGrid( const int nIndex, char** papszOptions )
{
    CHECK_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[nIndex]->get_outputDirectionGrid( );
    }
}
const char* ninjaArmy::getOutputGridProjection( const int nIndex, char ** papszOptions )
{
    CHECK_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_outputGridProjection( );
    }
}
const double ninjaArmy::getOutputGridCellSize( const int nIndex, char ** papszOptions )
{
    CHECK_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_outputGridCellSize( );
    }
}
const double ninjaArmy::getOutputGridxllCorner( const int nIndex, char ** papszOptions )
{
    CHECK_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_outputGridxllCorner( );
    }
}
const double ninjaArmy::getOutputGridyllCorner( const int nIndex, char ** papszOptions )
{
    CHECK_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_outputGridyllCorner( );
    }
}
const int ninjaArmy::getOutputGridnCols( const int nIndex, char ** papszOptions )
{
    CHECK_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_outputGridnCols( );
    }
}
const int ninjaArmy::getOutputGridnRows( const int nIndex, char ** papszOptions )
{
    CHECK_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_outputGridnRows( );
    }
}
int ninjaArmy::setOutputBufferClipping( const int nIndex, const double percent,
                                        char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_outputBufferClipping( percent ) );
}
int ninjaArmy::setWxModelGoogOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_wxModelGoogOutFlag( flag ) );
}
int ninjaArmy::setWxModelShpOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_wxModelShpOutFlag( flag ) );
}
int ninjaArmy::setWxModelAsciiOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_wxModelAsciiOutFlag( flag ) );
}
int ninjaArmy::setWxModelFgbzOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
                       ninjas[ nIndex ]->set_wxModelFgbzOutFlag( flag ) );
}
int ninjaArmy::setWxModelGeoTiffOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_wxModelGeoTiffOutFlag( flag ) );
}
int ninjaArmy::setGoogOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_googOutFlag( flag ) );
}
int ninjaArmy::setGoogResolution( const int nIndex, const double resolution,
                       const lengthUnits::eLengthUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_googResolution( resolution, units ) );
}
int ninjaArmy::setGoogColor(const int nIndex, string colorScheme, bool scaling)
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[nIndex]->set_googColor(colorScheme, scaling));
}
int ninjaArmy::setGoogConsistentColorScale(const int nIndex, bool flag, int numRuns)
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[nIndex]->set_googConsistentColorScale(flag, numRuns));
}
int ninjaArmy::setGoogResolution( const int nIndex, const double resolution,
                                  std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       //Parse units so it contains only lowercase letters
       std::transform( units.begin(), units.end(), units.begin(), ::tolower );
       try
       {
           ninjas[ nIndex ]->set_googResolution( resolution, lengthUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       catch( std::logic_error &e )
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}

int ninjaArmy::setGoogSpeedScaling
    ( const int nIndex, const KmlVector::egoogSpeedScaling scaling,
      char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_googSpeedScaling( scaling ) );
}

int ninjaArmy::setGoogSpeedScaling
    ( const int nIndex, std::string scaling, char ** papszOptions )
{
    int retval = NINJA_E_INVALID;
    IF_VALID_INDEX( nIndex, ninjas )
    {
       if( scaling == "equal_color" || scaling == "color" )
       {
           ninjas[ nIndex ]->set_googSpeedScaling( KmlVector::equal_color );
           retval = NINJA_SUCCESS;
       }
       else if( scaling == "equal_interval" || scaling == "interval" )
       {
           ninjas[ nIndex ]->set_googSpeedScaling( KmlVector::equal_interval );
           retval = NINJA_SUCCESS;
       }
       else
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Invalid speed scale '%s' in ninjaArmy::setGoogSpeedScaling()\nchoices are: 'equal_color', 'color', 'equal_interval', 'interval'", scaling.c_str());
           retval = NINJA_E_INVALID;
       }
    }
    return retval;
}

int ninjaArmy::setGoogLineWidth( const int nIndex, const double width,
                                 char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_googLineWidth( width ) );
}

int ninjaArmy::setShpOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_shpOutFlag( flag ) );
}
int ninjaArmy::setShpResolution( const int nIndex, const double resolution,
                      const lengthUnits::eLengthUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_shpResolution( resolution, units ) );
}

int ninjaArmy::setShpResolution( const int nIndex, const double resolution,
                                 std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       //Parse units so it contains only lowercase letters
       std::transform( units.begin(), units.end(), units.begin(), ::tolower );
       try
       {
           ninjas[ nIndex ]->set_shpResolution( resolution, lengthUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       catch( std::logic_error &e )
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}

int ninjaArmy::setAsciiOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_asciiOutFlag( flag ) );
}
int ninjaArmy::setAsciiAaigridOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_asciiAaigridOutFlag( flag ) );
}
int ninjaArmy::setAsciiJsonOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_asciiJsonOutFlag( flag ) );
}
int ninjaArmy::setAsciiProjOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_asciiProjOutFlag( flag ) );
}
int ninjaArmy::setAsciiGeogOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_asciiGeogOutFlag( flag ) );
}
int ninjaArmy::setAsciiUvOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_asciiUvOutFlag( flag ) );
}
int ninjaArmy::setAsciiResolution( const int nIndex, const double resolution,
                        const lengthUnits::eLengthUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_asciiResolution( resolution, units ) );
}
int ninjaArmy::setAsciiResolution( const int nIndex, const double resolution,
                                   std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       //Parse units so it contains only lowercase letters
       std::transform( units.begin(), units.end(), units.begin(), ::tolower );
       try
       {
           ninjas[ nIndex ]->set_asciiResolution( resolution, lengthUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       catch( std::logic_error &e )
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}
int ninjaArmy::setGeoTiffOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_geoTiffOutFlag( flag ) );
}
int ninjaArmy::setGeoTiffResolution( const int nIndex, const double resolution,
                                     const lengthUnits::eLengthUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_geoTiffResolution( resolution, units ) );
}
int ninjaArmy::setGeoTiffResolution( const int nIndex, const double resolution,
                                     std::string units, char ** papszOptions )
{
    int retval = NINJA_E_INVALID;
    IF_VALID_INDEX( nIndex, ninjas )
    {
        //Parse units so it contains only lowercase letters
        std::transform( units.begin(), units.end(), units.begin(), ::tolower );
        try
        {
            ninjas[ nIndex ]->set_geoTiffResolution( resolution, lengthUnits::getUnit( units ) );
            retval = NINJA_SUCCESS;
        }
        catch( std::logic_error &e )
        {
            ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
            retval = NINJA_E_INVALID;
        }
    }
    return retval;
}
int ninjaArmy::setAtmOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_atmOutFlag( flag ) );
}
int ninjaArmy::setVtkOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_vtkOutFlag( flag ) );
}

int ninjaArmy::setTxtOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_txtOutFlag( flag ) );
}
//PDF
int ninjaArmy::setPDFOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_pdfOutFlag( flag ) );
}
int ninjaArmy::setPDFResolution( const int nIndex, const double resolution,
                       const lengthUnits::eLengthUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_pdfResolution( resolution, units ) );
}

int ninjaArmy::setPDFLineWidth( const int nIndex, const float linewidth, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_pdfLineWidth( linewidth ) );
}


int ninjaArmy::setPDFResolution( const int nIndex, const double resolution,
                                  std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       //Parse units so it contains only lowercase letters
       std::transform( units.begin(), units.end(), units.begin(), ::tolower );
       try
       {
           ninjas[ nIndex ]->set_pdfResolution( resolution, lengthUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       catch( std::logic_error &e )
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}

int ninjaArmy::setPDFBaseMap( const int nIndex,
                              const int eType )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[nIndex]->set_pdfBaseMap( eType ) );
}

int ninjaArmy::setPDFDEM
( const int nIndex, const std::string dem_filename, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_pdfDEM( dem_filename ) );
}

int ninjaArmy::setPDFSize( const int nIndex, const double height, const double width,
                           const unsigned short dpi )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[nIndex]->set_pdfSize( height, width, dpi ));
}

int ninjaArmy::setFgbzOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_fgbzOutFlag( flag ) );
}

int ninjaArmy::setFgbzResolution( const int nIndex, const double resolution,
                                  const lengthUnits::eLengthUnits units, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_fgbzResolution( resolution, units ) );
}

int ninjaArmy::setFgbzResolution( const int nIndex, const double resolution,
                                  std::string units, char ** papszOptions )
{
   int retval = NINJA_E_INVALID;
   IF_VALID_INDEX( nIndex, ninjas )
   {
       //Parse units so it contains only lowercase letters
       std::transform( units.begin(), units.end(), units.begin(), ::tolower );
       try
       {
           ninjas[ nIndex ]->set_fgbzResolution( resolution, lengthUnits::getUnit( units ) );
           retval = NINJA_SUCCESS;
       }
       catch( std::logic_error &e )
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}

int ninjaArmy::setFgbzSpeedScaling( const int nIndex, const OutputWriter::eSpeedScaling scaling,
                                    char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_fgbzSpeedScaling( scaling ) );
}

int ninjaArmy::setFgbzSpeedScaling( const int nIndex, std::string scaling, char ** papszOptions )
{
    int retval = NINJA_E_INVALID;
    IF_VALID_INDEX( nIndex, ninjas )
    {
       if( scaling == "equal_color" || scaling == "color" )
       {
           ninjas[ nIndex ]->set_fgbzSpeedScaling( OutputWriter::equal_color );
           retval = NINJA_SUCCESS;
       }
       else if( scaling == "equal_interval" || scaling == "interval" )
       {
           ninjas[ nIndex ]->set_fgbzSpeedScaling( OutputWriter::equal_interval );
           retval = NINJA_SUCCESS;
       }
       else
       {
           ninjas[ nIndex ]->input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Invalid speed scale '%s' in ninjaArmy::setFgbzSpeedScaling()\nchoices are: 'equal_color', 'color', 'equal_interval', 'interval'", scaling.c_str());
           retval = NINJA_E_INVALID;
       }
    }
    return retval;
}

int ninjaArmy::setFgbzColor(const int nIndex, string colorScheme, bool scaling)
{
    IF_VALID_INDEX_TRY(nIndex, ninjas, ninjas[nIndex]->set_fgbzColor(colorScheme, scaling));
}

int ninjaArmy::setFgbzLineWidth( const int nIndex, const double width, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_fgbzLineWidth( width ) );
}

int ninjaArmy::setFgbzConsistentColorScale(const int nIndex, bool flag, int numRuns)
{
    IF_VALID_INDEX_TRY(nIndex, ninjas, ninjas[nIndex]->set_fgbzConsistentColorScale(flag, numRuns));
}

std::string ninjaArmy::getOutputPath( const int nIndex, char ** papszOptions )
{
    IF_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_outputPath();
    }
    return std::string("");
}

int ninjaArmy::getMapVisualizationFilenames( std::vector<std::string>& fgbzFilenamesStr, std::vector<std::string>& stationKmlFilenamesStr,
                                             std::vector<std::string>& wxModelFgbzFilenameStr, char ** papszOptions )
{
    fgbzFilenamesStr = fgbzFilenames;
    stationKmlFilenamesStr = stationKmlFilenames;
    wxModelFgbzFilenameStr = wxModelFgbzFilenames;

    return NINJA_SUCCESS;
}

/**
 * @brief Reset the army in able to reinitialize needed parameters
 *
 */
void ninjaArmy::reset()
{
    ninjas.clear();
}

void ninjaArmy::cancel()
{
    //FOR_EVERY( iter_ninja, ninjas )
    for(unsigned int i = 0; i < ninjas.size(); i++)
    {
        ninjas[i]->cancel = true;
    }
}

void ninjaArmy::cancelAndReset()
{
    cancel();
    reset();
}

void ninjaArmy::setCurrentMapVisualizationFilenames(int runNumber)
{
    fgbzFilenames[runNumber] = ninjas[runNumber]->input.fgbzFile;

    if(ninjas[runNumber]->input.stations.size() == 0)
    {
        stationKmlFilenames[runNumber] = "";
    } else
    {
        if(ninjas[runNumber]->input.stations[runNumber].stationKmlNames.size() == 0)
        {
            stationKmlFilenames[runNumber] = "";
        } else
        {
            // all these cout statements are various ways to access the given wxStation::stationKmlNames, which is SHARED across ninjas[runNumber].input.stations[runNumber]
            // the past attempt of doing a for loop over ninjas[runNumber]->input.stations[runNumber].stationKmlNames[stationIdx] makes NO sense, it results in DUPLICATION
            //std::cout << "ninjas[0]->input.stations[0].stationKmlNames[" << runNumber << "] = \"" << ninjas[0]->input.stations[0].stationKmlNames[runNumber] << "\"" << std::endl;
            //std::cout << "ninjas[" << runNumber << "]->input.stations[" << runNumber << "].stationKmlNames[" << runNumber << "] = \"" << ninjas[runNumber]->input.stations[runNumber].stationKmlNames[runNumber] << "\"" << std::endl;
            //std::cout << "wxStation::stationKmlNames[" << runNumber << "] = \"" << wxStation::stationKmlNames[runNumber] << "\"" << std::endl;
            stationKmlFilenames[runNumber] = ninjas[runNumber]->input.stations[runNumber].stationKmlNames[runNumber];
        }
    }

    // oh, this one is set to "!set" for non-wxModel runs, the storage of this filename always exists for each ninjas[i]
    if(ninjas[runNumber]->input.wxModelFgbzFile == "!set")
    {
        wxModelFgbzFilenames[runNumber] = "";
    } else
    {
        wxModelFgbzFilenames[runNumber] = ninjas[runNumber]->input.wxModelFgbzFile;
    }
}

void ninjaArmy::calcConsistentColorScaleSplits(const AsciiGrid<double>* const *inSpdGrids, const int nSets, double **outSplitVals, int *outSize, eArmySpeedScaling scaling)
{
    // make sure this value matches that of OutputWriter and KmlVector, or there will be problems using the calculated splitVals
    int numSplits = 6;

    *outSize = numSplits;
    *outSplitVals = new double[numSplits];

    switch(scaling)
    {
        case equal_color:  // divide legend speeds using equal color method (equal numbers of arrows for each color)
        {
            std::vector<double> combinedGridValues;
            size_t nMaxCells = static_cast<size_t>(inSpdGrids[0]->get_nRows()) * static_cast<size_t>(inSpdGrids[0]->get_nCols()) * static_cast<size_t>(nSets);
            combinedGridValues.reserve(nMaxCells);
            for(int j = 0; j < nSets; j++)
            {
                const AsciiGrid<double>* current_grid = inSpdGrids[j];
                for(int rowIdx = 0; rowIdx < current_grid->get_nRows(); rowIdx++)
                {
                    for(int colIdx = 0; colIdx < current_grid->get_nCols(); colIdx++)
                    {
                        double current_val = current_grid->get_cellValue(rowIdx, colIdx);
                        if(current_val != current_grid->get_noDataValue() && !std::isnan(current_val))
                        {
                            combinedGridValues.push_back(current_val);
                        }
                    }
                }
            }

            std::sort(combinedGridValues.begin(), combinedGridValues.end());

            size_t step = combinedGridValues.size() / static_cast<size_t>(numSplits - 1);

            // do the final value as the exact last index
            for(int i = 0; i < numSplits - 1; i++)
            {
                (*outSplitVals)[i] = combinedGridValues[i * step];
            }
            (*outSplitVals)[numSplits - 1] = combinedGridValues.back();

            // if you want to use the actual minVal as the first splitVal, comment this line out
            (*outSplitVals)[0] = 0.0;

            break;
        }
        case equal_interval:  // divide legend speeds using equal interval method (speed breaks divided equally over speed range)
        {
            double minVal = 9999;
            double maxVal = -9999;
            for(int j = 0; j < nSets; j++)
            {
                double current_minVal = inSpdGrids[j]->get_minValue();
                double current_maxVal = inSpdGrids[j]->get_maxValue();
                if( current_minVal < minVal )
                {
                    minVal = current_minVal;
                }
                if( current_maxVal > maxVal )
                {
                    maxVal = current_maxVal;
                }
            }

            double interval = maxVal / (float)(numSplits-1);
            //double interval = (maxVal - minVal) / (float)(numSplits-1);
            for(int i = 0; i < numSplits; i++)
            {
                (*outSplitVals)[i] = i * interval;
                //(*outSplitVals)[i] = i * interval + minVal;
            }
            break;
        }
        default:  // divide legend speeds using equal color method (equal numbers of arrows for each color)
        {
            std::vector<double> combinedGridValues;
            size_t nMaxCells = static_cast<size_t>(inSpdGrids[0]->get_nRows()) * static_cast<size_t>(inSpdGrids[0]->get_nCols()) * static_cast<size_t>(nSets);
            combinedGridValues.reserve(nMaxCells);
            for(int j = 0; j < nSets; j++)
            {
                const AsciiGrid<double>* current_grid = inSpdGrids[j];
                for(int rowIdx = 0; rowIdx < current_grid->get_nRows(); rowIdx++)
                {
                    for(int colIdx = 0; colIdx < current_grid->get_nCols(); colIdx++)
                    {
                        double current_val = current_grid->get_cellValue(rowIdx, colIdx);
                        if(current_val != current_grid->get_noDataValue() && !std::isnan(current_val))
                        {
                            combinedGridValues.push_back(current_val);
                        }
                    }
                }
            }

            std::sort(combinedGridValues.begin(), combinedGridValues.end());

            size_t step = combinedGridValues.size() / static_cast<size_t>(numSplits - 1);

            // do the final value as the exact last index
            for(int i = 0; i < numSplits - 1; i++)
            {
                (*outSplitVals)[i] = combinedGridValues[i * step];
            }
            (*outSplitVals)[numSplits - 1] = combinedGridValues.back();

            // if you want to use the actual minVal as the first splitVal, comment this line out
            (*outSplitVals)[0] = 0.0;

            break;
        }
    }
}

void ninjaArmy::writeConsistentColorScaleOutputs()
{
    ninjas[ninjas.size()-1]->input.Com->ninjaCom(ninjaComClass::ninjaNone, "Writing consistent color scale output files...");

    // ensure grids cover original DEM extents, for FLAMMAP, and for all simulation outputs
    // if output clipping was set by the user, don't buffer to overlap the DEM
    AsciiGrid<double> demGrid;
    if(ninjas[0]->input.outputBufferClipping <= 0.0)
    {
        GDALDatasetH hDS;
        hDS = GDALOpen(ninjas[0]->input.dem.fileName.c_str(), GA_ReadOnly);
        if(hDS == NULL)
        {
            ninjas[0]->input.Com->ninjaCom(ninjaComClass::ninjaNone, "Problem reading DEM during output file writing.");
        }
        GDAL2AsciiGrid((GDALDataset*)hDS, 1, demGrid);
        GDALClose(hDS);
    }

    if(ninjas[0]->input.googUseConsistentColorScale == true)
    {
        AsciiGrid<double> **resampledVelGrids = new AsciiGrid<double>*[ninjas.size()];
        for(int i = 0; i < ninjas.size(); i++)
        {
            resampledVelGrids[i] = new AsciiGrid<double> (ninjas[i]->VelocityGrid.resample_Grid(ninjas[i]->input.kmzResolution, AsciiGrid<double>::order0));
            if(ninjas[0]->input.outputBufferClipping <= 0.0)
            {
                resampledVelGrids[i]->BufferToOverlapGrid(demGrid);
            }
        }

        eArmySpeedScaling speedScaling = ninjaArmy::equal_interval;
        if(ninjas[0]->input.googSpeedScaling == KmlVector::equal_color)
        {
            speedScaling = ninjaArmy::equal_color;
        }
        if(ninjas[0]->input.googSpeedScaling == KmlVector::equal_interval)
        {
            speedScaling = ninjaArmy::equal_interval;
        }

        int numSplits;
        double *finalSpeedSplitVals = nullptr;
        calcConsistentColorScaleSplits(resampledVelGrids, ninjas.size(), &finalSpeedSplitVals, &numSplits, speedScaling);

        for(int i = 0; i < ninjas.size(); i++)
        {
            KmlVector ninjaKmlFiles;

            AsciiGrid<double> *angTempGrid = new AsciiGrid<double> (ninjas[i]->AngleGrid.resample_Grid(ninjas[i]->input.kmzResolution, AsciiGrid<double>::order0));
            AsciiGrid<double>* velTempGrid = resampledVelGrids[i];

            if(ninjas[0]->input.outputBufferClipping <= 0.0)
            {
                angTempGrid->BufferToOverlapGrid(demGrid);
            }

            #ifdef NINJAFOAM
            AsciiGrid<double> *turbTempGrid = nullptr;
            AsciiGrid<double> *colMaxTempGrid = nullptr;
            if(ninjas[i]->input.writeTurbulence)
            {
                //turbTempGrid = new AsciiGrid<double> (ninjas[i]->TurbulenceGrid.resample_Grid(ninjas[i]->input.kmzResolution, AsciiGrid<double>::order0));
                colMaxTempGrid = new AsciiGrid<double> (ninjas[i]->colMaxGrid.resample_Grid(ninjas[i]->input.kmzResolution, AsciiGrid<double>::order0));

                if(ninjas[0]->input.outputBufferClipping <= 0.0)
                {
                    //turbTempGrid->BufferToOverlapGrid(demGrid);
                    colMaxTempGrid->BufferToOverlapGrid(demGrid);
                }

                //ninjaKmlFiles.setTurbulenceGrid(*turbTempGrid, ninjas[i]->input.outputSpeedUnits);
                ninjaKmlFiles.setColMaxGrid(*colMaxTempGrid, ninjas[i]->input.outputSpeedUnits,  ninjas[i]->input.colMax_colHeightAGL, ninjas[i]->input.colMax_colHeightAGL_units);
            }
            #endif //NINJAFOAM
            #ifdef FRICTION_VELOCITY
            AsciiGrid<double> *ustarTempGrid = nullptr;
            if(ninjas[i]->input.frictionVelocityFlag == 1 && ninjas[i]->identify() == "ninja")
            {
                ustarTempGrid = new AsciiGrid<double> (ninjas[i]->UstarGrid.resample_Grid(ninjas[i]->input.kmzResolution, AsciiGrid<double>::order0));
                if(ninjas[0]->input.outputBufferClipping <= 0.0)
                {
                    ustarTempGrid->BufferToOverlapGrid(demGrid);
                }
                ninjaKmlFiles.setUstarGrid(*ustarTempGrid);
            }
            #endif //FRICTION_VELOCITY
            #ifdef EMISSIONS
            AsciiGrid<double> *dustTempGrid = nullptr;
            if(ninjas[i]->input.dustFlag == 1 && ninjas[i]->identify() == "ninja")
            {
                dustTempGrid = new AsciiGrid<double> (ninjas[i]->DustGrid.resample_Grid(ninjas[i]->input.kmzResolution, AsciiGrid<double>::order0));
                if(ninjas[0]->input.outputBufferClipping <= 0.0)
                {
                    dustTempGrid->BufferToOverlapGrid(demGrid);
                }
                ninjaKmlFiles.setDustGrid(*dustTempGrid);
            }
            #endif //EMISSIONS

            ninjaKmlFiles.setKmlFile(ninjas[i]->input.kmlFile);
            ninjaKmlFiles.setKmzFile(ninjas[i]->input.kmzFile);

            ninjaKmlFiles.setLegendFile(ninjas[i]->input.legFile);
            ninjaKmlFiles.setDateTimeLegendFile(ninjas[i]->input.dateTimeLegFile, ninjas[i]->input.ninjaTime);
            ninjaKmlFiles.setSpeedGrid(*velTempGrid, ninjas[i]->input.outputSpeedUnits);
            ninjaKmlFiles.setAngleFromNorth(ninjas[i]->input.dem.getAngleFromNorth());
            ninjaKmlFiles.setDirGrid(*angTempGrid);

            ninjaKmlFiles.setSpeedScaling(ninjas[i]->input.googSpeedScaling);
            ninjaKmlFiles.setColorScheme(ninjas[i]->input.googColor);
            ninjaKmlFiles.setVectorScaling(ninjas[i]->input.googVectorScale);
            ninjaKmlFiles.setLineWidth(ninjas[i]->input.googLineWidth);
            ninjaKmlFiles.setTime(ninjas[i]->input.ninjaTime);
            if(ninjas[i]->input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag)
            {
                std::vector<boost::local_time::local_date_time> times(ninjas[i]->init->getTimeList(ninjas[i]->input.ninjaTimeZone));
                ninjaKmlFiles.setWxModel(ninjas[i]->init->getForecastIdentifier(), times[0]);
            }

            ninjaKmlFiles.setSpeedSplitVals(finalSpeedSplitVals, numSplits);

            if(ninjaKmlFiles.writeKml())
            {
                if(ninjaKmlFiles.makeKmz())
                {
                    ninjaKmlFiles.removeKmlFile();
                }
            }

            delete angTempGrid;
            angTempGrid = nullptr;
            #ifdef NINJAFOAM
            delete turbTempGrid;
            turbTempGrid = nullptr;
            delete colMaxTempGrid;
            colMaxTempGrid = nullptr;
            #endif //NINJAFOAM
            #ifdef FRICTION_VELOCITY
            delete ustarTempGrid;
            ustarTempGrid = nullptr;
            #endif //FRICTION_VELOCITY
            #ifdef EMISSIONS
            delete dustTempGrid;
            dustTempGrid = nullptr;
            #endif //EMISSIONS
        }

        //cleanup
        for(int i = 0; i < ninjas.size(); i++)
        {
            delete resampledVelGrids[i];
            resampledVelGrids[i] = nullptr;
        }
        delete[] resampledVelGrids;
        resampledVelGrids = nullptr;

        delete[] finalSpeedSplitVals;
        finalSpeedSplitVals = nullptr;
    }

    if(ninjas[0]->input.fgbzUseConsistentColorScale == true)
    {
        AsciiGrid<double> **resampledVelGrids = new AsciiGrid<double>*[ninjas.size()];
        for(int i = 0; i < ninjas.size(); i++)
        {
            resampledVelGrids[i] = new AsciiGrid<double> (ninjas[i]->VelocityGrid.resample_Grid(ninjas[i]->input.fgbzResolution, AsciiGrid<double>::order0));
            if(ninjas[0]->input.outputBufferClipping <= 0.0)
            {
                resampledVelGrids[i]->BufferToOverlapGrid(demGrid);
            }
        }

        eArmySpeedScaling speedScaling = ninjaArmy::equal_interval;
        if(ninjas[0]->input.fgbzSpeedScaling == OutputWriter::equal_color)
        {
            speedScaling = ninjaArmy::equal_color;
        }
        else if(ninjas[0]->input.fgbzSpeedScaling == OutputWriter::equal_interval)
        {
            speedScaling = ninjaArmy::equal_interval;
        }

        int numSplits;
        double *finalSpeedSplitVals = nullptr;
        calcConsistentColorScaleSplits(resampledVelGrids, ninjas.size(), &finalSpeedSplitVals, &numSplits, speedScaling);

        for(int i = 0; i < ninjas.size(); i++)
        {
            OutputWriter output;

            AsciiGrid<double> *angTempGrid = new AsciiGrid<double> (ninjas[i]->AngleGrid.resample_Grid(ninjas[i]->input.fgbzResolution, AsciiGrid<double>::order0));
            AsciiGrid<double>* velTempGrid = resampledVelGrids[i];

            if(ninjas[0]->input.outputBufferClipping <= 0.0)
            {
                angTempGrid->BufferToOverlapGrid(demGrid);
            }

            output.setSpeedGrid(*velTempGrid, ninjas[i]->input.outputSpeedUnits);
            output.setAngleFromNorth(ninjas[i]->input.dem.getAngleFromNorth());
            output.setDirGrid(*angTempGrid);

            output.setSpeedScaling(ninjas[i]->input.fgbzSpeedScaling);
            output.setColorScheme(ninjas[i]->input.fgbzColor);
            output.setVectorScaling(ninjas[i]->input.fgbzVectorScale);
            output.setLineWidth(ninjas[i]->input.fgbzLineWidth);
            output.setNinjaTime(ninjas[i]->input.ninjaTime);

            if(ninjas[i]->input.initializationMethod == WindNinjaInputs::wxModelInitializationFlag)
            {
                output.setWxModel(ninjas[i]->init->getForecastIdentifier());
            }
            #ifdef NINJAFOAM
            else if(ninjas[i]->input.initializationMethod == WindNinjaInputs::foamWxModelInitializationFlag)
            {
                output.setWxModel(ninjas[i]->input.foamWxForecastIdentifier);
            }
            #endif

            output.setSplitVals(finalSpeedSplitVals, static_cast<unsigned short>(numSplits));

            output.write(ninjas[i]->input.fgbzFile, "FlatGeoBufZip");

            delete angTempGrid;
            angTempGrid = nullptr;
        }

        //cleanup
        for(int i = 0; i < ninjas.size(); i++)
        {
            delete resampledVelGrids[i];
            resampledVelGrids[i] = nullptr;
        }
        delete[] resampledVelGrids;
        resampledVelGrids = nullptr;

        delete[] finalSpeedSplitVals;
        finalSpeedSplitVals = nullptr;
    }

    if(ninjas[0]->input.wxModelGoogOutFlag == true || ninjas[0]->input.wxModelFgbzOutFlag == true)
    {
        // the wxModel outputs aren't resampling datasets, can just reuse the same single set of outputs once for each output type
        AsciiGrid<double> **wxModelVelGrids = new AsciiGrid<double>*[ninjas.size()];
        for(int i = 0; i < ninjas.size(); i++)
        {
            wxModelVelGrids[i] = ninjas[i]->init->getWxSpeedGrid();
        }

        if(ninjas[0]->input.wxModelGoogOutFlag == true && ninjas[0]->input.googUseConsistentColorScale == true)
        {
            eArmySpeedScaling speedScaling = ninjaArmy::equal_interval;
            if(ninjas[0]->input.wxModelGoogSpeedScaling == KmlVector::equal_color)
            {
                speedScaling = ninjaArmy::equal_color;
            }
            if(ninjas[0]->input.wxModelGoogSpeedScaling == KmlVector::equal_interval)
            {
                speedScaling = ninjaArmy::equal_interval;
            }

            int numSplits;
            double *finalSpeedSplitVals = nullptr;
            calcConsistentColorScaleSplits(wxModelVelGrids, ninjas.size(), &finalSpeedSplitVals, &numSplits, speedScaling);

            for(int i = 0; i < ninjas.size(); i++)
            {
                KmlVector wxModelKmlFiles;

                wxModelKmlFiles.setKmlFile(ninjas[i]->input.wxModelKmlFile);
                wxModelKmlFiles.setKmzFile(ninjas[i]->input.wxModelKmzFile);
                wxModelKmlFiles.setLegendFile(ninjas[i]->input.wxModelLegFile);
                wxModelKmlFiles.setDateTimeLegendFile(ninjas[i]->input.dateTimewxModelLegFile, ninjas[i]->input.ninjaTime);
                wxModelKmlFiles.setSpeedGrid(*ninjas[i]->init->getWxSpeedGrid(), ninjas[i]->input.outputSpeedUnits);
                wxModelKmlFiles.setAngleFromNorth(ninjas[i]->input.dem.getAngleFromNorth());
                wxModelKmlFiles.setDirGrid(*ninjas[i]->init->getWxAngleGrid());

                wxModelKmlFiles.setSpeedScaling(ninjas[i]->input.wxModelGoogSpeedScaling);
                wxModelKmlFiles.setColorScheme(ninjas[i]->input.googColor);
                wxModelKmlFiles.setVectorScaling(ninjas[i]->input.googVectorScale);
                wxModelKmlFiles.setLineWidth(ninjas[i]->input.wxModelGoogLineWidth);
                wxModelKmlFiles.setTime(ninjas[i]->input.ninjaTime);

                std::vector<boost::local_time::local_date_time> times(ninjas[i]->init->getTimeList(ninjas[i]->input.ninjaTimeZone));
                wxModelKmlFiles.setWxModel(ninjas[i]->init->getForecastIdentifier(), times[0]);

                wxModelKmlFiles.setSpeedSplitVals(finalSpeedSplitVals, numSplits);

                if(wxModelKmlFiles.writeKml())
                {
                    if(wxModelKmlFiles.makeKmz())
                    {
                        wxModelKmlFiles.removeKmlFile();
                    }
                }
            }

            delete[] finalSpeedSplitVals;
            finalSpeedSplitVals = nullptr;
        }

        if(ninjas[0]->input.wxModelFgbzOutFlag == true && ninjas[0]->input.fgbzUseConsistentColorScale == true)
        {
            eArmySpeedScaling speedScaling = ninjaArmy::equal_interval;
            if(ninjas[0]->input.wxModelFgbzSpeedScaling == OutputWriter::equal_color)
            {
                speedScaling = ninjaArmy::equal_color;
            }
            else if(ninjas[0]->input.wxModelFgbzSpeedScaling == OutputWriter::equal_interval)
            {
                speedScaling = ninjaArmy::equal_interval;
            }

            int numSplits;
            double *finalSpeedSplitVals = nullptr;
            calcConsistentColorScaleSplits(wxModelVelGrids, ninjas.size(), &finalSpeedSplitVals, &numSplits, speedScaling);

            for(int i = 0; i < ninjas.size(); i++)
            {
                OutputWriter wxModelFgbzFiles;

                wxModelFgbzFiles.setSpeedGrid(*ninjas[i]->init->getWxSpeedGrid(), ninjas[i]->input.outputSpeedUnits);
                wxModelFgbzFiles.setAngleFromNorth(ninjas[i]->input.dem.getAngleFromNorth());
                wxModelFgbzFiles.setDirGrid(*ninjas[i]->init->getWxAngleGrid());

                wxModelFgbzFiles.setSpeedScaling(ninjas[i]->input.wxModelFgbzSpeedScaling);
                wxModelFgbzFiles.setColorScheme(ninjas[i]->input.fgbzColor);
                wxModelFgbzFiles.setVectorScaling(ninjas[i]->input.fgbzVectorScale);
                wxModelFgbzFiles.setLineWidth(ninjas[i]->input.wxModelFgbzLineWidth);
                wxModelFgbzFiles.setNinjaTime(ninjas[i]->input.ninjaTime);

                wxModelFgbzFiles.setWxModel(ninjas[i]->init->getForecastIdentifier());

                wxModelFgbzFiles.setSplitVals(finalSpeedSplitVals, static_cast<unsigned short>(numSplits));

                wxModelFgbzFiles.write(ninjas[i]->input.wxModelFgbzFile, "FlatGeoBufZip");
            }

            delete[] finalSpeedSplitVals;
            finalSpeedSplitVals = nullptr;
        }

        //cleanup
        delete[] wxModelVelGrids;
        wxModelVelGrids = nullptr;
    }

    ninjas[ninjas.size()-1]->input.Com->ninjaCom(ninjaComClass::ninjaNone, "Finished writing output files!");
}

void ninjaArmy::initLocalData(void)
{
    const char *pszTmp = NULL;
    pszTmp = CPLGenerateTempFilename( NULL );
    pszTmp = CPLFormFilename( NULL, pszTmp, ".tif" );
    pszTmpColorRelief = CPLStrdup( pszTmp );
}

void ninjaArmy::copyLocalData( const ninjaArmy &A )
{
    CPLFree( (void*)pszTmpColorRelief );
    pszTmpColorRelief = CPLStrdup( A.pszTmpColorRelief );
}

void ninjaArmy::destoryLocalData(void)
{
    CPLPushErrorHandler( CPLQuietErrorHandler );
    GDALDatasetH hDS = GDALOpen( pszTmpColorRelief, GA_ReadOnly );
    if( hDS != NULL )
    {
        GDALClose( hDS );
        GDALDriverH hDrv = GDALGetDriverByName( "GTiff" );
        assert( hDrv );
        GDALDeleteDataset( hDrv, pszTmpColorRelief );
    }
    else
    {
        GDALClose( hDS );
    }

    CPLFree( (void*)pszTmpColorRelief );
    CPLPopErrorHandler();
}
