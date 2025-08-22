/******************************************************************************
 *
 * $Id: ninja_conv.cpp 2258 2013-03-18 23:50:59Z kyle.shannon $
 *
 * Project:  WindNinja
 * Purpose:  C API
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

#include "windninja.h"
#include "ninjaArmy.h"
#include "ninjaException.h"

#ifdef _OPENMP
    omp_lock_t netCDF_lock;
#endif

/**
 * \file windninja.cpp
 *
 * Public C API for windninja.
 */

NinjaErr handleException()
{
    try
    {
        throw;
    }
    catch( std::bad_alloc& e )
    {
        return NINJA_E_BAD_ALLOC;
    }
    catch( cancelledByUser& e )
    {
        return NINJA_E_CANCELLED;
    }
    catch( std::exception& e )
    {
        return NINJA_E_OTHER;
    }
    catch( ... )
    {
        return NINJA_E_UNKNOWN;
    }
}

extern "C"
{
/**
 * \brief Create a new suite of domain average windninja runs.
 *
 * Use this method to create a finite, known number of runs for windninja.
 * There are other creation methods that automatically allocate the correct
 * number of runs for the input type. 
 *
 * \see NinjaMakeWeatherModelArmy
 * \see NinjaMakePointArmy
 *
 * Avaliable Creation Options:
 *                             None
 *
 * \param numNinjas The number of runs to create.
 * \param momentumFlag Flag specifying if the mass and momentum solver should be used.
 * \param speedList List of wind speeds to simulate.
 * \param speedUnits String indicating wind speed units ("mph", "mps", "kph", "knots").
 * \param directionList List of wind directions to simulate in degrees.
 * \param yearList List of years to simulate (only needed if diurnal or stability is going to be used), can be NULL.
 * \param monthList List of months to simulate (only needed if diurnal or stability is going to be used), can be NULL.
 * \param dayList List of days to simulate (only needed if diurnal or stability is going to be used), can be NULL.
 * \param hourList List of hours to simulate (only needed if diurnal or stability is going to be used), can be NULL.
 * \param minuteList List of minutes to simulate (only needed if diurnal or stability is going to be used), can be NULL.
 * \param timeZone A string representing a valid timezone.
 * \param airTempList List of air temperatures (only needed if diurnal or stability is going to be used), can be NULL.
 * \param airTempUnits String indicating air temperature units ("F", "C", or "K"), can be NULL.
 * \param cloudCoverList List of cloud covers (only needed if diurnal or stability is going to be used), can be NULL.
 * \param cloudCoverUnits String indicating cloud cover units ("fraction" or "percent"), can be NULL.
 * \param options Key, value option pairs from the options listed above, can be NULL.
 *
 * \return An opaque handle to a ninjaArmy on success, NULL otherwise.
 */

WINDNINJADLL_EXPORT NinjaArmyH* NinjaMakeDomainAverageArmy
    ( unsigned int numNinjas, bool momentumFlag, const double * speedList, const char * speedUnits, const double * directionList, char ** options )
//    ( unsigned int numNinjas, bool momentumFlag, const double * speedList, const char * speedUnits, const double * directionList, const int * yearList, const int * monthList, const int * dayList, const int * hourList,
//      const int * minuteList, const char * timeZone, const double * airTempList, const char * airTempUnits, const double * cloudCoverList, const char * cloudCoverUnits, char ** options )
{

#ifndef NINJAFOAM
    if(momentumFlag == true)
    {
        throw std::runtime_error("momentumFlag cannot be set to true. WindNinja was not compiled with mass and momentum support.");
    }
#endif

    //Get the number of elements in the arrays
/*     size_t length1 = sizeof(speedList) / sizeof(speedList[0]);
    size_t length2 = sizeof(directionList) / sizeof(directionList[0]); */
//    size_t length1 = sizeof(yearList) / sizeof(yearList[0]);
//    size_t length2 = sizeof(monthList) / sizeof(monthList[0]);
//    size_t length3 = sizeof(dayList) / sizeof(dayList[0]);
//    size_t length4 = sizeof(hourList) / sizeof(hourList[0]);
//    size_t length5 = sizeof(minuteList) / sizeof(minuteList[0]);
//    size_t length6 = sizeof(airTempList) / sizeof(airTempList[0]);
//    size_t length7 = sizeof(cloudCoverList) / sizeof(cloudCoverList[0]);
//
//    if(!(length1 == length2 == length3 == length4 == length5 == length6 == length7))
//    {
//        throw std::runtime_error("yearList, monthList, dayList, hourList, minuteList, airTempList, and cloudCoverList must be the same length!");
//   

    NinjaArmyH* army;

    try
    {
        army = reinterpret_cast<NinjaArmyH*>( new ninjaArmy() );
        reinterpret_cast<ninjaArmy*>( army )->makeDomainAverageArmy( numNinjas, momentumFlag);

        for(int i=0; i<reinterpret_cast<ninjaArmy*>( army )->getSize(); i++) 
        {
            reinterpret_cast<ninjaArmy*>( army )->setInputSpeed( i, speedList[i], std::string( speedUnits ) );
            
            reinterpret_cast<ninjaArmy*>( army )->setInputDirection( i, directionList[i] );

//            reinterpret_cast<ninjaArmy*>( army )->setDateTime( i, yearList[i], monthList[i], dayList[i], hourList[i], minuteList[i], 0, timeZone );
//
//            reinterpret_cast<ninjaArmy*>( army )->setUniAirTemp( i, airTempList[i], std::string( airTempUnits ) );
//            
//            reinterpret_cast<ninjaArmy*>( army )->setUniCloudCover( i, cloudCoverList[i], std::string( cloudCoverUnits ) );
        }

        return army;
    }
    catch( bad_alloc& )
    {
        return NULL;
    }
}

/**
 * \brief Automatically allocate and generate a ninjaArmy from a weather station file.
 *
 * This method will create a set of runs for windninja based on the contents of
 * a weather station file and list of datetimes specified by arrays of years, months, days, and hours 
 * where the ith element of each array specifies a datetime within range of datetimes contained
 * in the weather station file. 
 *
 * Avaliable Creation Options:
 *                             None
 *
 * \param yearList A pointer to an array of years.
 * \param monthList A pointer to an array of months.
 * \param dayList A pointer to an array of days.
 * \param hourList A pointer to an array of hours.
 * \param MinuteList A pointer to an array of hours.
 * \param timeZone a timezone string representing a valid timezone
 * \param stationFileName A valid path to a station file or list of station files.
 * \param elevationFile A valid path to an elevation file.
 * \param matchPointsFlag A flag representing whether to match points or not. Default is true.
 * \param momentumFlag A flag representing whether to use the momentum solver or not (the momentum solver is not currently supported in point initializations). Default is false.
 * \param options Key, value option pairs from the options listed above, can be NULL.
 *
 * \return An opaque handle to a ninjaArmy on success, NULL otherwise.
 */
WINDNINJADLL_EXPORT NinjaArmyH* NinjaMakePointArmy
    (  int * yearList, int * monthList, int * dayList, int * hourList, int * minuteList, int size, char * timeZone, char * stationFileName, char * elevationFile, bool matchPointsFlag, bool momentumFlag, char ** options)
{
    if(momentumFlag == true)
    {
        throw std::runtime_error("The momentum solver is not available for use with Point Initialization runs.");
    }

    //Get the number of elements in the arrays

    NinjaArmyH* army;
    try{
        std::vector <boost::posix_time::ptime> timeList;
        for(size_t i=0; i<size; i++){
            timeList.push_back(boost::posix_time::ptime(boost::gregorian::date(yearList[i], monthList[i], dayList[i]), boost::posix_time::time_duration(hourList[i],minuteList[i],0,0)));
        }

        pointInitialization::SetRawStationFilename(stationFileName);

        std::vector<std::string> sFiles;
        sFiles.push_back(stationFileName);
        pointInitialization::storeFileNames(sFiles);

        // TODO: Include check for using multiple .csv files
        //sFiles=pointInitialization::openCSVList(vm["wx_station_filename"].as<std::string>());
        //pointInitialization::storeFileNames(sFiles);
        
        army = reinterpret_cast<NinjaArmyH*>( new ninjaArmy() );
        reinterpret_cast<ninjaArmy*>( army )->makePointArmy
        (   timeList,
            std::string(timeZone),
            std::string(stationFileName),
            std::string(elevationFile),
            matchPointsFlag,
            momentumFlag);
        return army;
    }
    catch( armyException & e ){
        return NULL;
    }
    return NULL;
}

/**
 * \brief Automatically allocate and generate a ninjaArmy from a forecast file.
 *
 * This method will create a set of runs for windninja based on the contents of
 * the weather forecast file.  One run is done for each timestep in the forecast 
 * file.
 *
 * Avaliable Creation Options:
 *                             None
 *                             TODO: include parameters for start/stop times and a list of timesteps as options->for cases where you don't want to simulate every time step in the forecast file
 *
 * \param forecastFilename A valid NOMADS/UCAR based weather model file.
 * \param timezone a timezone string representing a valid timezone, e.g.
 *                 America/Boise.
 *                 See WINDNINJA_DATA/date_time_zonespec.csv
 * \param momentumFlag A flag representing whether to use the momentum solver or not.
 * \param options Key, value option pairs from the options listed above, can be NULL.
 *
 * \return An opaque handle to a ninjaArmy on success, NULL otherwise.
 */
WINDNINJADLL_EXPORT NinjaArmyH* NinjaMakeWeatherModelArmy
    ( const char * forecastFilename, const char * timezone, bool momentumFlag, char ** options )
{
#ifndef NINJAFOAM
    if(momentumFlag == true)
    {
        throw std::runtime_error("bMomentumFlag cannot be set to true. WindNinja was not compiled with mass and momentum support.");
    }
#endif

    NinjaArmyH* army;
    try
    {
        army = reinterpret_cast<NinjaArmyH*>( new ninjaArmy() );

        reinterpret_cast<ninjaArmy*>( army )->makeWeatherModelArmy
        (   std::string( forecastFilename ),
            std::string( timezone ),
            momentumFlag );
        return army;
    }
    catch( armyException & e )
    {
        return NULL;
    }
    
    return NULL;
}

/**
 * \brief Destroy a suite of windninja runs.
 *
 * Destory the ninjaArmy and free all associated memory.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 *
 * \return NINJA_SUCCESS on success, NINJA_E_NULL_PTR on failure, although this
 *                       can be ignored.  The error is only returned if the
 *                       handle was null.  In this case, the function is a
 *                       no-op.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaDestroyArmy
    ( NinjaArmyH * army, char ** options )
{
    if( NULL != army )
    {
       delete reinterpret_cast<ninjaArmy*>( army );
       army = NULL;
       return NINJA_SUCCESS;
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Fetch DEM file using a point.
 *
 * \param adfPoint A pointer to an array of two doubles representing the point. [latitude, longitude]
 * \param adfBuff length of buffer for x and y
 * \param units buffer units
 * \param dfCellSize The resolution of the DEM file in meters.
 * \param pszDstFile Output file name
 * \param papszOptions options
 * 
 *
 * \return NINJA_SUCCESS on success, NINJA_E_INVALID otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaFetchDEMPoint(NinjaArmyH * army, double * adfPoint, double *adfBuff, const char* units, double dfCellSize, char * pszDstFile, char* fetchType, char ** papszOptions ){
    return reinterpret_cast<ninjaArmy*>( army )->fetchDEMPoint(adfPoint, adfBuff, units, dfCellSize, pszDstFile, fetchType, papszOptions);
}
/**
 * \brief Fetch DEM file using a bounding box
 * 
 * This method will fetch a DEM file using a bounding box and a resolution from the specified source.
 *
 * \param boundsBox A pointer to an array of four doubles representing the bounding box. [north, east, south, west]
 * \param fileName A valid path to a DEM file.
 * \param resolution The resolution of the DEM file in meters.
 * \param fetchType A string representing the source of the DEM file (e.g. "srtm", "gmted", "relief").
 * 
 * \return NINJA_SUCCESS on success, NINJA_E_INVALID otherwise.
 */

WINDNINJADLL_EXPORT NinjaErr NinjaFetchDEMBBox(NinjaArmyH * army, double *boundsBox, const char *fileName, double resolution, char * fetchType, char ** papszOptions){
    return reinterpret_cast<ninjaArmy*>( army )->fetchDEMBBox(boundsBox, fileName, resolution, fetchType);
}

/**
 * \brief Fetch Forecast file from UCAR/THREDDS server.
 *
 * This method will fetch a forecast file from the UCAR/THREDDS server.
 *
 * \param wx_model_type A string representing a valid weather model type (e.g. "NOMADS-HRRR-CONUS-3-KM")
 * \param numNinjas Number of Ninjas
 * \param elevation_file A valid path to an elevation file.
 *
 * \return Forecast file name on success, "exception" otherwise.
 */

WINDNINJADLL_EXPORT const char* NinjaFetchForecast(NinjaArmyH * army, const char*wx_model_type,  unsigned int numNinjas, const char * elevation_file, char ** papszOptions)
{
    return reinterpret_cast<ninjaArmy*>( army )->fetchForecast(wx_model_type, numNinjas, elevation_file);
    
}

/**
 * \brief Fetch Station forecast files using bbox from elevation file
 *
 * Avaliable Creation Options:
 *                             None
 *                             TODO: Add option parameter to specify a buffer to use in station fetching
 *
 * \param yearList A pointer to an array of years.
 * \param monthList A pointer to an array of months.
 * \param dayList A pointer to an array of days.
 * \param hourList A pointer to an array of hours.
 * \param minuteList A pointer to an array of minutes.
 * \param elevation_file A valid path to an elevation file.
 * \param osTimeZone A string representing a valid timezone.
 * \param fetchLatestFlag An integer representing whether to fetch the latest forecast.
 * \param output_path An optional valid path to a custom output directory, can be NULL.
 * \param options Key, value option pairs from the options listed above, can be NULL.
 *
 * \return Station file name on success, "exception" otherwise.
 * TODO: This function currently doesn't return a the path to a station file, need to determine what the proper behavior is
 *       Note: the pointInitialization class currently only has static public functions. We should consider if this is the best 
 *             approach or if the class should be refactored. For example, I was going to add a function to return a path to
 *             the stationLocationFilename (the path on disk to the list of station files that the user will need). But right
 *             now this isn't possible since that path is created by a static function, writeStationLocationFile. If we change 
 *             these functions to non-static to enhance functionality, we'll need to add accessor functions in ninja/ninjaArmy to access 
 *             functions on the pointInitialziation object. Another option for this immediate use case is to just change writeStationLocationFile
 *             to return the path to the file rather than a bool for success/failure. This might be the simplest for now.
 *             
 */
WINDNINJADLL_EXPORT NinjaErr NinjaFetchStation(const int* yearList, const int * monthList, const int * dayList, const int * hourList, const int * minuteList, const int size,
                                               const char* elevationFile, double buffer, const char* units, const char* timeZone, bool fetchLatestFlag, const char* outputPath, char ** options)
{
    std::vector <boost::posix_time::ptime> timeList;
    for(size_t i=0; i<size; i++){
        timeList.push_back(boost::posix_time::ptime(boost::gregorian::date(yearList[i], monthList[i], dayList[i]), boost::posix_time::time_duration(hourList[i], minuteList[i], 0, 0)));
    }

    wxStation::SetStationFormat(wxStation::newFormat);

    //Generate a directory to store downloaded station data
    std::string stationPathName = pointInitialization::generatePointDirectory(std::string(elevationFile), std::string(outputPath), fetchLatestFlag);
    pointInitialization::SetRawStationFilename(stationPathName);
    pointInitialization::setStationBuffer(buffer, units);
    bool success = pointInitialization::fetchStationFromBbox(std::string(elevationFile), timeList, timeZone, fetchLatestFlag);
    if(!success){
        return NINJA_E_INVALID;
    }
    pointInitialization::writeStationLocationFile(stationPathName, std::string(elevationFile), fetchLatestFlag);
    return NINJA_SUCCESS;
}

/**
 * \brief Start the simulations.
 *
 * Run all of the members of the ninjaArmy using one or more processors.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nprocessors number of processors to use when compiled with OpenMP
 *                    support.
 *
 * \return NINJA_SUCCESS on succes, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaStartRuns
    ( NinjaArmyH * army, const unsigned int nprocessors, char ** papszOptions )
{
    if( NULL != army )
    {
        try
        {
            return reinterpret_cast<ninjaArmy*>( army )->startRuns( nprocessors );
        }
        catch( ... )
        {
            return handleException();
        }
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the initialization method.
 *
 * Set the initialization method for a single run.  There are currently three
 * initialization methods, descibed below.
 *
 * Domain-Averaged Initialization
 * ------------------------------
 * key -> "domain_average"
 *
 * Description: Initialize the entire field with a constant speed and
 *              direction.
 *
 * Point Initialization
 * ---------------------
 * key-> "point"
 *
 * Description: Initialize the field using point data described in a weather
 *              model file.  This is a file consisting of a location an weather
 *              observations for a point somewhere on the landscape.  The point
 *              *does not* have to be within the domain of the input DEM or
 *              LCP.  The file format is discussed in the documentation for
 *              NinjaSetWxStationFilename
 * \see NinjaSetWxStationFilename
 *
 * Weather Model Initialization
 * ----------------------------
 * key-> "wxmodel"
 *
 * Description: Initialize the field using coarse-scale weather model data.
 *              These data come from UCAR/thredds through version 2.2.0,
 *              although it may change in the future.  This initialization
 *              method should be used in conjuction with NinjaMakeArmy.
 * \see NinjaMakeArmy
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param initializationMethod a string representation of a valid
 *                             initialization method (see 'key' above).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetInitializationMethod
    (NinjaArmyH * army, const int nIndex, const char * initializationMethod, char ** papszOptions )
{
    if( NULL != army && NULL != initializationMethod )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setInitializationMethod
            ( nIndex, std::string( initializationMethod ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}
        
WINDNINJADLL_EXPORT NinjaErr NinjaInit
    ( char ** papszOptions )
{
    NinjaErr retval = NINJA_E_INVALID;

    retval = NinjaInitialize();

    return retval;
}

/**
 * \brief Set the number of CPUs to use for simulations.
 *
 * \note Only valid with OpenMP support
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param nCPUs Thread count.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetNumberCPUs
    ( NinjaArmyH * army, const int nIndex, const int nCPUs, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setNumberCPUs( nIndex, nCPUs );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the communication handler for simulations.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param comType Type of communication. For now, comType is always "cli".
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetCommunication
    ( NinjaArmyH * army, const int nIndex, const char * comType, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setNinjaCommunication
            ( nIndex, std::string( comType ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Get the set communication handler for a given simulation.
 * only useable with the GUI
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to get the communication handler from, that has already been created by a call to NinjaSetCommunication.
 *
 * \return a pointer to the ninjaComClass of the given ninja on success, a NULL pointer otherwise.
 */
//#ifdef NINJA_GUI
WINDNINJADLL_EXPORT ninjaComClass * NinjaGetCommunication
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->getNinjaCom( nIndex );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}
//#endif //NINJA_GUI

/**
 * \brief Set the DEM to use for the simulations.
 *
 * \see NinjaSetInMemoryDem
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param fileName Path to a valid DEM file on disk.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetDem
    ( NinjaArmyH * army, const int nIndex, const char * fileName, char ** papszOptions )
{
    if( NULL != army && NULL != fileName )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setDEM
            ( nIndex, std::string( fileName ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set an in-memory DEM to use for the simulations.
 *
 * \note NinjaSetOutputPath must be called if an in-memory DEM
 *       is used.
 *
 * \see NinjaSetPosition
 * \see NinjaSetDem
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param demValues An array of elevation values (must be in meters).
 * \param nXSize The number of pixels in the x-direction.
 * \param nYSize The number of pixels in the y-direction.
 * \param geoRef The georeferencing transform.
 *
 *               geoRef[0]  top left x
 *               geoRef[1]  w-e pixel resolution
 *               geoRef[2]  rotational coefficient, zero for north up images
 *               geoRef[3]  top left y
 *               geoRef[4]  rotational coefficient, zero for north up images
 *               geoRef[5]  n-s pixel resolution (negative value)
 *
 * \param prj The projection definition string.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetInMemoryDem
    ( NinjaArmyH * army, const int nIndex, const double * demValues,
      const int nXSize, const int nYSize, const double * geoRef, const char * prj, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setDEM
            ( nIndex, demValues, nXSize, nYSize, geoRef, std::string( prj ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set additional information related to the DEM.
 *
 * \note Must be called after NinjaSetDem is called. This function should
 *       not be called if NinjaSetInMemoryDem is called.
 *
 * \see NinjaSetDem
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetPosition
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setPosition
            ( nIndex );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the input wind speed for a domain-average simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param speed The input speed.
 * \param units The input speed units ("mph", "mps", "kph", "kts").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetInputSpeed
    ( NinjaArmyH * army, const int nIndex, const double speed,
      const char * units, char ** papszOptions )
{
    if( NULL != army && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setInputSpeed
            ( nIndex, speed, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the input wind direction for a domain-average simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param speed The input direction.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetInputDirection
    ( NinjaArmyH * army, const int nIndex, const double direction, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setInputDirection( nIndex, direction );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the input wind height for a domain-average simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param speed The input wind height above the canopy.
 * \param units The input wind height units ("ft", "m").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetInputWindHeight
    ( NinjaArmyH * army, const int nIndex, const double height, const char * units, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setInputWindHeight
                    ( nIndex, height, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the output wind height for a domain-average simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param speed The output wind height above the canopy.
 * \param units The output wind height units ("ft", "m").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputWindHeight
    ( NinjaArmyH * army, const int nIndex, const double height,
      const char * units, char ** papszOptions )
{
    if( NULL != army && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setOutputWindHeight
            ( nIndex, height, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the output wind speed units.
 *
 * \note This function currently only applies to outputs
 *       written to disk. In-memory wind speed output units
 *       are mps.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param units The output speed units ("mph", "mps", "kph", "kts").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputSpeedUnits
    ( NinjaArmyH * army, const int nIndex, const char * units, char ** papszOptions )
{
    if( NULL != army && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setOutputSpeedUnits
            ( nIndex, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the diurnal flag for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag on = 1, off = 2.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetDiurnalWinds
    ( NinjaArmyH * army, const int nIndex, const bool flag, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setDiurnalWinds( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set a uniform air temperture for a domain-average simulation.
 *
 * \note This function only needs to be called if diurnal winds are on.
 *
 * \see NinjaSetDiurnalWinds
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param temp Air temperature.
 * \param units Air temperature units ("K", "C", "R", "F").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetUniAirTemp
    ( NinjaArmyH * army, const int nIndex, const double temp,
      const char * units, char ** papszOptions )
{
    if( NULL != army && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setUniAirTemp
            ( nIndex, temp, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set a uniform cloud cover for a domain-average simulation.
 *
 * \note This function only needs to be called if diurnal winds are on.
 *
 * \see NinjaSetDiurnalWinds
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param cloud_cover Cloud cover.
 * \param units Cloud cover units ("percent", "fraction").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetUniCloudCover
    ( NinjaArmyH * army, const int nIndex, const double cloud_cover,
      const char * units, char ** papszOptions )
{
    if( NULL != army && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setUniCloudCover
            ( nIndex, cloud_cover, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the date and time for a domain-average simulation.
 *
 * \note This function only needs to be called if diurnal winds are on.
 *
 * \see NinjaSetDiurnalWinds
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param yr Year.
 * \param mo Month.
 * \param day Day.
 * \param hr Hour.
 * \param min Minute.
 * \param sec Second.
 * \param timeZoneString Time zone string. Can be set to "auto-detect".
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetDateTime
    ( NinjaArmyH * army, const int nIndex, const int yr, const int mo,
      const int day, const int hr, const int min, const int sec,
      const char * timeZoneString, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setDateTime
            ( nIndex, yr, mo, day, hr, min, sec, std::string( timeZoneString ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set a weather station file name for a point simulation.
 *
 * \note Only valid if point initialization is used.
 *
 * \see NinjaSetInitializationMethod
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param station_filename Weather station file name.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetWxStationFilename
    ( NinjaArmyH * army, const int nIndex, const char * station_filename, char ** papszOptions )
{
    if( NULL != army )
    {
         return reinterpret_cast<ninjaArmy*>( army )->setWxStationFilename
            ( nIndex, std::string( station_filename ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set a uniform vegeation cover to use for a simulation.
 *
 * \note Not valid if a Landscape (*.lcp) file is used.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param vegetation Vegetation option to use ("grass", "brush", "trees").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetUniVegetation
    ( NinjaArmyH * army, const int nIndex, const char * vegetation, char ** papszOptions )
{
    if( NULL != army && NULL != vegetation )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setUniVegetation
            ( nIndex, std::string( vegetation ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Get the diurnal flag set for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param papszOptions options
 *
 * \return Array of strings indicating each wxstation.
 */
WINDNINJADLL_EXPORT char ** NinjaGetWxStations
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions );

/**
 * \brief Get the diurnal flag set for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return flag indicating whether or not the diurnal parameterization is on (1 = on, 0 = off).
 */
WINDNINJADLL_EXPORT int NinjaGetDiurnalWindFlag
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->getDiurnalWindFlag( nIndex );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Get the initialization method for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return string indicating the initialization method.
 */
WINDNINJADLL_EXPORT const char * NinjaGetInitializationMethod
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions )
{
    if( NULL != army )
    {
        return strdup(reinterpret_cast<ninjaArmy*>(army)->getInitializationMethodString(nIndex).c_str());
    }
    else
    {
        return NULL;
    }
}

/*-----------------------------------------------------------------------------
 *  Dust Methods
 *-----------------------------------------------------------------------------*/
#ifdef EMISSIONS
WINDNINJADLL_EXPORT NinjaErr NinjaSetDustFilename
    (NinjaArmyH * army, const int nIndex, const char* filename, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setDustFilename
                ( nIndex, std::string(filename) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT NinjaErr NinjaSetDustFlag
    ( NinjaArmyH * army, const int nIndex, const int flag, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setDustFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}
#endif //EMISSIONS

/*-----------------------------------------------------------------------------
 *  Stability Methods
 *-----------------------------------------------------------------------------*/
/**
 * \brief Set the stability flag for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag on = 1, off = 2.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetStabilityFlag
    ( NinjaArmyH * army, const int nIndex, const bool flag, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setStabilityFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the alpha stability for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param stability_ The alpha stability to use.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetAlphaStability
    ( NinjaArmyH * army, const int nIndex, const double stability_, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setAlphaStability( nIndex, stability_ );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/*-----------------------------------------------------------------------------
 *  NinjaFoam Methods
 *-----------------------------------------------------------------------------*/
#ifdef NINJAFOAM
/**
 * \brief Set the mesh count for a simulation.
 *
 * \note Only for use with the momentum solver.
 *
 * \see NinjaCreateArmy
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param meshCount The number of cells to use in the mesh.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetMeshCount
    ( NinjaArmyH * army, const int nIndex, const int meshCount, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setMeshCount
            ( nIndex, meshCount );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}
#endif //NINJAFOAM

/*-----------------------------------------------------------------------------
 *  Mesh Methods
 *-----------------------------------------------------------------------------*/
/**
 * \brief Set the mesh resolution choice.
 *
 * \see NinjaSetMeshResolution
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param choice The mesh resolution choice ("fine", "medium", "coarse").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetMeshResolutionChoice
    ( NinjaArmyH * army, const int nIndex, const char * choice, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setMeshResolutionChoice
            ( nIndex, std::string( choice ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the mesh resolution.
 *
 * \see NinjaSetMeshResolutionChoice
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param choice The mesh resolution.
 * \param units The mesh resolution units ("ft", "m").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetMeshResolution
    (NinjaArmyH * army, const int nIndex, const double resolution, const char * units, char ** papszOptions )
{
    if( NULL != army && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setMeshResolution
            ( nIndex, resolution, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the number of vertical layers in the mesh.
 *
 * \note Only for use with the conservation of mass solver.
 *
 * \see NinjaCreateArmy
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param nLayers The number of layers to use (20 is typcial).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr  NinjaSetNumVertLayers
    ( NinjaArmyH * army, const int nIndex, const int nLayers, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setNumVertLayers( nIndex, nLayers );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/*-----------------------------------------------------------------------------
 *  Output Methods
 *-----------------------------------------------------------------------------*/
/**
 * \brief Set the output path for a simulation.
 *
 * \note This must be set if an in-memory DEM is used and outputs are written
 *       to disk. The path must exist, it will not be created at runtime.
 *
 * \see NinjaSetInMemoryDem
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param path The full path where outputs should be written.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputPath
    ( NinjaArmyH * army, const int nIndex, const char * path, char ** papszOptions)
{
    if( NULL != army ){
        return reinterpret_cast<ninjaArmy*>( army )->setOutputPath( nIndex, std::string( path ) );
    } else {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the output speed grid resolution.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param resolution The resolution of the output speed grid.
 * \param units The units of the resolution of the output speed grid.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputSpeedGridResolution
    ( NinjaArmyH * army, const int nIndex, const double resolution, const char * units, char ** papszOptions )
{
    if( NULL != army && NULL != units )
    {
        
        lengthUnits::eLengthUnits unitsEnum = lengthUnits::getUnit(std::string(units));

        return reinterpret_cast<ninjaArmy*>( army )->setOutputSpeedGridResolution
            ( nIndex, resolution, unitsEnum );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the output direction grid resolution.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param resolution The resolution of the output direction grid.
 * \param units The units of the resolution of the output direction grid. (look at getUnit inside ninjaUnits.cpp)
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputDirectionGridResolution
    ( NinjaArmyH * army, const int nIndex, const double resolution, const char * units, char ** papszOptions )
{
    if( NULL != army && NULL != units )
    {
        lengthUnits::eLengthUnits unitsEnum = lengthUnits::getUnit(std::string(units));
        return reinterpret_cast<ninjaArmy*>( army )->setOutputDirectionGridResolution
            ( nIndex, resolution, unitsEnum );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Get the output speed grid from a simulation.
 *
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridxllCorner
 * \see NinjaGetOutputGridyllCorner
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param papszOptions first element is the resolution of the output grid as a c style string, second element is the units of the resolution of the output grid ("ft", "m", ...).
 *
 * \return An array of speed values in mps.
 */
WINDNINJADLL_EXPORT const double* NinjaGetOutputSpeedGrid
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions)
{
    if( NULL != army ) {
           return reinterpret_cast<ninjaArmy*>( army )->getOutputSpeedGrid( nIndex );
           return reinterpret_cast<ninjaArmy*>( army )->getOutputSpeedGrid( nIndex );
    } else {
        return NULL;
    }
}

/**
 * \brief Get the output direction grid from a simulation.
 *
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridxllCorner
 * \see NinjaGetOutputGridyllCorner
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return An array of direction values.
 */
WINDNINJADLL_EXPORT const double* NinjaGetOutputDirectionGrid
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions)
{
    if( NULL != army ) {
        return reinterpret_cast<ninjaArmy*>( army )->getOutputDirectionGrid( nIndex);
    } else {
        return NULL;
    }
}

/**
 * \brief Get the output grid projection string from a simulation.
 *
 * \see NinjaGetOutputSpeedGrid
 * \see NinjaGetOutputDirectionGrid
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridxllCorner
 * \see NinjaGetOutputGridyllCorner
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return The output grid projeciton string.
 */
WINDNINJADLL_EXPORT const char* NinjaGetOutputGridProjection
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions )
{
    if( NULL != army ) {
        return reinterpret_cast<ninjaArmy*>( army )->getOutputGridProjection( nIndex );
    } else {
        return NULL;
    }
}

/**
 * \brief Get the output grid cell size from a simulation.
 *
 * \see NinjaGetOutputSpeedGrid
 * \see NinjaGetOutputDirectionGrid
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridxllCorner
 * \see NinjaGetOutputGridyllCorner
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return The output grid cell size in m.
 */
WINDNINJADLL_EXPORT const double NinjaGetOutputGridCellSize
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions )
{
    if( NULL != army ) {
        return reinterpret_cast<ninjaArmy*>( army )->getOutputGridCellSize( nIndex );
    } else {
        throw std::runtime_error("no army");
    }
}

/**
 * \brief Get the x-coordinate of the lower left corner of the output grid from a simulation.
 *
 * \see NinjaGetOutputSpeedGrid
 * \see NinjaGetOutputDirectionGrid
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridyllCorner
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return The lower left x-coordinate (in m) of the output grid.
 */
WINDNINJADLL_EXPORT const double NinjaGetOutputGridxllCorner
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions )
{
    if( NULL != army ) {
        return reinterpret_cast<ninjaArmy*>( army )->getOutputGridxllCorner( nIndex );
    } else {
        throw std::runtime_error("no army");
    }
}

/**
 * \brief Get the y-coordinate of the lower left corner of the output grid from a simulation.
 *
 * \see NinjaGetOutputSpeedGrid
 * \see NinjaGetOutputDirectionGrid
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridxllCorner
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return The lower left y-coordinate (in m) of the output grid.
 */
WINDNINJADLL_EXPORT const double NinjaGetOutputGridyllCorner
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions )
{
    if( NULL != army ) {
        return reinterpret_cast<ninjaArmy*>( army )->getOutputGridyllCorner( nIndex );
    } else {
        throw std::runtime_error("no ninjaArmy");
    }
}

/**
 * \brief Get the number of columns in the output grid from a simulation.
 *
 * \see NinjaGetOutputSpeedGrid
 * \see NinjaGetOutputDirectionGrid
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridxllCorner
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return The number of columns in the output grid.
 */
WINDNINJADLL_EXPORT const int NinjaGetOutputGridnCols
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions )
{
    if( NULL != army ) {
        return reinterpret_cast<ninjaArmy*>( army )->getOutputGridnCols( nIndex );
    } else {
        throw std::runtime_error("no ninjaArmy");
    }
}

/**
 * \brief Get the number of rows in the output grid from a simulation.
 *
 * \see NinjaGetOutputSpeedGrid
 * \see NinjaGetOutputDirectionGrid
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridxllCorner
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return The number of rows in the output grid.
 */
WINDNINJADLL_EXPORT const int NinjaGetOutputGridnRows
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions )
{
    if( NULL != army ) {
        return reinterpret_cast<ninjaArmy*>( army )->getOutputGridnRows( nIndex );
    } else {
        throw std::runtime_error("no ninjaArmy");
    }
}

/**
 * \brief Set the output buffer clipping for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param percent The percent by which to clip the output.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputBufferClipping
    ( NinjaArmyH * army, const int nIndex, const double percent, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setOutputBufferClipping( nIndex, percent );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the flag to write the weather model winds used for initialzation as
 *        a Google Earth file.
 *
 * \note Only valid if wxModelInitialization is used.
 *
 * \see NinjaSetInitializationMethod
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not the weather model winds will be
 *             written as a Google Earth file (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetWxModelGoogOutFlag
    ( NinjaArmyH * army, const int nIndex, const bool flag, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setWxModelGoogOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the flag to write the weather model winds used for initialzation as
 *        a shapefile.
 *
 * \note Only valid if wxModelInitialization is used.
 *
 * \see NinjaSetInitializationMethod
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not the weather model winds will be
 *             written as a shapefile (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetWxModelShpOutFlag
    ( NinjaArmyH * army, const int nIndex, const bool flag, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setWxModelShpOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}

/**
 * \brief Set the flag to write the weather model winds used for initialzation as
 *        a raster file.
 *
 * \note Only valid if wxModelInitialization is used.
 *
 * \see NinjaSetInitializationMethod
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not the weather model winds will be
 *             written as a raster file (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetWxModelAsciiOutFlag
    ( NinjaArmyH * army, const int nIndex, const bool flag, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setWxModelAsciiOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}

/**
 * \brief Set the flag to write Google Earth output for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not Google Earth output will be
 *             written (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogOutFlag
    ( NinjaArmyH * army, const int nIndex, const bool flag, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setGoogOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}

/**
 * \brief Set the resolution of the Google Earth output for a simulation.
 *
 * \note Only valid if NinjaSetGoogOutFlag is set to 1.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param resolution The resolution at which to write the Google Earth output.
 * \param units The units of the Google Earth output resolution ("ft", "m").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogResolution
    ( NinjaArmyH * army, const int nIndex, const double resolution,
      const char * units, char ** papszOptions )
{
    if( NULL != army && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setGoogResolution
            ( nIndex, resolution, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the scaling of the Google Earth output for a simulation.
 *
 * \note Only valid if NinjaSetGoogOutFlag is set to 1.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param scaling The scaling at which to write the Google Earth output. ("equal_color", "color", "equal_interval", "interval")
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogSpeedScaling
    ( NinjaArmyH * army, const int nIndex, const char * scaling, char ** papszOptions )
{
    if( NULL != army && NULL != scaling )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setGoogSpeedScaling
            ( nIndex, std::string( scaling ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the line width of the Google Earth output for a simulation.
 *
 * \note Only valid if NinjaSetGoogOutFlag is set to 1.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param width The line width at which to write the Google Earth output.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogLineWidth
    ( NinjaArmyH * army, const int nIndex, const double width, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setGoogLineWidth( nIndex, width );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the Color Scheme of the Google Earth output for a simulation.
 *
 * \note Only valid if NinjaSetGoogOutFlag is set to 1.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param colorScheme A string that specifies the color scheme ("default", "ROPGW", "oranges", "blues", "pinks", "greens", "magic_beans", "pink_to_greens").
 * \param scaling The flag which determines if vector scaling will be used.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogColor
    ( NinjaArmyH * army, const int nIndex, std::string colorScheme, bool scaling, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setGoogColor(nIndex, colorScheme, scaling);
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the flag to use a Consistent Color Scheme for all Google Earth Outputs of a simulation.
 *
 * \note Only valid if NinjaSetGoogOutFlag is set to 1.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag that determines whether consistent color scaling will be used.
 * \param numRuns The number of runs that will be simulated
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogConsistentColorScale
    ( NinjaArmyH * army, const int nIndex, bool flag, int numRuns, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setGoogConsistentColorScale(nIndex, flag, numRuns);
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the flag to write shapefile output for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not shapefile output will be
 *             written (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetShpOutFlag
    ( NinjaArmyH * army, const int nIndex, const bool flag, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setShpOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}

/**
 * \brief Set the resolution of the shapefile output for a simulation.
 *
 * \note Only valid if NinjaSetShpOutFlag is set to 1.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param resolution The resolution at which to write the shapefile output.
 * \param units The units of the shapefile output resolution ("ft", "m").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetShpResolution
    ( NinjaArmyH * army, const int nIndex, const double resolution,
      const char * units, char ** papszOptions )
{
    if( NULL != army && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setShpResolution
            ( nIndex, resolution, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the flag to write raster output for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not raster output will be
 *             written (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetAsciiOutFlag
    ( NinjaArmyH * army, const int nIndex, const bool flag, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setAsciiOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}


/**
 * \brief Set the resolution of the raster output for a simulation.
 *
 * \note Only valid if NinjaSetAsciiOutFlag is set to 1.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param resolution The resolution at which to write the raster output.
 * \param units The units of the raster output resolution ("ft", "m").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetAsciiResolution
    ( NinjaArmyH * army, const int nIndex, const double resolution,
      const char * units, char ** papszOptions )
{
    if( NULL != army && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setAsciiResolution
            ( nIndex, resolution, units );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the resolution of the raster output for a simulation.
 *
 * \note Only valid if NinjaSetAsciiOutFlag is set to 1.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param flag That flag that determines whether to write the .atm file.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetAsciiAtmFile
    ( NinjaArmyH * army, bool flag, char ** papszOptions)
{
    if( NULL != army)
    {
        try
        {
            reinterpret_cast<ninjaArmy*>(army)->set_writeFarsiteAtmFile(flag);
            return NINJA_SUCCESS;
        }
        catch (const std::exception& e)
        {
            return NINJA_E_OTHER;
        }
    }
    else
    {
        return 1;
    }
}

/**
 * \brief Set the flag to write VTK output for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not VTK output will be
 *             written (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetVtkOutFlag
    ( NinjaArmyH * army, const int nIndex, const bool flag, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setVtkOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the flag to write Txt output for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not VTK output will be
 *             written (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetTxtOutFlag
    ( NinjaArmyH * army, const int nIndex, const bool flag, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setTxtOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}

/**
 * \brief Set the flag to write PDF output for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not VTK output will be
 *             written (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetPDFOutFlag
    ( NinjaArmyH* army, const int nIndex, const bool flag, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setPDFOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}

/**
 * \brief Set the line width of the PDF output for a simulation.
 *
 * \note Only valid if NinjaSetPDFOutFlag is set to 1.
 * 
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param lineWidth The line width of the PDF output.
 * \param papszOptions options.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetPDFLineWidth
    ( NinjaArmyH* army, const int nIndex, const float lineWidth, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setPDFLineWidth( nIndex, lineWidth );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the base map of the PDF output for a simulation.
 *
 * \note Only valid if NinjaSetPDFOutFlag is set to 1.
 * 
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param eType The base map at which to write the PDF output ("TOPOFIRE", "HILLSHADE").
 * \param papszOptions options.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetPDFBaseMap
    ( NinjaArmyH* army, const int nIndex, const int eType, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setPDFBaseMap( nIndex, eType );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the DEM of the PDF output for a simulation.
 *
 * \note Only valid if NinjaSetPDFOutFlag is set to 1.
 * 
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param demFileName The filepath of the simulation DEM.
 * \param papszOptions options.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetPDFDEM
    ( NinjaArmyH* army, const int nIndex, const char * demFileName, char ** papszOptions )
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setPDFDEM( nIndex, std::string( demFileName ), papszOptions );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the size of the PDF output for a simulation
 *
 * \note Only valid if NinjaSetPDFOutFlag is set to 1.
 * 
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param height The height at which to write the PDF output.
 * \param width The width at which to write the PDF output.
 * \param dpi The dpi at which to write the PDF output.
 * \param papszOptions options.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetPDFSize
    ( NinjaArmyH* army, const int nIndex, const double height, const double width, const unsigned short dpi, char ** papszOptions)
{
    if( NULL != army )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setPDFSize( nIndex, height, width, dpi);
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }   
}

/**
 * \brief Set the resolution of the PDF output for a simulation.
 *
 * \note Only valid if NinjaSetPDFOutFlag is set to 1.
 * 
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param resolution The resolution at which to write the PDF output.
 * \param units The units of the PDF output resolution ("ft", "m").
 * \param papszOptions options.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetPDFResolution
    ( NinjaArmyH* army, const int nIndex, const double resolution, const char * units, char ** papszOptions )
{
    if( NULL != army && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( army )->setPDFResolution
            ( nIndex, resolution, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Get the output path for a simulation.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param papszOptions options.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT const char * NinjaGetOutputPath
    ( NinjaArmyH * army, const int nIndex, char ** papszOptions )
{
    if( NULL != army ) {
        return strdup(reinterpret_cast<ninjaArmy*>( army )->getOutputPath( nIndex ).c_str());
    } else {
        return NULL;
    }
}

/*-----------------------------------------------------------------------------
 *  Termination Methods
 *-----------------------------------------------------------------------------*/

 /**
 * \brief Resets the Army.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param papszOptions options.
 * 
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaReset( NinjaArmyH * army, char ** papszOptions )
{
    if( NULL != army )
    {
        reinterpret_cast<ninjaArmy*>( army )->reset();
        return NINJA_SUCCESS;
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Cancels the Army.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param papszOptions options
 * 
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaCancel( NinjaArmyH * army, char ** papszOptions )
{
    if( NULL != army )
    {
        reinterpret_cast<ninjaArmy*>( army )->cancel();
        return NINJA_SUCCESS;
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Cancels and resets the Army.
 *
 * \param army An opaque handle to a valid ninjaArmy.
 * \param papszOptions options
 * 
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaCancelAndReset( NinjaArmyH * army, char ** papszOptions )
{
    if( NULL != army )
    {
        reinterpret_cast<ninjaArmy*>( army )->cancelAndReset();
        return NINJA_SUCCESS;
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

}

/*-----------------------------------------------------------------------------
 *  Helper Methods
 *-----------------------------------------------------------------------------*/

/**
 * \brief calls wxStation::writeBlankStationFile(), which writes a weather station csv file with no data, just a header.
 *
 * \param outputStationFilename A csv file to write a blank weather station file to.
 * \param papszOptions options
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaWriteBlankWxStationFile( const char * outputStationFilename, char ** papszOptions )
{
    wxStation::writeBlankStationFile(outputStationFilename);

    bool doesOutputFileExist = CPLCheckForFile((char*)outputStationFilename, NULL);
    if( doesOutputFileExist )
    {
        return NINJA_SUCCESS;
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}
