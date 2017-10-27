/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  A concrete class for initializing WindNinja wind fields using
 *			 the point initialization input method (weather stations)
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

#include "pointInitialization.h"

const std::string pointInitialization::dtoken = "33e3c8ee12dc499c86de1f2076a9e9d4";

const std::string pointInitialization::dvar = "wind_speed,wind_direction,air_temp,"
                                             "solar_radiation,cloud_layer_1_code";

const std::string pointInitialization::ndvar = "wind_speed,wind_direction,air_temp,"
                                              "solar_radiation,cloud_layer_1_code,"
                                              "cloud_layer_2_code,cloud_layer_3_code";

const std::string pointInitialization::baseUrl = "http://api.mesowest.net/v2/stations/";
std::string pointInitialization::rawStationFilename = "";
double pointInitialization::stationBuffer;
std::vector<std::string> pointInitialization::stationFiles;

extern boost::local_time::tz_database globalTimeZoneDB;

pointInitialization::pointInitialization() : initialize()
{
    dfInvDistWeight = atof( CPLGetConfigOption( "NINJA_POINT_INV_DIST_WEIGHT",
                                                "1.0" ) );
    CPLDebug("NINJA", "Setting NINJA_POINT_INV_DIST_WEIGHT to %lf",
             dfInvDistWeight);
}

pointInitialization::~pointInitialization()
{
	
}

void pointInitialization::SetRawStationFilename(std::string filename)
{
    std::string a="wxStation";
    rawStationFilename =filename;
}

/**
 * This function initializes the 3d mesh wind field with initial velocity values
 * based on a number of known surface wind locations (wxStations).
 * The method used to fill (ie. interpolate) the 3d field is to first identify
 * the highest wxStation above the vegetation.  This "above vegetation
 * height" is used as the interpolation height to interpolate horizontally on
 * this 2d field.  All wxStations are vertically interpolated to this
 * height using a Monin-Obukov similarity profile.  Then horizontal
 * interpolation is performed on the "above vegetation surface" using inverse
 * distance squared weighting.  Note that under spatially changing vegetation
 * height, the "above vegetation surface" is stair-stepped in regard to
 * distance from the ground.
 * Last, diurnal components are added.
 */
void pointInitialization::initializeFields(WindNinjaInputs &input,
		Mesh const& mesh,
		wn_3dScalarField& u0,
		wn_3dScalarField& v0,
		wn_3dScalarField& w0,
		AsciiGrid<double>& cloud)
{
    setGridHeaderData(input, cloud);
    
    setInitializationGrids(input);

    initializeWindToZero(mesh, u0, v0, w0);

    initializeBoundaryLayer(input);

    initializeWindFromProfile(input, mesh, u0, v0, w0);

    if((input.diurnalWinds==true) && (profile.profile_switch==windProfile::monin_obukov_similarity))
    {
        addDiurnalComponent(input, mesh, u0, v0, w0);
    }

    cloud = cloudCoverGrid;
}

void pointInitialization::setInitializationGrids(WindNinjaInputs& input)
{
    Aspect aspect;
    Slope slope;
    Shade shade;
    Solar solar;

    if(input.diurnalWinds == true)  //compute values needed for diurnal computations
    {
        aspect.compute_gridAspect(&input.dem, input.numberCPUs);
        slope.compute_gridSlope(&input.dem, input.numberCPUs);
        double aspect_temp = 0; //just placeholder, basically
        double slope_temp = 0;  //just placeholder, basically
        solar.compute_solar(input.ninjaTime, input.latitude, input.longitude, aspect_temp, slope_temp);
        shade.compute_gridShade(&input.dem, solar.get_theta(), solar.get_phi(), input.numberCPUs);
    }

    double *u, *v, *T, *cc, *X, *Y, *influenceRadius;
    u = new double [input.stationsScratch.size()];
    v = new double [input.stationsScratch.size()];
    T = new double [input.stationsScratch.size()];
    cc = new double [input.stationsScratch.size()];
    X = new double [input.stationsScratch.size()];
    Y = new double [input.stationsScratch.size()];
    influenceRadius = new double [input.stationsScratch.size()];
    //height above ground of highest station, used as height of 2d layer to interpolate horizontally to
    double maxStationHeight = -1;	

    for(unsigned int ii = 0; ii<input.stationsScratch.size(); ii++)
    {
        if(input.stationsScratch[ii].get_height() > maxStationHeight)
        {
            maxStationHeight = input.stationsScratch[ii].get_height();
        }
        sd_to_uv(input.stationsScratch[ii].get_speed(), input.stationsScratch[ii].get_direction(), &u[ii], &v[ii]);
        T[ii] = input.stationsScratch[ii].get_temperature();
        cc[ii] = input.stationsScratch[ii].get_cloudCover();
        X[ii] = input.stationsScratch[ii].get_projXord();
        Y[ii] = input.stationsScratch[ii].get_projYord();
        influenceRadius[ii] = input.stationsScratch[ii].get_influenceRadius();
    }

    input.inputWindHeight = maxStationHeight;  //for use later during vertical fill of 3D grid
    input.surface.Z = input.inputWindHeight;

    airTempGrid.interpolateFromPoints(T, X, Y, influenceRadius, input.stationsScratch.size(), dfInvDistWeight);
    cloudCoverGrid.interpolateFromPoints(cc, X, Y, influenceRadius, input.stationsScratch.size(), dfInvDistWeight);

    //Check one grid to be sure that the interpolation completely filled the grid
    if(cloudCoverGrid.checkForNoDataValues())
    {
        throw std::runtime_error("Fill interpolation from the wx stations didn't completely fill the grids. " \
                        "To be sure everything is filled, let at least one wx station have an infinite influence radius. " \
                        "This is specified by defining the influence radius to be a value less than zero in the wx " \
                        "station file.");
    }

    double U_star, anthropogenic_, cg_, bowen_, albedo_;
    int i_, j_;

    cellDiurnal cDiurnal;
    if( input.diurnalWinds == true ) {
        cDiurnal.create( &input.dem, &shade, &solar );
    }

    //now interpolate all stations vertically to the maxStationHeight
    for(unsigned int ii = 0; ii<input.stationsScratch.size(); ii++)
    {
        //if station is not at the 2d interp layer height of maxStationHeight,
        //interpolate vertically using profile to this height
        if(input.stationsScratch[ii].get_height() != maxStationHeight)	
        {	
                profile.inputWindHeight = input.stationsScratch[ii].get_height();
                //get surface properties
                //if station is in the dem domain
                if(input.dem.check_inBounds(input.stationsScratch[ii].get_projXord(),
                    input.stationsScratch[ii].get_projYord()))	
                {
                    input.dem.get_cellIndex(input.stationsScratch[ii].get_projXord(),
                                            input.stationsScratch[ii].get_projYord(), &i_, &j_);

                    profile.Roughness = (input.surface.Roughness)(i_, j_);
                    profile.Rough_h = (input.surface.Rough_h)(i_, j_);
                    profile.Rough_d = (input.surface.Rough_d)(i_, j_);

                    if(input.diurnalWinds == true)	//compute values needed for diurnal computation
                    {
                        double projXord = input.stationsScratch[ii].get_projXord();
                        double projYord = input.stationsScratch[ii].get_projYord();
                        cDiurnal.initialize(projXord, projYord, aspect(i_, j_), slope(i_, j_), cloudCoverGrid(i_, j_),
                                            airTempGrid(i_, j_), input.stationsScratch[ii].get_speed(),
                                            input.stationsScratch[ii].get_height(), (input.surface.Albedo)(i_, j_),
                                            (input.surface.Bowen)(i_, j_), (input.surface.Cg)(i_, j_),
                                            (input.surface.Anthropogenic)(i_, j_), (input.surface.Roughness)(i_, j_),
                                            (input.surface.Rough_h)(i_, j_), (input.surface.Rough_d)(i_, j_));

                        cDiurnal.compute_cell_diurnal_parameters(i_, j_,&profile.ObukovLength, &U_star, &profile.ABL_height);
                            
                    }
                    else
                    {
                        //compute neutral ABL height
                        double f;
                        double velocity;

                        //compute f -> Coriolis parameter
                        if(input.latitude<=90.0 && input.latitude>=-90.0)
                        {
                            // f = 2 * omega * sin(theta)
                            // f should be about 10^-4 for mid-latitudes
                            // (1.4544e-4) here is 2 * omega = 2 * (2 * pi radians) / 24 hours = 1.4544e-4 seconds^-1
                            // obtained from Stull 1988 book
                            f = (1.4544e-4) * sin(pi/180 * input.latitude);	
                            if(f<0)
                                f = -f;
                        }
                        else
                        {
                            //if latitude is not available, set f to mid-latitude value
                            f = 1e-4;	
                        }
                        
                        if(f==0.0)	//zero will give division by zero below
                            f = 1e-8;	//if latitude is zero, set f small

                        //compute neutral ABL height
                        velocity=std::pow(u[ii]*u[ii]+v[ii]*v[ii],0.5);     //Velocity is the velocity magnitude
                        U_star = velocity*0.4/(log((profile.inputWindHeight +
                                                    profile.Rough_h - profile.Rough_d)/profile.Roughness));
                                
                        //compute neutral ABL height
                        profile.ABL_height = 0.2 * U_star / f;	//from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
                        profile.ObukovLength = 0.0;
                    }
                }
                else
                {	
                    //if station is not in dem domain, use grass roughness
                    profile.Roughness = 0.01;
                    profile.Rough_h = 0.0;
                    profile.Rough_d = 0.0;
                    albedo_ = 0.25;
                    bowen_ = 1.0;
                    cg_ = 0.15;
                    anthropogenic_ = 0.0;

                    if(input.diurnalWinds == true)	//compute values needed for diurnal computation
                    {
                        double projXord = input.stationsScratch[ii].get_projXord();
                        double projYord = input.stationsScratch[ii].get_projYord();
                        cDiurnal.initialize(projXord, projYord, 0.0, 0.0,
                                            cloudCoverGrid(i_, j_), airTempGrid(i_, j_),
                                            input.stationsScratch[ii].get_speed(),
                                            input.stationsScratch[ii].get_height(), albedo_, bowen_, cg_,
                                            anthropogenic_, profile.Roughness, profile.Rough_h, profile.Rough_d);

                        cDiurnal.compute_cell_diurnal_parameters(i_, j_, &profile.ObukovLength, &U_star, &profile.ABL_height);
                    }
                    else
                    {
                        //compute neutral ABL height
                        
                        double f;
                        double velocity;

                        //compute f -> Coriolis parameter
                        if(input.latitude<=90.0 && input.latitude>=-90.0)
                        {
                            // f should be about 10^-4 for mid-latitudes
                            // (1.4544e-4) here is 2 * omega = 2 * (2 * pi radians) / 24 hours = 1.4544e-4 seconds^-1
                            // obtained from Stull 1988 book
                            // f = 2 * omega * sin(theta)
                            f = (1.4544e-4) * sin(pi/180 * input.latitude);	
                                if(f<0)
                                    f = -f;
                        }
                        else
                        {
                            f = 1e-4;	//if latitude is not available, set f to mid-latitude value
                        }
                
                        if(f==0.0)	//zero will give division by zero below
                            f = 1e-8;	//if latitude is zero, set f small

                        //compute neutral ABL height
                        velocity=std::pow(u[ii]*u[ii]+v[ii]*v[ii],0.5);     //Velocity is the velocity magnitude
                        U_star = velocity*0.4/(log((profile.inputWindHeight +
                                                    profile.Rough_h - profile.Rough_d)/profile.Roughness));
                        
                        //compute neutral ABL height
                        //from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
                        profile.ABL_height = 0.2 * U_star / f;	
                        profile.ObukovLength = 0.0;
                    }
                }

                //this is height above THE GROUND!! (not "z=0" for the log profile)
                profile.AGL=maxStationHeight + profile.Rough_h;			

                wind_sd_to_uv(input.stationsScratch[ii].get_speed(),
                            input.stationsScratch[ii].get_direction(), &u[ii], &v[ii]);
                profile.inputWindSpeed = u[ii];	
                u[ii] = profile.getWindSpeed();
                profile.inputWindSpeed = v[ii];
                v[ii] = profile.getWindSpeed();
        }
        else
        {
            //else station is already at 2d interp layer height
            wind_sd_to_uv(input.stationsScratch[ii].get_speed(), input.stationsScratch[ii].get_direction(), &u[ii], &v[ii]);
        }
    }

    uInitializationGrid.interpolateFromPoints(u, X, Y, influenceRadius, input.stationsScratch.size(), dfInvDistWeight);
    vInitializationGrid.interpolateFromPoints(v, X, Y, influenceRadius, input.stationsScratch.size(), dfInvDistWeight);

    input.surface.windSpeedGrid.set_headerData(uInitializationGrid);
    input.surface.windGridExists = true;

    for(int i=0; i<input.dem.get_nRows(); i++)
    {
        for(int j=0; j<input.dem.get_nCols(); j++)
        {
            wind_uv_to_sd(uInitializationGrid(i,j), vInitializationGrid(i,j),
                    &(speedInitializationGrid)(i,j), &(dirInitializationGrid)(i,j));
        }
    }

    input.surface.windSpeedGrid = speedInitializationGrid;

    delete[] u;
    u = NULL;
    delete[] v;
    v = NULL;
    delete[] T;
    T = NULL;
    delete[] cc;
    cc = NULL;
    delete[] X;
    X = NULL;
    delete[] Y;
    Y = NULL;
    delete[] influenceRadius;
    influenceRadius = NULL;
}

vector<string> pointInitialization::openCSVList(string csvPath)
{
    vector<string> csvList;
    FILE *wxStationList = VSIFOpen( csvPath.c_str(), "r" );
    while(1){
        const char* f = CPLReadLine(wxStationList);
//        cout<<f<<endl;
        if (f == NULL)
            break;
        if(strstr(f,".csv")){
//            cout<<"f is what we are looking for!"<<f<<endl;            
            csvList.push_back(f);
        }
//        else{
//        cout<<"f is NOT WHAT we are looking for!"<<f<<endl;  
//        }
        
        
    }
    VSIFClose(wxStationList);
        
    
    return csvList;
}

vector<wxStation> pointInitialization::readWxStations(string demFileName, string timeZone) //This is how we handle the old format now!
{
    vector<wxStation> tWork;
//    cout<<rawStationFilename<<endl;
    vector<string>wxLoc;
    wxLoc.push_back(rawStationFilename);
    storeFileNames(wxLoc);
    
    vector<string>stationLocs;
    stationLocs=fetchWxStationID();
    
    vector<vector<preInterpolate> > wxVector;  
    vector<vector<preInterpolate> > wxOrganized;
    
//    for (int i=0;i<stationLocs.size();i++)
//    {
    vector<preInterpolate> singleStationData;
    singleStationData = readDiskLine(demFileName, rawStationFilename);    
    wxVector.push_back(singleStationData);
//    cout<<wxVector[0].size()<<endl;
//        for (int i=0;i<wxVector[0].size();i++)
//        {
//          cout<<wxVector[0][i].stationName<<endl;
//        }
        for (int i=0;i<wxVector[0].size();i++)
        {
            vector<preInterpolate> tempVec;
            tempVec.push_back(wxVector[0][i]);
            wxOrganized.push_back(tempVec);
        }
//        cout<<wxOrganized.size()<<endl;
//        for (int i=0;i<wxOrganized.size();i++)
//        {
//            cout<<"i "<<i<<endl;
//            for (int k=0;k<wxOrganized[i].size();k++)
//            {
//                cout<<"k "<<k<<endl;
//                cout<<wxOrganized[i][k].stationName<<endl;
//            }
//        }
        vector<wxStation> readyToGo;
        readyToGo=interpolateNull(demFileName,wxOrganized,timeZone);
        

    
    
    return readyToGo;
}

vector<wxStation> pointInitialization::interpolateFromDisk(std::string demFile,
                                                      std::vector<boost::posix_time::ptime> timeList,
                                                      std::string timeZone)
{
    vector<preInterpolate> diskData;
    vector<vector<preInterpolate> > wxVector;

    for (int i=0;i<stationFiles.size();i++)
    {
        vector<preInterpolate> singleStationData;
        singleStationData = readDiskLine(demFile, stationFiles[i]);

//        diskData.insert(diskData.end(),singleStationData.begin(),singleStationData.end());
        wxVector.push_back(singleStationData);
    }
    vector<boost::posix_time::ptime> outaTime;
    boost::posix_time::ptime noTime;
    outaTime.push_back(noTime);
    vector<vector<preInterpolate> > interpolatedDataSet;
    vector<wxStation> readyToGo;   
       
    if (wxVector[0][0].datetime==noTime)
    {        
        CPLDebug("STATION_FETCH", "noTime");
        readyToGo=interpolateNull(demFile,wxVector,timeZone);
    }
    else
    {
        //does all interpolation 
        interpolatedDataSet=interpolateTimeData(demFile,wxVector,timeList);
        readyToGo=makeWxStation(interpolatedDataSet,demFile);
    }
    for (int i=0;i<readyToGo.size();i++)
    {
        bool a=wxStation::check_station(readyToGo[i]);
        if (a != true)
        {
            CPLDebug("STATION_FETCH", "check_station failed on #%d: %s", i, readyToGo[i].get_stationName().c_str());
        }
        else
        {
            CPLDebug("STATION_FETCH", "check_station passed on #%d: %s", i, readyToGo[i].get_stationName().c_str());
        }
    }
    return readyToGo;
}

vector<pointInitialization::preInterpolate> pointInitialization::readDiskLine(std::string demFile,
                                                                                 std::string stationLoc)
{
    std::string oErrorString = "";
    preInterpolate oStation;
    std::vector<preInterpolate> oStations;
    preInterpolate work;
    std::vector<preInterpolate> vecwork;
    work.stationName="aaaaaaaa";
    vecwork.push_back(work);

    OGRDataSourceH hDS;
    hDS = OGROpen( stationLoc.c_str(), FALSE, NULL );

    if( hDS == NULL )
    {
        oErrorString = "Cannot open csv file: ";
        oErrorString += stationLoc;
        throw( std::runtime_error( oErrorString ) );
    }

    double dfTempValue = 0.0;
    OGRLayer *poLayer;
    OGRFeature *poFeature;
    OGRFeatureDefn *poFeatureDefn;
    OGRFieldDefn *poFieldDefn;
    poLayer = (OGRLayer*)OGR_DS_GetLayer( hDS, 0 );

    OGRLayerH hLayer;
    hLayer=OGR_DS_GetLayer(hDS,0);
    OGR_L_ResetReading(hLayer);
    int fCount=OGR_L_GetFeatureCount(hLayer,1);

    CPLDebug("STATION_FETCH", "Reading csvName: %s", rawStationFilename.c_str());

    const char* station;
    int idx=0;

    poLayer->ResetReading();

    const char *pszKey;
    std::string oStationName;
    std::string datetime;

    poLayer->ResetReading();

    while( ( poFeature = poLayer->GetNextFeature() ) != NULL )
    {
        poFeatureDefn = poLayer->GetLayerDefn();

        // get Station name
        oStationName = poFeature->GetFieldAsString( 0 );
        oStation.stationName=oStationName;
        pszKey = poFeature->GetFieldAsString( 1 );

        //LAT LON COORDINATES
        if( EQUAL( pszKey, "geogcs" ) )
        {
            //check for valid latitude in degrees
            dfTempValue = poFeature->GetFieldAsDouble( 3 );

            if( dfTempValue > 90.0 || dfTempValue < -90.0 ) {
                OGRFeature::DestroyFeature( poFeature );
                OGR_DS_Destroy( hDS );

                oErrorString = "Bad latitude in weather station csv file";
                oErrorString += " at station: ";
                oErrorString += oStationName;

                throw( std::domain_error( oErrorString ) );
            }

            //check for valid longitude in degrees
            dfTempValue = poFeature->GetFieldAsDouble( 4 );

            if( dfTempValue < -180.0 || dfTempValue > 360.0 )
            {
                OGRFeature::DestroyFeature( poFeature );
                OGR_DS_Destroy( hDS );

                oErrorString = "Bad longitude in weather station csv file";
                oErrorString += " at station: ";
                oErrorString += oStationName;

                throw( std::domain_error( oErrorString ) );
            }

            const char *pszDatum = poFeature->GetFieldAsString( 2 );
            oStation.lat=poFeature->GetFieldAsDouble(3);
            oStation.lon=poFeature->GetFieldAsDouble(4);
            oStation.datumType=pszDatum;
            oStation.coordType=pszKey;

        }
        else if( EQUAL( pszKey, "projcs" ) )
        {
            oStation.lat=poFeature->GetFieldAsDouble(3);
            oStation.lon=poFeature->GetFieldAsDouble(4);
            oStation.coordType=pszKey;
        }
        else
        {
            oErrorString = "Invalid coordinate system: ";
            oErrorString += poFeature->GetFieldAsString( 1 );
            oErrorString += " at station: ";
            oErrorString += oStationName;

            throw( std::domain_error( oErrorString ) );
        }

        //MIDDLE STUFF
        pszKey = poFeature->GetFieldAsString( 6 );
        dfTempValue = poFeature->GetFieldAsDouble( 5 );

        if( dfTempValue <= 0.0 )
        {
            oErrorString = "Invalid height: ";
            oErrorString += poFeature->GetFieldAsString( 5 );
            oErrorString += " at station: ";
            oErrorString += oStationName;

            throw( std::domain_error( oErrorString ) );
        }
        if( EQUAL( pszKey, "meters" ) )
        {
            oStation.height=dfTempValue;
            oStation.heightUnits=lengthUnits::meters;
        }
        else if( EQUAL( pszKey, "feet" ) )
        {
            oStation.height=dfTempValue;
            oStation.heightUnits=lengthUnits::feet;
        }
        else
        {
            oErrorString = "Invalid units for height: ";
            oErrorString += poFeature->GetFieldAsString( 6 );
            oErrorString += " at station: ";
            oErrorString += oStationName;

            throw( std::domain_error( oErrorString ) );
        }

        //WIND SPEED
        pszKey = poFeature->GetFieldAsString( 8 );
        dfTempValue = poFeature->GetFieldAsDouble( 7 );

        if( dfTempValue < 0.0 )
        {
            dfTempValue=0.0;
            oErrorString = "Invalid value for speed: ";
            oErrorString += poFeature->GetFieldAsString( 7 );
            oErrorString += " at station: ";
            oErrorString += oStationName;
            throw( std::domain_error( oErrorString ) );
        }

        if ( EQUAL( pszKey, "mps" ) )
        {
            oStation.speed=dfTempValue;
            oStation.inputSpeedUnits=velocityUnits::metersPerSecond;
        }
        else if( EQUAL( pszKey, "mph" ) )
        {
            oStation.speed=dfTempValue;
            oStation.inputSpeedUnits=velocityUnits::milesPerHour;
        }
        else if( EQUAL( pszKey, "kph" ) )
        {
            oStation.speed=dfTempValue;
            oStation.inputSpeedUnits=velocityUnits::kilometersPerHour;
        }
        else if( EQUAL(pszKey, "kts"))
        {
            oStation.speed=dfTempValue;
            oStation.inputSpeedUnits=velocityUnits::knots;
        }
        else
        {
            oErrorString = "Invalid units for speed: ";
            oErrorString += poFeature->GetFieldAsString( 8 );
            oErrorString += " at station: ";
            oErrorString += oStationName;
            throw( std::domain_error( oErrorString ) );
        }

        //WIND DIRECTION
        dfTempValue = poFeature->GetFieldAsDouble( 9 );

        if( dfTempValue > 360.1 || dfTempValue < 0.0 )
        {
            oErrorString = "Invalid value for direction: ";
            oErrorString += poFeature->GetFieldAsString( 9 );
            oErrorString += " at station: ";
            oErrorString += oStationName;
            dfTempValue=0.0;
        }

        oStation.direction=dfTempValue;

        //TEMPERATURE
        pszKey = poFeature->GetFieldAsString( 11 );

        if( EQUAL(pszKey, "f" ) )
        {
            oStation.temperature=poFeature->GetFieldAsDouble(10);
            oStation.tempUnits=temperatureUnits::F;
        }
        else if( EQUAL( pszKey, "c" ) )
        {
            oStation.temperature=poFeature->GetFieldAsDouble(10);
            oStation.tempUnits=temperatureUnits::C;
        }
        else if( EQUAL( pszKey, "k" ) )
        {
            oStation.temperature=poFeature->GetFieldAsDouble(10);
            oStation.tempUnits=temperatureUnits::K;
        }
        else
        {
            oErrorString = "Invalid units for temperature: ";
            oErrorString += poFeature->GetFieldAsString( 11 );
            oErrorString += " at station: ";
            oErrorString += oStationName;
            throw( std::domain_error( oErrorString ) );
        }

        //CLOUD COVER
        dfTempValue = poFeature->GetFieldAsDouble( 12 );

        if( dfTempValue > 100.0 || dfTempValue < 0.0 )
        {
            oErrorString = "Invalid value for cloud cover: ";
            oErrorString += poFeature->GetFieldAsString( 12 );
            oErrorString += " at station: ";
            oErrorString += oStationName;
            dfTempValue=0.0; //TEMPORARY UNTIL SOLRAD IS FIXED
        }

        oStation.cloudCover=dfTempValue;
        oStation.cloudCoverUnits=coverUnits::percent;

        //RADIUS OF INFLUENCE
        pszKey = poFeature->GetFieldAsString( 14 );
        dfTempValue = poFeature->GetFieldAsDouble( 13 );

        if( EQUAL( pszKey, "miles" ) )
        {
            oStation.influenceRadius=dfTempValue;
            oStation.influenceRadiusUnits=lengthUnits::miles;
        }
        else if( EQUAL( pszKey, "feet" ) )
        {
            oStation.influenceRadius=dfTempValue;
            oStation.influenceRadiusUnits=lengthUnits::feet;
        }
        else if( EQUAL( pszKey, "km" ) )
        {
            oStation.influenceRadius=dfTempValue;
            oStation.influenceRadiusUnits=lengthUnits::kilometers;
        }
        else if( EQUAL( pszKey, "meters" ) )
        {
            oStation.influenceRadius=dfTempValue;
            oStation.influenceRadiusUnits=lengthUnits::meters;
        }
        else
        {
            oErrorString = "Invalid units for influence radius: ";
            oErrorString += poFeature->GetFieldAsString( 14 );
            oErrorString += " at station: ";
            oErrorString += oStationName;
            throw( std::domain_error( oErrorString ) );
        }

        datetime=poFeature->GetFieldAsString(15);
        std::string trunk=datetime.substr(0,datetime.size()-1);

        boost::posix_time::ptime abs_time;

        boost::posix_time::time_input_facet *fig=new boost::posix_time::time_input_facet;
        fig->set_iso_extended_format();
        std::istringstream iss(trunk);
        iss.imbue(std::locale(std::locale::classic(),fig));
        iss>>abs_time;
        oStation.datetime=abs_time;

        oStations.push_back(oStation);
        OGRFeature::DestroyFeature( poFeature );
    }

    OGR_DS_Destroy( hDS );

    return oStations;
}

vector<std::string> pointInitialization::fetchWxStationID()
{
    vector<std::string> stationNames;
    for (int k=0;k<stationFiles.size();k++)
    {
        OGRDataSourceH hDS;
        hDS = OGROpen(stationFiles[k].c_str(), FALSE, NULL);

        OGRLayer *poLayer;
        OGRFeature *poFeature;
        poLayer = (OGRLayer *) OGR_DS_GetLayer(hDS, 0);

        std::string oStationName;

        OGRLayerH hLayer;
        hLayer = OGR_DS_GetLayer(hDS, 0);
        OGR_L_ResetReading(hLayer);

        poLayer->ResetReading();
        while ((poFeature = poLayer->GetNextFeature()) != NULL) {
            // get Station name
            oStationName = poFeature->GetFieldAsString(0);
            stationNames.push_back(oStationName);
            OGRFeature::DestroyFeature( poFeature );
        }
    }

    return stationNames;
}


vector<wxStation> pointInitialization::makeWxStation(vector<vector<preInterpolate> > data, std::string demFile)
{
    CPLDebug("STATION_FETCH", "converting Interpolated struct to wxStation...");
    vector<std::string> stationNames;
    vector<wxStation> stationData;
//    OGRDataSourceH hDS;
//    hDS = OGROpen( rawStationFilename.c_str(), FALSE, NULL );
//
//    OGRLayer *poLayer;
//    OGRFeature *poFeature;
//    OGRFeatureDefn *poFeatureDefn;
//    poLayer = (OGRLayer*)OGR_DS_GetLayer( hDS, 0 );
//
//    std::string oStationName;
//
//    OGRLayerH hLayer;
//    hLayer=OGR_DS_GetLayer(hDS,0);
//    OGR_L_ResetReading(hLayer);
//
//    poLayer->ResetReading();
//    while( ( poFeature = poLayer->GetNextFeature() ) != NULL )
//    {
//        poFeatureDefn = poLayer->GetLayerDefn();
//
//        // get Station name
//        oStationName = poFeature->GetFieldAsString( 0 );
//        stationNames.push_back(oStationName);
//    }
    stationNames=fetchWxStationID();

    int statCount;
    statCount=stationNames.size();

    int specCount=statCount;

    vector<int> idxCount;
    int j=0;
    int q=0;

    for (int i=0;i<statCount;i++)
    {
        CPLDebug("STATION_FETCH", "looks at: %d", q);
        CPLDebug("STATION_FETCH", "starts at: %d", j);

        int idx1=0;
        for(j;j<specCount;j++)
        {
            if(stationNames[j]==stationNames[q])
            {
                idx1++;
            }
        }
        idxCount.push_back(idx1);
        j=std::accumulate(idxCount.begin(),idxCount.end(),0);
        q=j;
        if (j==statCount)
        {
            break;
        }
    }

    CPLDebug("STATION_FETCH", "idxCount size: %ld", idxCount.size());
    vector<int> countLimiter;

    for (int ei=1;ei<=idxCount.size();ei++)
    {
        int rounder=idxCount.size()-ei;
        int e=std::accumulate(idxCount.begin(),idxCount.end()-rounder,0);
        countLimiter.push_back(e);
    }
    vector<vector<preInterpolate> >stationDataList;
    stationDataList=data;
    //here is where a wxstation is made
    for (int i=0;i<idxCount.size();i++)
    {
        wxStation subDat;
        subDat.set_stationName(stationDataList[i][0].stationName);

        std::string CoordSys=stationDataList[i][0].datumType;

        if (CoordSys=="projcs")
        {
            //do something?
        }
        else //WGS84!
        {
            const char* stCoorDat=CoordSys.c_str();
            subDat.set_location_LatLong(stationDataList[i][0].lat,stationDataList[i][0].lon,
                    demFile,stCoorDat);
        }

        for (int k=0;k<stationDataList[i].size();k++)
        {    
            subDat.set_speed(stationDataList[i][k].speed, stationDataList[i][k].inputSpeedUnits);
            subDat.set_direction(stationDataList[i][k].direction);
            subDat.set_temperature(stationDataList[i][k].temperature, stationDataList[i][k].tempUnits);
            subDat.set_cloudCover(stationDataList[i][k].cloudCover, stationDataList[i][k].cloudCoverUnits);
            subDat.set_influenceRadius(stationDataList[i][k].influenceRadius, stationDataList[i][k].influenceRadiusUnits);
            subDat.set_height(stationDataList[i][k].height, stationDataList[i][k].heightUnits);
            subDat.set_datetime(stationDataList[i][k].datetime);
        }

        stationData.push_back(subDat);
    }
    

    return stationData;
}

vector<wxStation> pointInitialization::interpolateNull(std::string demFileName,
                                                    vector<vector<preInterpolate> > vecStations,
                                                    std::string timeZone)
{
    CPLDebug("STATION_FETCH", "no interpolation needed");

    vecStations[0][0].cloudCoverUnits=coverUnits::percent;

    vector<wxStation> refinedDat;
    refinedDat=makeWxStation(vecStations,demFileName);
    //fixes time!
    boost::local_time::time_zone_ptr timeZonePtr;
    timeZonePtr = globalTimeZoneDB.time_zone_from_region(timeZone);
    boost::posix_time::ptime standard = boost::posix_time::second_clock::universal_time();
    for (int i=0;i<refinedDat.size();i++)
    {
        refinedDat[i].datetimeList.assign(1,standard);
    }

    return refinedDat;
}

/**
 * @brief interpolates raw data WRT time
 *
 * @param demFileName: used to set coord system in interpolated data
 * @param vecStations: the raw data to be interpolated
 * @param timeList: the desired time and steps
 *
 */
vector<vector<pointInitialization::preInterpolate> > pointInitialization::interpolateTimeData(std::string demFileName,
                        vector<vector<pointInitialization::preInterpolate> > vecStations,
                        std::vector<boost::posix_time::ptime> timeList)
{
    CPLDebug("STATION_FETCH", "Interpolating time data");

    boost::posix_time::ptime tempq;
    boost::posix_time::ptime init;

    vector<vector<preInterpolate> > Selectify;

    boost::posix_time::time_duration zero(0, 0, 0, 0);
    boost::posix_time::time_duration max(48, 0, 0, 0);
    boost::posix_time::time_duration one(0, 1, 0, 0);

    boost::posix_time::time_duration buffer;
    boost::posix_time::time_duration avgBuffer;
    vector<boost::posix_time::time_duration> avgBufferList;
    boost::posix_time::time_duration bufferSum;

    int totalsize=vecStations.size();

    //Creates a vector of time buffers to be used to interpolate the raw data with the timeList
    for (int j=0; j<totalsize; j++)
    {
        vector<boost::posix_time::time_duration> buffers;
        for (int i=0; i<vecStations[j].size()-1; i++)
        {
            buffer = vecStations[j][i].datetime - vecStations[j][i+1].datetime;
            if (buffer <= zero)
            {
                buffer = buffer.invert_sign();
            }
            if (buffer >= max)
            {
                buffer = buffers[0];
            }
            buffers.push_back(buffer);
        }

        bufferSum = std::accumulate(buffers.begin(), buffers.end(), zero);
        avgBuffer = bufferSum / buffers.size();
        avgBufferList.push_back(avgBuffer);
    }

    for (int k=0; k<totalsize; k++)
    {
        int timesize = 0;
        vector<preInterpolate> subSelectify;
        int qq;
        qq = vecStations[k].size();

        for (int j=0; j<timeList.size(); j++)
        {
            boost::posix_time::ptime comparator;
            comparator = timeList[j];

            int counter=0;
            for (int i=0; i<qq; i++)
            {
                boost::posix_time::time_duration difference;
                difference = comparator - vecStations[k][i].datetime;
                if (difference <= zero)
                {
                    difference = difference.invert_sign();
                }
                if (difference <= avgBufferList[k])
                {
                    counter++;

                    if (counter>2)
                    {
                        continue;
                    }

                    subSelectify.push_back(vecStations[k][i]);
                    continue;
                }
                if (difference <= avgBufferList[k] + one && counter < 2)
                {
                    subSelectify.push_back(vecStations[k][i]);
                    counter++;
                    continue;
                }
            }

            timesize++;
        }

        Selectify.push_back(subSelectify);
    }

    CPLDebug("STATION_FETCH", "Time data Interpolated...\nTemporally Interpolating wx Data...");

    vector<vector<preInterpolate> > lowVec;
    vector<vector<preInterpolate> > highVec;
    for(int j=0; j<Selectify.size(); j++)
    {
        vector<preInterpolate> lowStations;
        vector<preInterpolate> highstations;

        for(int i=0; i<Selectify[j].size(); i+=2)
        {
              lowStations.push_back(Selectify[j][i]);
        }

        for(int k=1; k<Selectify[j].size(); k+=2)
        {
            highstations.push_back(Selectify[j][k]);
        }

        lowVec.push_back(lowStations);
        highVec.push_back(highstations);
    }
    //SETTING WX TIMELIST
    vector<vector<preInterpolate> > interpolatedWxData;
    for(int ey=0; ey<Selectify.size(); ey++)
    {
        vector<preInterpolate> subInter;
        for(int ex=0; ex<timeList.size(); ex++)
        {
            preInterpolate timeStorage;
            timeStorage.datetime = timeList[ex];
            subInter.push_back(timeStorage);
        }
        interpolatedWxData.push_back(subInter);
    }
    //SETTING COORD SYS, DATUM, LAT, LON, HEIGH, HU, RADIUS OF INFLUENCE,NAME
    for(int k=0; k<Selectify.size(); k++)
    {
        double latitude;
        double longitude;
        double height;
        double radiusInfluence;
        std::string datum;
        std::string coord;
        std::string stationName;

        latitude = vecStations[k][0].lat;
        longitude = vecStations[k][0].lon;
        height = vecStations[k][0].height;
        radiusInfluence = vecStations[k][0].influenceRadius;
        datum = vecStations[k][0].datumType;
        coord = vecStations[k][0].coordType;
        const char* newdatum = "WGS84";
        stationName = vecStations[k][0].stationName;

        std::string demfile = demFileName;

        for(int i=0; i<timeList.size(); i++)
        {
            interpolatedWxData[k][i].lat = latitude;
            interpolatedWxData[k][i].lon = longitude;
            interpolatedWxData[k][i].datumType = datum;
            interpolatedWxData[k][i].coordType = coord;
            interpolatedWxData[k][i].height = height;
            interpolatedWxData[k][i].heightUnits = lengthUnits::meters;
            interpolatedWxData[k][i].influenceRadius = radiusInfluence;
            interpolatedWxData[k][i].influenceRadiusUnits = lengthUnits::meters;
            interpolatedWxData[k][i].stationName = stationName;
        }
    }
    //INTERPOLATING WIND SPEED
    for(int k=0; k<Selectify.size(); k++)
    {
        for(int i=0; i<timeList.size(); i++)
        {
            double low;
            double high;
            double inter;
            boost::posix_time::ptime pLow = lowVec[k][i].datetime;
            boost::posix_time::ptime pHigh = highVec[k][i].datetime;
            boost::posix_time::ptime pInter = timeList[i];

            low = unixTime(pLow);
            high = unixTime(pHigh);
            inter = unixTime(pInter);

            double speed1;
            double speed2;
            double speedI;

            speed1 = lowVec[k][i].speed;
            speed2 = highVec[k][i].speed;

            speedI = interpolator(inter,low, high, speed1, speed2);
            if(speedI > 113.000)
            {
                speedI = speed1;

            }

            interpolatedWxData[k][i].speed = speedI;
            interpolatedWxData[k][i].inputSpeedUnits = vecStations[k][0].inputSpeedUnits;
        }
    }
    //INTERPOLATING WIND DIRECITON
    for(int k=0; k<Selectify.size(); k++)
    {
        for(int i=0; i<timeList.size(); i++)
        {
            double lowDir;
            double highDir;
            double interDir;

            lowDir = lowVec[k][i].direction;
            highDir = highVec[k][i].direction;

            interDir = interpolateDirection(lowDir,highDir);
            interpolatedWxData[k][i].direction = interDir;
        }
    }

    //INTERPOLATING TEMPERATURE
    for(int k=0; k<Selectify.size(); k++)
    {
        for(int i=0; i<timeList.size(); i++)
        {
            double low;
            double high;
            double inter;

            boost::posix_time::ptime pLow = lowVec[k][i].datetime;
            boost::posix_time::ptime pHigh = highVec[k][i].datetime;
            boost::posix_time::ptime pInter = timeList[i];

            low = unixTime(pLow);
            high = unixTime(pHigh);
            inter = unixTime(pInter);

            double lowTemp;
            double highTemp;
            double interTemp;

            lowTemp = lowVec[k][i].temperature;
            highTemp = highVec[k][i].temperature;
            interTemp = interpolator(inter, low, high, lowTemp, highTemp);

            if(interTemp > 57.0)
            {
                interTemp = highTemp;
                if(interTemp > 57.0)
                {
                    interTemp = lowTemp;
                }
                if(interTemp > 57.0)
                {
                    interTemp = 25;
                }
            }

            interpolatedWxData[k][i].temperature = interTemp;
            interpolatedWxData[k][i].tempUnits = vecStations[k][0].tempUnits;
        }
    }

    //INTERPOLATING CLOUD COVER
    for(int k=0; k<Selectify.size(); k++)
    {
        for(int i=0; i<timeList.size(); i++)
        {
            double low;
            double high;
            double inter;

            boost::posix_time::ptime pLow = lowVec[k][i].datetime;
            boost::posix_time::ptime pHigh = highVec[k][i].datetime;
            boost::posix_time::ptime pInter = timeList[i];

            low = unixTime(pLow);
            high = unixTime(pHigh);
            inter = unixTime(pInter);

            double lowCloud;
            double highCloud;
            double interCloud;

            lowCloud = lowVec[k][i].cloudCover;
            highCloud = highVec[k][i].cloudCover;

            interCloud = interpolator(inter, low, high, lowCloud, highCloud);

            if(interCloud > 1.0)
            {
                interCloud = highCloud;
                if(interCloud > 1.0)
                {
                    interCloud = lowCloud;
                }
            }

            interpolatedWxData[k][i].cloudCover = interCloud;
            interpolatedWxData[k][i].cloudCoverUnits = coverUnits::percent;
        }
    }
    return interpolatedWxData;
}

double pointInitialization::unixTime(boost::posix_time::ptime time)
{
    boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1));
    boost::posix_time::time_duration::sec_type  dNew= (time - epoch).total_seconds();
    double stepDuration;
    stepDuration = dNew;

    return stepDuration;
}

double pointInitialization::interpolator(double iPoint, double lowX, double highX, double lowY, double highY)
{
    double work = 0.0;
    double slope = (highY - lowY) / (highX - lowX);
    double pointS = (iPoint - lowX);
    double result = lowY + pointS * slope;

    if(result < 0.0000)
    {
        result = work;
    }

    return result;
}

//interpolateDirection uses a MEAN OF CIRCULAR QUANTITIES equation, converts from polar to cartesian,
//averages and then returns degrees. See https://en.wikipedia.org/wiki/Mean_of_circular_quantities
double pointInitialization::interpolateDirection(double lowDir, double highDir)
{
    double lowRad = lowDir * PI / 180.0;
    double highRad = highDir * PI / 180.0;
    double sinSum = sin(lowRad) + sin(highRad);
    double cosSum = cos(lowRad) + cos(highRad);
    double average = atan2(sinSum, cosSum);
    double degAverage = average * 180.0 / PI;

    if (degAverage < 0.0)
    {
        degAverage = degAverage + 360.0;
    }

    return degAverage;
}

std::string pointInitialization::BuildTime(std::string year_0, std::string month_0,
                                      std::string day_0, std::string clock_0,
                                      std::string year_1, std::string month_1,
                                      std::string day_1, std::string clock_1)
{
    //builds the time component of a url
    std::string startTime = "&start=" + year_0 + month_0 + day_0 + clock_0;
    std::string endTime = "&end=" + year_1 + month_1 + day_1 + clock_1;
    std::string timeString = startTime + endTime;

    return timeString;
}

vector<std::string> pointInitialization::UnifyTime(vector<boost::posix_time::ptime> timeList)
{
    vector<std::string> buildTimes;
    vector<std::string> work;
    work.push_back("0");
    stringstream startstream;
    stringstream endstream;
    boost::posix_time::time_facet *facet = new boost::posix_time::time_facet("%Y%m%d%H%M");
    boost::posix_time::time_duration buffer(1, 0, 0, 0);

    startstream.imbue(locale(startstream.getloc(), facet));
    timeList[0] = timeList[0] - buffer;
    startstream<<timeList[0];

    endstream.imbue(locale(endstream.getloc(), facet));
    timeList[timeList.size()-1] = timeList[timeList.size() - 1] + buffer;
    endstream<<timeList[timeList.size() - 1];

    std::string startString=startstream.str();
    std::string endString=endstream.str();

    std::string year_0 = startString.substr(0, 4);
    std::string month_0 = startString.substr(4, 2);
    std::string day_0 = startString.substr(6, 2);
    std::string clock_0 = startString.substr(8);
    std::string year_1 = endString.substr(0, 4);
    std::string month_1 = endString.substr(4, 2);
    std::string day_1 = endString.substr(6, 2);
    std::string clock_1 = endString.substr(8);

    buildTimes.push_back(year_0);
    buildTimes.push_back(month_0);
    buildTimes.push_back(day_0);
    buildTimes.push_back(clock_0);
    buildTimes.push_back(year_1);
    buildTimes.push_back(month_1);
    buildTimes.push_back(day_1);
    buildTimes.push_back(clock_1);

    return buildTimes;
}

//Gets MetaData for stations if turned on
void pointInitialization::fetchMetaData(std::string fileName, std::string demFile, bool write)
{
    CPLDebug("STATION_FETCH", "Downloading Station MetaData...");

    std::string component="&network=1,2&output=geojson";
    std::string bbox;
    std::string url;
    std::string tokfull;

    GDALDataset  *poDS;
    poDS = (GDALDataset *) GDALOpen(demFile.c_str(), GA_ReadOnly );

    double bounds[4];
    bool bRet;

    bRet = GDALGetBounds(poDS, bounds);

    std::string URLat = CPLSPrintf("%.6f", bounds[0]);
    std::string URLon = CPLSPrintf("%.6f", bounds[1]);
    std::string LLLat = CPLSPrintf("%.6f", bounds[2]);
    std::string LLLon = CPLSPrintf("%.6f", bounds[3]);

    bbox = "&bbox=" + LLLon + "," + LLLat + "," + URLon + "," + URLat;
    tokfull = "&token=" + dtoken;
    url = baseUrl + "metadata?" + bbox + component + tokfull;

    std::string csvName;
    if (fileName.substr(fileName.size() - 4, 4) == ".csv")
    {
        csvName = fileName;
    }
    else
    {
        csvName = fileName + ".csv";
    }
    ofstream outFile;
    if (write == true)
    {
        CPLDebug("STATION_FETCH", "writing MetaData for stations...");
        outFile.open(csvName.c_str());
        std::string header="Station_name,STID,Latitude,Longitude,Elevation,Status,MnetID";
        outFile<<header<<endl;
    }

    OGRDataSourceH hDS;
    OGRLayerH hLayer;
    OGRFeatureH hFeature;
    hDS=OGROpen(url.c_str(),0,NULL);

    if (hDS==NULL)
    {
        throw std::runtime_error("Bad metadata in the downloaded station file.");
    }

    hLayer = OGR_DS_GetLayer(hDS, 0);
    OGR_L_ResetReading(hLayer);

    int fCount = OGR_L_GetFeatureCount(hLayer, 1);

    int idx1 = 0;
    int idx2 = 0;
    int idx3 = 0;
    int idx4 = 0;
    int idx5 = 0;
    int idx6 = 0;
    int idx7 = 0;

    double latitude;
    double longitude;
    const char* stid;
    const char* stationName;
    const char* status;
    int mnetID;
    const char* elevation;

    for(int ex=0; ex<fCount; ex++)
    {
        hFeature = OGR_L_GetFeature(hLayer, ex);

        idx1 = OGR_F_GetFieldIndex(hFeature, "STID");
        stid = (OGR_F_GetFieldAsString(hFeature, idx1));

        idx2 = OGR_F_GetFieldIndex(hFeature, "name");
        stationName = (OGR_F_GetFieldAsString(hFeature, idx2));

        idx3 = OGR_F_GetFieldIndex(hFeature, "latitude");
        latitude = (OGR_F_GetFieldAsDouble(hFeature, idx3));

        idx4 = OGR_F_GetFieldIndex(hFeature, "longitude");
        longitude = (OGR_F_GetFieldAsDouble(hFeature, idx4));

        idx5 = OGR_F_GetFieldIndex(hFeature, "status");
        status = (OGR_F_GetFieldAsString(hFeature, idx5));

        idx6 = OGR_F_GetFieldIndex(hFeature, "mnet_id");
        mnetID = (OGR_F_GetFieldAsInteger(hFeature, idx6));

        idx7 = OGR_F_GetFieldIndex(hFeature, "elevation");
        elevation = (OGR_F_GetFieldAsString(hFeature, idx7));

        if(write == true)
        {
            outFile<<stid<<",\""<<stationName<<"\","<<latitude<<","<<longitude<<","<<elevation<<","<<status<<","<<mnetID<<endl;
        }
        OGR_F_Destroy( hFeature );
    }

    OGR_DS_Destroy(poDS);
    OGR_DS_Destroy(hDS);
}

std::string pointInitialization::BuildMultiUrl(std::string station_ids,
                                               std::string yearx,
                                               std::string monthx,
                                               std::string dayx,
                                               std::string clockx,
                                               std::string yeary,
                                               std::string monthy,
                                               std::string dayy,
                                               std::string clocky)
{
    // builds a url for multiple known stations for a specific start and stop time
    std::string network = "1,2";
    std::string nEtworkFull = "&network=" + network;
    std::string timesand = pointInitialization::BuildTime(yearx, monthx, dayx, clockx,
                                                          yeary, monthy, dayy, clocky);
    std::string tokfull = "&token=" + dtoken;
    std::string stidfull = "stid=" + station_ids;
    std::string svarfull = "&vars=" + ndvar;
    std::string output = "&output=geojson";
    std::string url = baseUrl + "timeseries?" + stidfull + nEtworkFull + svarfull + timesand+output + tokfull;

    return url;
}

std::string pointInitialization::BuildMultiLatest(std::string station_ids)
{
    //builds a url for multiple known stations for the latest n hours
    std::string pasthourstr = "60";
    std::string network = "1,2";
    std::string nEtworkFull = "&network=" + network;
    std::string timesand= "&recent=" + pasthourstr;
    std::string tokfull = "&token=" + dtoken;
    std::string stidfull = "stid=" + station_ids;
    std::string svarfull = "&vars=" + ndvar;
    std::string output = "&output=geojson";
    std::string url = baseUrl + "timeseries?" + stidfull + nEtworkFull + svarfull + timesand + output + tokfull;

    return url;
}

std::string pointInitialization::BuildBboxUrl(std::string lat1,
                                              std::string lon1,
                                              std::string lat2,
                                              std::string lon2,
                                              std::string yearx,
                                              std::string monthx,
                                              std::string dayx,
                                              std::string clockx,
                                              std::string yeary,
                                              std::string monthy,
                                              std::string dayy,
                                              std::string clocky)
{
    //builds a url for a given box of latitude longitude coordinates
    std::string network = "1,2";
    std::string nEtworkFull = "&network=" + network;
    std::string timesand = pointInitialization::BuildTime(yearx, monthx, dayx, clockx,
                                                          yeary, monthy, dayy, clocky);
    std::string tokfull = "&token=" + dtoken;
    std::string bbox = "&bbox=" + lon1 + "," + lat1 + "," + lon2 + "," + lat2;
    std::string svarfull = "&vars=" + ndvar;
    std::string output = "&output=geojson";
    std::string url = baseUrl + "timeseries?" + bbox + nEtworkFull + svarfull + timesand + output + tokfull;

    return url;
}

std::string pointInitialization::BuildBboxLatest(std::string lat1,
                                                 std::string lon1,
                                                 std::string lat2,
                                                 std::string lon2)

{
    //builds a url for a bounding box within the latest n hours
    std::string active = "&status=active";
    std::string pasthourstr = "60";
    std::string timesand = "&recent=" + pasthourstr;
    std::string network = "1,2";
    std::string nEtworkFull = "&network=" + network;
    std::string tokfull = "&token=" + dtoken;
    std::string bbox = "&bbox=" + lon1 + "," + lat1 + "," + lon2 + "," + lat2;
    std::string svarfull = "&vars=" + ndvar;
    std::string output = "&output=geojson";
    std::string url = baseUrl + "timeseries?" + bbox + nEtworkFull + svarfull + active + timesand + output + tokfull;

    return url;
}

std::string pointInitialization::BuildUnifiedBbox(double lat1, double lon1, double lat2,
                                                  double lon2, std::string yearx, std::string monthx,
                                                  std::string dayx, std::string clockx, std::string yeary,
                                                  std::string monthy, std::string dayy, std::string clocky)
{
    std::string workUrl="WIP";

    std::string URLat=CPLSPrintf("%.6f",lat2);
    std::string URLon=CPLSPrintf("%.6f",lon2);
    std::string LLLat=CPLSPrintf("%.6f",lat1);
    std::string LLLon=CPLSPrintf("%.6f",lon1);

    std::string URL=BuildBboxUrl(LLLat, LLLon, URLat, URLon, yearx, monthx, dayx, clockx,
                                 yeary, monthy, dayy, clocky);

    return URL;
}

std::string pointInitialization::BuildUnifiedLTBbox(double lat1, double lon1, double lat2, double lon2)
{
    std::string URLat = CPLSPrintf("%.6f", lat2);
    std::string URLon = CPLSPrintf("%.6f", lon2);
    std::string LLLat = CPLSPrintf("%.6f", lat1);
    std::string LLLon = CPLSPrintf("%.6f", lon1);

    std::string URL = BuildBboxLatest(LLLat, LLLon, URLat, URLon);

    return URL;
}

void pointInitialization::setStationBuffer(double buffer, std::string units)
{
    if (units=="km" || units=="kilometers" || units=="kilometres")
    {
        lengthUnits::toBaseUnits(buffer, lengthUnits::kilometers);
    }
    if (units=="miles"||units=="mi")
    {
        lengthUnits::toBaseUnits(buffer, lengthUnits::miles);
    }
    if (units=="feet"||units=="ft")
    {
        lengthUnits::toBaseUnits(buffer, lengthUnits::feet);
    }
    if (units=="meters"||units=="m")
    {
        lengthUnits::toBaseUnits(buffer, lengthUnits::meters);
    }

    stationBuffer = buffer;
}

double pointInitialization::getStationBuffer()
{
        return stationBuffer;
}

vector<std::string> pointInitialization::Split(char* str,const char* delim)
{
    //Splits strings into vectors of strings based on a delimiter, a "," is used for most functions
    char* saveptr;
    char* token = strtok_r(str, delim, &saveptr);

    vector<std::string> result;

    while(token != NULL)
    {
        result.push_back(token);
        token = strtok_r(NULL, delim, &saveptr);
    }
    return result;
}

/*
 * A bit about how the function below works:
 *
 * WindNinja uses cloud_cover(%) for point initialization simulations.
 * RAWS weather stations report solar radiation, which can be approximated to cloud cover
 * using various formulae.
 * FAA/NWS stations from airports do no report solar radiation. These stations have
 * instrumentation to report cloud elevation and coverage. They are reported in a series of codes.
 * CLR corresponds to clear, no clouds, FEW means some clouds, SCT means scattered, BKN indicates
 * broken clouds, more than scatterd and OVC is overcast. It is accepted in aviation that these codes
 * correspond to the okta scale, where cloud cover is divided into eighths. (0 for completely clear
 * 8 for completely cloudy. for more information see the wikipedia page linked below.
 * Mesowest however, has a different system. on the mesowest website (see link below),
 * for KMSO, cloud cover, and current weather conditions share a single column, meaning if it is cloudy and rainy,
 * rainy takes preference. When downloading data from the Mesowest API, cloud cover is reported
 * in three different variables, layer_1, layer_2, and layer_3. For right now, layer_1 is all
 * that is downloaded and used. within a layer, 2 pieces of data are included, the elevation of
 * the clouds, and the coverage. Elevation is reporeted in hundereds of feet.
 * and the last digit indicates cloud coverage. for example 1206 would be 12000 ft and coverage=6
 *  Unfortunately this last digit does no correspond to anything described above. Therefore I had to
 * make up my own conversion. To do this, I first disproved to notion that this last digit directly
 * indicated an okta. at 9:00 on June 8th 2016 the weather outside near KMSO was clear with a few
 * clouds. NOAA reported this as FEW120, and going outside confirmed this. downloading data from the
 * mesowest API from KMSO indicated 1206. If this last digit was an okta, then the sky should have
 * been very cloudy, nearly overcast. Obviously it wasn't. I then downloaded data from KSFO, KSEA,
 * and KJFK and compared NOAA cloud data and oktas with the mesowest data. From this, I was able to
 * determine the following conversion: a mesowest 6 corresponded to a NOAA FEW, a 2 equaled SCT, 3
 * equals BKN, 4 equals OVC.
 * **************************************
 * NOAA   Mesowest  Okta  %cloud        *
 * FEW    6         2/8   25            *
 * SCT    2         4/8   50            *
 * BKN    3         6/8   75            *
 * OVC    4         8/8   100           *
 * CLR    1         0/8   0             *
 * **************************************
 * We are limited by the precision of the mesowest data, as it seems to only return five different
 * values, therefore to convert to Okta and %cloud cover, I used the middle value for each NOAA code
 * ie 2/8 for FEW rather than 3/8 or 1/8.
 *
 * This function then takes in mesowest data, and extracts out the last digit as a string and
 * writes a new array that converts the mesowest data to cloud cover.
 *
 * NOAA Cloud symbols and info
 * http://www.wrh.noaa.gov/mesowest/Metar_cloud_legend.png
 * NOAA KMSO page
 * http://www.wrh.noaa.gov/mesowest/getobext.php?wfo=mso&sid=KMSO&num=48&raw=0&dbn=m
 * Wiki page on Okta
 * https://en.wikipedia.org/wiki/Okta
 * Mesowest KMSO page
 * http://mesowest.utah.edu/cgi-bin/droman/meso_base_dyn.cgi?stn=kmso
 * A URL that will download data from KMSO with just cloud data
 * http://api.mesowest.net/v2/stations/timeseries?stid=kmso&vars=cloud_layer_1_code&latest=1440&token=33e3c8ee12dc499c86de1f2076a9e9d4
 *
 *
 *
 * */

vector<std::string> pointInitialization::InterpretCloudData(const double *dbCloud, int counter)
{
    //converts NWS/FAA cloud information to %cloud cover for a single station
    std::string sa;
    std::string sb;
    std::ostringstream ss;
    std::ostringstream st;

    for(int i=0; i<counter; i++)
    {
        ss<<dbCloud[i]<<",";
        sa=(ss.str());
    }

    const char* delim(",");
    char *cloudstr = &sa[0u];
    vector<std::string> cloudcata;
    cloudcata = Split(cloudstr, delim);

    vector<std::string> cloudcatb;

    for(int i=0;i<counter;i++)
    {
        st<<cloudcata[i][cloudcata[i].size()-1]<<",";
        sb=(st.str());
    }

    char *cloudstrb = &sb[0u];
    cloudcatb = Split(cloudstrb, delim);

    std::string few = "25";
    std::string sct = "50";
    std::string bkn = "75";
    std::string ovc = "100";
    std::string clr = "0";
    std::string mesofew = "6";
    std::string mesosct = "2";
    std::string mesobkn = "3";
    std::string mesoovc = "4";
    std::string mesoclr = "1";
    std::string mesonull = "0";

    vector<std::string> lowclouddat;

    for(int i=0; i<counter; i++)
    {
        if (cloudcatb[i]==mesofew)
        {
            lowclouddat.push_back(few);
        }
        if (cloudcatb[i]==mesosct)
        {
            lowclouddat.push_back(sct);
        }
        if (cloudcatb[i]==mesobkn)
        {
            lowclouddat.push_back(bkn);
        }
        if (cloudcatb[i]==mesoovc)
        {
            lowclouddat.push_back(ovc);
        }
        if (cloudcatb[i]==mesoclr)
        {
            lowclouddat.push_back(clr);
        }
        if (cloudcatb[i]==mesonull)
        {
            lowclouddat.push_back(clr);
        }
    }

    return lowclouddat;
}

vector<std::string> pointInitialization::CompareClouds(vector<std::string>low, vector<std::string>med,
                                                       vector<std::string>high, int countlow, int countmed,
                                                       int counthigh)
{
    vector<std::string> work;
    work.push_back("wip");

    vector<int> lowcloudint;
    vector<int> medcloudint;
    vector<int> highcloudint;
    vector<int> overcloud;
    vector<std::string> totalCloudcat;
    std::string aa;

    for (int ti=0; ti<countlow; ti++)
    {
        int numa = atoi(low.at(ti).c_str());
        lowcloudint.push_back(numa);
    }
    for (int ti=0; ti<countmed; ti++)
    {
        int numb = atoi(med.at(ti).c_str());
        medcloudint.push_back(numb);
    }
    for (int ti=0; ti<counthigh; ti++)
    {
        int numc = atoi(high.at(ti).c_str());
        highcloudint.push_back(numc);
    }
    for (int ti=0; ti<countlow; ti++)
    {
        if (lowcloudint[ti] >= medcloudint[ti] && lowcloudint[ti] >= highcloudint[ti])
        {
            overcloud.push_back(lowcloudint[ti]);
            continue;
        }
        if (highcloudint[ti] >= medcloudint[ti] && highcloudint[ti] >= lowcloudint[ti])
        {
            overcloud.push_back(highcloudint[ti]);
            continue;
        }
        if (medcloudint[ti] >= lowcloudint[ti] && medcloudint[ti] >= highcloudint[ti])
        {
            overcloud.push_back(medcloudint[ti]);
            continue;
        }
    }

    int overLen = overcloud.size();

    std::string cloudstring;
    std::ostringstream cs;

    for(int i=0;i<overLen;i++)
    {
        cs<<overcloud[i]<<",";
        cloudstring = (cs.str());

    }
    const char* delim(",");
    char *cloudstr = &cloudstring[0u];
    totalCloudcat = Split(cloudstr, delim);

    return totalCloudcat;
}

vector<std::string> pointInitialization::UnifyClouds(const double *dvCloud, const double *dwCloud,
                                        const double *dxCloud, int count1, int count2, int count3, int backupcount)
{
    vector<std::string> work;
    work.push_back("wip");
    vector<std::string> daCloud;
    vector<std::string> dcCloud;
    vector<std::string> deCloud;
    vector<std::string> sCloudData;

    if(count1 == 0)
    {
       CPLDebug("STATION_FETCH", "no cloud data exists, using air temp count to zero out data");
       count1 = backupcount;
       if(backupcount == 0)
       {
           CPLDebug("STATION_FETCH", "this station is terrible, don't use it, writing data as -9999");
           sCloudData.push_back("-9999");
       }
    }
    if(count2 == 0)
    {
        count2 = count1;
    }
    if(count3 == 0)
    {
        count3 = count1;
    }

    if(dvCloud == 0)
    {
        for(int ska=0; ska<count1; ska++)
        {
            daCloud.push_back("0");
        }
    }
    else
    {
        daCloud = InterpretCloudData(dvCloud, count1);
    }
    if(dwCloud == 0)
    {
        for(int ska=0; ska<count2; ska++)
        {
            dcCloud.push_back("0");
        }
    }
    else
    {
        dcCloud = InterpretCloudData(dwCloud, count2);
    }
    if(dxCloud == 0)
    {
        for(int ska=0; ska<count3; ska++)
        {
            deCloud.push_back("0");
        }
    }
    else
    {
        deCloud = InterpretCloudData(dxCloud, count3);
    }

    sCloudData = CompareClouds(daCloud, dcCloud, deCloud, count1, count2, count3);

    return sCloudData;
}

vector<double> pointInitialization::Irradiate(const double* solrad, int smallcount, int largecount,
                                            std::string timeZone, double lat, double lon, char** times)
{
    //will eventually convert solar radiation to cloud cover, this function doesn't work yet.

    vector<double> blegh;
    vector<double> outCloud;
    double work=0.000;
    blegh.push_back(work);

    for (int j=0;j<largecount;j++)
    {
        char *trunk=times[j];
        Solar sol;
        bool silver;

        boost::posix_time::ptime abs_time;

        boost::posix_time::time_input_facet *fig=new boost::posix_time::time_input_facet;
        fig->set_iso_extended_format();
        std::istringstream iss(trunk);
        iss.imbue(std::locale(std::locale::classic(),fig));
        iss>>abs_time;

        boost::local_time::tz_database tz_db;
        tz_db.load_from_file( FindDataPath("date_time_zonespec.csv") );
        boost::local_time::time_zone_ptr timeZonePtr;
        timeZonePtr = tz_db.time_zone_from_region(timeZone);

        boost::local_time::local_date_time startLocal(abs_time,timeZonePtr);

        double zero=0.000000;
        double one=1.0000000;

        silver=sol.compute_solar(startLocal,lat,lon,zero,zero);

        double senor=sol.get_solarIntensity();
        double solFrac;

        solFrac=solrad[j]/senor;
        if (solFrac<=zero)
        {
            solFrac=one;
        }
        if (solFrac>one)
        {
            solFrac=one;
        }
        if (isnan(solFrac))
        {
            solFrac=one;
        }
        solFrac=one-solFrac;
        solFrac=100*solFrac;
        outCloud.push_back(solFrac);
    }

    return outCloud;
}

vector<std::string> pointInitialization::fixWindDir(const double *winddir, std::string filler, int count)
{
    std::string sa;
    std::ostringstream ss;
    vector<std::string>direction;

    if (winddir==0)
    {
        for (int jw=0;jw<count;jw++)
        {
            direction.push_back(filler);
        }
    }
    else
    {
        for(int i=0;i<count;i++)
        {
            ss<<winddir[i]<<",";
            sa=(ss.str());
        }
        const char* delim(",");
        char *cloudstr=&sa[0u];
        direction=Split(cloudstr,delim);
    }

    return direction;
}

/**@brief Builds the time list for a pointInitialization run.
 *
 * @param startYear Start year for the simulation.
 * @param startMonth Start month for the simulation.
 * @param startDay Start day for the simulation.
 * @param startHour Start hour for the simulation.
 * @param startMinute Start minute for the simulation.
 * @param endYear End year for the simulation.
 * @param endMonth End month for the simulation.
 * @param endDay End day for the simulation.
 * @param endHour End hour for the simulation.
 * @param endMinute End minute for the simulation.
 * @param nTimeSteps Number of time steps for the simulation.
 * @param timeZone String identifying time zone (must match strings in the file "date_time_zonespec.csv".
 * @return Vector of datetimes in UTC.
 */
std::vector<boost::posix_time::ptime>
pointInitialization::getTimeList(int startYear, int startMonth, int startDay,
                                    int startHour, int startMinute, int endYear,
                                    int endMonth, int endDay, int endHour, int endMinute,
                                    int nTimeSteps, std::string timeZone)
{
    boost::local_time::tz_database tz_db;
    tz_db.load_from_file( FindDataPath("date_time_zonespec.csv") );
    boost::local_time::time_zone_ptr timeZonePtr;
    timeZonePtr = tz_db.time_zone_from_region(timeZone);
    
    endHour=endHour;
    startHour=startHour;
    
    boost::gregorian::date dStart(startYear,startMonth,startDay);
    boost::posix_time::time_duration dStartTime(startHour,startMinute,0,0);
    boost::local_time::local_date_time startLocal(dStart,dStartTime,timeZonePtr,true);

    boost::gregorian::date dEnd(endYear,endMonth,endDay);
    boost::posix_time::time_duration dEndTime(endHour,endMinute,0,0);
    boost::local_time::local_date_time endLocal(dEnd,dEndTime,timeZonePtr,true);

    boost::posix_time::ptime startUtc=startLocal.utc_time();
    boost::posix_time::ptime endUtc=endLocal.utc_time();
    printf("\n");

    boost::posix_time::time_duration diffTime=endUtc-startUtc;
    boost::posix_time::time_duration stepTime=diffTime/nTimeSteps;

    std::vector<boost::posix_time::ptime> timeOut;
    std::vector<boost::posix_time::ptime> timeConstruct;
    std::vector<boost::posix_time::ptime> timeList;
    std::vector<boost::posix_time::time_duration> timeStorage;

    timeOut.push_back(startUtc);
    for (int i=1;i<nTimeSteps;i++)
    {
        boost::posix_time::time_duration specTime;
        specTime=stepTime*i;
        timeOut.push_back(startUtc+specTime);
    }
    timeOut.push_back(endUtc);
    timeList=timeOut;

    return timeList;
}

/**@brief Fetches station data from bounding box.
 *
 * @param demFile Filename/path to the DEM on disk.
 * @param timeList Vector of datetimes in UTC for the simulation.
 */
bool pointInitialization::fetchStationFromBbox(std::string demFile,
                                               std::vector<boost::posix_time::ptime> timeList,
                                               std::string timeZone, bool latest)
{
    GDALDataset  *poDS;
    poDS = (GDALDataset *) GDALOpen(demFile.c_str(), GA_ReadOnly );
    if (poDS==NULL)
    {
        CPLDebug("STATION_FETCH", "Could not read DEM file for station fetching");
        return false;
    }

    double bounds[4];
    bool bRet;

    bRet=GDALGetBounds(poDS,bounds);
    if (bRet==false)
    {
        throw std::runtime_error("GDALGetBounds returned false, DEM file is lacking readable data.");
        return false;

    }

    double buffer;
    buffer=getStationBuffer();
    CPLDebug("STATION_FETCH", "Adding %fm to DEM for station fetching.", buffer);

    double projxL=bounds[2];
    double projyL=bounds[3];
    double projxR=bounds[0];
    double projyR=bounds[1];

    GDALPointFromLatLon(projyL,projxL,poDS,"WGS84"); //LowerLeft
    GDALPointFromLatLon(projyR,projxR,poDS,"WGS84"); //Upper Right
    projxL=projxL-buffer;
    projyL=projyL-buffer;
    projxR=projxR+buffer;
    projyR=projyR+buffer;

    GDALPointToLatLon(projyL,projxL,poDS,"WGS84");
    GDALPointToLatLon(projyR,projxR,poDS,"WGS84");

    bounds[2]=projxL;
    bounds[3]=projyL;
    bounds[0]=projxR;
    bounds[1]=projyR;

    std::string URL;

    if (latest==false)
    {
        vector<std::string>timeUTC;
        timeUTC=UnifyTime(timeList);


        URL=BuildUnifiedBbox(bounds[2],bounds[3],bounds[0],
                bounds[1], timeUTC[0],timeUTC[1],timeUTC[2],
                timeUTC[3],timeUTC[4],timeUTC[5],
                timeUTC[6],timeUTC[7]);
    }
    if (latest==true)
    {
        URL=BuildUnifiedLTBbox(bounds[2],bounds[3],bounds[0],bounds[1]);
    }
    CPLDebug("STATION_FETCH", "WxData URL: %s", URL.c_str());

    fetchStationData(URL, timeZone, latest);

    return true;
}

bool pointInitialization::fetchStationByName(std::string stationList,
                                             std::vector<boost::posix_time::ptime> timeList,
                                             std::string timeZone, bool latest)
{
    std::string URL;

    if (latest==true)
    {
        URL=BuildMultiLatest(stationList);
        CPLDebug("STATION_FETCH", "WxData URL: %s", URL.c_str());
    }
    if (latest==false)
    {
        vector<std::string>timeUTC;
        timeUTC=UnifyTime(timeList);
        URL=BuildMultiUrl(stationList,timeUTC[0],timeUTC[1],timeUTC[2],
                timeUTC[3],timeUTC[4],timeUTC[5],
                timeUTC[6],timeUTC[7]);
        CPLDebug("STATION_FETCH", "WxData URL: %s", URL.c_str());
    }

    fetchStationData(URL, timeZone, latest);

    return true;
}

void pointInitialization::storeFileNames(vector<std::string> statLoc)
{
    stationFiles=statLoc;
}
void pointInitialization::writeStationLocationFile(std::string demFile){
    std::string cName;
    stringstream statLen;
    statLen<<stationFiles.size();
    std::string pathName,rootFile;
    std::string baseName(CPLGetBasename(demFile.c_str()));
    pathName = CPLGetPath(demFile.c_str());
    rootFile = CPLFormFilename(pathName.c_str(), baseName.c_str(), NULL);    
//    cout<<baseName<<endl;
//    cout<<baseName<<endl;
//    cout<<rootFile<<endl;
    cName=rootFile + "_" + "stations_" + statLen.str() + ".csv";
    cout<<cName<<endl;
//    exit(1);
    ofstream outFile;
    outFile.open(cName.c_str());    
    outFile<<"Station_File_List,"<<endl;
    for(int i=0;i<stationFiles.size();i++){
        outFile<<stationFiles[i]<<endl;
    }
}

void pointInitialization::fetchStationData(std::string URL,
                                std::string timeZone, bool latest)
{


    OGRDataSourceH hDS;
    OGRLayerH hLayer;
    OGRFeatureH hFeature;

    hDS=OGROpen(URL.c_str(),0,NULL);
    if (hDS==NULL)
    {
        CPLDebug("STATION_FETCH", "URL: %s", URL.c_str());
        throw std::runtime_error("OGROpen could not read the station file.\nPossibly no stations exist for the given parameters.");
    }

    hLayer=OGR_DS_GetLayer(hDS,0);
    OGR_L_ResetReading(hLayer);

    int idx=0;
    int idx1=0;
    int idx2=0;
    int idx3=0;
    int idx5=0;
    int idx6=0;
    int idx7=0;
    int idx8=0;
    int idx9=0;
    int idx10=0;
    int idx11=0;
    int idx12=0;
    int idx13=0;
    int idx14=0;
    int idx15=0;
    int idx16=0;

    int idxID=0;
    const char* writeID;
    vector<char> testId;

    vector<int> mnetid;

    int idxx1=0;
    int idxx2=0;
    int idxx3=0;
    const double *cloudlow = 0;
    const double *cloudmed = 0;
    const double *cloudhigh = 0;

    const double* rawsWind = 0;
    const double* rawsDir = 0;
    const double* rawsSolrad = 0;
    const double* rawsTemp = 0;
    double rawsLatitude = 0;
    double rawsLongitude = 0;
    const char* rawsStation = 0;
    char** rawsDateTime = 0;

    const double* metarWind = 0;
    const double* metarDir = 0;
    const double* metarTemp = 0;
    double metarLatitude;
    double  metarLongitude;
    const char* metarStation;
    char** metarDateTime;

    int fCount=OGR_L_GetFeatureCount(hLayer,1);
    std::string csvName;

    if (rawStationFilename.substr(rawStationFilename.size()-4,4)==".csv")
    {
        rawStationFilename.erase(rawStationFilename.size()-4,4);
        csvName=rawStationFilename;
        CPLDebug("STATION_FETCH", ".csv exists in stationFilename, Removing...");
    }
    else
    {
        csvName=rawStationFilename;
        CPLDebug("STATION_FETCH", "Path is Good...");
    }

    std::vector<std::string> stationCSVNames;

    for (int ex=0;ex<fCount;ex++)
    {


        hFeature=OGR_L_GetFeature(hLayer,ex);

        idx=OGR_F_GetFieldIndex(hFeature,"mnet_id");
        mnetid.push_back(OGR_F_GetFieldAsInteger(hFeature,idx));

        idxID=OGR_F_GetFieldIndex(hFeature,"STID");
        writeID=(OGR_F_GetFieldAsString(hFeature,idxID));

        boost::posix_time::ptime writeTime =boost::posix_time::second_clock::local_time();
        std::ostringstream timestream;

        boost::posix_time::time_facet *facet = new boost::posix_time::time_facet("%m-%d-%Y_%H%M_");

        timestream.imbue(std::locale(std::locale::classic(), facet));
        std::string tName;
        stringstream idStream;
        stringstream timeStream;
        stringstream ss;
        ss<<ex;
        idStream<<writeID;
        timeStream<<writeTime;



        if(csvName!="blank")
        {
            tName = csvName+idStream.str() + "-" + timeStream.str() + "-" + ss.str() + ".csv";
        }
        else
        {
            tName=idStream.str() + "-" + timeStream.str() + "-" + ss.str() + ".csv";
        }
        ofstream outFile;//writing to csv
        outFile.open(tName.c_str());
        CPLDebug("STATION_FETCH", "%d stations saved to %s", fCount, tName.c_str());
        CPLDebug("STATION_FETCH", "Downloading Data from MesoWest....");
        std::string header="\"Station_Name\",\"Coord_Sys(PROJCS,GEOGCS)\",\"Datum(WGS84,NAD83,NAD27)\",\"Lat/YCoord\",\"Lon/XCoord\",\"Height\",\"Height_Units(meters,feet)\",\"Speed\",\"Speed_Units(mph,kph,mps,kts)\",\"Direction(degrees)\",\"Temperature\",\"Temperature_Units(F,C)\",\"Cloud_Cover(%)\",\"Radius_of_Influence\",\"Radius_of_Influence_Units(miles,feet,meters,km)\",\"date_time\"";
        outFile<<header<<endl;
        stationCSVNames.push_back(tName);
        storeFileNames(stationCSVNames);

        if (mnetid[ex]==1) //METAR station uses cloud data
        {
            int count1=0;
            int count2=0;
            int count3=0;
            int countxx1=0;
            int countxx2=0;
            int countxx3=0;

            idx1=OGR_F_GetFieldIndex(hFeature,"wind_speed");
            metarWind=(OGR_F_GetFieldAsDoubleList(hFeature,idx1,&count1));

            idx2=OGR_F_GetFieldIndex(hFeature,"wind_direction");
            metarDir=(OGR_F_GetFieldAsDoubleList(hFeature,idx2,&count2));

            idx3=OGR_F_GetFieldIndex(hFeature,"air_temp");
            metarTemp=(OGR_F_GetFieldAsDoubleList(hFeature,idx3,&count3));

            idxx1=OGR_F_GetFieldIndex(hFeature,"cloud_layer_1_code");
            cloudlow=OGR_F_GetFieldAsDoubleList(hFeature,idxx1,&countxx1);

            idxx2=OGR_F_GetFieldIndex(hFeature,"cloud_layer_2_code");
            cloudmed=OGR_F_GetFieldAsDoubleList(hFeature,idxx2,&countxx2);

            idxx3=OGR_F_GetFieldIndex(hFeature,"cloud_layer_3_code");
            cloudhigh=OGR_F_GetFieldAsDoubleList(hFeature,idxx3,&countxx3);

            idx5=OGR_F_GetFieldIndex(hFeature,"latitude");
            metarLatitude=(OGR_F_GetFieldAsDouble(hFeature,idx5));

            idx6=OGR_F_GetFieldIndex(hFeature,"LONGITUDE");
            metarLongitude=(OGR_F_GetFieldAsDouble(hFeature,idx6));

            idx7=OGR_F_GetFieldIndex(hFeature,"STID");
            metarStation=(OGR_F_GetFieldAsString(hFeature,idx7));

            idx8=OGR_F_GetFieldIndex(hFeature,"date_times");
            metarDateTime=(OGR_F_GetFieldAsStringList(hFeature,idx8));

            vector<std::string>cloudkappa;
            cloudkappa=UnifyClouds(cloudlow,cloudmed,cloudhigh,countxx1,countxx2,countxx3,count3);
            vector<std::string>metarWindDirection;
            vector<std::string>metarTemperature;
            metarWindDirection=fixWindDir(metarDir,"0",count1);
            metarTemperature=fixWindDir(metarTemp,"-9999",count1);

            if (latest==true)
            {
                count1=1;
                int ez=0;

                 outFile<<metarStation<<",GEOGCS,"<<"WGS84,"<<metarLatitude<<","<<metarLongitude<<",10,"<<"meters,"<<metarWind[ez]<<",mps,"<<metarWindDirection[ez]<<","<<metarTemperature[ez]<<",C,"<<cloudkappa[ez]<<","<<"-1,"<<"km"<<endl;


            }
            else
            {
                for(int ez=0;ez<count1;ez++)
                {
                 outFile<<metarStation<<",GEOGCS,"<<"WGS84,"<<metarLatitude<<","<<metarLongitude<<",10,"<<"meters,"<<metarWind[ez]<<",mps,"<<metarWindDirection[ez]<<","<<metarTemperature[ez]<<",C,"<<cloudkappa[ez]<<","<<"-1,"<<"km,"<<metarDateTime[ez]<<endl;
                }
            }
        }

        if (mnetid[ex]==2) //RAWS STATION, solar radiation
        {
            int count9=0;
            int count10=0;
            int count11=0;
            int count12=0;

            idx9=OGR_F_GetFieldIndex(hFeature,"wind_speed");
            rawsWind=(OGR_F_GetFieldAsDoubleList(hFeature,idx9,&count9));

            idx10=OGR_F_GetFieldIndex(hFeature,"wind_direction");
            rawsDir=(OGR_F_GetFieldAsDoubleList(hFeature,idx10,&count10));

            idx11=OGR_F_GetFieldIndex(hFeature,"air_temp");
            rawsTemp=(OGR_F_GetFieldAsDoubleList(hFeature,idx11,&count11));

            idx12=OGR_F_GetFieldIndex(hFeature,"solar_radiation");
            rawsSolrad=(OGR_F_GetFieldAsDoubleList(hFeature,idx12,&count12));

            idx13=OGR_F_GetFieldIndex(hFeature,"latitude");
            rawsLatitude=(OGR_F_GetFieldAsDouble(hFeature,idx13));

            idx14=OGR_F_GetFieldIndex(hFeature,"LONGITUDE");
            rawsLongitude=(OGR_F_GetFieldAsDouble(hFeature,idx14));

            idx15=OGR_F_GetFieldIndex(hFeature,"STID");
            rawsStation=(OGR_F_GetFieldAsString(hFeature,idx15));

            idx16=OGR_F_GetFieldIndex(hFeature,"date_times");
            rawsDateTime=(OGR_F_GetFieldAsStringList(hFeature,idx16));

            int aZero;
            aZero=0;
            std::string baddata="-9999";
            vector<std::string>rawsWindDirection;
            rawsWindDirection=fixWindDir(rawsDir,"0",count9);
            vector<double> rawsCloudCover;
            rawsCloudCover=Irradiate(rawsSolrad,1,count12,timeZone,rawsLatitude,rawsLatitude,rawsDateTime);

            if (latest==true)
            {
                count9=1;
                int ez=0;

                if (rawsCloudCover.size()==aZero)
                {

                    outFile<<rawsStation<<",GEOGCS,"<<"WGS84,"<<rawsLatitude<<","<<rawsLongitude<<",10,"<<"meters,"<<rawsWind[ez]<<",mps,"<<rawsWindDirection[ez]<<","<<rawsTemp[ez]<<",C,"<<baddata<<","<<"-1,"<<"km"<<endl;
                }
                else
                {
                    outFile<<rawsStation<<",GEOGCS,"<<"WGS84,"<<rawsLatitude<<","<<rawsLongitude<<",10,"<<"meters,"<<rawsWind[ez]<<",mps,"<<rawsWindDirection[ez]<<","<<rawsTemp[ez]<<",C,"<<rawsCloudCover[ez]<<","<<"-1,"<<"km"<<endl;
                }
            }
            else
            {
                for (int ez=0;ez<count9;ez++)
                {
                    if (rawsCloudCover.size()==aZero)
                    {

                        outFile<<rawsStation<<",GEOGCS,"<<"WGS84,"<<rawsLatitude<<","<<rawsLongitude<<",10,"<<"meters,"<<rawsWind[ez]<<",mps,"<<rawsWindDirection[ez]<<","<<rawsTemp[ez]<<",C,"<<baddata<<","<<"-1,"<<"km,"<<rawsDateTime[ez]<<endl;
                    }
                    else
                    {
                        outFile<<rawsStation<<",GEOGCS,"<<"WGS84,"<<rawsLatitude<<","<<rawsLongitude<<",10,"<<"meters,"<<rawsWind[ez]<<",mps,"<<rawsWindDirection[ez]<<","<<rawsTemp[ez]<<",C,"<<rawsCloudCover[ez]<<","<<"-1,"<<"km,"<<rawsDateTime[ez]<<endl;
                    }
                }
            }
        }
        OGR_F_Destroy( hFeature );
    }
    CPLDebug("STATION_FETCH", "Data downloaded and saved....");
    OGR_DS_Destroy(hDS);
    CPLDebug("STATION_FETCH", "fetchStationData finished.");
}
