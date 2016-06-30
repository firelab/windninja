/******************************************************************************
 *
 * $Id: wxModelInitialization.cpp 816 2011-02-15 16:34:53Z jaforthofer $
 *
 * Project:  WindNinja
 * Purpose:  For writing FARSITE atmosphere files (*.atm)
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

#include "wxModelInitialization.h"


// #define NC_NOERR        0       /* No Error */

// #define NC_EBADID       (-33)   /* Not a netcdf id */
// #define NC_ENFILE       (-34)   /* Too many netcdfs open */
// #define NC_EEXIST       (-35)   /* netcdf file exists && NC_NOCLOBBER */
// #define NC_EINVAL       (-36)   /* Invalid Argument */
// #define NC_EPERM        (-37)   /* Write to read only */
// #define NC_ENOTINDEFINE (-38)   /* Operation not allowed in data mode */
// #define NC_EINDEFINE    (-39)   /* Operation not allowed in define mode */
// #define NC_EINVALCOORDS (-40)   /* Index exceeds dimension bound */
// #define NC_EMAXDIMS     (-41)   /* NC_MAX_DIMS exceeded */
// #define NC_ENAMEINUSE   (-42)   /* String match to name in use */
// #define NC_ENOTATT      (-43)   /* Attribute not found */
// #define NC_EMAXATTS     (-44)   /* NC_MAX_ATTRS exceeded */
// #define NC_EBADTYPE     (-45)   /* Not a netcdf data type */
// #define NC_EBADDIM      (-46)   /* Invalid dimension id or name */
// #define NC_EUNLIMPOS    (-47)   /* NC_UNLIMITED in the wrong index */
// #define NC_EMAXVARS     (-48)   /* NC_MAX_VARS exceeded */
// #define NC_ENOTVAR      (-49)   /* Variable not found */
// #define NC_EGLOBAL      (-50)   /* Action prohibited on NC_GLOBAL varid */
// #define NC_ENOTNC       (-51)   /* Not a netcdf file */
// #define NC_ESTS         (-52)   /* In Fortran, string too short */
// #define NC_EMAXNAME     (-53)   /* NC_MAX_NAME exceeded */
// #define NC_EUNLIMIT     (-54)   /* NC_UNLIMITED size already in use */
// #define NC_ENORECVARS   (-55)   /* nc_rec op when there are no record vars */
// #define NC_ECHAR        (-56)   /* Attempt to convert between text & numbers */
// #define NC_EEDGE        (-57)   /* Edge+start exceeds dimension bound */
// #define NC_ESTRIDE      (-58)   /* Illegal stride */
// #define NC_EBADNAME     (-59)   /* Attribute or variable name
//                                          contains illegal characters */
// /* N.B. following must match value in ncx.h */
// #define NC_ERANGE       (-60)   /* Math result not representable */
// #define NC_ENOMEM       (-61)   /* Memory allocation (malloc) failure */

// #define NC_EVARSIZE     (-62)   /* One or more variable sizes violate
//                                    format constraints */
// #define NC_EDIMSIZE     (-63)   /* Invalid dimension size */
// #define NC_ETRUNC       (-64)   /* File likely truncated or possibly corrupted */

// #define NC_NAT          0       /* NAT = 'Not A Type' (c.f. NaN) */
// #define NC_BYTE         1       /* signed 1 byte integer */
// #define NC_CHAR         2       /* ISO/ASCII character */
// #define NC_SHORT        3       /* signed 2 byte integer */
// #define NC_INT          4       /* signed 4 byte integer */
// #define NC_LONG         NC_INT  /* deprecated, but required for backward compatibility. */
// #define NC_FLOAT        5       /* single precision floating point number */
// #define NC_DOUBLE       6       /* double precision floating point number */
// #define NC_UBYTE        7       /* unsigned 1 byte int */
// #define NC_USHORT       8       /* unsigned 2-byte int */
// #define NC_UINT         9       /* unsigned 4-byte int */
// #define NC_INT64        10      /* signed 8-byte int */
// #define NC_UINT64       11      /* unsigned 8-byte int */
// #define NC_STRING       12      /* string */

extern boost::local_time::tz_database globalTimeZoneDB;

/**
 * Default constructor.
 */
wxModelInitialization::wxModelInitialization() : initialize()
{
    wxModelFileName = "";
    host = "thredds.ucar.edu";
    pfnProgress = NULL;
}

/**
 * Copy constructor.
 * @param A Object to copy.
 */
wxModelInitialization::wxModelInitialization(const wxModelInitialization& A) : initialize(A)
{
    wxModelFileName = A.wxModelFileName;
    host = A.host;
    heightVarName = A.heightVarName;
    path = A.path;
    pfnProgress = A.pfnProgress;
}

/**
 * Destructor.
 */
wxModelInitialization::~wxModelInitialization()
{
}

/**
 * Equals operator.
 * @param A Right-hand side.
 * @return A wxModelInitialization equal to the one on the right-hand side;
 */
wxModelInitialization& wxModelInitialization::operator= (wxModelInitialization const& A)
{
    if(&A != this) {
    initialize::operator=(A);
    wxModelFileName = A.wxModelFileName;
    host = A.host;
    heightVarName = A.heightVarName;
    path = A.path;
    pfnProgress = A.pfnProgress;
    }
    return *this;
}

/**
 * Fetch the host for the forecast
 *
 * @return a valid host name
 */
std::string wxModelInitialization::getHost()
{
    return host;
}

std::string wxModelInitialization::getPath()
{
    return path;
}

/**
 * Generate a file name for the forecast based on the first time in the
 * time list
 *
 * @return a base file name, or an empty string if a filename cannot be
 * generated
 */
std::string wxModelInitialization::generateForecastName()
{
    if( wxModelFileName.empty() )
    return wxModelFileName;
    std::vector<boost::local_time::local_date_time>timeList;
    timeList = getTimeList();
    std::string filename;
    filename =  bpt::to_iso_string(timeList[0].utc_time());
    //remove trailing seconds
    filename = filename.erase( filename.size() - 2 );
    filename += ".nc";
    return filename;
}
/**
 * Generate a url to fetch a forecast
 *
 * @param north latitude in degrees of the north bound
 * @param east longitude in degrees of the east bound
 * @param south latitude in degrees of the south bound
 * @param west longitude in degrees of the west bound
 * @param hours length of forecast in hours.
 *
 * @return valid url for the forecast
 */
std::string wxModelInitialization::generateUrl( double north, double east,
                        double south, double west,
                        int hours )
{
    std::vector<std::string> v = getVariableList();
    std::string url;
    url = url + getHost();
    url = url + getPath();
    url = url + "&var=";
    for( unsigned int i = 0;i < v.size();i++ ) {
        url += v[i];
        if( i != v.size() - 1 )
            url += ",";
    }

    /*
     * Replace the user defined variables in the list;
     */
    std::string replace;
    std::stringstream ss;
    size_t pos;

    /* north */
    replace = "USER_NORTH";
    if( url.find( replace ) != url.npos ) {
    pos = url.find( replace );
    ss << north;
    url.replace( pos, replace.size(), ss.str() );
    }
    ss.str("");

    /* west */
    replace = "USER_WEST";
    if( url.find( replace ) != url.npos ) {
    pos = url.find( replace );
    ss << west;
    url.replace( pos, replace.size(), ss.str() );
    }

    ss.str("");

    /* east */
    replace = "USER_EAST";
    if( url.find( replace ) != url.npos ) {
    pos = url.find( replace );
    ss << east ;
    url.replace( pos, replace.size(), ss.str() );
    }

    ss.str("");

    /* south */
    replace = "USER_SOUTH";
    if( url.find( replace ) != url.npos ) {
    pos = url.find( replace );
    ss << south;
    url.replace( pos, replace.size(), ss.str() );
    }

    ss.str("");

    /* time duration */
    replace = "USER_TIME";
    if( url.find( replace ) != url.npos ) {
    pos = url.find( replace );
    ss << hours;
    url.replace( pos, replace.size(), ss.str() );
    }

    return url;
}

/**
 * \brief Generate a buffer to download wxModel data within.
 *
 * Attempt to handle warping artifacts in the dem by scaling the buffer and
 * adding to the max/min of x and y.
 *
 * \param poDS GDAL Dataset to compute the bounds for
 * \param [out] bounds max y, max x, min y, min x (north, east, south, west)
 * \return 0 on success, non zero otherwise
 */
int wxModelInitialization::ComputeWxModelBuffer( GDALDataset *poDS, double bounds[4] )
{
    double corners[8];
    GDALGetCorners( poDS, corners );
    double maxX, maxY, minX, minY;
    
    //max/min of DEM in degrees
    maxX = MAX(corners[0], corners[2]);
    maxY = MAX(corners[1], corners[7]);
    minX = MIN(corners[4], corners[6]);
    minY = MIN(corners[3], corners[5]);
        
#ifdef MOBILE_APP
    CPLDebug("MOBILE_APP", "grid resolution = %f", this->getGridResolution());
    if(this->getGridResolution() == -1.0){ //if don't know the resolution
        bounds[0] = maxY + GetWxModelBuffer(maxY - minY);
        bounds[1] = maxX + GetWxModelBuffer(maxX - minX);
        bounds[2] = minY - GetWxModelBuffer(maxY - minY);
        bounds[3] = minX - GetWxModelBuffer(maxX - minX);
        
        CPLDebug("MOBILE_APP", "Y buffer = %f", GetWxModelBuffer(maxY - minY));
        CPLDebug("MOBILE_APP", "X buffer = %f", GetWxModelBuffer(maxX - minX));
    }
    else{//compute buffer based on wx grid size
        double projMaxX = maxX;
        double projMinX = minX;
        double projMaxY = maxY;
        double projMinY = minY;
    
        double wxMaxX, wxMaxY, wxMinX, wxMinY;
        double xBuffer, yBuffer;
        
        //convert to meters
        GDALPointFromLatLon( projMaxX, projMaxY, poDS, "WGS84");
        GDALPointFromLatLon( projMinX, projMinY, poDS, "WGS84");
        
        //buffer 3 times the wx model grid resolution
        if(this->getGridResolution() > 1.0){ //units are km (projected)
            wxMaxX = projMaxX + ( this->getGridResolution() * 1000 * 3 );
            wxMaxY = projMaxY + ( this->getGridResolution() * 1000 * 3 );
            
            //convert to degrees
            GDALPointToLatLon( wxMinX, wxMinY, poDS, "WGS84");
            GDALPointToLatLon( wxMaxX, wxMaxY, poDS, "WGS84");
        
            xBuffer = wxMaxX - maxX; //in degrees
            yBuffer = wxMaxY - maxY; //in degrees
        }
        else{ //units are deg (lat/lon)
            xBuffer = this->getGridResolution() * 3; //in degrees
            yBuffer = this->getGridResolution() * 3; //in degrees
        }
        
        CPLDebug("MOBILE_APP", "Y buffer = %f", xBuffer);
        CPLDebug("MOBILE_APP", "X buffer = %f", yBuffer);
    
        bounds[0] = maxY + yBuffer;
        bounds[1] = maxX + xBuffer;
        bounds[2] = minY - yBuffer;
        bounds[3] = minX - xBuffer;
    }
#else
    bounds[0] = maxY + GetWxModelBuffer(maxY - minY);
    bounds[1] = maxX + GetWxModelBuffer(maxX - minX);
    bounds[2] = minY - GetWxModelBuffer(maxY - minY);
    bounds[3] = minX - GetWxModelBuffer(maxX - minX);
#endif
    
    return 0;
}

/**
 * \brief Default buffer size for wxModel runs.
 *
 * Return 20% of the delta as base implementation, unless it is less than 1.0,
 * then use 1.0 degrees
 *
 * \param delta size difference in a dimension
 * \return buffer size in degrees
 */
double wxModelInitialization::GetWxModelBuffer(double delta)
{
    double buffer = delta * 0.2;
    return buffer > 1.0 ? buffer : 1.0;        
}

/**
 * Fetch the actual file for a given forecast.  File name handling is done
 * with GDAL cpl_conv functions.
 *
 * @param demFile full path to dem to extract the bounds
 * @param nHours how many forecast hours
 *
 * @return name of the file downloaded.
 *
 * @warning caller must pass the *full path* to the dem for proper
 *          return string
 *
 */
std::string wxModelInitialization::fetchForecast( std::string demFile,
                          int nHours )
{
    /*
     * Get the bounds of the dem
     */
    if(nHours < getStartHour() || nHours > getEndHour())
    {
        const char *pszErrMsg;
        pszErrMsg = CPLSPrintf("Invalid duration for the forecast.  You must " \
                               "select a duration at least one time_step "
                               "long for any given model.  The minimum value "
                               "for this model is %d, and the maximum is %d.",
                               getStartHour(), getEndHour() );
        throw std::logic_error(pszErrMsg);
    }
    GDALDataset *poDS = (GDALDataset*)GDALOpen( demFile.c_str(), GA_ReadOnly );
    double bounds[4];
    if( !GDALGetBounds( poDS, bounds ) )
    {
        throw badForecastFile("Could not download weather forecast, invalid "
                              "projection for the DEM");
    }

    /*
     * Buffer the bounds
     */

    ComputeWxModelBuffer(poDS, bounds);

    GDALClose( (GDALDatasetH)poDS );

    int bSaveForecast =
        CSLTestBoolean( CPLGetConfigOption( "NINJA_WX_KEEP_FORECAST", "NO" ) );
    /*
     * Save the file to a temporary place
     */
    std::string tempFileName( CPLGenerateTempFilename( "WINDNINJAFORECAST") );
    std::string url = generateUrl( bounds[0], bounds[1], bounds[2],
                                   bounds[3], nHours );

    /*
     * Get the data using CPLHTTPFetch
     */

    VSILFILE *fout;

    std::string urlAddress = "http://";
    urlAddress.append(url);

    CPLHTTPResult *poResult = CPLHTTPFetch( urlAddress.c_str(), NULL );
    CPLDebug( "WINDNINJA", "Forecast URL: %s", urlAddress.c_str() );
    if( poResult == NULL )
    {
        CPLHTTPDestroyResult( poResult );
        throw ( badForecastFile( "CPLHTTPResult is NULL!" ) );
    }

    if( poResult->nStatus != 0 )
    {
        CPLHTTPDestroyResult( poResult );
        throw ( badForecastFile( poResult->pszErrBuf ) );
    }

    fout = VSIFOpenL( tempFileName.c_str(), "w" );
    if( fout == NULL )
    {
        CPLHTTPDestroyResult( poResult );
        throw ( badForecastFile( "Failed to download forecast." ) );
    }
    VSIFWriteL( poResult->pabyData, poResult->nDataLen, 1, fout );
    CPLHTTPDestroyResult( poResult );
    VSIFCloseL( fout );

    /*
     * See if curl actually downloaded a netcdf file, if not delete the
     * temp file and leave
     */
    //Acquire a lock to protect the non-thread safe netCDF library
    {
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    int status, ncid;
    status = nc_open( tempFileName.c_str(), 0, &ncid );
    if ( status != NC_NOERR ) {
        if( !bSaveForecast )
        {
            VSIUnlink( tempFileName.c_str() );
        }
        throw ( badForecastFile( "Failed to download forecast. File is not an *.nc file" ) );
    }
    nc_close( ncid );
    }

    /*
     * Check the file to see if it contains "good" values
     */
    wxModelFileName = tempFileName;
    try{
        checkForValidData();    //throws a badForecastFile on fail
    }catch (badForecastFile& e) {   //catch a badForecastFile
        if( !bSaveForecast )
        {
            VSIUnlink(wxModelFileName.c_str()); //do some clean-up (delete the bad file)
        }
        throw;  //rethrow the exception to be caught somewhere else
    }

    /*
     * Rename the file
     */
    std::string path( CPLGetDirname( demFile.c_str() ) );
    std::string fileName( CPLGetFilename( demFile.c_str() ) );
    std::string newPath( path + "/" + getForecastIdentifier()
             + "-" + fileName + "/" );

    VSIMkdir( newPath.c_str(), 0777 );

    wxModelFileName = tempFileName;
    std::string outputPath = newPath + std::string(CPLGetBasename(generateForecastName().c_str())) + "/";
    VSIMkdir( outputPath.c_str(), 0777 );

    std::string newFileName( outputPath + generateForecastName() );

    /*
     * check for file existance? if the file is already there, this fails on
     * Linux, but not win32.
     */

    if( !bSaveForecast )
    {
        CPLMoveFile( newFileName.c_str(), tempFileName.c_str() );
    }
    else
    {
        CPLCopyFile( newFileName.c_str(), tempFileName.c_str() );
    }
    wxModelFileName = newFileName;
    try	{
    checkForValidData();
    }
    catch( badForecastFile &e ) {
    std::cout << endl << endl << endl << endl << e.what() << endl << endl << endl << endl;
    }

    return newFileName;
}

/**
 * Fetch a list of times that the forecast holds for the dataset.
 *
 * @param timeZoneString Time zone name from the file "date_time_zonespec.csv".
 * @throw runtim_error for bad file i/o
 * @return a vector of boost::posix_time::ptime objects for the forecast
 */
std::vector<blt::local_date_time> wxModelInitialization::getTimeList(std::string timeZoneString)
{
    if( aoCachedTimes.size() > 0 ) {
        return aoCachedTimes;
    }
    boost::local_time::time_zone_ptr timeZonePtr;
    timeZonePtr = globalTimeZoneDB.time_zone_from_region(timeZoneString.c_str());
    if( timeZonePtr == NULL ) {
        ostringstream os;
        os << "The time zone string: " << timeZoneString.c_str()
       << " does not match any in "
       << "the time zone database file: date_time_zonespec.csv.";
        throw std::runtime_error( os.str() );
    }
    aoCachedTimes = getTimeList( blt::time_zone_ptr ( timeZonePtr ) );
    return aoCachedTimes;
}
/**
 * Fetch a list of times that the forecast holds for the dataset, given
 * a variable name.
 *
 * @param timeZoneString Time zone name from the file "date_time_zonespec.csv".
 * @param pszVariable variable name.
 * @throw runtim_error for bad file i/o
 * @return a vector of boost::posix_time::ptime objects for the forecast
 */
std::vector<blt::local_date_time>
wxModelInitialization::getTimeList(const char *pszVariable,
                                   std::string timeZoneString)
{
    if( aoCachedTimes.size() > 0 ) {
        return aoCachedTimes;
    }
    boost::local_time::time_zone_ptr timeZonePtr;
    timeZonePtr = globalTimeZoneDB.time_zone_from_region(timeZoneString.c_str());
    if( timeZonePtr == NULL ) {
        ostringstream os;
        os << "The time zone string: " << timeZoneString.c_str()
       << " does not match any in "
       << "the time zone database file: date_time_zonespec.csv.";
        throw std::runtime_error( os.str() );
    }
    aoCachedTimes = getTimeList(pszVariable, blt::time_zone_ptr(timeZonePtr));
    return aoCachedTimes;
}

/**
 * Wrapper for default getTimeList.
 *
 * Most models (RUC, NAM, GFS) have constant time under the dimension 'time'.
 * This getTimeList call uses the 'Wind_speed' variable to fetch the time
 * dimension and return a list of times.
 *
 * Fetch a list of times that the forecast holds for the dataset.  It
 * is assumed that the time variable is "time" and the units string is
 * "units".  If this is not the case, such as in ndfd, this function needs
 * to be overridden.  Uses the netcdf api, so we need omp critical sections.
 *
 * @param timeZonePtr Pointer to a boost time zone object.
 * @throw runtime_error for bad file i/o
 * @return a vector of boost::posix_time::ptime objects for the forecast
 */
std::vector<blt::local_date_time> wxModelInitialization::getTimeList(blt::time_zone_ptr timeZonePtr)
{
    if( aoCachedTimes.size() > 0 )
        return aoCachedTimes;
    aoCachedTimes = getTimeList(NULL, timeZonePtr);
    return aoCachedTimes;
}

/**
 * Fetch a list of times that the forecast holds for the dataset.  It
 * is assumed that the time variable is "time" and the units string is
 * "units".  If this is not the case, such as in ndfd, this function needs
 * to be overridden.  Uses the netcdf api, so we need omp critical sections.
 *
 * @param timeZonePtr Pointer to a boost time zone object.
 * @param pszVariable the name of the variable to use.
 * @throw runtime_error for bad file i/o
 * @return a vector of boost::posix_time::ptime objects for the forecast
 */
std::vector<blt::local_date_time>
wxModelInitialization::getTimeList(const char *pszVariable,
                                   blt::time_zone_ptr timeZonePtr)
{

    std::vector<blt::local_date_time>timeList;

    //==========If the file is NAM or HRRR GRIB============================================================
    if(this->getForecastIdentifier() == "NCEP-NAM-12km-SURFACE-GRIB2" ||
       this->getForecastIdentifier() == "NCEP-HRRR-3km-SURFACE"){
        //Just 1 time step per forecast file for now.
        GDALDataset *srcDS;
        
        if( strstr( wxModelFileName.c_str(), ".tar" ) ){
            srcDS = (GDALDataset*)GDALOpenShared(CPLSPrintf( "/vsitar/%s", wxModelFileName.c_str() ), GA_ReadOnly);
        }
        else{        
            srcDS = (GDALDataset*)GDALOpenShared( wxModelFileName.c_str(), GA_ReadOnly );
        }


        if( srcDS == NULL ) {
            CPLDebug( "wxModelInitialization::getTimeList()",
                    "Bad forecast file" );
        }

        //get time info from 10u band
        GDALRasterBand *poBand;
        if( this->getForecastIdentifier() == "NCEP-NAM-12km-SURFACE-GRIB2" ) {
            poBand = srcDS->GetRasterBand( 9 );
        }
        else {
            poBand = srcDS->GetRasterBand( 33 );
        }

        const char *vt;
        vt = poBand->GetMetadataItem( "GRIB_VALID_TIME" );

        std::string validTimeString( vt );

        /*
         * Clean the string for the boost constructor.  Remove the prefix,
         * suffix, and delimiters
         */
        int pos = -1;
        while( validTimeString.find( " " ) != validTimeString.npos ) {
            pos = validTimeString.find( " " );
            validTimeString.erase( pos, 1 );
        }
        pos = validTimeString.find( "secUTC" );
        if( pos != validTimeString.npos ) {
            validTimeString.erase( pos, strlen( "secUTC" ) );
        }

        /*
         * Make a posix time in UTC/GMT
         */
        bpt::ptime time_t_epoch( boost::gregorian::date( 1970,1,1 ) );
        long validTime = atoi( validTimeString.c_str() );

        bpt::time_duration duration( 0,0,validTime );

        bpt::ptime first_pt( time_t_epoch + duration );  //UTC time
        blt::local_date_time first_pt_local( first_pt, timeZonePtr ); //local time
        timeList.push_back( first_pt_local );

        return timeList;

    }
    //===========End if NAM-GRIB=============================================================================


    //Acquire a lock to protect the non-thread safe netCDF library
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    int status, ncid, ndims, nvars, ngatts, unlimdimid;
    nc_type vartype;
    int varndims, varnatts;
    double *varvals;
    int vardimids[NC_MAX_VAR_DIMS];   /* dimension IDs */

    /*
     * Open the dataset
     */
    status = nc_open( wxModelFileName.c_str(), 0, &ncid );
    if ( status != NC_NOERR ) {
        ostringstream os;
        os << "The netcdf file: " << wxModelFileName
           << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }

    std::string timename;
    if(this->getForecastIdentifier() == "WRF-SURFACE" ||
       this->getForecastIdentifier() == "WRF-3D"){    //if forecast is WRF, parse times differently
            timename = "Times";
    }
    else if(pszVariable == NULL)
        timename = GetTimeName("time");
    else
        timename = GetTimeName(pszVariable);

    /*
     * If we can't get simple data from the file, return false
     */
    status = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);
    if ( status != NC_NOERR ) {
        ostringstream os;
        os << "The variables in the netcdf file: " << wxModelFileName
           << " cannot be read\n";
        throw std::runtime_error( os.str() );
    }

    /*
     * Check the variable names, return false if any aren't found
     */
    int varid;
    status = nc_inq_varid( ncid, timename.c_str(), &varid );
    if( status != NC_NOERR ) {
        ostringstream os;
        os << "The variable \"time\" in the netcdf file: "
           << wxModelFileName << " cannot be read\n";
        throw std::runtime_error( os.str() );
    }

    //==============If the forecast is WRF, parse Times============================

    if (timename == "Times"){
        /*
         * Check to see if we can read Times variable
         */
        status = nc_inq_var( ncid, varid, 0, &vartype, &varndims, vardimids,
                            &varnatts );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The values for variable \"Times\" "
                << " in the netcdf file: " << wxModelFileName
                << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }


        /*
         * Get varid for U10 --> use this to get length of time dimension
         */
        status = nc_inq_varid( ncid, "U10", &varid );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The variable \"U10\" in the netcdf file: "
            << wxModelFileName << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }


        /*
         * Get dimid for Time in U10
         */
        int dimid;
        status = nc_inq_dimid(ncid, "Time", &dimid );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The dimension \"Time\" "
                << " in the netcdf file: " << wxModelFileName
                << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }


        /*
         * Get length of the time dimension in U10
         */
        size_t time_len;
        status = nc_inq_dimlen( ncid, dimid, &time_len );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The time dimension for variable \"Times\" in the netcdf file: "
            << wxModelFileName << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }

        /*
         * Reset varid to 'Times'
         */
        int varid;
        status = nc_inq_varid( ncid, timename.c_str(), &varid );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The variable \"U10\" in the netcdf file: "
            << wxModelFileName << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }

        /*
         * Get dimid for DateStrLen
         */
        //int dimid;
        status = nc_inq_dimid(ncid, "DateStrLen", &dimid );
        //cout<<"dimid =" <<dimid<<endl;
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The dimension \"DateStrLen\" "
                << " in the netcdf file: " << wxModelFileName
                << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }

        /*
         * Get the length of the time string
         */
        size_t t_len;
        status = nc_inq_dimlen( ncid, dimid, &t_len );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The dimension for variable \"Times\" in the netcdf file: "
            << wxModelFileName << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }

        for( unsigned int t = 0;t < time_len;t++ ) {

            /*
             * Get value for one Times variable
            */
            char* tp = new char[t_len + 1];
            //char tp[t_len + 1];
            for (int i=0; i<t_len; i++){
                const size_t varindex[] = {t,i};  /* where to get value from */
                status = nc_get_var1_text( ncid, varid, varindex, tp+i );
            }
            if( status != NC_NOERR ) {
                ostringstream os;
                os << "The values for variable \"time\" "
                    << " in the netcdf file: " << wxModelFileName
                    << " cannot be read\n";
                throw std::runtime_error( os.str() );
            }

            tp[t_len] = '\0';
            std::string refString( tp );
            delete[] tp;

            /*
             * Clean the string for the boost constructor.  Remove the prefix,
             * suffix, and delimiters
             */
            int pos = -1;
            if( refString.find( "_" ) != refString.npos ) {
                pos = refString.find( "_" );
                refString.replace(pos, 1, 1, 'T');
                //refString.erase( pos, 1 );
            }
            while( refString.find( "-" ) != refString.npos ) {
                pos = refString.find( "-" );
                refString.erase( pos, 1 );
            }
            while( refString.find( ":" ) != refString.npos ) {
                pos = refString.find( ":" );
                refString.erase( pos, 1 );
            }
            pos = refString.find("Hour since ");
            if(pos != refString.npos)
            {
                refString.erase(pos, strlen("Hour since "));
            }

            /*
             * Make a posix time in UTC/GMT
             */
            bpt::ptime reference_pt;
            reference_pt = bpt::from_iso_string( refString );
            bpt::ptime first_pt( reference_pt );
            blt::local_date_time first_pt_local(first_pt, timeZonePtr);
            timeList.push_back( first_pt_local );

        } // end for loop
    } // end if for WRF files

    //===========If forecast is NOT WRF, parse times====================

    else {
        size_t t_len;
        status = nc_inq_attlen( ncid, varid, "units", &t_len );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The units for variable \"time\" in the netcdf file: "
            << wxModelFileName << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }
        char* tp = new char[t_len + 1];

        status = nc_get_att_text( ncid, varid, "units", tp );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The size of the units for variable \"time\" "
                << " in the netcdf file: " << wxModelFileName
                << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }
        tp[t_len] = '\0';
        std::string refString( tp );
        delete[] tp;

        /*
         * Clean the string for the boost constructor.  Remove the prefix,
         * suffix, and delimiters
         */

        std::string remove( "hours since " );
        int pos = -1;
        if( refString.find( remove ) != refString.npos )
            refString.erase( 0, remove.size() );

        if( refString.find( "Z" ) != refString.npos ) {
            pos = refString.find( "Z" );
            refString.erase( pos, 1 );
        }
        while( refString.find( ":" ) != refString.npos ) {
            pos = refString.find( ":" );
            refString.erase( pos, 1 );
        }
        while( refString.find( "-" ) != refString.npos ) {
            pos = refString.find( "-" );
            refString.erase( pos, 1 );
        }
        pos = refString.find( "Hour since " );
        if( pos != refString.npos )
        {
            refString.erase(pos, strlen("Hour since "));
        }

        /*
         * Make a posix time in UTC/GMT
         */
        bpt::ptime reference_pt;
        reference_pt = bpt::from_iso_string( refString );

        /*
         * Retrieve the first time in time1
         * Create an array
         * Fill the array
         */
        status = nc_inq_var( ncid, varid, 0, &vartype, &varndims, vardimids,
                            &varnatts );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The values for variable \"time\" "
                << " in the netcdf file: " << wxModelFileName
                << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }
        double d;
        static size_t varindex[] = {0};  /* where to get value from */
        status = nc_get_var1_double( ncid, varid, varindex, &d );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The values for variable \"time\" "
                << " in the netcdf file: " << wxModelFileName
                << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }
        /*
         * Add the duration in hours to the reference time
         */
        int timeid;
        size_t timelen;
        status = nc_inq_dimid( ncid, timename.c_str(), &timeid );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The values for variable \"time\" "
                << " in the netcdf file: " << wxModelFileName
                << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }
        status = nc_inq_dimlen( ncid, timeid, &timelen );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The values for variable \"time\" "
                << " in the netcdf file: " << wxModelFileName
                << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }
        varvals = new double[timelen];
        status = nc_get_var_double(ncid, varid, varvals);
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The values for variable \"time\" "
                << " in the netcdf file: " << wxModelFileName
                << " cannot be read\n";
            throw std::runtime_error( os.str() );
        }

        status = nc_close( ncid );
        if( status != NC_NOERR ) {
            ostringstream os;
            os << "The netcdf file: " << wxModelFileName
                << " cannot be closed\n";
            throw std::runtime_error( os.str() );
        }

        /*
         * Add the duration in hours to the reference time
         */

        for( unsigned int i = 0;i < timelen;i++ ) {
            bpt::time_duration td = bpt::hours( varvals[i] );
            bpt::ptime first_pt( reference_pt + td );
            blt::local_date_time first_pt_local(first_pt, timeZonePtr);
            timeList.push_back( first_pt_local );
        }

        delete[] varvals;

    } // end if for non-WRF forecasts

    return timeList;
}

/**
 *
 */
#ifdef NOMADS_ENABLE_3D
void wxModelInitialization::set3dGrids( WindNinjaInputs &input, Mesh const& mesh )
{
    return;
}

void wxModelInitialization::setGlobalAttributes(WindNinjaInputs &input)
{
    return;
}

void wxModelInitialization::buildWxMeshes(WindNinjaInputs &input, Mesh const& mesh)
{
    return;
}

void wxModelInitialization::buildWxScalarFields()
{
    return;
}

void wxModelInitialization::allocate(Mesh const& mesh)
{
    return;
}

void wxModelInitialization::deallocateTemp()
{
    return;
}
#endif

/**
 * Initializes u0, v0, w0, and cloud from a surface wx model.
 * @param input WindNinjaInputs object storing necessary input information.
 * @param mesh Mesh object for simulation.
 * @param u0 Initial field of u speeds.
 * @param v0 Initial field of v speeds.
 * @param w0 Initial field of w speeds.
 * @param cloud Cloud cover grid.
 */
void wxModelInitialization::initializeFields(WindNinjaInputs &input,
                         Mesh const& mesh,
                         wn_3dScalarField& u0,
                         wn_3dScalarField& v0,
                         wn_3dScalarField& w0,
                         AsciiGrid<double>& cloud,
                         AsciiGrid<double>& L,
                         AsciiGrid<double>& u_star,
                         AsciiGrid<double>& bl_height)
{
    int i, j, k;

    windProfile profile;
    profile.profile_switch = windProfile::monin_obukov_similarity;	//switch that detemines what profile is used...

    //input.inputWindHeight = 10.0;               //HARD CODED HERE AS 10 METERS, WE SHOULD DO THIS RIGHT SOMETIME!!!
    input.surface.Z = (*this).Get_Wind_Height();    //(ACTUALLY READ THE HEIGHT OF THE WINDS FROM THE FORECAST FILE)

    //make sure rough_h is set to zero if profile switch is 0 or 2

    //Read in wxModel grids (includes speed, direction, temperature and cloud cover grids)

    AsciiGrid<double> airTempGrid_wxModel;
    AsciiGrid<double> cloudCoverGrid_wxModel;
    AsciiGrid<double> speedInitializationGrid_wxModel;
    AsciiGrid<double> dirInitializationGrid_wxModel;
    AsciiGrid<double> uGrid_wxModel;
    AsciiGrid<double> vGrid_wxModel;
    AsciiGrid<double> wGrid_wxModel;


    setSurfaceGrids( input, airTempGrid_wxModel, cloudCoverGrid_wxModel, uGrid_wxModel,
             vGrid_wxModel, wGrid_wxModel );

#ifdef NOMADS_ENABLE_3D
    set3dGrids(input, mesh);
#endif

    //Make final grids with same header as dem
    AsciiGrid<double> airTempGrid;
    airTempGrid.set_headerData(input.dem);

    cloud.set_headerData(input.dem);

    AsciiGrid<double> speedInitializationGrid;
    speedInitializationGrid.set_headerData(input.dem);

    AsciiGrid<double> dirInitializationGrid;
    dirInitializationGrid.set_headerData(input.dem);

    AsciiGrid<double> uInitializationGrid;
    uInitializationGrid.set_headerData(input.dem);

    AsciiGrid<double> vInitializationGrid;
    vInitializationGrid.set_headerData(input.dem);

    //Interpolate from original wxModel grids to dem coincident grids
    airTempGrid.interpolateFromGrid(airTempGrid_wxModel, AsciiGrid<double>::order1);
    cloud.interpolateFromGrid(cloudCoverGrid_wxModel, AsciiGrid<double>::order1);
    uInitializationGrid.interpolateFromGrid(uGrid_wxModel, AsciiGrid<double>::order1);
    vInitializationGrid.interpolateFromGrid(vGrid_wxModel, AsciiGrid<double>::order1);
    /*
    ** Fill in speed and direction grids from interpolated U and V grids.
    */
    for(int i=0; i<speedInitializationGrid.get_nRows(); i++) {
        for(int j=0; j<speedInitializationGrid.get_nCols(); j++) {
            wind_uv_to_sd(uInitializationGrid(i,j), vInitializationGrid(i,j), &(speedInitializationGrid)(i,j), &(dirInitializationGrid)(i,j));
        }
    }

#ifdef NINJA_SPEED_TESTING
    // adjustment to increase drag on 10 m wx model winds
    speedInitializationGrid = speedInitializationGrid * input.speedDampeningRatio;
    for(int i=0; i<speedInitializationGrid.get_nRows(); i++) {
        for(int j=0; j<speedInitializationGrid.get_nCols(); j++) {
            wind_sd_to_uv(speedInitializationGrid(i,j), dirInitializationGrid(i,j), &(uInitializationGrid)(i,j), &(vInitializationGrid)(i,j));
        }
    }
#endif

    //Check for noData values
    if(airTempGrid.checkForNoDataValues() ||
       cloud.checkForNoDataValues() ||
       speedInitializationGrid.checkForNoDataValues() ||
       dirInitializationGrid.checkForNoDataValues())
    {
        throw std::logic_error("NoData values found in wx model interpolated grids.");
    }

    if(input.wxModelAsciiOutFlag==true || input.wxModelShpOutFlag==true || input.wxModelGoogOutFlag == true)
    {
        speedInitializationGrid_wxModel.set_headerData(uGrid_wxModel);
        dirInitializationGrid_wxModel.set_headerData(uGrid_wxModel);

        //now make speed and direction from u,v components
        for(int i=0; i<speedInitializationGrid_wxModel.get_nRows(); i++) {
            for(int j=0; j<speedInitializationGrid_wxModel.get_nCols(); j++) {
                if( uGrid_wxModel(i,j) == uGrid_wxModel.get_NoDataValue() ||
                    vGrid_wxModel(i,j) == vGrid_wxModel.get_NoDataValue() ) {
                    speedInitializationGrid_wxModel(i,j) = speedInitializationGrid_wxModel.get_NoDataValue();
                    dirInitializationGrid_wxModel(i,j) = dirInitializationGrid_wxModel.get_NoDataValue();
                }
                else
                    wind_uv_to_sd(uGrid_wxModel(i,j), vGrid_wxModel(i,j), &(speedInitializationGrid_wxModel)(i,j), &(dirInitializationGrid_wxModel)(i,j));
            }
        }

        //Write raw model output files
        std::string rootname, path;
        //rootname = CPLGetBasename(input.forecastFilename.c_str());
        path = CPLGetPath(input.forecastFilename.c_str());

        ostringstream wxModelTimestream;
        blt::local_time_facet* wxModelOutputFacet;
        wxModelOutputFacet = new blt::local_time_facet();
        wxModelTimestream.imbue(std::locale(std::locale::classic(), wxModelOutputFacet));
        wxModelOutputFacet->format("%m-%d-%Y_%H%M");
        wxModelTimestream << input.ninjaTime;
        rootname = getForecastIdentifier() + "-" + wxModelTimestream.str();

        std::string wxModelVelFileTemp = rootname + "_vel";
        std::string wxModelAngFileTemp = rootname + "_ang";
        std::string wxModelCldFileTemp = rootname + "_cld";
        std::string wxModelShpFileTemp = rootname;
        std::string wxModelDbfFileTemp = rootname;
        std::string wxModelKmlFileTemp = rootname;
        std::string wxModelKmzFileTemp = rootname;
        std::string wxModelLegFileTemp = rootname;
        std::string dateTimewxModelLegFileTemp = rootname + ".date_time";

        input.wxModelVelFile = CPLFormFilename(path.c_str(), wxModelVelFileTemp.c_str(), ".asc");
        input.wxModelAngFile = CPLFormFilename(path.c_str(), wxModelAngFileTemp.c_str(), ".asc");
        input.wxModelCldFile = CPLFormFilename(path.c_str(), wxModelCldFileTemp.c_str(), ".asc");
        input.wxModelShpFile = CPLFormFilename(path.c_str(), wxModelShpFileTemp.c_str(), "shp");
        input.wxModelDbfFile = CPLFormFilename(path.c_str(), wxModelDbfFileTemp.c_str(), "dbf");
        input.wxModelKmlFile = CPLFormFilename(path.c_str(), wxModelKmlFileTemp.c_str(), "kml");
        input.wxModelKmzFile = CPLFormFilename(path.c_str(), wxModelKmzFileTemp.c_str(), "kmz");
        input.wxModelLegFile = CPLFormFilename(path.c_str(), wxModelLegFileTemp.c_str(), "bmp");
        input.dateTimewxModelLegFile = CPLFormFilename(path.c_str(), dateTimewxModelLegFileTemp.c_str(), "bmp");
    }
    velocityUnits::fromBaseUnits(speedInitializationGrid_wxModel, input.outputSpeedUnits);

#pragma omp parallel sections
    {
    //write FARSITE files
#pragma omp section
    {
        try{
        if(input.wxModelAsciiOutFlag==true)
            {
            AsciiGrid<double> tempCloud(cloudCoverGrid_wxModel);
            tempCloud *= 100.0;
            tempCloud.write_Grid(input.wxModelCldFile.c_str(), 0);
            dirInitializationGrid_wxModel.write_Grid(input.wxModelAngFile.c_str(), 0);
            speedInitializationGrid_wxModel.write_Grid(input.wxModelVelFile.c_str(), 2);
            }
        }catch (exception& e)
        {
            input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during wxModel fire behavior file writing: %s", e.what());
        }catch (...)
        {
            input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during wxModel fire behavior file writing: Cannot determine exception type.");
        }

    }//end omp section

    //write shape files
#pragma omp section
    {
        try{
        if(input.wxModelShpOutFlag==true)
            {
            ShapeVector wxModelShapeFiles;

            wxModelShapeFiles.setDirGrid(dirInitializationGrid_wxModel);
            wxModelShapeFiles.setSpeedGrid(speedInitializationGrid_wxModel);
            wxModelShapeFiles.setDataBaseName(input.wxModelDbfFile);
            wxModelShapeFiles.setShapeFileName(input.wxModelShpFile);
            wxModelShapeFiles.makeShapeFiles();
            }
        }catch (exception& e)
        {
            input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during shape file writing: %s", e.what());
        }catch (...)
        {
            input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during shape file writing: Cannot determine exception type.");
        }
    }//end omp section

    //write kmz file
#pragma omp section
    {
        try{
        if(input.wxModelGoogOutFlag == true)
            {
            KmlVector wxModelKmlFiles;

            wxModelKmlFiles.setKmlFile(input.wxModelKmlFile);
            wxModelKmlFiles.setKmzFile(input.wxModelKmzFile);
            wxModelKmlFiles.setDemFile(input.dem.fileName);
            #ifdef EMISSIONS
            wxModelKmlFiles.setDustFlag(input.dustFlag);
            #endif
            wxModelKmlFiles.setLegendFile(input.wxModelLegFile);
            wxModelKmlFiles.setDateTimeLegendFile(input.dateTimewxModelLegFile, input.ninjaTime);
            wxModelKmlFiles.setSpeedGrid(speedInitializationGrid_wxModel, input.outputSpeedUnits);
            wxModelKmlFiles.setDirGrid(dirInitializationGrid_wxModel);
            wxModelKmlFiles.setLineWidth(input.wxModelGoogLineWidth);
            wxModelKmlFiles.setTime(input.ninjaTime);
            std::vector<boost::local_time::local_date_time> times(getTimeList(input.ninjaTimeZone));
            wxModelKmlFiles.setWxModel(getForecastIdentifier(), times[0]);

            if(wxModelKmlFiles.writeKml(input.wxModelGoogSpeedScaling))
            {
                if(wxModelKmlFiles.makeKmz())
                    wxModelKmlFiles.removeKmlFile();
            }
            }
        }catch (exception& e)
        {
            input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during Google Earth file writing: %s", e.what());
        }catch (...)
        {
            input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Exception caught during Google Earth file writing: Cannot determine exception type.");
        }
    }//end omp section

    }	//end parallel sections region

    //Check if grids are coincident
    if(!input.dem.checkForCoincidentGrids(airTempGrid))
        throw std::logic_error("Dem and airTempGrid are not coincident in wx model interpolation.");
    else if(!input.dem.checkForCoincidentGrids(cloud))
        throw std::logic_error("Dem and cloud are not coincident in wx model interpolation.");
    else if(!input.dem.checkForCoincidentGrids(speedInitializationGrid))
        throw std::logic_error("Dem and speedInitializationGrid are not coincident in wx model interpolation.");
    else if(!input.dem.checkForCoincidentGrids(dirInitializationGrid))
        throw std::logic_error("Dem and dirInitializationGrid are not coincident in wx model interpolation.");
    else if(!input.dem.checkForCoincidentGrids(uInitializationGrid))
        throw std::logic_error("Dem and uInitializationGrid are not coincident in wx model interpolation.");
    else if(!input.dem.checkForCoincidentGrids(vInitializationGrid))
        throw std::logic_error("Dem and vInitializationGrid are not coincident in wx model interpolation.");

    //Set windspeed grid for diurnal computation
    input.surface.set_windspeed(speedInitializationGrid);

    //initialize u0, v0, w0 equal to zero
#pragma omp parallel for default(shared) private(i,j,k)
    for(k=0;k<mesh.nlayers;k++)
    {
        for(i=0;i<mesh.nrows;i++)
        {
            for(j=0;j<mesh.ncols;j++)
            {
                u0(i, j, k) = 0.0;
                v0(i, j, k) = 0.0;
                w0(i, j, k) = 0.0;
            }
        }
    }

    //Monin-Obukhov length, surface friction velocity, and atmospheric boundary layer height
    L.set_headerData(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0.0);
    u_star.set_headerData(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0.0);
    bl_height.set_headerData(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), -1.0);

    //These are only needed if diurnal is turned on...
    AsciiGrid<double> height;	//height of diurnal flow above "z=0" in log profile
    AsciiGrid<double> uDiurnal;
    AsciiGrid<double> vDiurnal;
    AsciiGrid<double> wDiurnal;


    //compute diurnal wind, Monin-Obukhov length, surface friction velocity, and ABL height
    if(input.diurnalWinds == true)
    {
        height.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);	//height of diurnal flow above "z=0" in log profile
        uDiurnal.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);
        vDiurnal.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);
        wDiurnal.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);

        double aspect_temp = 0;	//just placeholder, basically
        double slope_temp = 0;	//just placeholder, basically

        Solar solar(input.ninjaTime, input.latitude, input.longitude, aspect_temp, slope_temp);
        Aspect aspect(&input.dem, input.numberCPUs);
        Slope slope(&input.dem, input.numberCPUs);
        Shade shade(&input.dem, solar.get_theta(), solar.get_phi(), input.numberCPUs);

        addDiurnal diurnal(&uDiurnal, &vDiurnal, &wDiurnal, &height, &L,
                        &u_star, &bl_height, &input.dem, &aspect, &slope,
                        &shade, &solar, &input.surface, &cloud,
                        &airTempGrid, input.numberCPUs, input.downDragCoeff,
                        input.downEntrainmentCoeff, input.upDragCoeff,
                        input.upEntrainmentCoeff);


        ////Testing: Print diurnal component as .kmz file
        //AsciiGrid<double> *diurnalVelocityGrid, *diurnalAngleGrid;
        //diurnalVelocityGrid=NULL;
        //diurnalAngleGrid=NULL;

        //KmlVector diurnalKmlFiles;
        //diurnalKmlFiles.com = input.Com;

        //diurnalAngleGrid = new AsciiGrid<double> ();
        //diurnalAngleGrid->set_headerData(input.dem);
        //diurnalVelocityGrid = new AsciiGrid<double> ();
        //diurnalVelocityGrid->set_headerData(input.dem);

        //double interMedVal;

        ////Change from u,v components to speed and direction
        //for(int i=0; i<diurnalVelocityGrid->nRows; i++)
        //{
        //	for(int j=0; j<diurnalVelocityGrid->nCols; j++)
        //	{
        //
        //	   (*diurnalVelocityGrid)[i][j]=pow(((uDiurnal)[i][j]*(uDiurnal)[i][j]+(vDiurnal)[i][j]*(vDiurnal)[i][j]),0.5);       //calculate velocity magnitude (in x,y plane; I decided to include z here so the wind is the total magnitude wind)
        //
        //             if ((uDiurnal)[i][j]==0.0 && (vDiurnal)[i][j]==0.0)
        //                  interMedVal=0.0;
        //             else
        //  		          interMedVal=-atan2((uDiurnal)[i][j], -(vDiurnal)[i][j]);
        //             if(interMedVal<0)
        //                  interMedVal+=2.0*pi;
        //             (*diurnalAngleGrid)[i][j]=(180.0/pi*interMedVal);
        //	}
        //}

        //diurnalVelocityGrid->write_Grid("diurnalSpeed.asc", 2);
        //
        //diurnalKmlFiles.setKmlFile("diurnal.kml");
        //diurnalKmlFiles.setKmzFile("diurnal.kmz");
        //diurnalKmlFiles.setDemFile(input.dem.fileName);
        //diurnalKmlFiles.setPrjString(input.prjString);
        //diurnalKmlFiles.setLegendFile("diurnalLegend.txt");
        //diurnalKmlFiles.setSpeedGrid(*diurnalVelocityGrid, input.outputSpeedUnits);
        //diurnalKmlFiles.setDirGrid(*diurnalAngleGrid);
        //diurnalKmlFiles.setLineWidth(googLineWidth);
        //diurnalKmlFiles.setTime(input.time);
        //diurnalKmlFiles.setDate(input.date);
        //if(diurnalKmlFiles.writeKml(googSpeedScaling))
        //{
        //	if(diurnalKmlFiles.makeKmz())
        //		diurnalKmlFiles.removeKmlFile();
        //}
        //if(diurnalAngleGrid)
        //{
        //	delete diurnalAngleGrid;
        //	diurnalAngleGrid=NULL;
        //}
        //if(diurnalVelocityGrid)
        //{
        //	delete diurnalVelocityGrid;
        //	diurnalVelocityGrid=NULL;
        //}


        //write files for debugging
        //shade->write_Grid("shade.asc", -1);
        //height->write_Grid("height.asc", 0);
        //aspect->write_Grid("aspect.asc",0);
        //slope->write_Grid("slope.asc", 1);

    }else{	//compute neutral ABL height

    double f;

    //compute f -> Coriolis parameter
    if(input.latitude<=90.0 && input.latitude>=-90.0)
        {
        f = (1.4544e-4) * sin(pi/180 * input.latitude);	// f = 2 * omega * sin(theta)
        // f should be about 10^-4 for mid-latitudes
        // (1.4544e-4) here is 2 * omega = 2 * (2 * pi radians) / 24 hours = 1.4544e-4 seconds^-1
        // obtained from Stull 1988 book
        if(f<0)
            f = -f;
        }else{
        f = 1e-4;	//if latitude is not available, set f to mid-latitude value
    }

    if(f==0.0)	//zero will give division by zero below
        f = 1e-8;	//if latitude is zero, set f small

    //compute neutral ABL height
#pragma omp parallel for default(shared) private(i,j)
    for(i=0;i<input.dem.get_nRows();i++)
        {
        for(j=0;j<input.dem.get_nCols();j++)
            {
            u_star(i,j) = speedInitializationGrid(i,j)*0.4/(log((input.inputWindHeight+input.surface.Rough_h(i,j)-input.surface.Rough_d(i,j))/input.surface.Roughness(i,j)));

            //compute neutral ABL height
            bl_height(i,j) = 0.2 * u_star(i,j) / f;	//from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
            }
        }
    }

    /*
     * Interpolate 2D wx model data to requested point locations
     */
    double X, Y, Z, uTemp, vTemp;
    element elem(&mesh);
    int elem_i, elem_j;
    double u_wn, v_wn;

    for(unsigned int i = 0; i < input.latList.size(); i++){
        X = input.projXList[i]; //projected (dem) coords
        Y = input.projYList[i]; //projected (dem) coords
        Z = input.heightList[i];

        uTemp = uGrid_wxModel.interpolateGrid(X, Y, AsciiGrid<double>::order1);  // wx speed at 10m height
        vTemp = vGrid_wxModel.interpolateGrid(X, Y, AsciiGrid<double>::order1);  // wx speed at 10m height

        X -= input.dem.xllCorner; //put into wn mesh coords
        Y -= input.dem.yllCorner; //put into wn mesh coords

        elem.get_uv(X, Y, elem_i, elem_j, u_wn, v_wn); // get u,v in wn mesh

        //profile stuff is a little weird bc we are not at nodes
        //profile is set based on southwest corner of current cell (elem_i, elem_j)
        profile.ObukovLength = L(elem_i,elem_j);
        profile.ABL_height = bl_height(elem_i,elem_j);
        profile.Roughness = input.surface.Roughness(elem_i,elem_j);
        profile.Rough_h = input.surface.Rough_h(elem_i,elem_j);
        profile.Rough_d = input.surface.Rough_d(elem_i,elem_j);
        profile.AGL = Z;  // height above the ground
        profile.inputWindHeight = input.inputWindHeight; // height above vegetation

        profile.inputWindSpeed = uTemp; // get wx speed at 10m height
        u10List.push_back(profile.getWindSpeed());

        profile.inputWindSpeed = vTemp; // get wx speed at 10m height
        v10List.push_back(profile.getWindSpeed());
    }

    //deallocate temporary grids
    airTempGrid_wxModel.deallocate();
    cloudCoverGrid_wxModel.deallocate();
    speedInitializationGrid_wxModel.deallocate();
    dirInitializationGrid_wxModel.deallocate();
    uGrid_wxModel.deallocate();
    vGrid_wxModel.deallocate();
    wGrid_wxModel.deallocate();

    //----3d WX model-------------------------------------------------

    //Initialize u0,v0,w0----------------------------------
    bool wxModel3d = false;
    int kk;
    double tempGradient;
    #ifdef NOMADS_ENABLE_3D
    if(this->getForecastReadable().find("3D") != std::string::npos){
        wxModel3d = true;
    }
    #endif
    if(wxModel3d == true){
#pragma omp parallel for default(shared) firstprivate(profile) private(i,j,k)
        for(i = 0; i < input.dem.get_nRows(); i++){
            for(j = 0; j < input.dem.get_nCols(); j++){

                profile.ObukovLength = L(i,j);
                profile.ABL_height = bl_height(i,j);
                profile.Roughness = input.surface.Roughness(i,j);
                profile.Rough_h = input.surface.Rough_h(i,j);
                profile.Rough_d = input.surface.Rough_d(i,j);

                for(k = 0; k < mesh.nlayers; k++){
                    profile.AGL=mesh.ZORD(i, j, k)-input.dem(i,j);  // height above the ground

                    if(u3d(i,j,k) != -9999) {  // if have 3d winds for current cell
                        u0(i, j, k) = u3d(i,j,k);
                    }
                    else{ // use log profile from first 3d layer down to ground
                        kk = k;
                        do{
                            kk++;
                            profile.inputWindHeight = mesh.ZORD(i,j,kk) - mesh.ZORD(i,j,0) - input.surface.Rough_h(i,j); // height above vegetation
                            profile.inputWindSpeed = u3d(i,j,kk);
                        }while (u3d(i,j,kk) == -9999);
                        u0(i, j, k) += profile.getWindSpeed();
                    }
                    if(v3d(i,j,k) != -9999){  // if have 3d winds for current cell
                        v0(i, j, k) = v3d(i,j,k);
                    }
                    else{
                        kk = k;
                        do{
                            kk++;
                            profile.inputWindHeight = mesh.ZORD(i,j,kk) - mesh.ZORD(i,j,0) - input.surface.Rough_h(i,j); // height above vegetation
                            profile.inputWindSpeed = v3d(i,j,kk);
                        }while (v3d(i,j,kk) == -9999);

                        v0(i, j, k) += profile.getWindSpeed();
                    }
                    if(w3d(i,j,k) != -9999){  // if have 3d winds for current cell
                        w0(i, j, k) = w3d(i,j,k);
                    }
                    else{
                        kk = k;
                        do{
                            kk++;
                            profile.inputWindHeight = mesh.ZORD(i,j,kk) - mesh.ZORD(i,j,0) - input.surface.Rough_h(i,j); // height above vegetation
                            profile.inputWindSpeed = w3d(i,j,kk);
                        }while (w3d(i,j,kk) == -9999);

                        w0(i, j, k) += profile.getWindSpeed();
                    }
                    if(air3d(i,j,k) == -9999){ //if don't have 3d T for current cell
                        kk = k;
                        do{ // find lowest 3d layer; these are perturbation potetential temperatures, not temperature!
                            kk++;
                        }while (air3d(i,j,kk) == -9999);
                        tempGradient = ( air3d(i,j,kk) - air3d(i,j,kk+1) ) /
                                       ( mesh.ZORD(i,j,kk+1) - mesh.ZORD(i,j,kk) ); // find gradient between lowest two 3D layers

                        for(int m = k; m<kk; m++){
                            air3d(i,j,m) = air3d(i,j,kk) + (tempGradient *
                                           ( mesh.ZORD(i,j,kk) - mesh.ZORD(i,j,m) )); //apply this gradient to current layer
                        }
                    }
                }
            }
        }
        u3d.deallocate();
        v3d.deallocate();
        w3d.deallocate();

        /*
         * Interpolate WX model data to requested point locations.
         */

        element elem(&mesh);
        std::vector<Mesh> meshList;
        meshList.push_back(xStaggerWxMesh);
        meshList.push_back(yStaggerWxMesh);
        meshList.push_back(zStaggerWxMesh);

        int elem_wx_i, elem_wx_j, elem_wx_k; // wx model cells
        int elem_i, elem_j, elem_k; // wn cells
        double u_wx, v_wx, w_wx;
        double x_wx, y_wx, z_wx;
        double u_wn, v_wn, w_wn;
        double x_wn, y_wn, z_wn;
        int wx_i, wn_i; //element indices for wn and wx model
        double z_ground; //wn ground
        double z_temp;
        double x, y, z; // locations to interpolate to

        for(unsigned int i = 0; i < input.latList.size(); i++){
            for(unsigned int j = 0; j < meshList.size(); j++){
                x = input.projXList[i]; //projected (dem) coords
                y = input.projYList[i]; //projected (dem) coords
                z = input.heightList[i]; //height above ground

                x -= input.dem.xllCorner; //put into wn mesh coords
                y -= input.dem.yllCorner; //put into wn mesh coords

                element elem_wx(&meshList[j]);

                elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get u and v coordinates in wx mesh
                wx_i = meshList[j].get_elemNum(elem_wx_i, elem_wx_j, 0); //wx_i is element number in wx mesh
                elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground
                z += z_wx; // add height above ground to wx model ground height, z is now the z to interpolate to

                elem_wx.get_uvw(x, y, z, elem_wx_i, elem_wx_j, elem_wx_k, u_wx, v_wx, w_wx); // find elem_k for this point
                if(elem_wx_k == 0){//if in first layer, use log profile
                    //profile is set based on southwest corner of current cell (elem_i, elem_j)
                    //----set profile stuff based on WN mesh-------------
                    elem.get_uv(x, y, elem_i, elem_j, u_wn, v_wn); // get elem_i, elem_j, u,v in wn mesh
                    wn_i = mesh.get_elemNum(elem_i, elem_j, 0); // wn_i is elem number in wn mesh
                    z_ground = z_wx;

                    profile.ObukovLength = L(elem_i,elem_j);
                    profile.ABL_height = bl_height(elem_i,elem_j);
                    profile.Roughness = input.surface.Roughness(elem_i,elem_j);
                    profile.Rough_h = input.surface.Rough_h(elem_i,elem_j);
                    profile.Rough_d = input.surface.Rough_d(elem_i,elem_j);
                    profile.AGL = input.heightList[i];  // height above the ground

                    elem_wx.get_xyz(wx_i, u_wx, v_wx, 1, x_wx, y_wx, z_temp); // get z at first layer in wx model mesh

                    profile.inputWindHeight = z_temp - z_ground - input.surface.Rough_h(elem_i, elem_j); // height above vegetation

                    if(j==0){
                        profile.inputWindSpeed = wxU3d.interpolate(x, y, z_temp); // get wx speed at first layer
                        u_wxList.push_back(profile.getWindSpeed());
                    }
                    else if(j==1){
                        profile.inputWindSpeed = wxV3d.interpolate(x, y, z_temp); // get wx speed at first layer
                        v_wxList.push_back(profile.getWindSpeed());
                    }
                    else{
                        profile.inputWindSpeed = wxW3d.interpolate(x, y, z_temp); // get wx speed at first layer
                        w_wxList.push_back(profile.getWindSpeed());
                    }
                }
                else{//else use linear interpolation
                    if(j==0){
                        u_wxList.push_back(wxU3d.interpolate(x,y,z));
                    }
                    else if(j==1){
                        v_wxList.push_back(wxV3d.interpolate(x,y,z));
                    }
                    else{
                        w_wxList.push_back(wxW3d.interpolate(x,y,z));
                    }
                }
            }
        }
        wxU3d.deallocate();
        wxV3d.deallocate();
        wxW3d.deallocate();
    } //end if 3d wx model
    else{//Initialize u0,v0,w0----------------------------------
#pragma omp parallel for default(shared) firstprivate(profile) private(i,j,k)
        for(i=0;i<input.dem.get_nRows();i++)
        {
            for(j=0;j<input.dem.get_nCols();j++)
            {
                profile.ObukovLength = L(i,j);
                profile.ABL_height = bl_height(i,j);
                profile.Roughness = input.surface.Roughness(i,j);
                profile.Rough_h = input.surface.Rough_h(i,j);
                profile.Rough_d = input.surface.Rough_d(i,j);
                profile.inputWindHeight = input.inputWindHeight;

                for(k=0;k<mesh.nlayers;k++)
                {
                    profile.AGL=mesh.ZORD(i, j, k)-input.dem(i,j);			//this is height above THE GROUND!! (not "z=0" for the log profile)

                    profile.inputWindSpeed = uInitializationGrid(i,j);
                    u0(i, j, k) += profile.getWindSpeed();

                    profile.inputWindSpeed = vInitializationGrid(i,j);
                    v0(i, j, k) += profile.getWindSpeed();

                    profile.inputWindSpeed = 0.0;
                    w0(i, j, k) += profile.getWindSpeed();
                }
            }
        }
    }

    // testing
    //std::string outFilename = "u0_6.png";
    //std::string scalarLegendFilename = "u0_6_legend";
    //std::string legendTitle = "u0_6";
    //std::string legendUnits = "(m/s)";
    //bool writeLegend = false;

	/*std::string filename;
    AsciiGrid<double> testGrid;
    testGrid.set_headerData(input.dem);
    testGrid.set_noDataValue(-9999.0);

	for(int k = 0; k < mesh.nlayers; k++){
        for(int i = 0; i < mesh.nrows; i++){
            for(int j = 0; j < mesh.ncols; j++ ){
                testGrid(i,j) = u0(i,j,k);
                filename = "u0from10mWind_" + boost::lexical_cast<std::string>(k);
            }
        }
        testGrid.write_Grid(filename.c_str(), 2);
        /*if(k == 6){
            testGrid.replaceNan( -9999.0 );
            testGrid.ascii2png( outFilename, scalarLegendFilename, legendUnits, legendTitle, writeLegend );
        }*/
	//} testGrid.deallocate();

    //Now add diurnal component if desired
    double AGL=0;                                //height above top of roughness elements
    if((input.diurnalWinds==true) && (profile.profile_switch==windProfile::monin_obukov_similarity))
    {
#pragma omp parallel for default(shared) private(i,j,k,AGL)
        for(k=1;k<mesh.nlayers;k++)	//start at 1, not zero because ground nodes must be zero for boundary conditions to work properly
        {
            for(i=0;i<mesh.nrows;i++)
            {
                for(j=0;j<mesh.ncols;j++)
                {
                    AGL=mesh.ZORD(i, j, k)-input.dem(i,j);	//this is height above THE GROUND!! (not "z=0" for the log profile)
                    if((AGL - input.surface.Rough_d(i,j)) < height(i,j))
                    {
                        u0(i, j, k) += uDiurnal(i,j);
                        v0(i, j, k) += vDiurnal(i,j);
                        w0(i, j, k) += wDiurnal(i,j);
                    }
                }
            }
        }
    }
}

/**
 * \brief Get the wind height from the forecast file
 * @param varName variable name passed in that each dem uses for wind height
 * @return double value of wind height
 */
double wxModelInitialization::GetWindHeight(std::string varName)
{
    /* Override with internal storage */
    varName = heightVarName;
    int status, ncid, height_id;
    size_t unit_len;
    double d;
    std::string var_name = varName;
    char *units;

    static size_t var_index[] = {0};
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif
    status = nc_open(wxModelFileName.c_str(), 0, &ncid);
    status = nc_inq_varid(ncid, var_name.c_str(), &height_id);

    if(status == 0)
    {
        status = nc_get_var1_double(ncid, height_id, var_index, &d);
    }

    if( status != 0 )
    {
        std::string err = "Failed to find height for " + varName;
        throw badForecastFile( err );
    }

    status = nc_inq_attlen(ncid, height_id, "units", &unit_len);
    units = (char*)malloc(unit_len + 1);
    status = nc_get_att_text(ncid, height_id, "units", units);
    units[unit_len] = '\0';

    if(EQUAL(units,"m") || EQUAL(units, "meters"))
    {
    }
    else if(EQUAL(units,"f") || EQUAL(units, "feet") || EQUAL(units, "ft"))
    {
        lengthUnits::toBaseUnits(d, "feet");
    }
    else
    {
        free(units);
        nc_close(ncid);
        throw badForecastFile("Cannot determine wind height units in "
                              "forecast file");
    }
    free(units);
    nc_close(ncid);

    return d;
}

/**
 * Get the name of the time dimension for a given variable
 *
 * Return "time" as a default
 *
 * @param pszVariable name of the variable to inquire
 * @return string representation of the time dimension(variable) name
 */
std::string wxModelInitialization::GetTimeName(const char *pszVariable)
{
    /* return time if pszVariable is null */
    if(pszVariable == NULL)
        //return "time";
        return "Times";
    int ncid;
    int status, varid, ndims, nvars, ngatts, unlimdimid;
    nc_type vartype;
    int varndims, varnatts;
    int vardimids[NC_MAX_VAR_DIMS];   /* dimension IDs */

    int timeid;
    nc_type timetype;
    int timendims, timenatts;
    int timedimids[NC_MAX_VAR_DIMS];
    char timename[NC_MAX_NAME+1];
    size_t namelength;
    char *tp;

    //std::string timestring = "time";
    std::string timestring = "Times";

    status = nc_open(wxModelFileName.c_str(), 0, &ncid);
    status = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);
    /*
    ** If we are looking for just a 'time' var, it may or may not be 'time'.
    ** In order to bandage #149, we'll check for time, time0, time1, time2...
    */
    status = nc_inq_varid(ncid, pszVariable, &varid);
    if (status != NC_NOERR && EQUALN(pszVariable, "time", strlen("time"))) {
        const char *pszT = NULL;
        int iSuffix = 0;
        while (status != NC_NOERR && iSuffix < 5) {
            pszT = CPLSPrintf("time%d", iSuffix);
            status = nc_inq_varid(ncid, pszT, &varid);
            iSuffix++;
        }
    }
    if (status != NC_NOERR) {
        nc_close(ncid);
        throw badForecastFile(CPLSPrintf(
            "Could not identify time dimension for variable: %s", pszVariable));
    }
    status = nc_inq_var(ncid, varid, 0, &vartype, &varndims, vardimids, &varnatts);
    for(int i = 0; i < varndims; i++) {
        status = nc_inq_dimname(ncid, vardimids[i], timename);
        status = nc_inq_varid(ncid, timename, &timeid);
        status = nc_inq_var(ncid, timeid, timename, &timetype, &timendims, timedimids, &timenatts);
        status = nc_inq_attlen(ncid, timeid, "standard_name", &namelength);
        if(status == 0 && namelength > 0) {
            tp = (char*)malloc(namelength + 1);
            status = nc_get_att_text(ncid, timeid, "standard_name", tp);
            tp[namelength] = '\0';
            if(strcmp("time", tp) == 0) {
                timestring =  std::string(timename);
                CPLDebug( "WINDNINJA", "Found time: %s for variable: %s",
                          timename, pszVariable );
                free( (void*)tp );
                break;
            }
            free( (void*)tp );
        }
    }
    nc_close(ncid);
    return timestring;
}

int wxModelInitialization::LoadFromCsv()
{
    char **papszLines = NULL;
    char **papszTokens = NULL;
    std::string p = FindDataPath( "thredds.csv" );
    if( p == "" )
        return 1;

    /*
    ** FIXME(kyle): Maybe need a stronger check here.  Also note, we don't free
    ** this, it just lives until the end of the application.  It should only be
    ** allocated once, and reused.
    */
    if( !papszThreddsCsv )
    {
        papszLines = CSLLoad2( p.c_str(), 10, 512, NULL );
        papszThreddsCsv = CSLDuplicate( papszLines );
    }
    else
    {
        CPLDebug( "WINDNINJA", "Using cached thredds csv values" );
        papszLines = CSLDuplicate( papszThreddsCsv );
    }
    int nRecords = CSLCount( papszLines );
    if( nRecords < 2 )
    {
        CSLDestroy( papszLines );
        return 1;
    }
    int i;
    for( i = 1; i < nRecords; i++ )
    {
        papszTokens = CSLTokenizeString2( papszLines[i], ",", 0 );
        CPLAssert( CSLCount( papszTokens ) == 4 );
        if( EQUAL( getForecastIdentifier().c_str(), papszTokens[0] ) )
        {
            host = papszTokens[1];
            path = papszTokens[2];
            heightVarName = papszTokens[3];
            CSLDestroy( papszTokens );
            break;
        }
        CSLDestroy( papszTokens );
    }
    CSLDestroy( papszLines );
    return 0;
}

void wxModelInitialization::SetProgressFunc( GDALProgressFunc pfnNew )
{
    pfnProgress = pfnNew;
}

std::string wxModelInitialization::getForecastReadable( const char bySwapWithSpace )
{
    char *s, *p;
    s = strdup( getForecastIdentifier().c_str() );
    p = strchr( s, bySwapWithSpace );
    while( p )
    {
        *p = bySwapWithSpace;
        p = strchr( s, ' ' );
    }
    std::string os = s;
    free( s );
    return os;
}
