/******************************************************************************
*
* $Id:$
*
* Project:  WindNinja
* Purpose:  Initialing with RegCM forecasts
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

#include "regcmSurfInitialization.h"

/**
* Constructor for the class initializes the variable names
*
*/
regcmSurfInitialization::regcmSurfInitialization() : wxModelInitialization()
{

}

/**
* Copy constructor.
* @param A Copied value.
*/
regcmSurfInitialization::regcmSurfInitialization(regcmSurfInitialization const& A ) : wxModelInitialization(A)
{
    wxModelFileName = A.wxModelFileName;
}

/**
* Destructor.
*/
regcmSurfInitialization::~regcmSurfInitialization()
{
}

/**
* Equals operator.
* @param A Value to set equal to.
* @return a copy of an object
*/
regcmSurfInitialization& regcmSurfInitialization::operator= (regcmSurfInitialization const& A)
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
double regcmSurfInitialization::getGridResolution()
{
    return -1.0;
}

/**
* Fetch the variable names
*
* @return a vector of variable names
*/
std::vector<std::string> regcmSurfInitialization::getVariableList()
{
    std::vector<std::string> varList;
    varList.push_back( "tas" );   // T at 2 m
    varList.push_back( "vas" );  // V at 10 m
    varList.push_back( "uas" );  // U at 10 m
    varList.push_back( "QCLOUD" );  // cloud water mixing ratio
    return varList;
}

/**
* Fetch a unique identifier for the forecast.  This string can be used for
* identifying and storing.
*
* @return unique name
*/
std::string regcmSurfInitialization::getForecastIdentifier()
{
    return std::string( "REGCM-SURFACE" );
}

/**
* Fetch the path to the forecast
*
* @return path to the forecast
*/
std::string regcmSurfInitialization::getPath()
{
    return std::string( "");
}

int regcmSurfInitialization::getStartHour()
{
    return 0;
}

int regcmSurfInitialization::getEndHour()
{
    return 3;
}

/**
* Checks the downloaded data to see if it is all valid.
* May not be functional yet for this class...
*/
void regcmSurfInitialization::checkForValidData()
{
    cout<<"No check for valid data!"<<endl;
}

/**
* Static identifier to determine if the netcdf file is a RegCM forecast.
* Uses netcdf c api
*
* @param fileName netcdf filename
*
* @return true if the forecast is a RegCM forecast
*/
bool regcmSurfInitialization::identify( std::string fileName )
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
     * :title = "ICTP Regional Climatic model V4""
     */
    size_t len;
    nc_type type;
    char* model;
    status = nc_inq_att( ncid, NC_GLOBAL, "title", &type, &len );
    if( status != NC_NOERR )
        identified = false;
    else {
        model = new char[len + 1];
        status = nc_get_att_text( ncid, NC_GLOBAL, "title", model );
        model[len] = '\0';
        std::string s( model );
        if( s.find("ICTP Regional Climatic model V4") == s.npos ) {
            identified = false;
        }

        delete[] model;
    }

    status = nc_close( ncid );

    return identified;
}


/**
* Sets the surface grids based on a RegCM (surface only!) forecast.
* @param input The WindNinjaInputs for misc. info.
* @param airGrid The air temperature grid to be filled.
* @param cloudGrid The cloud cover grid to be filled.
* @param uGrid The u velocity grid to be filled.
* @param vGrid The v velocity grid to be filled.
* @param wGrid The w velocity grid to be filled (filled with zeros here?).
*/
void regcmSurfInitialization::setSurfaceGrids( WindNinjaInputs &input,
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
     * Get global attribute projection
     * NORMER = Mercator
     */
    size_t len;
    nc_type type;
    char* mapproj;
    status = nc_inq_att( ncid, NC_GLOBAL, "projection", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute 'projection' in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        mapproj = new char[len + 1];
        status = nc_get_att_text( ncid, NC_GLOBAL, "projection", mapproj );
        mapproj[len] = '\0';
    }

    std::string mapProj ( mapproj );
    cout<<"projection = "<<mapProj<<endl;

    /*
     * Get global attribute grid_size_in_meters
     *
     */
    float dx, dy;
    status = nc_inq_att( ncid, NC_GLOBAL, "grid_size_in_meters", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute grid_size_in_meters in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "grid_size_in_meters", &dx );
        dy = dx;
    }

    cout <<"dx, dy = "<<dx<<", "<<dy<<endl;

    /*
     * Get global attributes latitude_of_projection_origin,
     *                       longitutde_of_projection_origin
     *
     */
    float cenLat, cenLon;
    status = nc_inq_att( ncid, NC_GLOBAL, "latitude_of_projection_origin", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute latitude_of_projection_origin in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "latitude_of_projection_origin", &cenLat );
    }
    status = nc_inq_att( ncid, NC_GLOBAL, "longitude_of_projection_origin", &type, &len );
    if( status != NC_NOERR ){
        ostringstream os;
        os << "Global attribute longitutde_of_projection_origin in the netcdf file: " << wxModelFileName
        << " cannot be opened\n";
        throw std::runtime_error( os.str() );
    }
    else {
        status = nc_get_att_float( ncid, NC_GLOBAL, "longitude_of_projection_origin", &cenLon );
    }

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
        CPLDebug( "regcmSurfInitialization::setSurfaceGrids()",
                 "Bad forecast file");
        throw badForecastFile("Cannot open forecast file in regcmSurfInitialization::setSurfaceGrids()");
    }
    else {
        GDALClose((GDALDatasetH) poDS ); // close original wxModel file
    }
        
    // open ds one by one, set projection, warp, then write to grid
    GDALDataset *srcDS, *wrpDS;
    std::string temp;
    std::vector<std::string> varList = getVariableList();

    /*
     * Set the initial values in the warped dataset to no data
     */
    GDALWarpOptions* psWarpOptions;

    for( unsigned int i = 0;i < varList.size();i++ ) {

        temp = "NETCDF:" + input.forecastFilename + ":" + varList[i];
        
        CPLPushErrorHandler(&CPLQuietErrorHandler);
        srcDS = (GDALDataset*)GDALOpenShared( temp.c_str(), GA_ReadOnly );
        CPLPopErrorHandler();
        if( srcDS == NULL ) {
            CPLDebug( "wrfInitialization::setSurfaceGrids()",
                    "Bad forecast file" );
        }

        cout<<"varList[i] = " <<varList[i]<<endl;

        /*
         * Set up spatial reference stuff for setting projections
         * and geotransformations
         */

        std::string projString;
        if(mapProj == "NORMER"){  //mercator
            projString = "PROJCS[\"World_Mercator\",GEOGCS[\"GCS_WGS_1984\",DATUM[\"WGS_1984\",\
                          SPHEROID[\"WGS_1984\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],\
                          UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Mercator_1SP\"],\
                          PARAMETER[\"False_Easting\",0],\
                          PARAMETER[\"False_Northing\",0],\
                          PARAMETER[\"Central_Meridian\","+boost::lexical_cast<std::string>(cenLon)+"],\
                          PARAMETER[\"latitude_of_origin\","+boost::lexical_cast<std::string>(cenLat)+"],\
                          UNIT[\"Meter\",1]]";
        }
        else throw badForecastFile("Cannot determine projection from the forecast file information.");

        OGRSpatialReference oSRS, oDemSRS, *poLatLong;
        char *srcWKT = NULL;
        char* prj2 = (char*)projString.c_str();
        oSRS.importFromWkt(&prj2);
        oSRS.exportToWkt(&srcWKT);

        oDemSRS.importFromEPSG(32612);

        printf("%s\n", srcWKT);

        OGRCoordinateTransformation *poCT;
        poLatLong = oSRS.CloneGeogCS();

        /*
         * Transform domain center from lat/long to RegCM space
         */
        double zCenter;
        bool transformed;
        zCenter = 0;
        double xCenter, yCenter;
        xCenter = (double)cenLon;
        yCenter = (double)cenLat;
        //double xCenterArray[2] = {-113, -112.3552}; //1st value is MOAD, 2nd is current domain center
        //double yCenterArray[2] = {43.6, 43.78432}; //1st value is MOAD, 2nd is current domain center

        poCT = OGRCreateCoordinateTransformation(poLatLong, &oSRS);

        if(poCT==NULL || !poCT->Transform(1, &xCenter, &yCenter))
            printf("Transformation failed.\n");

        //if(poCT==NULL || !poCT->Transform(2, xCenterArray, yCenterArray))  //for testing
            //printf("Transformation failed.\n");

        //cout<<"transformed = "<<transformed<<endl;
        //cout<<"xCenter = "<<xCenter<<endl;
        //cout<<"yCenter = "<<yCenter<<endl;

        /*
         * Set the geostransform for the RegCM file
         * upper corner is calculated from transformed x, y
         * (in RegCM space)
         */

        double ncols, nrows;
        int nXSize = srcDS->GetRasterXSize();
        int nYSize = srcDS->GetRasterYSize();
        ncols = (double)(nXSize)/2;
        nrows = (double)(nYSize)/2;

        double adfGeoTransform[6] = {(xCenter-(ncols*dx)), dx, 0,
                                    (yCenter+(nrows*dy)),
                                    0, -(dy)};

        //cout<<"ulcornerX = " <<(xCenter-(ncols*dx))<<endl;
        //cout<<"ulcornerY = " <<(yCenter+(nrows*dy))<<endl;
        //cout<<"ncols = " <<ncols<<endl;
        //cout<<"nrows = " <<nrows<<endl;
        //cout<<"dx = "<<dx<<endl;

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

        psWarpOptions = GDALCreateWarpOptions();

        int nBandCount = srcDS->GetRasterCount();

        psWarpOptions->nBandCount = nBandCount;

        //cout<<"nBandCount = " << nBandCount <<endl;

        psWarpOptions->padfDstNoDataReal =
            (double*) CPLMalloc( sizeof( double ) * nBandCount );
        psWarpOptions->padfDstNoDataImag =
            (double*) CPLMalloc( sizeof( double ) * nBandCount );

        for( int b = 0;b < srcDS->GetRasterCount();b++ ) {
            psWarpOptions->padfDstNoDataReal[b] = dfNoData;
            psWarpOptions->padfDstNoDataImag[b] = dfNoData;
        }

        if( pbSuccess == false )
            dfNoData = -9999.0;

        psWarpOptions->papszWarpOptions =
            CSLSetNameValue( psWarpOptions->papszWarpOptions,
                            "INIT_DEST", "NO_DATA" );


        wrpDS = (GDALDataset*) GDALAutoCreateWarpedVRT( srcDS, srcWKT,
                                                        dstWkt.c_str(),
                                                        GRA_NearestNeighbour,
                                                        1.0, psWarpOptions );

        //=======for testing==================================//
        /*AsciiGrid<double> tempGrid;
        AsciiGrid<double> temp2Grid;
        
        if( varList[i] == "U10" ) {
            GDAL2AsciiGrid( wrpDS, 12, tempGrid );
            if( CPLIsNan( dfNoData ) ) {
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

        //cout<<"bandNum to write is: " <<bandNum<<endl;

        if( varList[i] == "T2" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, airGrid );
        if( CPLIsNan( dfNoData ) ) {
        airGrid.set_noDataValue(-9999.0);
        airGrid.replaceNan( -9999.0 );
        }
    }
        else if( varList[i] == "V10" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, vGrid );
        if( CPLIsNan( dfNoData ) ) {
        vGrid.set_noDataValue(-9999.0);
        vGrid.replaceNan( -9999.0 );
        }
    }
        else if( varList[i] == "U10" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, uGrid );
        if( CPLIsNan( dfNoData ) ) {
        uGrid.set_noDataValue(-9999.0);
        uGrid.replaceNan( -9999.0 );
        }
    }
        else if( varList[i] == "QCLOUD" ) {
            GDAL2AsciiGrid( wrpDS, bandNum, cloudGrid );
        if( CPLIsNan( dfNoData ) ) {
        cloudGrid.set_noDataValue(-9999.0);
        cloudGrid.replaceNan( -9999.0 );
        }
    }
        CPLFree(srcWKT);
        delete poCT;
        GDALDestroyWarpOptions( psWarpOptions );
        GDALClose((GDALDatasetH) srcDS );
        GDALClose((GDALDatasetH) wrpDS );
    }
    cloudGrid /= 100.0;
    wGrid.set_headerData( uGrid );
    wGrid = 0.0;
}

double regcmSurfInitialization::Get_Wind_Height()
{
    return 10.0;
}


