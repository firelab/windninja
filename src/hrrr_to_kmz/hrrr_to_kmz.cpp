/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Executable for converting hrrr grib2 files to kmz without running WindNinja itself in a simulation
 * Author:   Loren Atwood <loren.atwood@usda.gov>
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

// got many of these functions from wxModelInitialization.cpp and ncepHrrrSurfInitialization.cpp


#include "ninja_init.h"
#include "ninja_conv.h"

#include "ascii_grid.h"

#include "ninjaUnits.h"
#include "KmlVector.h"



/**
* Static identifier to determine if the file is a HRRR forecast.
* If don't have access to grib api, identificaion is done based
* on filename and id of 10u band.
* @param fileName grib2 filename
*
* PCM 01/04/23: account for different variable sets/versions of HRRR. We cannot rely on specific band numbers as the sources and/or filters
* for the HRRR datasets might differ
* TODO - this should check for all bands we need and keep respective band numbers so that we don't have to re-check later
*
* @return true if the forecast is a NCEP HRRR forecast
*/
bool identify( std::string fileName )
{
    GDALDataset *srcDS = (GDALDataset*)GDALOpenShared( fileName.c_str(), GA_ReadOnly );

    if( srcDS == NULL ) {
        throw badForecastFile("during identify(), Cannot open forecast file.");
        return false;

    } else {
        bool identified = false;
        const int nRasterSets = srcDS->GetRasterCount();

        for (int i=1; i<=nRasterSets && !identified; i++) {
            GDALRasterBand *poBand = srcDS->GetRasterBand(i);
            const char* comment = poBand->GetMetadataItem("GRIB_COMMENT");
            if (comment && strcmp(comment, "u-component of wind [m/s]") == 0){
                const char* description = poBand->GetDescription();
                if (strncmp(description, "10[m] ", 6) == 0){
                    identified = true;
                }
            }
        }

        GDALClose( (GDALDatasetH)srcDS );
        return identified;
    }
}


/**
* Fetch the variable names
* @return a vector of variable names
*/
std::vector<std::string> getVariableList()
{
    std::vector<std::string> varList;
    varList.push_back( "2t" );
    varList.push_back( "10v" );
    varList.push_back( "10u" );
    varList.push_back( "tcc" ); // Total cloud cover 
    return varList;
}


/**
* Checks the downloaded data to see if it is all valid.
*/
void checkForValidData( std::string wxModelFileName )
{

}


/**
* Gets a time zone name string corresponding to a latitude and longitude point
* usually the input lat/lon point is the center latitude and longitude of the dataset, 
* or the center latitude and longitude of a bounding box subset of the dataset.
* @param lat The latitude used to locate the timezone.
* @param lon The longitude used to locate the timezone.
* @return timeZoneString The time zone name for the lat/lon point, found from the file "date_time_zonespec.csv".
*/
std::string getTimeZoneString( const double &lat, const double &lon )
{
    std::string timeZoneString = FetchTimeZone(lon, lat, NULL);
    if( timeZoneString == "" )
    {
        fprintf(stderr, "Could not get timezone for lat,lon %f,%f location!!!\n", lat, lon);
        std::exit(1);
    }

    return timeZoneString;
}


// PCM - we can't assume fixed band numbers since HRRR formats change and might be filtered to reduce size
GDALRasterBand* get10UBand (GDALDataset *srcDS) {
    const int nRasterSets = srcDS->GetRasterCount();

    for (int i=1; i<=nRasterSets; i++) {
        GDALRasterBand *poBand = srcDS->GetRasterBand(i);
        const char* comment = poBand->GetMetadataItem("GRIB_COMMENT");
        if (comment && strcmp(comment, "u-component of wind [m/s]") == 0){
            const char* description = poBand->GetDescription();
            if (strncmp(description, "10[m] ", 6) == 0){
                return poBand;
            }
        }
    }

    return NULL;
}


/**
 * Fetch the list of times that the hrrr file holds. It is assumed that
 * the time variable is "time" and the units string is "units". If this
 * is not the case, this function needs to be rewritten.
 *
 * @param timeZoneString Time zone name from the file "date_time_zonespec.csv".
 * @param wxModelFileName the input hrrr file to open and read times from.
 * @throw runtime_error for bad file i/o
 * @return a vector of boost::local_time::local_date_time objects for the forecast.
 */
std::vector<blt::local_date_time>
getTimeList( const std::string &timeZoneString, const std::string &wxModelFileName )
{

    const char *pszVariable = NULL;

    boost::local_time::time_zone_ptr timeZonePtr;
    timeZonePtr = globalTimeZoneDB.time_zone_from_region(timeZoneString.c_str());
    if( timeZonePtr == NULL ) {
        ostringstream os;
        os << "The time zone string: " << timeZoneString.c_str()
       << " does not match any in "
       << "the time zone database file: date_time_zonespec.csv.";
        throw std::runtime_error( os.str() );
    }

    std::vector<blt::local_date_time>timeList;


    // Just 1 time step per forecast file for now.
    GDALDataset *srcDS;
    
    if( strstr( wxModelFileName.c_str(), ".tar" ) ){
        srcDS = (GDALDataset*)GDALOpenShared(CPLSPrintf( "/vsitar/%s", wxModelFileName.c_str() ), GA_ReadOnly);
    }
    else{        
        srcDS = (GDALDataset*)GDALOpenShared( wxModelFileName.c_str(), GA_ReadOnly );
    }


    if( srcDS == NULL ) {
        throw badForecastFile("during getTimeList(), Cannot open forecast file.");
    }

    // get time info from 10u band
    GDALRasterBand *poBand = get10UBand(srcDS);
    if (poBand == NULL) {
        throw badForecastFile(" getTimeList() failed - no 10m UGRD band");
    }

    const char *vt;
    vt = poBand->GetMetadataItem( "GRIB_VALID_TIME" );

    std::string validTimeString( vt );

    // Clean the string for the boost constructor.  Remove the prefix, suffix, and delimiters
    int pos = -1;
    while( validTimeString.find( " " ) != validTimeString.npos ) {
        pos = validTimeString.find( " " );
        validTimeString.erase( pos, 1 );
    }
    pos = validTimeString.find( "secUTC" );
    if( pos != validTimeString.npos ) {
        validTimeString.erase( pos, strlen( "secUTC" ) );
    }

    // Make a posix time in UTC/GMT, to call the boost::local_time::local_date_time input UTC time constructor
    bpt::ptime time_t_epoch( boost::gregorian::date( 1970,1,1 ) );
    long validTime = atoi( validTimeString.c_str() );

    bpt::time_duration duration( 0,0,validTime );

    bpt::ptime first_pt( time_t_epoch + duration );  // UTC time
    blt::local_date_time first_pt_local( first_pt, timeZonePtr ); // local time
    timeList.push_back( first_pt_local );


    return timeList;
}


/**
* Sets the surface grids based on a hrrr forecast.
* @param wxModelFileName The hrrr input filename from which data are read from.
* @param timeBandIdx The band index from which data are read from, corresponding to a specific expected time from getTimeList().
* @param airGrid The air temperature grid to be filled.
* @param cloudGrid The cloud cover grid to be filled.
* @param uGrid The u velocity grid to be filled.
* @param vGrid The v velocity grid to be filled.
* @param wGrid The w velocity grid to be filled (filled with zeros).
* @param west The west side of the clipping box
* @param east The east side of the clipping box
* @param south The south side of the clipping box
* @param north The north side of the clipping box
*/
void setSurfaceGrids( const std::string &wxModelFileName, const int &timeBandIdx, const std::vector<boost::local_time::local_date_time> &timeList, AsciiGrid<double> &airGrid, AsciiGrid<double> &cloudGrid, AsciiGrid<double> &uGrid, AsciiGrid<double> &vGrid, AsciiGrid<double> &wGrid, const double &west, const double &east, const double &south, const double &north )
{

    // looks like the bands go in the same order as the timeList
    int bandNum = timeBandIdx+1;  // timeIdx is 0 to N-1, bandNum is 1 to N.


    GDALDataset *srcDS;
    srcDS = (GDALDataset*)GDALOpenShared( wxModelFileName.c_str(), GA_ReadOnly );

    if( srcDS == NULL ) {
        throw badForecastFile("Cannot open forecast file in setSurfaceGrids()");
    }

    GDALRasterBand *poBand;
    const char *gc;

    double dfNoData;
    int pbSuccess = false;
    bool convertToKelvin = false;

    // Search time list for our time to identify our band number for cloud/speed/dir
    // Right now, just one time step per file
    std::vector<int> bandList;
    for(unsigned int i = 0; i < timeList.size(); i++)
    {
        if(i+1 == bandNum)
        {
            //std::cout << "bandNum = " << bandNum << std::endl;
            for(unsigned int j = 1; j <= srcDS->GetRasterCount(); j++)
            {
                poBand = srcDS->GetRasterBand( j );
                gc = poBand->GetMetadataItem( "GRIB_COMMENT" );
                std::string bandName( gc );

                if ( bandName.find("Temperature") == 0) {
                    CPLDebug("HRRR", "2-m T found...");
                    gc = poBand->GetMetadataItem( "GRIB_SHORT_NAME" );
                    std::string shortName( gc);
                    if (shortName == "2-HTGL") {
                        if (bandName == "Temperature [C]") {
                            convertToKelvin = true;
                            bandList.push_back( j);  // 2t

                        } else if (bandName != "Temperature [K]") {
                            bandList.push_back( j);  // 2t
                        } else {
                            cout << "skipping unsupported forecast temperature unit: " << bandName << endl;
                        }
                    }
                }
            }
            for(unsigned int j = 1; j <= srcDS->GetRasterCount(); j++)
            { 
                poBand = srcDS->GetRasterBand( j );
                gc = poBand->GetMetadataItem( "GRIB_COMMENT" );
                std::string bandName( gc );

                if( bandName.find( "v-component of wind [m/s]" ) != bandName.npos ){
                    CPLDebug("HRRR", "v-component of wind found...");
                    gc = poBand->GetMetadataItem( "GRIB_SHORT_NAME" );
                    std::string bandName( gc );
                    if( bandName.find( "10-HTGL" ) != bandName.npos ){
                        bandList.push_back( j );  // 10v
                        break;
                    }
                }
            }
            for(unsigned int j = 1; j <= srcDS->GetRasterCount(); j++)
            { 
                poBand = srcDS->GetRasterBand( j );
                gc = poBand->GetMetadataItem( "GRIB_COMMENT" );
                std::string bandName( gc );

                if( bandName.find( "u-component of wind [m/s]" ) != bandName.npos ){
                    CPLDebug("HRRR", "u-component of wind found...");
                    gc = poBand->GetMetadataItem( "GRIB_SHORT_NAME" );
                    std::string bandName( gc );
                    if( bandName.find( "10-HTGL" ) != bandName.npos ){
                        bandList.push_back( j );  // 10u    
                        dfNoData = poBand->GetNoDataValue( &pbSuccess );
                        break;
                    }
                }
            }
            for(unsigned int j = 1; j <= srcDS->GetRasterCount(); j++)
            { 
                poBand = srcDS->GetRasterBand( j );
                gc = poBand->GetMetadataItem( "GRIB_COMMENT" );
                std::string bandName( gc );

                if( bandName.find( "Total cloud cover [%]" ) != bandName.npos ){
                    CPLDebug("HRRR", "cloud cover found...");
                    gc = poBand->GetMetadataItem( "GRIB_SHORT_NAME" );
                    std::string bandName( gc );
                    if( bandName.find( "0-RESERVED" ) != bandName.npos ||
                        bandName.find( "0-EATM" ) != bandName.npos){
                        bandList.push_back( j );  // Total cloud cover in % 
                        break;
                    }
                }
            }
        }
    }

    if(bandList.size() < 4) {
        GDALClose((GDALDatasetH) srcDS );
        throw std::runtime_error("Not enough bands detected in HRRR forecast file.");
    }

    // print out the bandList
    // useful for knowing which specific bands to use when replicating the gdalwarp with command line utilities
    //std::cout << "bandList =";
    //for ( unsigned int idx = 0; idx < bandList.size(); idx++ )
    //{
    //    std::cout << " " << bandList[idx];
    //}
    //std::cout << std::endl;


    std::string srcWkt;
    srcWkt = srcDS->GetProjectionRef();

    OGRSpatialReference oSRS, *poLatLong;
    oSRS.importFromWkt(srcWkt.c_str());
    poLatLong = oSRS.CloneGeogCS();
    char *dstWkt = NULL;
    poLatLong->exportToWkt(&dstWkt);
    delete poLatLong;


    GDALDataset *wrpDS;
    GDALWarpOptions* psWarpOptions;

    psWarpOptions = GDALCreateWarpOptions();


    int nBandCount = bandList.size();

    psWarpOptions->nBandCount = nBandCount;

    psWarpOptions->panSrcBands =
        (int *) CPLMalloc(sizeof(int) * nBandCount );
    psWarpOptions->panSrcBands[0] = bandList[0];
    psWarpOptions->panSrcBands[1] = bandList[1];
    psWarpOptions->panSrcBands[2] = bandList[2];
    psWarpOptions->panSrcBands[3] = bandList[3];

    psWarpOptions->panDstBands =
        (int *) CPLMalloc(sizeof(int) * nBandCount );
    psWarpOptions->panDstBands[0] = 1;
    psWarpOptions->panDstBands[1] = 2;
    psWarpOptions->panDstBands[2] = 3;
    psWarpOptions->panDstBands[3] = 4;

    psWarpOptions->padfDstNoDataReal =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );
    psWarpOptions->padfDstNoDataImag =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );


    if( pbSuccess == false )
        dfNoData = -9999.0;
   

    wrpDS = (GDALDataset*) GDALAutoCreateWarpedVRT( srcDS, srcWkt.c_str(),
                                                    dstWkt,
                                                    GRA_NearestNeighbour,
                                                    1.0, psWarpOptions );

    if (wrpDS == NULL) {
        throw std::runtime_error("Warp operation failed!");
    }

    std::vector<std::string> varList = getVariableList();

    for( unsigned int i = 0; i < varList.size(); i++ ) {
        if( varList[i] == "2t" ) {
            GDAL2AsciiGrid( wrpDS, i+1, airGrid );
            if( CPLIsNan( dfNoData ) ) {
                airGrid.set_noDataValue( -9999.0 );
                airGrid.replaceNan( -9999.0 );
            }

            if (convertToKelvin) {
                airGrid += 273.15;
            }
        }
        else if( varList[i] == "10v" ) {
            GDAL2AsciiGrid( wrpDS, i+1, vGrid );
            if( CPLIsNan( dfNoData ) ) {
                vGrid.set_noDataValue( -9999.0 );
                vGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "10u" ) {
            GDAL2AsciiGrid( wrpDS, i+1, uGrid );
            if( CPLIsNan( dfNoData ) ) {
                uGrid.set_noDataValue( -9999.0 );
                uGrid.replaceNan( -9999.0 );
            }
        }
        else if( varList[i] == "tcc" ) {
            GDAL2AsciiGrid( wrpDS, i+1, cloudGrid );
            if( CPLIsNan( dfNoData ) ) {
                cloudGrid.set_noDataValue( -9999.0 );
                cloudGrid.replaceNan( -9999.0 );
            }
        }
    }  // end for loop

    // if there are any clouds set cloud fraction to 1, otherwise set to 0.
    for(int i = 0; i < cloudGrid.get_nRows(); i++){
        for(int j = 0; j < cloudGrid.get_nCols(); j++){
            if(cloudGrid(i,j) < 0.0){
                cloudGrid(i,j) = 0.0;
            }
            else{
                cloudGrid(i,j) = 1.0;
            }
        }
    }

    wGrid.set_headerData( uGrid );
    wGrid = 0.0;

    GDALDestroyWarpOptions( psWarpOptions );
    GDALClose((GDALDatasetH) srcDS );
    GDALClose((GDALDatasetH) wrpDS );


    // clip the ascii grids to the input bounding box
    airGrid.clipGridInPlaceSnapToCells( west, east, south, north );
    cloudGrid.clipGridInPlaceSnapToCells( west, east, south, north );
    uGrid.clipGridInPlaceSnapToCells( west, east, south, north );
    vGrid.clipGridInPlaceSnapToCells( west, east, south, north );
    wGrid.clipGridInPlaceSnapToCells( west, east, south, north );

}


/**
* write the ascii grids to kmz for a wrf surface forecast
* @param outputPath The output path to save the output files to.
* @param forecastTime The boost::local_time::local_date_time corresponding to the data to be plotted, corresponding to a specific time from getTimeList().
* @param outputSpeedUnits The speed units to write the output kmz speed data to.
* @param uGrid The u velocity grid to be plotted.
* @param vGrid The v velocity grid to be plotted.
*/
void writeWxModelGrids( const std::string &outputPath, const boost::local_time::local_date_time &forecastTime, const velocityUnits::eVelocityUnits &outputSpeedUnits, const AsciiGrid<double> &uGrid_wxModel, const AsciiGrid<double> &vGrid_wxModel )
{

    // make speed and direction grids from u,v components
    
    AsciiGrid<double> speedInitializationGrid_wxModel( uGrid_wxModel );
    AsciiGrid<double> dirInitializationGrid_wxModel( uGrid_wxModel );

    for(int i=0; i<uGrid_wxModel.get_nRows(); i++)
    {
        for(int j=0; j<uGrid_wxModel.get_nCols(); j++)
        {
            if( uGrid_wxModel(i,j) == uGrid_wxModel.get_NoDataValue() ||
                vGrid_wxModel(i,j) == vGrid_wxModel.get_NoDataValue() ) {
                speedInitializationGrid_wxModel(i,j) = speedInitializationGrid_wxModel.get_NoDataValue();
                dirInitializationGrid_wxModel(i,j) = dirInitializationGrid_wxModel.get_NoDataValue();
            } else
            {
                wind_uv_to_sd(uGrid_wxModel(i,j), vGrid_wxModel(i,j), &(speedInitializationGrid_wxModel)(i,j), &(dirInitializationGrid_wxModel)(i,j));
            }
        }
    }


    // setup filename parts from the forecastIdentifier and using the forecast time
    ostringstream wxModelTimestream;
    blt::local_time_facet* wxModelOutputFacet;
    wxModelOutputFacet = new blt::local_time_facet();
    wxModelTimestream.imbue(std::locale(std::locale::classic(), wxModelOutputFacet));
    wxModelOutputFacet->format("%m-%d-%Y_%H%M");
    wxModelTimestream << forecastTime;
    std::string forecastIdentifier = "NCEP-HRRR-3km-SURFACE";
    std::string rootname = forecastIdentifier + "-" + wxModelTimestream.str();


    // don't forget the to output units unit conversion
    velocityUnits::fromBaseUnits(speedInitializationGrid_wxModel, outputSpeedUnits);


    // now do the kmz preparation and writing stuff

    KmlVector ninjaKmlFiles;

    ninjaKmlFiles.setKmlFile( CPLFormFilename(outputPath.c_str(), rootname.c_str(), "kml") );
    ninjaKmlFiles.setKmzFile( CPLFormFilename(outputPath.c_str(), rootname.c_str(), "kmz") );
    ////ninjaKmlFiles.setDemFile(dem_filename);  // turns out to be redundant and doesn't do anything, which is good because don't want this dependency

    ninjaKmlFiles.setLegendFile( CPLFormFilename(outputPath.c_str(), rootname.c_str(), "bmp") );
	ninjaKmlFiles.setSpeedGrid(speedInitializationGrid_wxModel, outputSpeedUnits);
	ninjaKmlFiles.setDirGrid(dirInitializationGrid_wxModel);

    //ninjaKmlFiles.setLineWidth(1.0);  // input.googLineWidth value
    ninjaKmlFiles.setLineWidth(3.0);  // input.wxModelGoogLineWidth value
    std::string dateTimewxModelLegFileTemp = CPLFormFilename(outputPath.c_str(), (rootname+"_date_time").c_str(), "bmp");
    ninjaKmlFiles.setTime(forecastTime);
    ninjaKmlFiles.setDateTimeLegendFile(dateTimewxModelLegFileTemp, forecastTime);
    ninjaKmlFiles.setWxModel(forecastIdentifier, forecastTime);

    // default values for input.wxModelGoogSpeedScaling/input.googSpeedScaling,input.googColor,input.googVectorScale are KmlVector::equal_interval,"default",false respectively
    if(ninjaKmlFiles.writeKml(KmlVector::equal_interval,"default",false))
	{
		if(ninjaKmlFiles.makeKmz())
			ninjaKmlFiles.removeKmlFile();
	}
}

void Usage()
{
    printf("hrrr_to_kmz [--osu/output_speed_units mph/mps/kph/kts]\n"
           "            [--op/output_path path]\n"
           "            [--bbox north south east west]\n"
           "            [--p/point cenLat cenLon lat_buff lon_buff]\n"
           "            input_hrrr_filename\n"
           "Defaults:\n"
           "    --output_speed_units mps\n"
           "    --output_path \".\"\n"
           "Note, the bbox and point clipping box inputs have to be in units of lat/lon\n");
    exit(1);
}

void checkArgs( int argIdx, int nSubArgs, char* arg, int argc )
{
    if( (argIdx+nSubArgs) >= argc )
    {
        std::cout << "not enough args for input " << arg << ", input " << arg << " requires " << nSubArgs << " args" << std::endl;
        Usage();
    }
}

int main( int argc, char* argv[] )
{
    std::string input_hrrr_filename = "";
    std::string outputSpeedUnits_str = "mps";
    std::string output_path = ".";

    bool isBbox = false;
    double north = 47.18597932702905;  // I guess default to a clip box over Missoula
    double south = 46.54752767224308;
    double east = -113.45031738281251;
    double west = -114.49401855468751;

    bool isPoint = false;
    double cenLat = (south+north)/2;
    double cenLon = (west+east)/2;
    double lat_buff = north-cenLat;
    double lon_buff = east-cenLon;

    // parse input arguments
    int i = 1;
    while( i < argc )
    {
        if( EQUAL(argv[i], "--output_speed_units") || EQUAL(argv[i], "--osu") )
        {
            checkArgs( i, 1, argv[i], argc );
            outputSpeedUnits_str = std::string( argv[++i] );
        } else if( EQUAL(argv[i], "--output_path") || EQUAL(argv[i], "--op") )
        {
            checkArgs( i, 1, argv[i], argc );
            output_path = std::string( argv[++i] );
        } else if( EQUAL(argv[i], "--bbox") )
        {
            checkArgs( i, 4, argv[i], argc );
            isBbox = true;
            north = CPLAtof( argv[++i] );
            south = CPLAtof( argv[++i] );
            east = CPLAtof( argv[++i] );
            west = CPLAtof( argv[++i] );
            cenLat = (south+north)/2;
            cenLon = (west+east)/2;
            lat_buff = north-cenLat;
            lon_buff = east-cenLon;
        } else if( EQUAL(argv[i], "--point") || EQUAL(argv[i], "--p") )
        {
            checkArgs( i, 4, argv[i], argc );
            isPoint = true;
            cenLat = CPLAtof( argv[++i] );
            cenLon = CPLAtof( argv[++i] );
            lat_buff = CPLAtof( argv[++i] );
            lon_buff = CPLAtof( argv[++i] );
            north = cenLat + lat_buff;
            south = cenLat - lat_buff;
            east = cenLon + lon_buff;
            west = cenLon - lon_buff;
        } else if( EQUAL(argv[i], "--help") || EQUAL(argv[i], "--h") || EQUAL(argv[i], "-help") || EQUAL(argv[i], "-h") )
        {
            Usage();
        } else if( input_hrrr_filename == "" )
        {
            input_hrrr_filename = argv[i];
        } else
        {
            printf("Invalid argument: \"%s\"\n", argv[i]);
            Usage();
        }
        i++;
    }

    if( input_hrrr_filename == "" )
    {
        std::cout << "please enter a valid input_hrrr_filename" << std::endl;
        Usage();
    }
    if( isBbox == false && isPoint == false )
    {
        std::cout << "No clipping box specified, must supply --bbox or --p/point" << std::endl;
        Usage();
    }
    if( isBbox == true && isPoint == true )
    {
        std::cout << "Too many clipping box options specified, must supply --bbox OR --p/point not BOTH" << std::endl;
        Usage();
    }

    int isValidFile = CPLCheckForFile(input_hrrr_filename.c_str(),NULL);
    if( isValidFile != 1 )
    {
        printf("input_hrrr_filename \"%s\" file does not exist!!\n", input_hrrr_filename.c_str());
        exit(1);
    }
    VSIDIR *pathDir;
    pathDir = VSIOpenDir( output_path.c_str(), 0, NULL);
    if( pathDir == NULL )
    {
        printf("output_path \"%s\" is not a valid path!!\n", output_path.c_str());
        exit(1);
    }
    VSICloseDir(pathDir);

    std::cout << "input_hrrr_filename = \"" << input_hrrr_filename.c_str() << "\"" << std::endl;
    std::cout << "output_speed_units = \"" << outputSpeedUnits_str.c_str() << "\"" << std::endl;
    std::cout << "output_path = \"" << output_path.c_str() << "\"" << std::endl;
    if( isBbox == true )
    {
        std::cout << "north south east west = " << north << " " << south << " " << east << " " << west << std::endl;
        std::cout << "  resulting cenLat cenLon = " << cenLat << " " << cenLon << std::endl;
        std::cout << "  resulting lat_buff lon_buff = " << lat_buff << " " << lon_buff << std::endl;
    }
    if( isPoint == true )
    {
        std::cout << "cenLat cenLon lat_buff lon_buff = " << cenLat << " " << cenLon << " " << lat_buff << " " << lon_buff << std::endl;
        std::cout << "  resulting north south east west = " << north << " " << south << " " << east << " " << west << std::endl;
    }


    NinjaInitialize();  // needed for GDALAllRegister()


    velocityUnits::eVelocityUnits outputSpeedUnits = velocityUnits::getUnit(outputSpeedUnits_str);


    if ( identify( input_hrrr_filename ) == false )
    {
        throw badForecastFile("input input_hrrr_filename is not a valid hrrr file!!!");
    }
    checkForValidData( input_hrrr_filename );


    std::string timeZoneString = getTimeZoneString( cenLat, cenLon );

    std::vector<boost::local_time::local_date_time> timeList = getTimeList( timeZoneString, input_hrrr_filename );

    for(unsigned int timeIdx = 0; timeIdx < timeList.size(); timeIdx++)
    //for(unsigned int timeIdx = 0; timeIdx < 1; timeIdx++)
    {
        boost::local_time::local_date_time forecastTime = timeList[timeIdx];

        AsciiGrid<double> airGrid;
        AsciiGrid<double> cloudGrid;
        AsciiGrid<double> uGrid;
        AsciiGrid<double> vGrid;
        AsciiGrid<double> wGrid;

        setSurfaceGrids( input_hrrr_filename, timeIdx, timeList, airGrid, cloudGrid, uGrid, vGrid, wGrid, west, east, south, north );

        writeWxModelGrids( output_path, forecastTime, outputSpeedUnits, uGrid, vGrid );
    }


    return 0;
}








