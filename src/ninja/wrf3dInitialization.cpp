/******************************************************************************
*
* $Id:$
*
* Project:  WindNinja
* Purpose:  Initializing with WRF forecasts
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

#include "wrf3dInitialization.h"

/**
 * Constructor for the class initializes the variable names
 *
 */
#ifdef NOMADS_ENABLE_3D
wrf3dInitialization::wrf3dInitialization() : wrfSurfInitialization()
{

}


/**
 * Copy constructor.
 * @param A Copied value.
 */

wrf3dInitialization::wrf3dInitialization(wrf3dInitialization const& A ) : wrfSurfInitialization(A)
{
    wxModelFileName = A.wxModelFileName;
}


/**
* Destructor.
*/

wrf3dInitialization::~wrf3dInitialization()
{
}


/**
 * Equals operator.
 * @param A Value to set equal to.
 * @return a copy of an object
 */

wrf3dInitialization& wrf3dInitialization::operator= (wrf3dInitialization const& A)
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
double ncepGfsSurfInitialization::getGridResolution()
{
    return -1.0;
}


/**
 * Fetch the 3d variable names
 *
 * @return a vector of variable names
 */

std::vector<std::string> wrf3dInitialization::get3dVariableList()
{
    
    std::vector<std::string> var3dList;
    /*
     * Always put T in var3dList[i] because it is not staggered and 
     * the reprojected image x/y grid dimensions are set based on the 
     * first variable in this list (dimensions for staggered variables 
     * are adjusted later as needed)
     */
    var3dList.push_back( "T" );  //perturbation potential temperature
    var3dList.push_back( "V" );
    var3dList.push_back( "U" );
    var3dList.push_back( "W" );
    var3dList.push_back( "QCLOUD" ); // cloud water mixing ratio
    var3dList.push_back( "PHB" );  // geopotential
    var3dList.push_back( "PH" );  // perturbation geopotential
    return var3dList;
    
}


/**
 * Fetch a unique identifier for the forecast.  This string can be used for
 * identifying and storing.
 *
 * @return unique name
 */

std::string wrf3dInitialization::getForecastIdentifier()
{
    
    return std::string( "WRF-3D" );
    
}



/**
  * Static identifier to determine if the netcdf file is a 3D WRF forecast.
  * Uses netcdf c api
  *
  * @param fileName netcdf filename
  *
  * @return true if the forecast is a WRF forecast
  */

bool wrf3dInitialization::identify( std::string fileName )
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
     * :TITLE = "...WRF..."
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
    
    /*
     * Check for 3-D variables: u, v, w
     */
     
    int varid;
    status = nc_inq_varid( ncid, "U", &varid );
    if( status != NC_NOERR ) {
        identified = false;
    }
    status = nc_inq_varid( ncid, "V", &varid );
    if( status != NC_NOERR ) {
        identified = false;
    }
    status = nc_inq_varid( ncid, "W", &varid );
    if( status != NC_NOERR ) {
        identified = false;
    }

    status = nc_close( ncid );

    return identified;
    
}


/**
 * @brief Sets the 3d grids based on a WRF forecast.
 * @param input The WindNinjaInputs for misc. info.
 * @param mesh The WindNinja computational mesh.
 */

void wrf3dInitialization::set3dGrids( WindNinjaInputs &input, Mesh const& mesh )
{
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Reading weather model data...");
    
    setGlobalAttributes(input);
    
    int bandNum = -1;

    //get time list
    std::vector<boost::local_time::local_date_time> timeList( getTimeList(input.ninjaTimeZone) );

    GDALDataset* poDS;
    std::string dstWkt;
    dstWkt = input.dem.prjString;
    if ( dstWkt.empty() ) {
        poDS = (GDALDataset*)GDALOpen( input.dem.fileName.c_str(), GA_ReadOnly );
        if( poDS == NULL ) {
            CPLDebug( "wrf3dInitialization::set3dGrids()",
                    "Bad projection reference" );
            throw("Cannot open dem file in wrf3dInitialization::set3dGrids()");
        }
        dstWkt = poDS->GetProjectionRef();
        if( dstWkt.empty() ) {
            CPLDebug( "wrf3dInitialization::set3dGrids()",
                    "Bad projection reference" );
            throw("Cannot open dem file in wrfedInitialization::set3dGrids()");
        }
        GDALClose((GDALDatasetH) poDS );
    }
    
#ifdef _OPENMP
    omp_guard netCDF_guard(netCDF_lock);
#endif

    CPLPushErrorHandler(CPLQuietErrorHandler);
    poDS = (GDALDataset*)GDALOpenShared( input.forecastFilename.c_str(), GA_ReadOnly );
    CPLPopErrorHandler();
    if( poDS == NULL ) {
        CPLDebug( "wrf3dInitialization::set3dGrids()",
                 "Bad forecast file");
        throw badForecastFile("Cannot open forecast file in wrf3dInitialization::set3dGrids()");
    }
    else {
        GDALClose((GDALDatasetH) poDS ); // close original wxModel file
    }
    
    int nLayers;
    int numStripRows = 0; //number of rows to strip from warped image
    int numStripCols = 0; // number of cols to strip from warped image

    /*
     * Open ds (variables) one by one, set projection, warp, then write to grid
     * Set the initial values in the warped dataset to no data
     */
    
    GDALDataset *srcDS, *wrpDS;
    std::string temp;
    std::vector<std::string> var3dList = get3dVariableList(); 
    GDALWarpOptions* psWarpOptions;

    for( unsigned int i = 0;i < var3dList.size();i++ ) {
        
        //cout<<"var3dList.size() = "<<var3dList.size()<<endl;
        
        temp = "NETCDF:" + input.forecastFilename + ":" + var3dList[i];
        
        CPLPushErrorHandler(CPLQuietErrorHandler);
        srcDS = (GDALDataset*)GDALOpenShared( temp.c_str(), GA_ReadOnly );
        CPLPopErrorHandler();
        if( srcDS == NULL ) {
            CPLDebug( "wrf3dInitialization::setSurfaceGrids()",
                    "Bad forecast file" );
        }

        //cout<<"var3dList[i] = " <<var3dList[i]<<endl;

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
                         PARAMETER[\"false_easting\",0],\PARAMETER[\"false_northing\",0],AUTHORITY[\"EPSG\",\"3031\"],\
                         AXIS[\"Easting\",UNKNOWN],AXIS[\"Northing\",UNKNOWN]]";
        }
        else if(mapProj == 3){  //mercator
            projString = "PROJCS[\"unnamed\",GEOGCS[\"unnamed ellipse\",\
                         DATUM[\"unknown\",SPHEROID[\"unnamed\",6378137,0]],PRIMEM[\"Greenwich\",0],\
                         UNIT[\"degree\",0.0174532925199433]],PROJECTION[\"Mercator_2SP\"],\
                         PARAMETER[\"standard_parallel_1\","+boost::lexical_cast<std::string>(trueLat1)+"],\
                         PARAMETER[\"central_meridian\","+boost::lexical_cast<std::string>(standLon)+"],\
                         PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],\
                         UNIT[\"Meter\",1],EXTENSION[\"PROJ4\",\"+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 \
                         +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs\"]]";
        }

        OGRSpatialReference oSRS, *poLatLong;
        char *srcWKT = NULL;
        char* prj2 = (char*)projString.c_str();
        oSRS.importFromWkt(&prj2);
        oSRS.exportToWkt(&srcWKT);
        
        //printf("%s\n", dstWkt.c_str());

        //printf("%s\n", srcWKT);

        OGRCoordinateTransformation *poCT;
        poLatLong = oSRS.CloneGeogCS();

        /*
         * Transform domain center from lat/long to WRF space
         */
        bool transformed;
        double xCenter, yCenter;
        //double zCenter;
        //zCenter = 0.0;
        xCenter = (double)cenLon;
        yCenter = (double)cenLat;
        //double xCenterArray[2] = {-113, -112.3552}; //1st value is MOAD, 2nd is current domain center
        //double yCenterArray[2] = {43.6, 43.78432}; //1st value is MOAD, 2nd is current domain center

        poCT = OGRCreateCoordinateTransformation(poLatLong, &oSRS);

        if(poCT==NULL || !poCT->Transform(1, &xCenter, &yCenter))
            printf("Transformation failed.\n");

        //if(poCT==NULL || !poCT->Transform(2, xCenterArray, yCenterArray))  //for testing
            //printf("Transformation failed.\n");


        /*
         * Set the geostransform for the WRF file
         * upper corner is calculated from transformed x, y
         * (in WRF space)
         */

        double ncols, nrows, topLeftX, topLeftY;
        int nXSize = srcDS->GetRasterXSize();
        int nYSize = srcDS->GetRasterYSize();
        ncols = (double)(nXSize)/2.0;
        nrows = (double)(nYSize)/2.0;
        topLeftX = xCenter-(ncols*dx);
        topLeftY = yCenter+(nrows*dy);

        
        if(var3dList[i] == "U"){
            topLeftX = (xCenter-(ncols*dx)-0.5*dx); // u stored at left face
        }
        else if(var3dList[i] == "V"){
            topLeftY = (yCenter+(nrows*dy)-0.5*dy); // v stored at south face
        }
        
        double adfGeoTransform[6] = {topLeftX, dx, 0,
                                     topLeftY, 0, -(dy)};


        srcDS->SetGeoTransform(adfGeoTransform);
        
        /*
         * Grab the first band to get the nodata value for the variable,
         * assume all bands have the same ndv
         */
        GDALRasterBand *poBand = srcDS->GetRasterBand( 1 );
        int pbSuccess;
        //double dfNoData = poBand->GetNoDataValue( &pbSuccess );
        dfNoData = poBand->GetNoDataValue( &pbSuccess );

        psWarpOptions = GDALCreateWarpOptions();

        int nBandCount = srcDS->GetRasterCount();
        
        psWarpOptions->nBandCount = nBandCount;

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
        GDAL2AsciiGrid( wrpDS, 12, tempGrid );
        if( CPLIsNan( dfNoData ) ) {
            tempGrid.set_noDataValue( -9999.0 );
            tempGrid.replaceNan( -9999.0 );
        }
        tempGrid.write_Grid("tempGrid", 2);
        
        std::string outFilename = "tempGrid.png";
        std::string scalarLegendFilename = "tempGrid_legend";
        std::string legendTitle = "tempGrid";
        std::string legendUnits = "(?)";
        bool writeLegend = TRUE;

        //tempGrid.ascii2png( outFilename, scalarLegendFilename, legendUnits, legendTitle, writeLegend );


        //Make final grids with same header as dem
        temp2Grid.set_headerData(input.dem);
        temp2Grid.interpolateFromGrid(tempGrid, AsciiGrid<double>::order1);
        temp2Grid.write_Grid("temp2Grid", 2);
        
        outFilename = "temp2Grid.png";
        scalarLegendFilename = "temp2Grid_legend";
        legendTitle = "temp2Grid";
        legendUnits = "(?)";
        writeLegend = TRUE;
        
        //temp2Grid.ascii2png( outFilename, scalarLegendFilename, legendUnits, legendTitle, writeLegend );*/
        //=======end testing=================================//
        
       
        
        if(var3dList[i] == "U" || var3dList[i] == "V" || var3dList[i] == "T" || var3dList[i] == "QCLOUD"){
            nLayers = wxModel_nLayers - 1;
        }
        else nLayers = wxModel_nLayers;
        
        /*
         * Set wxModel_nRows/nCols based on reprojected wx layer.
         * Strip outer edges in WX model image containing ndvs after warping 
         * and allocate appropriately sized wn_3dArrays to store 3d data.
         * The stripping procedure assumes an equal number of rows to
         * be stripped from the north/south edges and an equal number of cols to be 
         * stripped from the east/west edges.
         */
        
        if(i == 0){ // if on first variable in var3dList (all arrays can be set after re-projection of first variable)
            
            
            AsciiGrid<double> tempVarGrid(airGrid); // grid to store temporary wx data for mesh generation
            tempVarGrid = tempVarGrid.get_noDataValue();
            
            GDAL2AsciiGrid( wrpDS, 1, tempVarGrid ); //grab band number 1 to determine number of rows/cols with ndvs to strip
            tempVarGrid.set_noDataValue(-9999.0);
            tempVarGrid.replaceNan( -9999.0 );
            tempVarGrid.replaceValue(dfNoData, -9999.0);
            
            int ii=0, jj=0;
            bool stripRow = true, stripCol = true;
            
            do{ //look for ndvs in outer edges and strip row/col if ndvs are present
                for(int j=jj; j<tempVarGrid.get_nCols() - jj; j++){
                    if(tempVarGrid(ii,j) ==  tempVarGrid.get_noDataValue()){
                        stripRow = true;
                        break;
                    }
                    else stripRow = false;
                }
                if(stripRow == true){
                    ii++;
                }
                for(int i=ii; i<tempVarGrid.get_nRows() - ii; i++){
                    if(tempVarGrid(i,jj) == tempVarGrid.get_noDataValue()){
                        stripCol = true;
                        break;
                    }
                    else stripCol = false;
                }
                if(stripCol == true){
                    jj++;
                }
            }while (stripCol == true || stripRow == true);
            
            numStripRows = ii*2; //*2 to strip from north and south edges
            numStripCols = jj*2; //*2 to strip from east and west edges
            
            wxModel_nRows = tempVarGrid.get_nRows() - numStripRows;
            wxModel_nCols = tempVarGrid.get_nCols() - numStripCols;
            
            /*
             * check that the reprojected, stripped wx grid covers the DEM extent
             */
            double corner_x, corner_y;
            
            corner_x = input.dem.get_xllCorner(); //corner 0
            corner_y = input.dem.get_yllCorner();
            if(!tempVarGrid.check_inBounds(corner_x, corner_y)){
                throw badForecastFile("Forecast domain does not cover full DEM extent");
            }
            corner_x = input.dem.get_xllCorner() + input.dem.get_nCols() * input.dem.get_cellSize(); //corner 1
            corner_y = input.dem.get_yllCorner();
            if(!tempVarGrid.check_inBounds(corner_x, corner_y)){
                throw badForecastFile("Forecast domain does not cover full DEM extent");
            }
            corner_x = input.dem.get_xllCorner() + input.dem.get_nCols() * input.dem.get_cellSize(); //corner 2
            corner_y = input.dem.get_yllCorner() + input.dem.get_nRows() * input.dem.get_cellSize();
            if(!tempVarGrid.check_inBounds(corner_x, corner_y)){
                throw badForecastFile("Forecast domain does not cover full DEM extent");
            }
            corner_x = input.dem.get_xllCorner(); //corner 3
            corner_y = input.dem.get_yllCorner() + input.dem.get_nRows() * input.dem.get_cellSize();
            if(!tempVarGrid.check_inBounds(corner_x, corner_y)){
                throw badForecastFile("Forecast domain does not cover full DEM extent");
            }
            
            allocate(mesh);
        }
        
        
        /*
         * Figure out which band to grab based on which vertical layer and time step we are on.
         * Bands are ordered by layer, then time.
         *
         * T, U, V have n-1 layers; PH, PHB, W have n (i.e., T, U, V have (BOTTOM-TOP_GRID_DIMENSION - 1) layers
         *  and PH, PHB, W have BOTTOM-TOP_GRID_DIMENSION layers.
         */
        
        for(unsigned int k = 0; k <wxModel_nLayers; k++){ //loop over layers
                
            for (unsigned int i_i = 0; i_i < timeList.size(); i_i++){ // find the current time
                
                if(input.ninjaTime == timeList[i_i])
                {
                    bandNum = i_i*nLayers + k + 1;  //adjust to grab next layer
                    break;
                }
            }
            if(bandNum < 0)
                throw std::runtime_error("Could not match ninjaTime with a band number in the forecast file.");

            /*
             * remember, W, PHB, PH have one extra vertical layer
             * WE ARE GRABBING ALL LAYERS FOR ALL VARIABLES
             * just skip if current variable doesn't have this band
             */
            
            if(k == wxModel_nLayers-1){
                if(var3dList[i] == "U" || var3dList[i] == "V" || var3dList[i] == "T" || var3dList[i] == "QCLOUD"){
                    continue;
                }
            }

            
            //cout<<"Grabbing band number: " <<bandNum<<endl;

            if( var3dList[i] == "T" ) {
                GDAL2AsciiGrid( wrpDS, bandNum, airGrid );
                airGrid.set_noDataValue(-9999.0);
                airGrid.replaceNan( -9999.0 );
                airGrid.replaceValue(dfNoData, -9999.0);
                // skip the stripped outer cols/rows when filling array to avoid ndvs
                for(unsigned int i = numStripRows/2; i < airGrid.get_nRows()-numStripRows/2; i++){   
                    for(unsigned int j = numStripCols/2; j < airGrid.get_nCols()-numStripCols/2; j++){
                        airArray(i-numStripRows/2,j-numStripCols/2,k) = airGrid(i,j);
                    }
                }
            }
            else if( var3dList[i] == "V" ) {
                GDAL2AsciiGrid( wrpDS, bandNum, vGrid );
                vGrid.set_noDataValue(-9999.0);
                vGrid.replaceNan( -9999.0 );
                vGrid.replaceValue(dfNoData, -9999.0);
                for(unsigned int i = numStripRows/2; i < vGrid.get_nRows()-numStripRows/2; i++){
                    for(unsigned int j = numStripCols/2; j < vGrid.get_nCols()-numStripCols/2; j++){
                        vArray(i-numStripRows/2,j-numStripCols/2,k) = vGrid(i,j); 
                    }
                }
            }
            else if( var3dList[i] == "U" ) {
                GDAL2AsciiGrid( wrpDS, bandNum, uGrid );
                uGrid.set_noDataValue(-9999.0);
                uGrid.replaceNan( -9999.0 );
                uGrid.replaceValue(dfNoData, -9999.0);
                for(unsigned int i = numStripRows/2; i < uGrid.get_nRows()-numStripRows/2; i++){
                    for(unsigned int j = numStripCols/2; j < uGrid.get_nCols()-numStripCols/2; j++){
                        uArray(i-numStripRows/2,j-numStripCols/2,k) = uGrid(i, j);
                    }
                }
            }
            else if( var3dList[i] == "W" ) {
                GDAL2AsciiGrid( wrpDS, bandNum, wGrid );
                wGrid.set_noDataValue(-9999.0);
                wGrid.replaceNan( -9999.0 );
                wGrid.replaceValue(dfNoData, -9999.0);
                for(unsigned int i = numStripRows/2; i < wGrid.get_nRows()-numStripRows/2; i++){
                    for(unsigned int j = numStripCols/2; j < wGrid.get_nCols()-numStripCols/2; j++){
                        wArray(i-numStripRows/2,j-numStripCols/2,k) = wGrid(i,j); 
                    }
                }
            }
            else if( var3dList[i] == "QCLOUD" ) {
                GDAL2AsciiGrid( wrpDS, bandNum, cloudGrid );
                cloudGrid.set_noDataValue(-9999.0);
                cloudGrid.replaceNan( -9999.0 );
                cloudGrid.replaceValue(dfNoData, -9999.0);
                cloudGrid /= 100.0;
                for(unsigned int i = numStripRows/2; i < cloudGrid.get_nRows()-numStripRows/2; i++){
                    for(unsigned int j = numStripCols/2; j < cloudGrid.get_nCols()-numStripCols/2; j++){
                        cloudArray(i-numStripRows/2,j-numStripCols/2,k) = cloudGrid(i,j); 
                    }
                }
            }
             else if( var3dList[i] == "PHB" ) {
                GDAL2AsciiGrid( wrpDS, bandNum, phbGrid );
                phbGrid.set_noDataValue(-9999.0);
                phbGrid.replaceNan( -9999.0 );
                phbGrid.replaceValue(dfNoData, -9999.0);
                for(unsigned int i = numStripRows/2; i < phbGrid.get_nRows()-numStripRows/2; i++){
                    for(unsigned int j = numStripCols/2; j < phbGrid.get_nCols()-numStripCols/2; j++){
                        phbArray(i-numStripRows/2,j-numStripCols/2,k) = phbGrid(i,j);
                    }
                }
            }
             else if( var3dList[i] == "PH" ) {
                GDAL2AsciiGrid( wrpDS, bandNum, phGrid );
                phGrid.set_noDataValue(-9999.0);
                phGrid.replaceNan( -9999.0 );
                phGrid.replaceValue(dfNoData, -9999.0);
                for(unsigned int i = numStripRows/2; i < phGrid.get_nRows()-numStripRows/2; i++){
                    for(unsigned int j = numStripCols/2; j < phGrid.get_nCols()-numStripCols/2; j++){
                        phArray(i-numStripRows/2,j-numStripCols/2,k) = phGrid(i,j);
                    }
                }
            }
        } //end loop over layers

        CPLFree(srcWKT);
        delete poCT;
        GDALDestroyWarpOptions( psWarpOptions );
        GDALClose((GDALDatasetH) srcDS );
        GDALClose((GDALDatasetH) wrpDS );
        
    } // end loop over variable
    
    
    
    /*
     * Build WX model meshes.
     */
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Generating weather model meshes...");
    buildWxMeshes(input, mesh);
    
    
    // testing ------------- print ascii grids of height above ground
    
    /*wn_3dScalarField tempHag;
    tempHag.allocate(&mesh);
    
    for(int i = 0; i < mesh.nrows; i++){
        for(int j = 0; j < mesh.ncols; j++){
            for(int k = 0; k < mesh.nlayers; k++){
                tempHag(i,j,k) = mesh.ZORD(i,j,k) - mesh.ZORD(i,j,0);
            }
        }
    }
    
	std::string filename;
    AsciiGrid<double> testGrid;
    testGrid.set_headerData(mesh.ncols, mesh.nrows,
                            mesh.XORD(0,0,0), mesh.YORD(0,0,0),
                            mesh.meshResolution, -9999.0, 0.0,
                            input.dem.prjString.c_str());
    testGrid.set_noDataValue(-9999.0);
	
	for(int k = 0; k < mesh.nlayers; k++){
        for(int i = 0; i <mesh.nrows; i++){
            for(int j = 0; j < mesh.ncols; j++ ){
                testGrid(i,j) = tempHag(i,j,k);
                filename = "wn_hag" + boost::lexical_cast<std::string>(k);
            }
        }
        testGrid.write_Grid(filename.c_str(), 2);
	}
    testGrid.deallocate();
    //------------------------------------------end testing*/
    
	
    
    /*
     * Build wn_3dScalarFields linked with WX model meshes.
     */
     
    buildWxScalarFields();
    
    
    //TESTING=============================================
    /*std::string filename;
    AsciiGrid<double> testGrid;
    
    testGrid.set_headerData(uArray.cols_, uArray.rows_,
                            uGrid.get_xllCorner(), uGrid.get_yllCorner(),
                            uGrid.get_cellSize(), uGrid.get_noDataValue(), 0.0,
                            input.dem.prjString.c_str());
    testGrid = -1;
	
	//cout<<"uGrid rows, cols = "<<uGrid.get_nRows()<< ", "<<uGrid.get_nCols()<<endl;
	//cout<<"uArray rows, cols = "<<uArray.rows_<<", "<<uArray.cols_<<endl;
	//cout<<"testGrid rows, cols = "<<testGrid.get_nRows() <<", "<<testGrid.get_nCols()<<endl;
	
	//std::string outFilename = "uArray0.png";
    //std::string scalarLegendFilename = "uArray_legend";
    //std::string legendTitle = "uArray";
    //std::string legendUnits = "(m/s)";
    //bool writeLegend = TRUE;
	
	for(int k = 0; k < 10; k++){
        for(int i = 0; i < uArray.rows_; i++){
            for(int j = 0; j < uArray.cols_; j++ ){
                testGrid(i,j) = uArray(i,j,k);
                filename = "uArray_" + boost::lexical_cast<std::string>(k);
            }
        }
        testGrid.write_Grid(filename.c_str(), 2);
        //if(k = 10){
            //testGrid.ascii2png( outFilename, scalarLegendFilename, legendUnits, legendTitle, writeLegend );
        //}
	}
    testGrid.deallocate();*/
    
    //input.dem.write_Grid("demGrid", 2);
    //END TESTING=============================================
    
    
    /*
     * Interpolate WX model mesh wn_3dScalarFields 
     * to the WN computational mesh.
     */
    
    #ifdef _OPENMP
    double startWxInterpolation = 0.0;
    double endWxInterpolation = 0.0;
    startWxInterpolation = omp_get_wtime();
    #endif
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Interpolating u-component of wind...");
    wxU3d.interpolateScalarData(u3d, mesh, input);
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Interpolating v-component of wind...");        
    wxV3d.interpolateScalarData(v3d, mesh, input);
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Interpolating w-component of wind...");
    wxW3d.interpolateScalarData(w3d, mesh, input);
    
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Interpolating temperature...");
    wxAir3d.interpolateScalarData(air3d, mesh, input);
        
    //input.Com->ninjaCom(ninjaComClass::ninjaNone, "Interpolating cloud cover...");
    //wxCloud3d.interpolateScalarData(cloud3d, mesh, input);
    
    #ifdef _OPENMP
    endWxInterpolation = omp_get_wtime();
    input.Com->ninjaCom(ninjaComClass::ninjaNone, "Interpolation time was %lf seconds.",endWxInterpolation-startWxInterpolation);
    #endif
    

    /*
     * deallocate temporary storage.
     */
    
    deallocateTemp();
    
}

/**
 * @brief Allocates storage for temporary and final data arrays.
 * @param mesh WindNinja computational mesh.
 */
void wrf3dInitialization::allocate(Mesh const& mesh)
{
    /*
     * create wn_3dArrays to store values to send to wn_3dScalarFields.
     * same extent/resolution as wx model mesh.
     * ndvs have been stripped from outer edges.
     * minus/plus 1 to adjust for staggered/non-staggered variables.
     */

    phbArray.allocate(wxModel_nRows, wxModel_nCols, wxModel_nLayers);  
    phArray.allocate(wxModel_nRows, wxModel_nCols, wxModel_nLayers); 
    airArray.allocate(wxModel_nRows, wxModel_nCols, wxModel_nLayers-1);
     
    uArray.allocate(wxModel_nRows, wxModel_nCols+1, wxModel_nLayers-1); 
    vArray.allocate(wxModel_nRows+1, wxModel_nCols, wxModel_nLayers-1);  
    wArray.allocate(wxModel_nRows, wxModel_nCols, wxModel_nLayers);
    cloudArray.allocate(wxModel_nRows, wxModel_nCols, wxModel_nLayers-1);
    
    zStaggerElevationArray.allocate(wxModel_nRows, wxModel_nCols, wxModel_nLayers);
    xStaggerElevationArray.allocate(wxModel_nRows, wxModel_nCols+1, wxModel_nLayers-1 + 1);  // +1 to add extra layer whose bottom face is the ground surface
    yStaggerElevationArray.allocate(wxModel_nRows+1, wxModel_nCols, wxModel_nLayers-1 + 1);  // +1 to add extra layer whose bottom face is the ground surface
    cellCenterElevationArray.allocate(wxModel_nRows, wxModel_nCols, wxModel_nLayers-1 + 1);  // +1 to add extra layer whose bottom face is the ground surface
    
    /*
     * Build wn_3dScalarFields to store variables in 
     * computational mesh space. 
     */
    
    u3d.allocate(&mesh);
    v3d.allocate(&mesh);
    w3d.allocate(&mesh);
    air3d.allocate(&mesh); //perturbation potential temperature
    cloud3d.allocate(&mesh);
}

/**
 *
 * @brief Deallocates temporary storage.
 *
 */
void wrf3dInitialization::deallocateTemp()
{
    uGrid.deallocate();
    vGrid.deallocate();
    wGrid.deallocate();
    cloudGrid.deallocate();
    airGrid.deallocate();
    phGrid.deallocate();
    phbGrid.deallocate();
        
    phbArray.deallocate();
    phArray.deallocate();
    airArray.deallocate();
    uArray.deallocate();
    vArray.deallocate();
    wArray.deallocate();
    cloudArray.deallocate();

    wxAir3d.deallocate();
    wxCloud3d.deallocate();
}


/**
 * @brief Sets the requried global attribute data from the WRF forecast.
 * @param input WindNinja inputs.
 */

void wrf3dInitialization::setGlobalAttributes(WindNinjaInputs &input)
{
    
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

    /*
     * Get global attributes DX, DY
     *
     */
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
    if(dx != dy){
        ostringstream os;
        os << "Global attribute DX in the netcdf file = " << dx
        <<" and Global attribute DY = "<< dy << ". DX, DY must be equal.\n";
        throw std::runtime_error( os.str() );
    }

    /*
     * Get global attributes CEN_LAT, CEN_LON
     *
     */
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

    
}


/**
 * @brief Builds the necessary WX model meshes.
 * @param input WindNinja inputs.
 */

void wrf3dInitialization::buildWxMeshes(WindNinjaInputs &input, Mesh const& mesh)
{
    
    
    /*
     * Compute elevation arrays from PHB, PH.
     * If variable is staggered in x or y, interpolate elevation from
     * cell center to the cell face; this is done by calculating 
     * elevations at the Gauss points for each element. 
     */
    for(unsigned int k = 0; k < wxModel_nLayers; k++){
        for(unsigned int i = 0; i < phArray.rows_; i++){
            for(unsigned int j = 0; j < phArray.cols_; j++){
                // bottom face of z-staggered array layer 0 is the ground
                zStaggerElevationArray(i,j,k) = (phbArray(i,j,k)+phArray(i,j,k))/9.81;
                if(k < wxModel_nLayers - 1 + 1){ // + 1 for addtional layer at ground (not in real WX mesh)
                    if(k == 0){ // we are adding an additional layer whose bottom face is the ground
                        cellCenterElevationArray(i,j,k) = zStaggerElevationArray(i,j,k);
                    }
                    else{
                        cellCenterElevationArray(i,j,k) = 0.5*(phbArray(i,j,k-1)+phArray(i,j,k-1)+
                                                               phbArray(i,j,k)+phArray(i,j,k))/9.81;
                    }
                }
            }
        }
    }
    
    /*
     * build wx model meshes for:
     * 1. cell center variables (T)
     * 2. variables staggered in x (U)
     * 3. variables staggered in y (V)
     * 4. variables staggered in z (W, elevation variables)
     */
    
    double xOffset, yOffset;
    double xllNinja, yllNinja, xllWxModel, yllWxModel;
    input.dem.get_cellPosition(0, 0, &xllNinja, &yllNinja);
    
    airGrid.get_cellPosition(0, 0, &xllWxModel, &yllWxModel);
    xllWxModel += airGrid.get_cellSize(); //adjust to account for stripped rows/cols after warping
    yllWxModel += airGrid.get_cellSize();
    
    xOffset = xllWxModel - xllNinja;
    yOffset = yllWxModel - yllNinja;
    
    cellCenterWxMesh.buildFrom3dWeatherModel(input, cellCenterElevationArray, airGrid.cellSize,
                                 cellCenterElevationArray.rows_, cellCenterElevationArray.cols_, 
                                 wxModel_nLayers-1+1, xOffset, yOffset); // +1 is for extra layer at ground
    zStaggerWxMesh.buildFrom3dWeatherModel(input, zStaggerElevationArray, wGrid.cellSize,
                                 zStaggerElevationArray.rows_, zStaggerElevationArray.cols_, 
                                 wxModel_nLayers, xOffset, yOffset);
                                 

    
    // testing: write x-y grids for all layers of an elevation array
    /*std::string filename;
    AsciiGrid<double> testGrid;
    testGrid.set_headerData(wGrid);
    testGrid.set_noDataValue(-9999.0);*/
    
    /*std::string outFilename = "zStaggerElevation.png";
    std::string scalarLegendFilename = "zStaggerElevation_legend";
    std::string legendTitle = "zStaggerElevation";
    std::string legendUnits = "(m)";
    bool writeLegend = TRUE;*/
	
	/*for(int k = 0; k<zStaggerWxMesh.nlayers; k++){
        for(int i = 0; i<zStaggerElevationArray.rows_; i++){
            for(int j = 0; j<zStaggerElevationArray.cols_; j++ ){
                testGrid(i,j) = zStaggerElevationArray(i,j,k);
                filename = "zStaggerElevationArray" + boost::lexical_cast<std::string>(k);
            }
        }
        testGrid.write_Grid(filename.c_str(), 2);
    }*/
    /*for(int k = 0; k<phbArray.layers_; k++){
        for(int i = 0; i<phbArray.rows_; i++){
            for(int j = 0; j<phbArray.cols_; j++ ){
                testGrid(i,j) = phbArray(i,j,k);
                filename = "phbArray" + boost::lexical_cast<std::string>(k);
            }
        }testGrid.write_Grid(filename.c_str(), 2);
        /*if(k == 0){
            testGrid.write_Grid(filename.c_str(), 2);
            testGrid.replaceNan( -9999.0 );
            //testGrid.ascii2png( outFilename, scalarLegendFilename, legendUnits, legendTitle, writeLegend );
        }*/
	//}
    //testGrid.deallocate();  
    
    

    //get elevations for x and y staggered wx model meshes
     
    element elemElevation(&zStaggerWxMesh);
    
    double x, y, z;
    int elem_i, elem_j, elem_k;
    x = 0;
    y = 0;
    z = 0;
    
    // ----- x-staggered mesh -------------------------------------------------------------------------
    for(int i = 0; i < zStaggerWxMesh.NUMEL; i++)     //Start loop over elements
    {
        elemElevation.node0 = zStaggerWxMesh.get_node0(i);  //get the global node number of local node 0 of element i
        for(int j = 0; j < elemElevation.NUMQPTV; j++)             //Start loop over quadrature points in the element
        {    
            zStaggerWxMesh.get_elemIndex(i, elem_i, elem_j, elem_k);
            
            if(elem_k == 0){ // fill in lowest layer ground elevation
                elemElevation.get_xyz(i, 0, -1, -1, x, y, z);  // get x-staggered elevation at ground face
                xStaggerElevationArray(elem_i, elem_j+1, elem_k) = z; 
            }
            elemElevation.get_xyz(i, 0, -1, 0, x, y, z);  // get x-staggered elevation
            xStaggerElevationArray(elem_i, elem_j+1, elem_k+1) = z; // +1 bc we are adding extra layer at ground
                        
            if(elem_i == (zStaggerElevationArray.rows_ - 2)){ // if on top-most row of elements
                if(elem_k == 0){
                    elemElevation.get_xyz(i, 0, 1, -1, x, y, z); 
                    xStaggerElevationArray(elem_i+1, elem_j+1, elem_k) = z; 
                }
                elemElevation.get_xyz(i, 0, 1, 0, x, y, z);
                xStaggerElevationArray(elem_i+1, elem_j+1, elem_k+1) = z;
            }
            if(elem_j == 0){
                if(elem_k == 0){
                    elemElevation.get_xyz(i, -1, -1, -1, x, y, z);
                    xStaggerElevationArray(elem_i, elem_j, elem_k) = z; 
                }
                elemElevation.get_xyz(i, -1, -1, 0, x, y, z); //get left-most elevation
                xStaggerElevationArray(elem_i, elem_j, elem_k+1) = z;
                if(elem_i = (zStaggerElevationArray.rows_ - 2)){ // if on top-most row of elements
                    if(elem_k == 0){
                        elemElevation.get_xyz(i, -1, 1, -1, x, y, z);
                        xStaggerElevationArray(elem_i+1, elem_j, elem_k) = z; 
                    }
                    elemElevation.get_xyz(i, -1, 1, 0, x, y, z);
                    xStaggerElevationArray(elem_i+1, elem_j, elem_k+1) = z;
                }    
            }
            if(elem_j == zStaggerElevationArray.cols_ - 2){ //if at right-most column
                if(elem_k == 0){
                    elemElevation.get_xyz(i, 1, -1, -1, x, y, z);
                    xStaggerElevationArray(elem_i, elem_j+2, elem_k) = z; 
                }
                elemElevation.get_xyz(i, 1, -1, 0, x, y, z); //get right-most elevation
                xStaggerElevationArray(elem_i, elem_j+2, elem_k+1) = z;
                if(elem_i = (zStaggerElevationArray.rows_ - 2)){ // if on top-most row of elements
                    if(elem_k == 0){
                        elemElevation.get_xyz(i, 1, 1, -1, x, y, z);
                        xStaggerElevationArray(elem_i+1, elem_j+2, elem_k) = z; 
                    }
                    elemElevation.get_xyz(i, 1, 1, 0, x, y, z);
                    xStaggerElevationArray(elem_i+1, elem_j+2, elem_k+1) = z;
                }
            }
        }
    }

    // ----- y-staggered mesh -------------------------------------------------------------------------
    for(int i = 0; i < zStaggerWxMesh.NUMEL; i++)     //Start loop over elements
    {
        elemElevation.node0 = zStaggerWxMesh.get_node0(i);  //get the global node number of local node 0 of element i
        for(int j = 0; j < elemElevation.NUMQPTV; j++)             //Start loop over quadrature points in the element
        {    
            zStaggerWxMesh.get_elemIndex(i, elem_i, elem_j, elem_k);
             
            if(elem_k == 0){ // fill in lowest layer ground elevation
                elemElevation.get_xyz(i, -1, 0, -1, x, y, z);
                yStaggerElevationArray(elem_i+1, elem_j, elem_k) = z;
            }
            elemElevation.get_xyz(i, -1, 0, 0, x, y, z);  // get y-staggered elevation
            yStaggerElevationArray(elem_i+1, elem_j, elem_k+1) = z;  // +1 bc we are adding extra layer at ground
            
            if(elem_j == (zStaggerElevationArray.cols_ - 2)){ // if on right-most column of elements
                if(elem_k == 0){
                    elemElevation.get_xyz(i, 1, 0, -1, x, y, z);
                    yStaggerElevationArray(elem_i+1, elem_j+1, elem_k) = z;
                }
                elemElevation.get_xyz(i, 1, 0, 0, x, y, z);
                yStaggerElevationArray(elem_i+1, elem_j+1, elem_k+1) = z;
            }
            
            if(elem_i == 0){
                if(elem_k == 0){
                    elemElevation.get_xyz(i, -1, -1, -1, x, y, z);
                    yStaggerElevationArray(elem_i, elem_j, elem_k) = z;
                }
                elemElevation.get_xyz(i, -1, -1, 0, x, y, z); // get bottom-most elevation
                yStaggerElevationArray(elem_i, elem_j, elem_k+1) = z;
                if(elem_j == zStaggerElevationArray.cols_ - 2){ // if at bottom-right corner
                    if(elem_k == 0){
                        elemElevation.get_xyz(i, 1, -1, -1, x, y, z); 
                        yStaggerElevationArray(elem_i, elem_j+1, elem_k) = z;
                    }
                    elemElevation.get_xyz(i, 1, -1, 0, x, y, z); 
                    yStaggerElevationArray(elem_i, elem_j+1, elem_k+1) = z;
                }
            }
            if(elem_i = zStaggerElevationArray.rows_ - 2){ //if on top-most row of elements
                if(elem_k == 0){
                    elemElevation.get_xyz(i, -1, 1, -1, x, y, z);
                    yStaggerElevationArray(elem_i+2, elem_j, elem_k) = z;
                }
                elemElevation.get_xyz(i, -1, 1, 0, x, y, z); // get top-most elevation
                yStaggerElevationArray(elem_i+2, elem_j, elem_k+1) = z;
                if(elem_j == zStaggerElevationArray.cols_ - 2){ // if at top-right corner
                    if(elem_k == 0){
                        elemElevation.get_xyz(i, 1, 1, -1, x, y, z); 
                        yStaggerElevationArray(elem_i+2, elem_j+1, elem_k) = z;
                    }
                    elemElevation.get_xyz(i, 1, 1, 0, x, y, z); 
                    yStaggerElevationArray(elem_i+2, elem_j+1, elem_k+1) = z;
                }
            }
        }
    }

    uGrid.get_cellPosition(0, 0, &xllWxModel, &yllWxModel );
    xllWxModel += uGrid.get_cellSize(); //adjust to account for stripped rows/cols after warping
    yllWxModel += uGrid.get_cellSize();
    
    xOffset = xllWxModel + 0.5 * uGrid.get_cellSize() - xllNinja; //not sure why the 0.5*cell size should be here...
    yOffset = yllWxModel - yllNinja;
    
    xStaggerWxMesh.buildFrom3dWeatherModel(input, xStaggerElevationArray, uGrid.cellSize,
                                 xStaggerElevationArray.rows_, xStaggerElevationArray.cols_, 
                                 wxModel_nLayers-1+1, xOffset, yOffset);  // +1 is for extra layer at ground
    
    vGrid.get_cellPosition(0, 0, &xllWxModel, &yllWxModel );
    xllWxModel += vGrid.get_cellSize(); //adjust to account for stripped rows/cols after warping
    yllWxModel += vGrid.get_cellSize();
    
    xOffset = xllWxModel - xllNinja;
    yOffset = yllWxModel + 0.5 * vGrid.get_cellSize() - yllNinja; //not sure why the 0.5*cell size should be here...
    
    yStaggerWxMesh.buildFrom3dWeatherModel(input, yStaggerElevationArray, vGrid.cellSize,
                                 yStaggerElevationArray.rows_, yStaggerElevationArray.cols_, 
                                 wxModel_nLayers-1+1, xOffset, yOffset);  // +1 is for extra layer at ground

    
    //testing: write out vtk files for viewing the meshes ------------------------------------------------------
    /*volVTK vtk;
    vtk.writeMeshVolVTK(zStaggerWxMesh.XORD, zStaggerWxMesh.YORD, zStaggerWxMesh.ZORD, 
                        zStaggerWxMesh.ncols, zStaggerWxMesh.nrows, zStaggerWxMesh.nlayers,
                        "zStaggerWxMesh.vtk");
    vtk.writeMeshVolVTK(cellCenterWxMesh.XORD, cellCenterWxMesh.YORD, cellCenterWxMesh.ZORD, 
                        cellCenterWxMesh.ncols, cellCenterWxMesh.nrows, cellCenterWxMesh.nlayers,
                        "cellCenterStaggerWxMesh.vtk");
                        
    vtk.writeMeshVolVTK(xStaggerWxMesh.XORD, xStaggerWxMesh.YORD, xStaggerWxMesh.ZORD, 
                        xStaggerWxMesh.ncols, xStaggerWxMesh.nrows, xStaggerWxMesh.nlayers,
                        "xStaggerWxMesh.vtk");
                        
    vtk.writeMeshVolVTK(yStaggerWxMesh.XORD, yStaggerWxMesh.YORD, yStaggerWxMesh.ZORD, 
                        yStaggerWxMesh.ncols, yStaggerWxMesh.nrows, yStaggerWxMesh.nlayers,
                        "yStaggerWxMesh.vtk");*/
    // end testing -----------------------------------------------------------------------------------------------------
}


/**
 * @brief Builds wn_scalarFields linked iwth WX model meshes.
 */

void wrf3dInitialization::buildWxScalarFields()
{
    
    /*
     * write wn_3dArrays to wn_3dScalarFields.
     * Note these are still in WX model resolution/extent.
     */
    
    wxU3d.allocate(&xStaggerWxMesh);
    wxV3d.allocate(&yStaggerWxMesh);
    wxW3d.allocate(&zStaggerWxMesh);
    wxAir3d.allocate(&cellCenterWxMesh);
    wxCloud3d.allocate(&cellCenterWxMesh);
    
    
    for(int k = 0; k < xStaggerWxMesh.nlayers; k++){
        for(int i = 0; i < xStaggerElevationArray.rows_; i++){
            for(int j = 0; j < xStaggerElevationArray.cols_; j++ ){
                if(k == 0){
                    wxU3d(i,j,k) = 0.0;
                }
                else{
                    wxU3d(i,j,k) = uArray(i,j,k-1); // -1 because we added extra layer at ground
                }
            }
        }
    }
    for(int k = 0; k < yStaggerWxMesh.nlayers; k++){
        for(int i = 0; i < yStaggerElevationArray.rows_; i++){
            for(int j = 0; j < yStaggerElevationArray.cols_; j++ ){
                if(k == 0){
                    wxV3d(i,j,k) = 0.0;
                }
                else{
                    wxV3d(i,j,k) = vArray(i,j,k-1);  // -1 because we added extra layer at ground
                }
            }
        }
    }
    for(int k = 0; k < zStaggerWxMesh.nlayers; k++){
        for(int i = 0; i < zStaggerElevationArray.rows_; i++){
            for(int j = 0; j < zStaggerElevationArray.cols_; j++ ){
                wxW3d(i,j,k) = wArray(i,j,k);
            }
        }
    }
    for(int k = 0; k < cellCenterWxMesh.nlayers; k++){
        for(int i = 0; i < cellCenterElevationArray.rows_; i++){
            for(int j = 0; j < cellCenterElevationArray.rows_; j++ ){
                if(k == 0){
                    wxAir3d(i,j,k) = 0.0;
                    wxCloud3d(i,j,k) = 0.0;
                }
                else{
                    wxAir3d(i,j,k) = airArray(i,j,k-1);  // -1 because we added extra layer at ground
                    wxCloud3d(i,j,k) = cloudArray(i,j,k-1);  // -1 because we added extra layer at ground
                }
            }
        }
    }
}
#endif
