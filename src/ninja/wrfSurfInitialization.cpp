/******************************************************************************
*
* $Id:$
*
* Project:  WindNinja
* Purpose:  Initialing with WRF forecasts
* Author:   Natalie Wagenbrenner <nwagenbrenner@gmail.com>
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

#include "wrfSurfInitialization.h"

/**
* Constructor for the class initializes the variable names
*
*/
wrfSurfInitialization::wrfSurfInitialization() : wxModelInitialization()
{

}

/**
* Copy constructor.
* @param A Copied value.
*/
wrfSurfInitialization::wrfSurfInitialization(wrfSurfInitialization const& A ) : wxModelInitialization(A)
{
    wxModelFileName = A.wxModelFileName;
}

/**
* Destructor.
*/
wrfSurfInitialization::~wrfSurfInitialization()
{
}

/**
* Equals operator.
* @param A Value to set equal to.
* @return a copy of an object
*/
wrfSurfInitialization& wrfSurfInitialization::operator= (wrfSurfInitialization const& A)
{
    if(&A != this) {
        wxModelInitialization::operator=(A);
    }
    return *this;
}

/**
*@brief Returns horizontal grid resolution of the model
*@return return grid resolution (in km unless < 1, then degrees)
*/
double wrfSurfInitialization::getGridResolution()
{
    return -1.0;
}

/**
* Fetch the variable names
*
* @return a vector of variable names
*/
std::vector<std::string> wrfSurfInitialization::getVariableList()
{
    std::vector<std::string> varList;
    varList.push_back( "T2" );   // T at 2 m
    varList.push_back( "V10" );  // V at 10 m
    varList.push_back( "U10" );  // U at 10 m
    varList.push_back( "QCLOUD" );  // cloud water mixing ratio
    return varList;
}

/**
* Fetch a unique identifier for the forecast.  This string can be used for
* identifying and storing.
*
* @return unique name
*/
std::string wrfSurfInitialization::getForecastIdentifier()
{
    return std::string( "WRF-SURFACE" );
}

/**
* Fetch the path to the forecast
*
* @return path to the forecast
*/
std::string wrfSurfInitialization::getPath()
{
    return std::string( "");
}

int wrfSurfInitialization::getStartHour()
{
    return 0;
}

int wrfSurfInitialization::getEndHour()
{
    return 84;
}

/**
* Checks the downloaded data to see if it is all valid.
* May not be functional yet for this class...
*/
void wrfSurfInitialization::checkForValidData()
{
    // open ds variable by variable
    GDALDataset *srcDS;
    std::string temp;
    std::string srcWkt;
    int nBands = 0;
    bool noDataValueExists;
    bool noDataIsNan;

    std::vector<std::string> varList = getVariableList();

    //Acquire a lock to protect the non-thread safe netCDF library
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    for( unsigned int i = 0;i < varList.size();i++ ) {

        temp = "NETCDF:\"" + wxModelFileName + "\":" + varList[i];
        //cout << "temp = " <<temp<<endl;

        CPLPushErrorHandler(&CPLQuietErrorHandler);
        srcDS = (GDALDataset*)GDALOpen( temp.c_str(), GA_ReadOnly );
        CPLPopErrorHandler();
        if( srcDS == NULL )
            throw badForecastFile("Cannot open forecast file.");

        //Get total bands (time steps)
        nBands = srcDS->GetRasterCount();
        //cout<<"nBands = " <<nBands<<endl;
        nBands = srcDS->GetRasterCount();
        int nXSize, nYSize;
        GDALRasterBand *poBand;
        int pbSuccess;
        double dfNoData;
        double *padfScanline;

        nXSize = srcDS->GetRasterXSize();
        nYSize = srcDS->GetRasterYSize();

        //cout<<"nXsize = " <<nXSize<<endl;
        //cout<<"nYsize = " <<nYSize<<endl;

        //loop over all bands for this variable (bands are time steps)
        for(int j = 1; j <= nBands; j++)
        {
            poBand = srcDS->GetRasterBand( j );

            pbSuccess = 0;
            dfNoData = poBand->GetNoDataValue( &pbSuccess );
            if( pbSuccess == false )
                noDataValueExists = false;
            else
            {
                noDataValueExists = true;
                noDataIsNan = cplIsNan(dfNoData);
            }

            //set the data
            padfScanline = new double[nXSize*nYSize];
            poBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, padfScanline, nXSize, nYSize,
                    GDT_Float64, 0, 0);
            for(int k = 0;k < nXSize*nYSize; k++)
            {
                //Check if value is no data (if no data value was defined in file)
                if(noDataValueExists)
                {
                    if(noDataIsNan)
                    {
                        if(cplIsNan(padfScanline[k]))
                            throw badForecastFile("Forecast file contains no_data values.");
                    }else
                    {
                        if(padfScanline[k] == dfNoData)
                            throw badForecastFile("Forecast file contains no_data values.");
                    }
                }
                if( varList[i] == "T2" )   //units are Kelvin
                {
                    if(padfScanline[k] < 180.0 || padfScanline[k] > 340.0)  //these are near the most extreme temperatures ever recored on earth
                        throw badForecastFile("Temperature is out of range in forecast file.");
                }
                else if( varList[i] == "V10" )  //units are m/s
                {
                    if(std::abs(padfScanline[k]) > 220.0)
                        throw badForecastFile("V-velocity is out of range in forecast file.");
                }
                else if( varList[i] == "U10" )  //units are m/s
                {
                    if(std::abs(padfScanline[k]) > 220.0)
                        throw badForecastFile("U-velocity is out of range in forecast file.");
                }
                else if( varList[i] == "QCLOUD" )  //units are kg/kg
                {
                    if(padfScanline[k] < -0.0001 || padfScanline[k] > 100.0)
                        throw badForecastFile("Total cloud cover is out of range in forecast file.");
                }
            }

            delete [] padfScanline;
        }

        GDALClose((GDALDatasetH) srcDS );
    }
}

/**
* Static identifier to determine if the netcdf file is a WRF forecast.
* Uses netcdf c api
*
* @param fileName netcdf filename
*
* @return true if the forecast is a WRF forecast
*/
bool wrfSurfInitialization::identify( std::string fileName )
{
    bool identified = true;

    //Acquire a lock to protect the non-thread safe netCDF library
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    /*
     * Open the dataset
     */

    int status, ncid, ndims, nvars, ngatts, unlimdimid;
    status = nc_open( fileName.c_str(), 0, &ncid );
    if ( status != NC_NOERR ) {
        identified = false;
    }

    /*
     * Check the global attributes for the following tag:
     * :TITLE = "...WRF...""
     */
    size_t len;
    nc_type type;
    char* model;
    status = nc_inq_att( ncid, NC_GLOBAL, "TITLE", &type, &len );
    if( status != NC_NOERR )
        identified = false;
    else {
        model = new char[len + 1];
        status = nc_get_att_text( ncid, NC_GLOBAL, "TITLE", model );
        model[len] = '\0';
        std::string s( model );
        if( s.find("WRF") == s.npos ) {
            identified = false;
        }

        delete[] model;
    }

    status = nc_close( ncid );

    return identified;
}


/**
* Sets the surface grids based on a WRF (surface only!) forecast.
* @param input The WindNinjaInputs for misc. info.
* @param airGrid The air temperature grid to be filled.
* @param cloudGrid The cloud cover grid to be filled.
* @param uGrid The u velocity grid to be filled.
* @param vGrid The v velocity grid to be filled.
* @param wGrid The w velocity grid to be filled (filled with zeros here?).
*/
void wrfSurfInitialization::setSurfaceGrids( WindNinjaInputs &input,
        AsciiGrid<double> &airGrid,
        AsciiGrid<double> &cloudGrid,
        AsciiGrid<double> &uGrid,
        AsciiGrid<double> &vGrid,
        AsciiGrid<double> &wGrid )
{

    int bandNum = -1;

    //get time list
    std::vector<boost::local_time::local_date_time> timeList( getTimeList(input.ninjaTimeZone) );
    //Search time list for our time to identify our band number for cloud/speed/dir
    for(unsigned int i = 0; i < timeList.size(); i++)
    {
        if(input.ninjaTime == timeList[i])
        {
            bandNum = i + 1;
            //cout<<"Grabbing band number: " <<bandNum<<endl;
            break;
        }
    }
    if(bandNum < 0)
        throw std::runtime_error("Could not match ninjaTime with a band number in the forecast file.");

    GDALDataset* poDS;
    //attempt to grab the projection from the dem?
    //check for member prjString first
    std::string dstWkt;
    dstWkt = input.dem.prjString;
    if ( dstWkt.empty() ) {
        //try to open original
        poDS = (GDALDataset*)GDALOpen( input.dem.fileName.c_str(), GA_ReadOnly );
        if( poDS == NULL ) {
            CPLDebug( "wrfInitialization::setSurfaceGrids()",
                    "Bad projection reference" );
            throw("Cannot open dem file in wrfInitialization::setSurfaceGrids()");
        }
        dstWkt = poDS->GetProjectionRef();
        if( dstWkt.empty() ) {
            CPLDebug( "wrfInitialization::setSurfaceGrids()",
                    "Bad projection reference" );
            throw("Cannot open dem file in wrfInitialization::setSurfaceGrids()");
        }
        GDALClose((GDALDatasetH) poDS );
    }

//==========get global attributes to set projection===========================
    //Acquire a lock to protect the non-thread safe netCDF library
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    /*
     * Open the dataset
     */
    int status, ncid;
    status = nc_open( wxModelFileName.c_str(), 0, &ncid );
    if ( status != NC_NOERR ) {
        ostringstream os;
        os << "The netcdf file: " << wxModelFileName
           << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }

    /*
     * Get global attribute MAP_PROJ
     * 1 = Lambert Conformal Conic
     * 2 = Polar Stereographic
     * 3 = Mercator
     * 6 = Lat/Long
     */

    int mapProj;
    nc_type type;
    size_t len;
    status = nc_inq_att( ncid, NC_GLOBAL, "MAP_PROJ", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute 'MAP_PROJ' in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_int( ncid, NC_GLOBAL, "MAP_PROJ", &mapProj );
    }

    //cout<<"MAP_PROJ = "<<mapProj<<endl;

    /*
     * Get global attributes DX, DY
     *
     */
    float dx, dy;
    status = nc_inq_att( ncid, NC_GLOBAL, "DX", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute DX in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "DX", &dx );
    }
    status = nc_inq_att( ncid, NC_GLOBAL, "DY", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute DY in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "DY", &dy );
    }

    /*
     * Get global attributes CEN_LAT, CEN_LON
     *
     */
    float cenLat, cenLon;
    status = nc_inq_att( ncid, NC_GLOBAL, "CEN_LAT", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute CEN_LAT in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "CEN_LAT", &cenLat );
    }
    status = nc_inq_att( ncid, NC_GLOBAL, "CEN_LON", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute CEN_LON in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "CEN_LON", &cenLon );
    }

    /*
     * Get global attributes MOAD_CEN_LAT, STAND_LON
     *
     */
    float moadCenLat, standLon;
    status = nc_inq_att( ncid, NC_GLOBAL, "MOAD_CEN_LAT", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute MOAD_CEN_LAT in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "MOAD_CEN_LAT", &moadCenLat );
    }
    status = nc_inq_att( ncid, NC_GLOBAL, "STAND_LON", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute STAND_LON in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "STAND_LON", &standLon );
    }

    /*
     * Get global attributes TRUELAT1, TRUELAT2
     *
     */
    float trueLat1, trueLat2;
    status = nc_inq_att( ncid, NC_GLOBAL, "TRUELAT1", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute TRUELAT1 in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "TRUELAT1", &trueLat1 );
    }
    status = nc_inq_att( ncid, NC_GLOBAL, "TRUELAT2", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute TRUELAT2 in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "TRUELAT2", &trueLat2 );
    }

    /*
     * Get global attribute BOTTOM-TOP_GRID_DIMENSION
     *
     */
    status = nc_inq_att( ncid, NC_GLOBAL, "BOTTOM-TOP_GRID_DIMENSION", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute BOTTOM-TOP_GRID_DIMENSION  in the netcdf file: "
        << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_int( ncid, NC_GLOBAL, "BOTTOM-TOP_GRID_DIMENSION", &wxModel_nLayers );
    }
    
    /*
     * Get global attribute WEST-EAST_GRID_DIMENSION
     * Not currently used. WX model x/y dims set based on
     * reprojected image (in DEM space).
     */
    /*status = nc_inq_att( ncid, NC_GLOBAL, "WEST-EAST_GRID_DIMENSION", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute WEST-EAST_GRID_DIMENSION  in the netcdf file: "
        << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_int( ncid, NC_GLOBAL, "WEST-EAST_GRID_DIMENSION", &wxModel_nCols );
    }*/
    
    /*
     * Get global attribute SOUTH-NORTH_GRID_DIMENSION
     * Not currently used. WX model x/y dims set based on
     * reprojected image (in DEM space).
     */
    /*status = nc_inq_att( ncid, NC_GLOBAL, "SOUTH-NORTH_GRID_DIMENSION", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute SOUTH-NORTH_GRID_DIMENSION  in the netcdf file: "
        << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_int( ncid, NC_GLOBAL, "SOUTH-NORTH_GRID_DIMENSION", &wxModel_nRows );
    }*/

    /*
     * Close the dataset
     *
     */
    status = nc_close( ncid );
    if( status != NC_NOERR ) {
        ostringstream os;
        os << "The netcdf file: " << wxModelFileName
           << " cannot be closed\n";
        throw std::runtime_error( os.str() );
    }

//======end get global attributes========================================

    CPLPushErrorHandler(&CPLQuietErrorHandler);
    poDS = (GDALDataset*)GDALOpenShared( input.forecastFilename.c_str(), GA_ReadOnly );
    CPLPopErrorHandler();
    if( poDS == NULL ) {
        CPLDebug( "wrfInitialization::setSurfaceGrids()",
                 "Bad forecast file");
        throw badForecastFile("Cannot open forecast file in wrfInitialization::setSurfaceGrids()");
    }
    else {
        GDALClose((GDALDatasetH) poDS ); // close original wxModel file
    }
        
    // open ds one by one, set projection, warp, then write to grid
    GDALDataset *srcDS, *wrpDS;
    std::string temp;
    std::vector<std::string> varList = getVariableList();

    double coordinateTransformationAngle = 0.0;

    /*
     * Set the initial values in the warped dataset to no data
     */
    for( unsigned int i = 0;i < varList.size();i++ ) {

        temp = "NETCDF:\"" + input.forecastFilename + "\":" + varList[i];
        
        CPLPushErrorHandler(&CPLQuietErrorHandler);
        srcDS = (GDALDataset*)GDALOpenShared( temp.c_str(), GA_ReadOnly );
        CPLPopErrorHandler();
        if( srcDS == NULL ) {
            CPLDebug( "wrfInitialization::setSurfaceGrids()",
                    "Bad forecast file" );
        }

        CPLDebug("WX_MODEL_INITIALIZATION", "varList[i] = %s", varList[i].c_str());

        /*
         * Set up spatial reference stuff for setting projections
         * and geotransformations
         */

        std::string projString;
        if(mapProj == 1){  //lambert conformal conic
            projString = "PROJCS[\"WGC 84 / WRF Lambert\",GEOGCS[\"WGS 84\",DATUM[\"World Geodetic System 1984\",\
                          SPHEROID[\"WGS 84\",6378137.0,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],\
                          PRIMEM[\"Greenwich\",0.0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.017453292519943295],\
                          AXIS[\"Geodetic longitude\",EAST],AXIS[\"Geodetic latitude\",NORTH],AUTHORITY[\"EPSG\",\"4326\"]],\
                          PROJECTION[\"Lambert_Conformal_Conic_2SP\"],\
                          PARAMETER[\"central_meridian\","+boost::lexical_cast<std::string>(standLon)+"],\
                          PARAMETER[\"latitude_of_origin\","+boost::lexical_cast<std::string>(moadCenLat)+"],\
                          PARAMETER[\"standard_parallel_1\","+boost::lexical_cast<std::string>(trueLat1)+"],\
                          PARAMETER[\"false_easting\",0.0],PARAMETER[\"false_northing\",0.0],\
                          PARAMETER[\"standard_parallel_2\","+boost::lexical_cast<std::string>(trueLat2)+"],\
                          UNIT[\"m\",1.0],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]";
        }
        else if(mapProj == 2){  //polar stereographic
            projString = "PROJCS[\"WGS 84 / Antarctic Polar Stereographic\",GEOGCS[\"WGS 84\",\
                         DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],\
                         AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],\
                         UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],\
                         AUTHORITY[\"EPSG\",\"4326\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],\
                         PROJECTION[\"Polar_Stereographic\"],\
                         PARAMETER[\"latitude_of_origin\","+boost::lexical_cast<std::string>(moadCenLat)+"],\
                         PARAMETER[\"central_meridian\","+boost::lexical_cast<std::string>(standLon)+"],\
                         PARAMETER[\"scale_factor\",1],\
                         PARAMETER[\"false_easting\",0],\
                         PARAMETER[\"false_northing\",0],AUTHORITY[\"EPSG\",\"3031\"],\
                         AXIS[\"Easting\",UNKNOWN],AXIS[\"Northing\",UNKNOWN]]";
        }
        else if(mapProj == 3){  //mercator
            projString = "PROJCS[\"World_Mercator\",GEOGCS[\"GCS_WGS_1984\",DATUM[\"WGS_1984\",\
                          SPHEROID[\"WGS_1984\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],\
                          UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Mercator_1SP\"],\
                          PARAMETER[\"False_Easting\",0],\
                          PARAMETER[\"False_Northing\",0],\
                          PARAMETER[\"Central_Meridian\","+boost::lexical_cast<std::string>(standLon)+"],\
                          PARAMETER[\"latitude_of_origin\","+boost::lexical_cast<std::string>(moadCenLat)+"],\
                          UNIT[\"Meter\",1]]";
        }
        else if(mapProj == 6){  //lat/long
            throw badForecastFile("Cannot initialize with a forecast file in lat/long spacing. \
                                   Forecast file must be in a projected coordinate system.");
        }
        else throw badForecastFile("Cannot determine projection from the forecast file information.");

        OGRSpatialReference oSRS, *poLatLong;
        char *srcWKT = NULL;
        char* prj2 = (char*)projString.c_str();
        oSRS.importFromWkt(&prj2);
        oSRS.exportToWkt(&srcWKT);

        CPLDebug("WX_MODEL_INITIALIZATION", "srcWKT= %s", srcWKT);

        OGRCoordinateTransformation *poCT;
        poLatLong = oSRS.CloneGeogCS();

        /*
         * Transform domain center from lat/long to WRF space
         */
        double zCenter;
        zCenter = 0;
        double xCenter, yCenter;
        xCenter = (double)cenLon;
        yCenter = (double)cenLat;

#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    oSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */

        poCT = OGRCreateCoordinateTransformation(poLatLong, &oSRS);
        delete poLatLong;

        if(poCT==NULL || !poCT->Transform(1, &xCenter, &yCenter))
            printf("Transformation failed.\n");

        CPLDebug("WX_MODEL_INITIALIZATION", "xCenter, yCenter= %f, %f", xCenter, yCenter);

        /*
         * Set the geostransform for the WRF file
         * upper corner is calculated from transformed x, y
         * (in WRF space)
         */

        double ncols, nrows;
        int nXSize = srcDS->GetRasterXSize();
        int nYSize = srcDS->GetRasterYSize();
        ncols = (double)(nXSize)/2;
        nrows = (double)(nYSize)/2;

        double adfGeoTransform[6] = {(xCenter-(ncols*dx)), dx, 0,
                                    (yCenter+(nrows*dy)),
                                    0, -(dy)};

        CPLDebug("WX_MODEL_INITIALIZATION", "ulcornerX, ulcornerY= %f, %f", (xCenter-(ncols*dx)), (yCenter+(nrows*dy)));
        CPLDebug("WX_MODEL_INITIALIZATION", "nXSize, nYsize= %d, %d", nXSize, nYSize);
        CPLDebug("WX_MODEL_INITIALIZATION", "dx= %f", dx);

        srcDS->SetGeoTransform(adfGeoTransform);


        /*if( varList[i] == "U10" ) {  // just a check
            AsciiGrid<double> tempSrcGrid;   
            GDAL2AsciiGrid( srcDS, 12, tempSrcGrid );
            tempSrcGrid.write_Grid("before_warp", 2);
        }*/

        /*
         * Grab the first band to get the nodata value for the variable,
         * assume all bands have the same ndv
         */
        GDALRasterBand *poBand = srcDS->GetRasterBand( 1 );
        int pbSuccess;
        double dfNoData = poBand->GetNoDataValue( &pbSuccess );

        int nBandCount = srcDS->GetRasterCount();

        CPLDebug("WX_MODEL_INITIALIZATION", "band count = %d", nBandCount);

        if( pbSuccess == false )
            dfNoData = -9999.0;

        // compute the coordinateTransformationAngle, the angle between the y coordinate grid lines of the pre-warped and warped datasets,
        // going FROM the y coordinate grid line of the pre-warped dataset TO the y coordinate grid line of the warped dataset
        // in this case, going FROM weather model projection coordinates TO dem projection coordinates
        if( varList[i] == "U10" )
        {
            if( CSLTestBoolean(CPLGetConfigOption("DISABLE_COORDINATE_TRANSFORMATION_ANGLE_CALCULATIONS", "FALSE")) == false )
            {
                // direct calculation of FROM wx TO dem, already has the appropriate sign
                if(!GDALCalculateCoordinateTransformationAngle( srcDS, coordinateTransformationAngle, dstWkt.c_str() ))  // this is FROM wx TO dem
                {
                    printf("Warning: Unable to calculate coordinate transform angle for the wxModel.");
                }
            }
        }

        wrpDS = (GDALDataset*) GDALAutoCreateWarpedVRT( srcDS, srcWKT,
                                                        dstWkt.c_str(),
                                                        GRA_NearestNeighbour,
                                                        1.0, NULL );

        //=======for testing==================================//
        /*AsciiGrid<double> tempGrid;
        AsciiGrid<double> temp2Grid;
        
        if( varList[i] == "U10" ) {
            GDAL2AsciiGrid( wrpDS, 12, tempGrid );
            if( cplIsNan( dfNoData ) ) {
                tempGrid.set_noDataValue(-9999.0);
                tempGrid.replaceNan( -9999.0 );
            }
            tempGrid.write_Grid("after_warp", 2);


            //Make final grids with same header as dem
            temp2Grid.set_headerData(input.dem);
            temp2Grid.interpolateFromGrid(tempGrid, AsciiGrid<double>::order1);
            temp2Grid.set_noDataValue(-9999.0);
            temp2Grid.write_Grid("after_interpolation", 2);
        }*/
        //=======end testing=================================//

        CPLDebug("WX_MODEL_INITIALIZATION", "band number to write = %d", bandNum);

        if( varList[i] == "T2" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, airGrid );
        if( cplIsNan( dfNoData ) ) {
        airGrid.set_noDataValue(-9999.0);
        airGrid.replaceNan( -9999.0 );
        }
    }
        else if( varList[i] == "V10" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, vGrid );
        if( cplIsNan( dfNoData ) ) {
        vGrid.set_noDataValue(-9999.0);
        vGrid.replaceNan( -9999.0 );
        }
    }
        else if( varList[i] == "U10" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, uGrid );
        if( cplIsNan( dfNoData ) ) {
        uGrid.set_noDataValue(-9999.0);
        uGrid.replaceNan( -9999.0 );
        }
    }
        else if( varList[i] == "QCLOUD" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, cloudGrid );
        if( cplIsNan( dfNoData ) ) {
        cloudGrid.set_noDataValue(-9999.0);
        cloudGrid.replaceNan( -9999.0 );
        }
    }
        CPLFree(srcWKT);
        delete poCT;
        GDALClose((GDALDatasetH) srcDS );
        GDALClose((GDALDatasetH) wrpDS );
    }
    //don't allow small negative values in cloud cover
    for(int i=0; i<cloudGrid.get_nRows(); i++){
        for(int j=0; j<cloudGrid.get_nCols(); j++){
            if(cloudGrid(i,j) < 0.0){
                cloudGrid(i,j) = 0.0;
            }
        }
    }
    cloudGrid /= 100.0;
    wGrid.set_headerData( uGrid );
    wGrid = 0.0;

    //use the coordinateTransformationAngle to correct the angles of the output dataset
    //to convert from the original dataset projection angles to the warped dataset projection angles
    if( CSLTestBoolean(CPLGetConfigOption("DISABLE_COORDINATE_TRANSFORMATION_ANGLE_CALCULATIONS", "FALSE")) == false )
    {
        // need an intermediate spd and dir set of ascii grids
        AsciiGrid<double> speedGrid;
        AsciiGrid<double> dirGrid;
        speedGrid.set_headerData(uGrid);
        dirGrid.set_headerData(uGrid);
        for(int i=0; i<uGrid.get_nRows(); i++)
        {
            for(int j=0; j<uGrid.get_nCols(); j++)
            {
                if( uGrid(i,j) == uGrid.get_NoDataValue() || vGrid(i,j) == vGrid.get_NoDataValue() )
                {
                    speedGrid(i,j) = speedGrid.get_NoDataValue();
                    dirGrid(i,j) = dirGrid.get_NoDataValue();
                } else
                {
                    wind_uv_to_sd(uGrid(i,j), vGrid(i,j), &(speedGrid)(i,j), &(dirGrid)(i,j));
                }
            }
        }

        // use the coordinateTransformationAngle to correct each spd,dir, u,v dataset for the warp
        for(int i=0; i<dirGrid.get_nRows(); i++)
        {
            for(int j=0; j<dirGrid.get_nCols(); j++)
            {
                if( speedGrid(i,j) != speedGrid.get_NoDataValue() && dirGrid(i,j) != dirGrid.get_NoDataValue() )
                {
                    dirGrid(i,j) = wrap0to360( dirGrid(i,j) - coordinateTransformationAngle ); //convert FROM wxModel projection coordinates TO dem projected coordinates
                    // always recalculate the u and v grids from the corrected dir grid, the changes need to go together
                    wind_sd_to_uv(speedGrid(i,j), dirGrid(i,j), &(uGrid)(i,j), &(vGrid)(i,j));
                }
            }
        }

        // cleanup the intermediate grids
        speedGrid.deallocate();
        dirGrid.deallocate();
    }
}

double wrfSurfInitialization::Get_Wind_Height()
{
    return 10.0;
}

