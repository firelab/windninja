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
: writeFarsiteAtmFile(false)
{
    ninjas.push_back(new ninja());
    initLocalData();
}

/**
* @brief Constructor that allocates numNinjas of ninjas or ninjafoams.
*
* @param numNinjas Number of ninjas to allocate.
* @param momentumFlag flag inidicating if it is a NinjaFoam run
*/
#ifdef NINJAFOAM
ninjaArmy::ninjaArmy(int numNinjas, bool momentumFlag)
: writeFarsiteAtmFile(false)
{
    ninjas.resize(numNinjas);  //allocate vector with enough memory for all ninjas
    for(unsigned int i = 0; i < ninjas.size(); i++)
    {
        if(momentumFlag == true){
            ninjas[i] = new NinjaFoam();
        }
        else{
             ninjas[i] = new ninja();
        }
    }
    initLocalData();
}
#endif

/**
* @brief Constructor that allocates numNinjas of ninjas.
*
* @param numNinjas Number of ninjas to allocate.
*/
#ifndef NINJAFOAM
ninjaArmy::ninjaArmy(int numNinjas)
: writeFarsiteAtmFile(false)
{
    ninjas.resize(numNinjas);  //allocate vector with enough memory for all ninjas
    for(unsigned int i = 0; i < ninjas.size(); i++)
    {
        ninjas[i] = new ninja();
    }
    initLocalData();
}
#endif

/**
* @brief Copy constructor.
*
* @param A Object to copy.
*/
ninjaArmy::ninjaArmy(const ninjaArmy& A)
{
    writeFarsiteAtmFile = A.writeFarsiteAtmFile;
    ninjas = A.ninjas;
    copyLocalData( A );
}

/**
* @brief Destructor.
*
*/
ninjaArmy::~ninjaArmy()
{
    delete ninjas[0];
    destoryLocalData();
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
        writeFarsiteAtmFile = A.writeFarsiteAtmFile;
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


/**
 * @brief ninjaArmy::makeStationArmy Makes an army (array) of ninjas for a Point Initialization run.
 * @param timeList vector of simulation times
 * @param timeZone
 * @param stationFileName
 * @param demFile
 */
void ninjaArmy::makeStationArmy(std::vector<boost::posix_time::ptime> timeList,
                             string timeZone, string stationFileName,
                             string demFile, bool matchPoints, bool override)
{
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
        CPLDebug("STATION_FETCH","LOCAL: %s",(localTimeList[i].to_string()).c_str());
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
}

/**
 * @brief Makes an army (array) of ninjas for a weather forecast run.
 *
 * @param forecastFilename Name of forecast file.
 * @param timeZone String identifying time zone (must match strings in the file "date_time_zonespec.csv".
 */
void ninjaArmy::makeArmy(std::string forecastFilename, std::string timeZone, bool momentumFlag)
{
  return makeArmy(forecastFilename, timeZone, std::vector<blt::local_date_time>(), momentumFlag);
}

/**
 * @brief Makes an army (array) of ninjas for a weather forecast run.
 *
 * @param forecastFilename Name of forecast file.
 * @param timeZone String identifying time zone (must match strings in the file "date_time_zonespec.csv".
 * @param times a vector of times to run from the forecast.  If the vector is
 *        empty, run all of the times in the forecast
 */
void ninjaArmy::makeArmy(std::string forecastFilename, std::string timeZone, std::vector<blt::local_date_time> times, bool momentumFlag)
{
    wxModelInitialization* model;
    
    tz = timeZone;
    
    //for a list of paths forecast files
    if( strstr( forecastFilename.c_str(), ".csv" ) ){
        FILE *fcastList = VSIFOpen( forecastFilename.c_str(), "r" );
        if(fcastList == NULL){
            throw std::runtime_error(std::string("Forecast list ") + forecastFilename.c_str() +
                  std::string(" cannot be opened."));
        }
        while(1){
            const char* f = CPLReadLine(fcastList);
            if(f == NULL){
                break;
            }
            wxList.push_back(f);
        }
        VSIFClose(fcastList);
        
        model = wxModelInitializationFactory::makeWxInitialization(wxList[0]); 
        
        ninjas.resize(wxList.size());
        
        for(unsigned int i = 0; i < wxList.size(); i++)
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
#endif //NINJAFOAM
        }
        
        std::vector<boost::local_time::local_date_time> timeList = model->getTimeList(timeZone);
        
        for(unsigned int i = 0; i < wxList.size(); i++)
        {
            ninjas[i]->set_date_time(timeList[0]);
            ninjas[i]->set_wxModelFilename(wxList[i]);
            ninjas[i]->set_initializationMethod(WindNinjaInputs::wxModelInitializationFlag);
            ninjas[i]->set_inputWindHeight( (*model).Get_Wind_Height() );
            ninjas[i]->setArmySize(wxList.size());
        }       
        delete model;
    }
    
    //Factory function that identifies the type of forecast file and makes appropriate class.
    else{
        model = wxModelInitializationFactory::makeWxInitialization(forecastFilename);

        try
        {
            model->checkForValidData();
        }
        catch(armyException &e)
        {
            std::cout << "Bad forecast file, exiting" << endl;
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
}

void ninjaArmy::set_writeFarsiteAtmFile(bool flag)
{
    writeFarsiteAtmFile = flag;
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
    try{
        if(ninjas.size() > 1){
            for(unsigned int i=0; i<ninjas.size()-1; i++){
                for(unsigned int j=i+1; j<ninjas.size(); j++){
                    if(ninjas[i]->input == ninjas[j]->input &&
                       ninjas[i]->get_initializationMethod() == WindNinjaInputs::domainAverageInitializationFlag){
                            throw std::runtime_error("Multiple runs were requested with the same input parameters.");
                    }
                }
            }
        }
    }catch (exception& e)
    {
        std::cout << "Exception caught: " << e.what() << endl;
        status = false;
        throw;
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
            throw std::runtime_error("Error generating the NINJAFOAM directory.");
        }
    }
#endif //NINJAFOAM

#ifdef _OPENMP
    omp_set_nested(false);
    //omp_set_dynamic(true);
#endif

    setAtmFlags();
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
               printf("Return of false from simulate_wind()");
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
                    diurnal_ninja->input.initializationMethod = WindNinjaInputs::foamDomainAverageInitializationFlag;
                }
                else{
                    throw std::runtime_error("ninjaArmy: Initialization method not set properly.");
                }
                diurnal_ninja->input.inputWindHeight = ninjas[0]->input.outputWindHeight;
                //if case is re-used resolution may not be set, set mesh resolution based on ninjas[0]
                diurnal_ninja->set_meshResolution(ninjas[0]->get_meshResolution(), lengthUnits::getUnit("m")); 
                if(!diurnal_ninja->simulate_wind()){
                    printf("Return of false from simulate_wind()");
                }
                //set output path on original ninja for the GUI
                ninjas[0]->input.outputPath = diurnal_ninja->input.outputPath;

                //set filenames for atm file writing
                ninjas[0]->input.velFile = diurnal_ninja->get_VelFileName();
                ninjas[0]->input.angFile = diurnal_ninja->get_AngFileName();
                ninjas[0]->input.cldFile = diurnal_ninja->get_CldFileName();
            } 
#endif //NINJAFOAM            

            //write farsite atmosphere file
            if(writeFarsiteAtmFile)
                writeFarsiteAtmosphereFile();

        }catch (bad_alloc& e)
        {
            std::cout << "Exception bad_alloc caught: " << e.what() << endl;
            std::cout << "WindNinja appears to have run out of memory." << endl;
            status = false;
            throw;
        }catch (cancelledByUser& e)
        {
            std::cout << "Exception caught: " << e.what() << endl;
            status = false;
            throw;
        }catch (exception& e)
        {
            std::cout << "Exception caught: " << e.what() << endl;
            status = false;
            throw;
        }catch (...)
        {
            std::cout << "Exception caught: Cannot determine exception type." << endl;
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
                        diurnal_ninja->input.initializationMethod = WindNinjaInputs::foamDomainAverageInitializationFlag;
                    }
                    else{
                        throw std::runtime_error("ninjaArmy: Initialization method not set properly.");
                    }
                    diurnal_ninja->input.inputWindHeight = ninjas[i]->input.outputWindHeight;
                    //if case is re-used resolution may not be set, set mesh resolution based on ninjas[0]
                    diurnal_ninja->set_meshResolution(ninjas[0]->get_meshResolution(), lengthUnits::getUnit("m")); 
                    if(!diurnal_ninja->simulate_wind()){
                        throw std::runtime_error("ninjaArmy: Error in ninja::simulate_wind().");
                    }
                    //set output path on original ninja for the GUI
                    ninjas[i]->input.outputPath = diurnal_ninja->input.outputPath;

                    //set filenames for atm file writing
                    ninjas[i]->input.velFile = diurnal_ninja->get_VelFileName();
                    ninjas[i]->input.angFile = diurnal_ninja->get_AngFileName();
                    ninjas[i]->input.cldFile = diurnal_ninja->get_CldFileName();
                } 
                //store data for atmosphere file
                if(writeFarsiteAtmFile)
                {
                    atmosphere.push( ninjas[i]->get_date_time(),   ninjas[i]->get_VelFileName(),
                                     ninjas[i]->get_AngFileName(), ninjas[i]->get_CldFileName() );
                }

                //delete all but ninjas[0] (ninjas[0] is used to set the output path in the GUI)
                if( i != 0  )
                {
                    delete ninjas[i];
                    ninjas[i] = NULL;
                }

            }catch (bad_alloc& e)
            {
                std::cout << "Exception bad_alloc caught: " << e.what() << endl;
                std::cout << "WindNinja appears to have run out of memory." << endl;
                status = false;
            }catch (cancelledByUser& e)
            {
                std::cout << "Exception caught: " << e.what() << endl;
                status = false;
            }catch (exception& e)
            {
                std::cout << "Exception caught: " << e.what() << endl;
                status = false;
            }catch (...)
            {
                std::cout << "Exception caught: Cannot determine exception type." << endl;
                status = false;
            }
        }
        try{
            //write farsite atmosphere file
            if(writeFarsiteAtmFile)
                writeFarsiteAtmosphereFile();

        }catch (bad_alloc& e)
        {
            std::cout << "Exception bad_alloc caught: " << e.what() << endl;
            std::cout << "WindNinja appears to have run out of memory." << endl;
            status = false;
            throw;
        }catch (cancelledByUser& e)
        {
            std::cout << "Exception caught: " << e.what() << endl;
            status = false;
            throw;
        }catch (exception& e)
        {
            std::cout << "Exception caught: " << e.what() << endl;
            status = false;
            throw;
        }catch (...)
        {
            std::cout << "Exception caught: Cannot determine exception type." << endl;
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
     
        //create MEM datasets for GTiff output writer
        ninjas[0]->readInputFile();
        ninjas[0]->set_position();
        ninjas[0]->set_uniVegetation();
        ninjas[0]->mesh.buildStandardMesh(ninjas[0]->input);
        
        int nXSize = ninjas[0]->input.dem.get_nCols(); //57; 
        int nYSize = ninjas[0]->input.dem.get_nRows(); //70; 
    
        GDALDriverH hDriver = GDALGetDriverByName( "MEM" );
        
        hSpdMemDS = GDALCreate(hDriver, "", nXSize, nYSize, 1, GDT_Float64, NULL);
        hDirMemDS = GDALCreate(hDriver, "", nXSize, nYSize, 1, GDT_Float64, NULL);
        hDustMemDS = GDALCreate(hDriver, "", nXSize, nYSize, 1, GDT_Float64, NULL);

	#pragma omp parallel for //spread runs on single threads
        //FOR_EVERY(iter_ninja, ninjas) //Doesn't work with omp
        for( int i = 0; i < ninjas.size(); i++ )
        {
            try
            {
                //list of paths to forecast files, possibly in various zip archives
                if( wxList.size() > 1 )
                {
                    wxModelInitialization* model;
                    model = wxModelInitializationFactory::makeWxInitialization(wxList[i]); 
                
                    timeList = model->getTimeList(tz);
                    ninjas[i]->set_date_time(timeList[0]);
                    ninjas[i]->set_wxModelFilename( wxList[i] );
                    ninjas[i]->set_date_time( timeList[0] );
                    //set in-memory datasets for GTiff output writer
                    ninjas[i]->set_memDs(hSpdMemDS, hDirMemDS, hDustMemDS); 
                    
                    delete model;
                }

                //start the run
                ninjas[i]->simulate_wind();	//runs are done on 1 thread each since omp_set_nested(false)

                //store data for atmosphere file
                if(writeFarsiteAtmFile)
                {
                    atmosphere.push( ninjas[i]->get_date_time(),   ninjas[i]->get_VelFileName(),
                                     ninjas[i]->get_AngFileName(), ninjas[i]->get_CldFileName() );
                }

                //delete all but ninjas[0] (ninjas[0] is used to set the output path in the GUI)
                if( i != 0  )
                {
                    delete ninjas[i];
                    ninjas[i] = NULL;
                }

            }catch (bad_alloc& e)
            {
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
#ifdef _OPENMP
                anErrors[omp_get_thread_num()] = NINJA_CANCEL_USER_EXC;
                asMessages[omp_get_thread_num()] = "Exception cacneled by user caught:";
                asMessages[omp_get_thread_num()] + e.what();
                asMessages[omp_get_thread_num()] += "\n";
                status = false;
#else
                throw;
#endif
            }catch (badForecastFile& e)
            {
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
            if(writeFarsiteAtmFile)
                writeFarsiteAtmosphereFile();

        }catch (bad_alloc& e)
        {
            std::cout << "Exception bad_alloc caught: " << e.what() << endl;
            std::cout << "WindNinja appears to have run out of memory." << endl;
            status = false;
            throw;
        }catch (cancelledByUser& e)
        {
            std::cout << "Exception caught: " << e.what() << endl;
            status = false;
            throw;
        }catch (exception& e)
        {
            std::cout << "Exception caught: " << e.what() << endl;
            status = false;
            throw;
        }catch (...)
        {
            std::cout << "Exception caught: Cannot determine exception type." << endl;
            status = false;
            throw;
        }
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

    setAtmFlags();

    //set number of threads for the run
    ninjas[0]->set_numberCPUs(1);
    try
    {
        //start the run
        if(!ninjas[0]->simulate_wind())
            printf("Return of false from simulate_wind()");

        //write farsite atmosphere file
        writeFarsiteAtmosphereFile();

    }
    catch (bad_alloc& e)
    {
        std::cout << "Exception bad_alloc caught: " << e.what() << endl;
        std::cout << "WindNinja appears to have run out of memory." << endl;
        status = false;
        throw;
    }
    catch (cancelledByUser& e)
    {
        std::cout << "Exception caught: " << e.what() << endl;
        status = false;
        throw;
    }
    catch (exception& e)
    {
        std::cout << "Exception caught: " << e.what() << endl;
        status = false;
        throw;
    }
    catch (...)
    {
        std::cout << "Exception caught: Cannot determine exception type." << endl;
        status = false;
        throw;
    }
    return status;
}

/**
 * @brief write the atm file
 *
 * Write one or more atm files if needed.
 * @see setAtmFlag
 */
void ninjaArmy::writeFarsiteAtmosphereFile()
{
    if(writeFarsiteAtmFile)
    {
        //If wxModelInitialization or pointInitialization, make one .atm with all runs (times) listed,
        //else the setAtmFlags() function has already set each ninja to write their own atm file,
        //so don't do it here!
        if(ninjas[0]->get_initializationMethod() == WindNinjaInputs::wxModelInitializationFlag ||
           (ninjas[0]->get_initializationMethod() == WindNinjaInputs::pointInitializationFlag && ninjas.size() > 1))
        {
            //Set directory path from first ninja's velocity file
            std::string filePath = CPLGetPath( ninjas[0]->get_VelFileName().c_str() );

            //Get filename from first ninja's velFile
            std::string fileroot( CPLGetBasename(ninjas[0]->get_VelFileName().c_str()) );
            int stringPos = fileroot.find_last_of('_');
            if(stringPos > 0)
                fileroot.erase(stringPos);
            else
                throw std::runtime_error("Problem writing FARSITE atmosphere file.  The ninja ASCII velocity filename appears to be malformed.");

            //Form atm filename
            std::string filename( CPLFormFilename(filePath.c_str(), fileroot.c_str(), "atm") );

            //Write atm file
            atmosphere.writeAtmFile(filename, ninjas[0]->get_outputSpeedUnits(),
                                              ninjas[0]->get_outputWindHeight() );
        }
    }
}

/**
 * @brief Determine what type of atm file to write.
 *
 * If the run is a weather model run, we only need to write one atm file for
 * all of the output files.  This atm file will be named after the *first*
 * run of all the runs.
 */
void ninjaArmy::setAtmFlags()
{
    if(writeFarsiteAtmFile)
    {
        //if it's not a weather model or point run, set all ninja's atm write flags
        if(!(ninjas[0]->get_initializationMethod() == WindNinjaInputs::wxModelInitializationFlag) &&
           !(ninjas[0]->get_initializationMethod() == WindNinjaInputs::pointInitializationFlag && 
               ninjas.size() > 1))
        {
            //FOR_EVERY( ninja, ninjas )
            for(unsigned int i = 0; i < ninjas.size(); i++)
            {
                ninjas[i]->set_writeAtmFile(true);
            }
        }
    }
}

void ninjaArmy::setSize( int nSize, bool momentumFlag )
{
    int i;
    for( i=0; i < ninjas.size();i ++) 
        delete ninjas[i];
    ninjas.resize( nSize );
    for( i = 0; i < nSize; i++ ){
#ifdef NINJAFOAM
        if(momentumFlag)
            ninjas[i] = new NinjaFoam();
        else
            ninjas[i] = new ninja();
#else
        ninjas[i] = new ninja();
#endif
    }
}

/*-----------------------------------------------------------------------------
 *  Ninja Communication Methods
 *-----------------------------------------------------------------------------*/

int ninjaArmy::setNinjaCommunication( const int nIndex, const int RunNumber,
                           const ninjaComClass::eNinjaCom comType,
                           char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_ninjaCommunication( RunNumber, comType ) );
}

int ninjaArmy::setNinjaCommunication( const int nIndex, std::string comType,
                           char ** papszOptions )
{
    int retval = NINJA_E_INVALID;
    IF_VALID_INDEX( nIndex, ninjas )
    {
        std::transform( comType.begin(), comType.end(), comType.begin(), ::tolower );
        if( comType == "ninjaCLICom" || comType == "cli" )
        {
            ninjas[ nIndex ]->set_ninjaCommunication
                ( nIndex, ninjaComClass::ninjaCLICom );
            retval = NINJA_SUCCESS;
        }
        else if( comType == "ninjaQuietCom" || comType == "quiet" )
        {
            ninjas[ nIndex ]->set_ninjaCommunication
                ( nIndex, ninjaComClass::ninjaQuietCom );
            retval = NINJA_SUCCESS;
        }
        else
        {
            retval = NINJA_E_INVALID;
        }
    }
    return retval;
}

#ifdef NINJA_GUI
int ninjaArmy::setNinjaComNumRuns( const int nIndex, const int RunNumber,
                                   char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_ComNumRuns( RunNumber ) );
}

ninjaComClass * ninjaArmy::getNinjaCom( const int nIndex, char ** papszOptions )
{
    IF_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_Com();
    }
    return NULL; //if not valid index
}
#endif //NINJA-GUI

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

int ninjaArmy::setGeotiffOutFilename( const int nIndex, const std::string filename,
                                char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_geotiffOutFilename( filename ) );
}

int ninjaArmy::setGeotiffOutFlag( const int nIndex, const bool flag, char ** papszOptions )
{
    IF_VALID_INDEX_TRY( nIndex, ninjas, ninjas[ nIndex ]->set_geotiffOutFlag( flag ) );
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
       catch( std::logic_error &e )
       {
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
       catch( std::logic_error &e )
       {
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
}

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
    IF_VALID_INDEX_TRY( nIndex, ninjas,
            ninjas[ nIndex ]->set_meshResChoice( choice ) );
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
const double* ninjaArmy::getOutputSpeedGrid( const int nIndex, char ** papszOptions )
{
    CHECK_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_outputSpeedGrid( );
    }
}
const double* ninjaArmy::getOutputDirectionGrid( const int nIndex, char ** papszOptions )
{
    CHECK_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_outputDirectionGrid( );
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
    IF_VALID_INDEX_TRY( nIndex,ninjas,ninjas[nIndex]->set_googColor(colorScheme,scaling));
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
           retval = NINJA_E_INVALID;
       }
   }
   return retval;
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

std::string ninjaArmy::getOutputPath( const int nIndex, char ** papszOptions )
{
    IF_VALID_INDEX( nIndex, ninjas )
    {
        return ninjas[ nIndex ]->get_outputPath();
    }
    return std::string("");
}
/**
 * @brief Reset the army in able to reinitialize needed parameters
 *
 */
void ninjaArmy::reset()
{
    ninjas.clear();
    writeFarsiteAtmFile = false;
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
