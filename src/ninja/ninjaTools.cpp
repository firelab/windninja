#include "ninjaTools.h"

/**
* @brief Default constructor.
*
*/
ninjaTools::ninjaTools()
{
    Com = new ninjaComClass();
    Com->runNumber = 9999;
    Com->printRunNumber = false;

    nomadsCount = 0;
    while( apszNomadsKeys[nomadsCount][0] != NULL )
    {
        nomadsCount++;
    }
    nomadsModels = new NomadsWxModel*[nomadsCount];
    int i = 0;
    while( apszNomadsKeys[i][0] != NULL )
    {
        nomadsModels[i] = new NomadsWxModel( apszNomadsKeys[i][0] );
        i++;
    }
}

/**
* @brief Destructor.
*
*/
ninjaTools::~ninjaTools()
{
    if(nomadsModels)
    {
        for(int i = 0; i < nomadsCount; i++)
        {
            if(nomadsModels[i])
            {
                free(nomadsModels[i]);
            }
        }

        free(nomadsModels);
    }

    delete Com;
}

/**
* @brief Copy constructor.
*
* @param An Object to copy.
*/
/*ninjaTools::ninjaTools(const ninjaTools& A)
{
    nomadsCount = A.nomadsCount;
    nomadsModels = new NomadsWxModel*[nomadsCount];
    for(int i = 0; i < nomadsCount; i++)
    {
        nomadsModels[i] = new NomadsWxModel( apszNomadsKeys[i][0] ); // this should ACTUALLY be something like "new NomadsWxModel( A.nomadsModels[i] )" or something like that, but I don't think it has a proper copy constructor setup. I guess just replicate the constructor for now.
    }

    Com = new ninjaComClass(*A.Com);
}*/

/**
* @brief Equals operator.
*
* @param A Right-hand side.
* @return An Object equal to the one on the right-hand side;
*/
/*ninjaTools& ninjaTools::operator=(ninjaTools const& A)
{
    if(&A != this)
    {
        // I don't even want to know where to begin for the nomadsModels here

        delete Com;
        Com = new ninjaComClass();
        *Com = *A.Com;
    }
    return *this;
}*/

void ninjaTools::fetchWeatherModelData(const char* modelName, const char* demFile, int hours)
{
    try
    {
        wxModelInitialization *model = NULL;

        model = wxModelInitializationFactory::makeWxInitializationFromId(std::string(modelName));

        if(!model)
        {
            throw std::runtime_error(std::string("Weather model not found: ") + modelName);
        }

        std::string forecastFileName = model->fetchForecast(demFile, hours);
        if(forecastFileName == "exception")
        {
            Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: ninjaTools::fetchWeatherModelData() returned an invalid forecastFileName.");
            return NINJA_E_INVALID;
        }
    }
    catch(armyException &e)
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        throw;
    }
    catch( exception& e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        throw;
    }
    catch( ... )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        throw;
    }
}

void ninjaTools::fetchArchiveWeatherModelData(const char* modelName, const char* demFile, const char* timeZone, int startYear, int startMonth, int startDay, int startHour, int endYear, int endMonth, int endDay, int endHour)
{
    try
    {
        wxModelInitialization *model = wxModelInitializationFactory::makeWxInitializationFromId(std::string(modelName));

        boost::gregorian::date startDate(startYear, startMonth, startDay);
        boost::gregorian::date endDate(endYear, endMonth, endDay);

        boost::local_time::tz_database tz_db;
        tz_db.load_from_file( FindDataPath("date_time_zonespec.csv") );
        boost::local_time::time_zone_ptr timeZonePtr;
        timeZonePtr = tz_db.time_zone_from_region(timeZone);

        boost::local_time::local_date_time ldtStart(
            startDate,
            boost::posix_time::hours(startHour),
            timeZonePtr,
            boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR
            );

        boost::local_time::local_date_time ldtEnd(
            endDate,
            boost::posix_time::hours(endHour),
            timeZonePtr,
            boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR
            );

        boost::posix_time::ptime startUTC = ldtStart.utc_time();
        boost::posix_time::ptime endUTC   = ldtEnd.utc_time();

        int hours = 0;

        auto* forecastModel = dynamic_cast<GCPWxModel*>(model);
        forecastModel->setDateTime(startUTC.date(),
                                   endUTC.date(),
                                   boost::lexical_cast<std::string>(startUTC.time_of_day().hours()),
                                   boost::lexical_cast<std::string>(endUTC.time_of_day().hours()));

        std::string forecastFileName = forecastModel->fetchForecast(demFile, hours);
        if(forecastFileName == "exception")
        {
            Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: ninjaTools::fetchArchiveWeatherModelData() returned an invalid forecastFileName.");
            return NINJA_E_INVALID;
        }
    }
    catch(armyException &e)
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        throw;
    }
    catch( exception& e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        throw;
    }
    catch( ... )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        throw;
    }
}

std::vector<std::string> ninjaTools::getForecastIdentifiers()
{
    ncepGfsSurfInitialization gfs;
    ncepNamSurfInitialization nam;
    ncepNamAlaskaSurfInitialization namAla;
    ncepRapSurfInitialization rap;
    ncepNdfdInitialization ndfd;

    modelIdentifiers.push_back(ndfd.getForecastIdentifier());
    modelIdentifiers.push_back(nam.getForecastIdentifier());
    modelIdentifiers.push_back(rap.getForecastIdentifier());
    modelIdentifiers.push_back(namAla.getForecastIdentifier());
    modelIdentifiers.push_back(gfs.getForecastIdentifier());

    for(int i = 0; i < nomadsCount; i++)
    {
        modelIdentifiers.push_back(nomadsModels[i]->getForecastIdentifier());
    }

    GCPWxModel archive;
    modelIdentifiers.push_back(archive.getForecastIdentifier());
    return modelIdentifiers;
}

std::vector<std::string> ninjaTools::getTimeList(const char* fileName, std::string timeZone)
{
    wxModelInitialization *model = NULL;
    model = wxModelInitializationFactory::makeWxInitialization(fileName);
    std::vector<blt::local_date_time> temp = model->getTimeList(timeZone);
    std::vector<std::string> timeList;

    for(int i = 0; i < temp.size(); i++)
    {
        timeList.push_back(temp[i].to_string());
    }

    return timeList;
}

int ninjaTools::getStartHour(const char* modelIdentifier)
{
    wxModelInitialization *model = NULL;
    model = wxModelInitializationFactory::makeWxInitializationFromId(modelIdentifier);
    return model->getStartHour();
}

int ninjaTools::getEndHour(const char* modelIdentifier)
{
    wxModelInitialization *model = NULL;
    model = wxModelInitializationFactory::makeWxInitializationFromId(modelIdentifier);
    return model->getEndHour();
}


int ninjaTools::fetchStationFromBBox( const int* yearList, const int * monthList, const int * dayList, const int * hourList, const int * minuteList, const int size, const char* elevationFile, double buffer, const char* units, const char* timeZone, bool fetchLatestFlag, const char* outputPath, bool locationFileFlag, char ** papszOptions )
{
    try
    {
        std::vector <boost::posix_time::ptime> timeList;
        for(size_t i=0; i<size; i++)
        {
            timeList.push_back(boost::posix_time::ptime(boost::gregorian::date(yearList[i], monthList[i], dayList[i]), boost::posix_time::time_duration(hourList[i], minuteList[i], 0, 0)));
        }

        wxStation::SetStationFormat(wxStation::newFormat);

        if(!fetchLatestFlag)
        {
            boost::local_time::tz_database tz_db;
            tz_db.load_from_file( FindDataPath("date_time_zonespec.csv") );
            boost::local_time::time_zone_ptr timeZonePtr;
            timeZonePtr = tz_db.time_zone_from_region(timeZone);

            boost::local_time::local_date_time start(timeList[0], timeZonePtr);
            boost::local_time::local_date_time stop(timeList[1], timeZonePtr);

            pointInitialization::setLocalStartAndStopTimes(start, stop);
        }

        //Generate a directory to store downloaded station data
        std::string stationPathName = pointInitialization::generatePointDirectory(std::string(elevationFile), std::string(outputPath), fetchLatestFlag);
        pointInitialization::SetRawStationFilename(stationPathName);
        pointInitialization::setStationBuffer(buffer, units);
        bool success = pointInitialization::fetchStationFromBbox(std::string(elevationFile), timeList, timeZone, fetchLatestFlag);
        if(!success)
        {
            Com->ninjaCom(ninjaComClass::ninjaFailure, "pointInitialization::fetchStationFromBbox() failed.");
            return NINJA_E_INVALID;
        }
        if(locationFileFlag)
        {
            pointInitialization::writeStationLocationFile(stationPathName, std::string(elevationFile), fetchLatestFlag);
        }

        return NINJA_SUCCESS;
    }
    catch(armyException &e)
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( exception& e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( ... )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        return NINJA_E_INVALID;
    }
}

int ninjaTools::fetchStationByName( const int* yearList, const int * monthList, const int * dayList, const int * hourList, const int * minuteList, const int size, const char* elevationFile, const char* stationList, const char* timeZone, bool fetchLatestFlag, const char* outputPath, bool locationFileFlag, char ** papszOptions )
{
    try
    {
        std::vector <boost::posix_time::ptime> timeList;
        for(size_t i=0; i<size; i++)
        {
            timeList.push_back(boost::posix_time::ptime(boost::gregorian::date(yearList[i], monthList[i], dayList[i]), boost::posix_time::time_duration(hourList[i], minuteList[i], 0, 0)));
        }

        wxStation::SetStationFormat(wxStation::newFormat);

        if(!fetchLatestFlag)
        {
            boost::local_time::tz_database tz_db;
            tz_db.load_from_file( FindDataPath("date_time_zonespec.csv") );
            boost::local_time::time_zone_ptr timeZonePtr;
            timeZonePtr = tz_db.time_zone_from_region(timeZone);

            boost::local_time::local_date_time start(timeList[0], timeZonePtr);
            boost::local_time::local_date_time stop(timeList[1], timeZonePtr);

            pointInitialization::setLocalStartAndStopTimes(start, stop);
        }

        //Generate a directory to store downloaded station data
        std::string stationPathName = pointInitialization::generatePointDirectory(std::string(elevationFile), std::string(outputPath), fetchLatestFlag);
        pointInitialization::SetRawStationFilename(stationPathName);
        bool success = pointInitialization::fetchStationByName(std::string(stationList), timeList, timeZone, fetchLatestFlag);
        if(!success)
        {
            Com->ninjaCom(ninjaComClass::ninjaFailure, "pointInitialization::fetchStationByName() failed.");
            return NINJA_E_INVALID;
        }
        if(locationFileFlag)
        {
            pointInitialization::writeStationLocationFile(stationPathName, std::string(elevationFile), fetchLatestFlag);
        }

        return NINJA_SUCCESS;
    }
    catch(armyException &e)
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( exception& e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( ... )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        return NINJA_E_INVALID;
    }
}

int ninjaTools::getTimeList( const int * inputYearList, const int * inputMonthList, const int * inputDayList, const int * inputHourList, const int * inputMinuteList, int * outputYearList, int* outputMonthList, int * outputDayList, int * outputHourList, int* outputMinuteList, int nTimeSteps, const char* timeZone )
{
    try
    {
        std::vector<boost::posix_time::ptime> timeList =
            pointInitialization::getTimeList(
                inputYearList[0], inputMonthList[0], inputDayList[0],
                inputHourList[0], inputMinuteList[0],
                inputYearList[1], inputMonthList[1], inputDayList[1],
                inputHourList[1], inputMinuteList[1],
                nTimeSteps, std::string(timeZone)
                );

        for (int i = 0; i < nTimeSteps; ++i)
        {
            const boost::posix_time::ptime& time = timeList[i];
            boost::gregorian::date date = time.date();
            boost::posix_time::time_duration timeDuration = time.time_of_day();

            outputYearList[i]   = static_cast<int>(date.year());
            outputMonthList[i]  = static_cast<int>(date.month());
            outputDayList[i]    = static_cast<int>(date.day());
            outputHourList[i]   = timeDuration.hours();
            outputMinuteList[i] = timeDuration.minutes();
        }

        return NINJA_SUCCESS;
    }
    catch(armyException &e)
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( exception& e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( ... )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        return NINJA_E_INVALID;
    }
}

int ninjaTools::generateSingleTimeObject( int inputYear, int inputMonth, int inputDay, int inputHour, int inputMinute, const char * timeZone, int * outYear, int * outMonth, int* outDay, int * outHour, int * outMinute )
{
    try
    {
        boost::posix_time::ptime timeObject =
            pointInitialization::generateSingleTimeObject(inputYear, inputMonth, inputDay, inputHour, inputMinute, std::string(timeZone));

        const boost::gregorian::date& date = timeObject.date();
        const boost::posix_time::time_duration& td = timeObject.time_of_day();

        *outYear   = static_cast<int>(date.year());
        *outMonth  = static_cast<int>(date.month());
        *outDay    = static_cast<int>(date.day());
        *outHour   = td.hours();
        *outMinute = td.minutes();

        return NINJA_SUCCESS;
    }
    catch(armyException &e)
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( exception& e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( ... )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        return NINJA_E_INVALID;
    }
}

int ninjaTools::checkTimeDuration( int* yearList, int* monthList, int * dayList, int * minuteList, int *hourList, int listSize, char ** papszOptions )
{
    try
    {
        std::vector <boost::posix_time::ptime> timeList;
        for(size_t i=0; i < listSize; i++)
        {
            timeList.push_back(boost::posix_time::ptime(boost::gregorian::date(yearList[i], monthList[i], dayList[i]), boost::posix_time::time_duration(hourList[i],minuteList[i],0,0)));
        }

        int isValid = pointInitialization::checkFetchTimeDuration(timeList);
        if(isValid == -2)
        {
            Com->ninjaCom(ninjaComClass::ninjaFailure, "pointInitialization::checkFetchTimeDuration() failed.");
            return NINJA_E_OTHER;
        }

        return NINJA_SUCCESS;
    }
    catch(armyException &e)
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( exception& e )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: %s", e.what());
        return NINJA_E_INVALID;
    }
    catch( ... )
    {
        Com->ninjaCom(ninjaComClass::ninjaFailure, "Exception caught: Cannot determine exception type.");
        return NINJA_E_INVALID;
    }
}

/*-----------------------------------------------------------------------------
 *  Ninja Communication Methods
 *-----------------------------------------------------------------------------*/

int ninjaTools::setNinjaComMessageHandler( ninjaComMessageHandler pMsgHandler, void *pUser,
                                           char ** papszOptions )
{
    try
    {
        Com->set_messageHandler(pMsgHandler, pUser);
    }
    catch( ... )
    {
        std::cerr << "CRITICAL: ninjaTools level ninjaComMessageHandler not set. Messages will NOT be delivered." << std::endl;
        return NINJA_E_INVALID;
    }
    return NINJA_SUCCESS;
}

int ninjaTools::setNinjaMultiComStream( FILE* stream,
                                        char ** papszOptions )
{
    try
    {
        Com->multiStream = stream;
    }
    catch( ... )
    {
        std::cerr << "ERROR: ninjaTools level ninjaCom multiStream FILE pointer not set." << std::endl;
        return NINJA_E_INVALID;
    }
    return NINJA_SUCCESS;
}
