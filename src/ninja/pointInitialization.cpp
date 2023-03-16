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

std::string pointInitialization::dtoken = "33e3c8ee12dc499c86de1f2076a9e9d4"; //This is the base token for fetching
const std::string pointInitialization::backup_token = "33e3c8ee12dc499c86de1f2076a9e9d4"; //the same token repeated just in case
const std::string pointInitialization::dvar = "wind_speed,wind_direction,air_temp," //variables we want
                                             "solar_radiation,cloud_layer_1_code"; //from the API

const std::string pointInitialization::ndvar = "wind_speed,wind_direction,air_temp," //variables we
                                              "solar_radiation,cloud_layer_1_code," //want for
                                              "cloud_layer_2_code,cloud_layer_3_code";//Airport stations

const std::string pointInitialization::baseUrl = "http://api.mesowest.net/v2/stations/"; //API baseurl
std::string pointInitialization::rawStationFilename = ""; //make the station name blank at first
double pointInitialization::stationBuffer; //Buffer
std::vector<std::string> pointInitialization::stationFiles; //Where the files are stored
std::string pointInitialization::tzAbbrev; // Abbreviation of the time zone
vector<blt::local_date_time> pointInitialization::start_and_stop_times; //Storage for the start and stop time as a local obj
//Stores the start and stop time in local time from getTimeList so that we can name the files properly
bool pointInitialization::enforce_limits = true; //Enfore limitations on the API ->set to false if the user provides a custom key
std::string pointInitialization::error_msg = "An Error Occured, Possibly no Data Exists for request"; //generic error message
//Set to whether or not we enforce the limits of 1 year and buffer range
extern blt::tz_database globalTimeZoneDB;

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
                                            input.stationsScratch[ii].get_cloudCover(), input.stationsScratch[ii].get_temperature(),
                                            input.stationsScratch[ii].get_speed(),
                                            input.stationsScratch[ii].get_height(), albedo_, bowen_, cg_,
                                            anthropogenic_, profile.Roughness, profile.Rough_h, profile.Rough_d);

                        input.dem.get_nearestCellIndex(input.stationsScratch[ii].get_projXord(),
                                                input.stationsScratch[ii].get_projYord(), &i_, &j_);

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
/** @brief
 * Check to see if the station data is within the range of user desired times
 * If not, throw a tantrum...
 */
bool pointInitialization::validateTimeData(vector<vector<preInterpolate> > wxStationData, vector<bpt::ptime> timeList)
{
    vector<bpt::ptime> stationStarts;
    vector<bpt::ptime> stationStops;
    vector<std::string> stationNames; //Just for organization purposes, probably not necessary after initial debugging

    for (int i=0; i<wxStationData.size();i++) //loop over all the stations
    {
        bpt::ptime SD_start;
        bpt::ptime SD_stop;

        SD_start = wxStationData[i][0].datetime;
        SD_stop = wxStationData[i][wxStationData[i].size()-1].datetime;
        stationStarts.push_back(SD_start);
        stationStops.push_back(SD_stop);
        stationNames.push_back(wxStationData[i][0].stationName);
    }
    bpt::ptime start_TL = timeList[0];
    bpt::ptime end_TL = timeList[timeList.size()-1];

    vector<bool> startChecks; //Check all weather stations against timelist, if at least one station has data, we can run a simulation, if no station are available, throw an exception
    vector<bool> endChecks;

    /*
     * BAD cases:
     * a: end time is less than the start time of the data set
     * b: start time is greater than the end time of the data set
     * Both of these will throw exceptions preventing further simulation
     * At least one dataset should be valid to continue simulation (hopefully)
     */

    if (start_TL>end_TL)
    {
        cout<<"EXCEPTION CAUGHT: First time step is further in the future than the last, consider changing bounds!"<<endl;
        return false;
    }

    for(int j=0; j<stationNames.size(); j++)
    {
        if(start_TL>stationStops[j])
        {
            CPLDebug("STATION_FETCH","Time list start time begins later than all data for %i : %s ",j,stationNames[j].c_str());
            startChecks.push_back(false);
        }
        if(end_TL<stationStarts[j])
        {
            CPLDebug("STATION_FETCH","Time list ends before data starts for %i : %s ",j,stationNames[j].c_str());
            endChecks.push_back(false);
        }
    }
    CPLDebug("STATION_FETCH","FAILED STARTS: %lu",startChecks.size());
    CPLDebug("STATION_FETCH","FAILED ENDS: %lu",endChecks.size());
    CPLDebug("STATION_FETCH","NUM STATIONS: %lu",stationNames.size());

    if (startChecks.size() >= stationNames.size() || endChecks.size() >= stationNames.size())
    {
        CPLDebug("STATION_FETCH","WARNING: NO DATA IS VALID WITHIN PROVIDED TIME RANGE!!");
        return false;
    }
    else
    {
        CPLDebug("STATION_FETCH","TIME DATA AND USER BOUNDS ARE AGREEABLE!");
        return true;
    }
}
/**
 * @brief pointInitialization::generatePointDirectory
 * Creates a directory to store the downloaded weather stations
 * @param demFile
 * @param outPath
 * @param latest
 * @return
 */
std::string pointInitialization::generatePointDirectory(string demFile, string outPath, bool latest)
{
    std::string subDem;
    std::string xDem;
    std::string fullPath;
   
//    xDem = demFile.substr(0,demFile.find(".",0));
//    std::size_t found = xDem.find_last_of("/");
//    subDem=xDem.substr(found+1);
    subDem = std::string(CPLGetBasename(demFile.c_str())); //Use cross platform stuff to avoid weird errors

    //NEW WAY
    stringstream timeStream,timeStream2;
    bpt::time_facet *facet = new bpt::time_facet("%Y-%m-%d-%H%M");
    timeStream.imbue(locale(timeStream.getloc(),facet));               
    std::string timeComponent;    
    
    if(latest==true) // if it is a "now" type sim, we name the directory with the current time
    {
        bpt::ptime writeTime =bpt::second_clock::local_time();
        timeStream<<writeTime;
        timeComponent = timeStream.str();
    }
    if (latest==false) //If it is a time series we name the directory with both the start and stop time
    {
        timeStream2.imbue(locale(timeStream2.getloc(),facet));                            
        
        timeStream<<start_and_stop_times[0].local_time(); //Name files with Local Times
        timeStream2<<start_and_stop_times[1].local_time();

//        cout<<start_and_stop_times[0].local_time()<<endl;
//        cout<<start_and_stop_times[1].local_time()<<endl;

        timeComponent = timeStream.str()+"-"+timeStream2.str(); //because its local time, add the time zone

    }
    
    std::string newDirPart = "WXSTATIONS-"+timeComponent+"-"+subDem;
    fullPath = std::string(CPLFormFilename(outPath.c_str(),newDirPart.c_str(),NULL));
    CPLDebug("STATION_FETCH","Generating Directory: %s",fullPath.c_str());    
    VSIMkdir(fullPath.c_str(),0777);
    return fullPath;
}/**
 * @brief pointInitialization::removeBadDirectory
 * FOR CLI runs, remove the directory because the downloader failed to download the stations
 * probably because there are not stations available for the user specified reqs
 * @param badStationPath
 * @return
 */
bool pointInitialization::removeBadDirectory(string badStationPath)
{
    CPLDebug("STATION_FETCH","Cleaning up bad directory...");
    VSIRmdir(badStationPath.c_str());
    return true;
}
/**
 * @brief pointInitialization::writeStationOutFile
 * Writes an interpolated CSV of the weather data to disk
 * Replaces functions found in wxStation.cpp
 * @param stationVect
 * @param basePathName
 * @param demFileName
 */
void pointInitialization::writeStationOutFile(std::vector<wxStation> stationVect,
                                              string basePathName, string demFileName, bool latest)
{
    std::string header="\"Station_Name\",\"Coord_Sys(PROJCS,GEOGCS)\",\"Datum(WGS84,NAD83,NAD27)\",\"Lat/YCoord\",\"Lon/XCoord\",\"Height\",\"Height_Units(meters,feet)\",\"Speed\",\"Speed_Units(mph,kph,mps,kts)\",\"Direction(degrees)\",\"Temperature\",\"Temperature_Units(F,C)\",\"Cloud_Cover(%)\",\"Radius_of_Influence\",\"Radius_of_Influence_Units(miles,feet,meters,km)\",\"date_time\"";
    std::string subDem;
    std::string xDem;
    std::string fullPath;


//    xDem = demFileName.substr(0,demFileName.find(".",0));
//    std::size_t found = xDem.find_last_of("/");
//    subDem=xDem.substr(found+1); //gets just a piece of the DEM

    subDem = std::string(CPLGetBasename(demFileName.c_str())); //Use cross platform stuff to avoid weird errors

    stringstream timeStream,timeStream2;
    bpt::time_facet *facet = new bpt::time_facet("%Y-%m-%d_%H%M");
    timeStream.imbue(locale(timeStream.getloc(),facet));
    std::string timeComponent;

    if(latest==true) // if it is a "now" type sim, we name the directory with the current time
    {
        bpt::ptime writeTime =bpt::second_clock::local_time();
        timeStream<<writeTime;
        timeComponent = timeStream.str();
    }
    if (latest==false) //If it is a time series we name the directory with both the start and stop time
    {
        timeStream2.imbue(locale(timeStream2.getloc(),facet));
        
        timeStream<<start_and_stop_times[0].local_time(); //Name files with Local Times
        timeStream2<<start_and_stop_times[1].local_time();

        timeComponent = timeStream.str()+"-"+timeStream2.str(); //because its local time, add the time zone

    }
    std::string fileComponent = subDem+"_interpolate_"+timeComponent+"-";

    for (int j=0;j<stationVect.size();j++)
    {
        ofstream outFile;
        wxStation curVec = stationVect[j];
        std::ostringstream xs;
        xs<<j;
        std::string invidiualFileComponent = fileComponent + curVec.stationName;
        std::string writePath = std::string(CPLFormFilename(basePathName.c_str(),invidiualFileComponent.c_str(),".csv")); //This is safer than manually stiching the filenames
        CPLDebug("STATION_FETCH","WRITING STEP CSV FILE: %s",writePath.c_str());
//        std::string writePath=fullPath+curVec.stationName+".csv";
        outFile.open(writePath.c_str());
        outFile<<header<<endl;

        for(int udx=0;udx<curVec.heightList.size();udx++)
        {
            bpt::ptime abs_time;
            abs_time = curVec.datetimeList[udx];
            std::string strTime = bpt::to_iso_extended_string(abs_time)+"Z";
            outFile<<curVec.stationName<<","<<"GEOGCS"<<","<<"WGS84"<<","<<curVec.lat<<","<<curVec.lon<<",";
            outFile<<curVec.heightList[udx]<<",meters,"<<curVec.speedList[udx]<<","<<"mps"<<","<<curVec.directionList[udx];
            outFile<<","<<curVec.temperatureList[udx]<<",K,"<<curVec.cloudCoverList[udx]*100<<","<<curVec.influenceRadiusList[udx];
            outFile<<",meters,"<<strTime<<endl;
        }
        outFile.close();
    }
}


/**
 * @brief pointInitialization::openCSVList
 * for CLI runs, if the user provides a CSV pointing to a list of weather stations
 * @param csvPath
 * @return
 */
vector<string> pointInitialization::openCSVList(string csvPath)
{
    vector<string> csvList;
    FILE *wxStationList = VSIFOpen( csvPath.c_str(), "r" ); //If we detect that this is a pointer csv
    std::string wxStationDir = std::string(CPLGetPath(csvPath.c_str()));
    while(1){ //Open it up and check it
        const char* f = CPLReadLine(wxStationList);
        if (f == NULL) //Means its not what we want
            break;
        if(strstr(f,".csv")){ //It is what we want
//            csvList.push_back(wxStationDir+"/"+f); //This works
            csvList.push_back(std::string(CPLFormFilename(wxStationDir.c_str(),f,NULL))); //But this is safer for cross platform

        }       
    }
    VSIFClose(wxStationList);
    return csvList;
}
/**
 * @brief pointInitialization::readWxStations
 * If it is determined that the station data is in the old format
 * it gets passed through here and returned as a wxStation Object
 * @param demFileName
 * @param timeZone
 * @return
 */
vector<wxStation> pointInitialization::readWxStations(string demFileName, string timeZone) //This is how we handle the old format now!
{
    vector<string>wxLoc; //list of station locations
    wxLoc.push_back(rawStationFilename); //add the one file to the list
    storeFileNames(wxLoc); //store the file for later
    
    vector<string>stationLocs; //get the id for each station
    stationLocs=fetchWxStationID();
    
    vector<vector<preInterpolate> > wxVector;  
    vector<vector<preInterpolate> > wxOrganized;
    
    vector<preInterpolate> singleStationData;
    singleStationData = readDiskLine(demFileName, rawStationFilename);    //Read in the station data as uninterpolated data
    wxVector.push_back(singleStationData); //store it in the vector

    for (int i=0;i<wxVector[0].size();i++) //loop over all the data
    {
        vector<preInterpolate> tempVec;
        tempVec.push_back(wxVector[0][i]);
        wxOrganized.push_back(tempVec);
    }

    vector<wxStation> readyToGo;
    readyToGo=interpolateNull(demFileName,wxOrganized,timeZone); //make the data into a weather station without interpolating

    return readyToGo;
}
/**
 * @brief pointInitialization::interpolateFromDisk
 * Determines whether the data has time in it, and needs to be interpolated
 * or whether the data is 1 step and can be passed on
 * @param demFile
 * @param timeList
 * @param timeZone
 * @return
 */
vector<wxStation> pointInitialization::interpolateFromDisk(std::string demFile,
                                                      std::vector<bpt::ptime> timeList,
                                                      std::string timeZone)
{
    vector<preInterpolate> diskData;
    vector<vector<preInterpolate> > wxVector;

    /*
     * Generate a vector of weather data stored in a struct called "preInterpolate"
     * which is like the old wxStation class.
     *
     * Each step is a struct
     * each vector of steps is a station
     * the vector of vectors is all the data
     *
     * this gets turned into
     * each wxStation object is a station, where all data is stored in object specific data arrays
     * the vector of stations is all the data
     */
    //Reads in the data as a vector of vectors of structs
    for (int i=0;i<stationFiles.size();i++)
    {
        vector<preInterpolate> singleStationData;
        singleStationData = readDiskLine(demFile, stationFiles[i]);

//        diskData.insert(diskData.end(),singleStationData.begin(),singleStationData.end());
        wxVector.push_back(singleStationData);
    }
    vector<bpt::ptime> outaTime;
    bpt::ptime noTime;
    outaTime.push_back(noTime);
    vector<vector<preInterpolate> > interpolatedDataSet;
    vector<wxStation> readyToGo;   
       
    if (wxVector[0][0].datetime==noTime) //If its a "WindNinja NOW" style run, new format, 1 step
    {        
        CPLDebug("STATION_FETCH", "noTime");
        readyToGo=interpolateNull(demFile,wxVector,timeZone); //Does a "Fake Interpolation", Converst the 1 step into a wxStation Object, ready to be used in simulation
    }
    else //If its a time series
    {
        //Sanity check, make sure user provided time range is usable with the data the user wants to use
        //Not needed for CLI station-fetch runs, but necessary for everything else
        //If it is a CLI station-fetch run, case should be ideal, as timelist and desired times
        //are identical
        bool stationsCool = validateTimeData(wxVector,timeList);
        if (stationsCool==false)
        {
            //This is bad, kill it with fire!
            //need informative and concise warning meassages.
            error_msg="User Provided start and stop times are both outside datasets time span!";
            throw std::runtime_error("User Provided start and stop times are both outside datasets time span!");
        }
        //does all interpolation 
        CPLDebug("STATION_FETCH","User Provided start & Stop times are good..");
        interpolatedDataSet=interpolateTimeData(demFile,wxVector,timeList); //Creates a number of wxStation Objects
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
/**
 * @brief pointInitialization::readDiskLine
 * Read in the wxStation data from Disk
 * used to be in wxStation.cpp
 * @param demFile
 * @param stationLoc
 * @return
 */
vector<pointInitialization::preInterpolate> pointInitialization::readDiskLine(std::string demFile,
                                                                                 std::string stationLoc)
{
    std::string oErrorString = "";
    preInterpolate oStation;
    std::vector<preInterpolate> oStations;

    OGRDataSourceH hDS;
    hDS = OGROpen( stationLoc.c_str(), FALSE, NULL );

    if( hDS == NULL )
    {
        oErrorString = "Cannot open csv file: ";
        oErrorString += stationLoc;
        error_msg = oErrorString;
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

    CPLDebug("STATION_FETCH", "Reading csvName: %s", stationLoc.c_str());

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
                error_msg = oErrorString;
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
                error_msg = oErrorString;
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
            CPLDebug("STATION_FETCH","PROJCS FOUND!");
            const char *pszDatum = poFeature->GetFieldAsString( 2 );
            oStation.lat=poFeature->GetFieldAsDouble(3); //set the projected coordinates
            oStation.lon=poFeature->GetFieldAsDouble(4);
            oStation.datumType=pszDatum; //Set the datum type
            oStation.coordType=pszKey; //set the coord type
        }
        else
        {
            oErrorString = "Invalid coordinate system: ";
            oErrorString += poFeature->GetFieldAsString( 1 );
            oErrorString += " at station: ";
            oErrorString += oStationName;
            error_msg = oErrorString;
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
            error_msg = oErrorString;
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
            error_msg = oErrorString;
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
            error_msg = oErrorString;
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
            error_msg = oErrorString;
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
            error_msg = oErrorString;
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
            error_msg = oErrorString;
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
            error_msg = oErrorString;
            throw( std::domain_error( oErrorString ) );
        }

        datetime=poFeature->GetFieldAsString(15);
        std::string trunk=datetime.substr(0,datetime.size()-1);

        bpt::ptime abs_time;

        bpt::time_input_facet *fig=new bpt::time_input_facet;
        fig->set_iso_extended_format();
        std::istringstream iss(trunk);
        iss.imbue(std::locale(std::locale::classic(),fig));
        iss>>abs_time;
        oStation.datetime=abs_time;

        //if it's not a "WindNinja NOW" type simulation and we can't detect the datetime
        bpt::ptime noTime;
        if(datetime != "" && oStation.datetime==noTime){
            oErrorString = "Invalid datetime format: ";
            oErrorString += poFeature->GetFieldAsString( 15 );
            oErrorString += " at station: ";
            oErrorString += oStationName;
            error_msg = oErrorString;
            throw( std::domain_error( oErrorString ) );
        }

        oStations.push_back(oStation);
        OGRFeature::DestroyFeature( poFeature );
    }

    OGR_DS_Destroy( hDS );

    return oStations;
}
/**
 * @brief pointInitialization::fetchWxStationID
 * Opens the on disk station file to read the ID for each station.
 * ie (KMSO)
 * @return
 */
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
/**
 * @brief pointInitialization::directTemporalInterpolation
 * Determines if interpolation is necessary for a given timestep
 * it isn't necessary if the timestep borders the available data
 * in which case, choose the closest single point
 * @param posIdx
 * @param negIdx
 * @return
 */
int pointInitialization::directTemporalInterpolation(int posIdx, int negIdx)
{
    if (posIdx>=0 && negIdx>=0)//Interpolation Necessary
    {
        return 0;
    }
    if (posIdx>=0 && negIdx<0) //Use only positive step, no interpolation necessary
    {
        return 1;
    }
    if (posIdx<0 && negIdx>=0) //Use only negative step, no interpolation necessary
    {
        return 2;
    }

    throw std::runtime_error("invalid index combination");
}

/**
 * @brief pointInitialization::checkWxStationSize
 * @param wxStationIDs
 * This is kind of a final sanity check.
 *
 * Quickly checks the file formats to make sure they are all the same
 * they should be good by this point, but this is just in case
 *
 * If its an old format, just carry on because the old format is weird
 *
 * if its a new format, you can't have multiple unique stations in one file
 * so check to see if the number of physical files on disk match the number of
 * unique stations we find in our list, if they are different, it means
 * someone is gaslighting us and we need to stop before we hit a segfault.
 *
 * @return true if its good, throw an error if its not.
 */
bool pointInitialization::checkWxStationSize(vector<string> wxStationIDs)
{
    std::vector<int> formatVec;
    for(int i=0;i<stationFiles.size();i++) //Quickly check to be sure that everything is the same format
    {
        int file_format = wxStation::GetHeaderVersion(stationFiles[i].c_str());
        formatVec.push_back(file_format);
    }
    if(std::equal(formatVec.begin()+1,formatVec.end(),formatVec.begin())!=true)
    {
        error_msg = "ERROR: Stations are not all of the same type!";
        throw std::runtime_error("ERROR: Stations are not all of the same type!");
    }
    if(formatVec[0]==1) //If its an old format, don't do this check and just carry on.
    {
        return true;
    }

    //Check to see how many unique stations there are
    std::sort(wxStationIDs.begin(),wxStationIDs.end());
    int uniqueStations = std::unique(wxStationIDs.begin(), wxStationIDs.end()) - wxStationIDs.begin();

    if(stationFiles.size()!=uniqueStations) //This is very bad, and we can't continue
    {
        error_msg = "ERROR: Mutliple different Stations "
                    "detected in one file! Each unique station must be in a unique file!";
        throw std::runtime_error("ERROR: Mutliple different Stations"
                                 " detected in one file! Each unique station must be in a unique file!");
        return false;
    }

    return true;
}

/**
 * @brief pointInitialization::makeWxStation
 * Converts the nested vectors of station data into a wxStation Object, which stores
 * data as vectors inside the object.
 * @param data
 * @param demFile
 * @return
 */
vector<wxStation> pointInitialization::makeWxStation(vector<vector<preInterpolate> > data, std::string demFile)
{
    CPLDebug("STATION_FETCH", "converting Interpolated struct to wxStation...");
    vector<std::string> stationNames;
    vector<wxStation> stationData;

    stationNames=fetchWxStationID(); //Get the weather station names
    bool final_sanity_check = checkWxStationSize(stationNames); //Check to make sure everything is good

    if(final_sanity_check==false)
    {
        error_msg = "FAILED Size check, check data on disk!";
        throw std::runtime_error("FAILED Size check, check data on disk!");
    }

    int statCount;
    statCount=stationNames.size();

    int specCount=statCount;

    vector<int> idxCount;
    int j=0;
    int q=0;

    /*
     * This sorts the stations based on station name
     * to organize them into unique wxStations
     */

    for (int i=0;i<statCount;i++) //loop over the stations
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

    for (int ei=1;ei<=idxCount.size();ei++) //This appears to do nothing
    {
        int rounder=idxCount.size()-ei;
        int e=std::accumulate(idxCount.begin(),idxCount.end()-rounder,0);
        countLimiter.push_back(e);
    }

    vector<vector<preInterpolate> >stationDataList;
    stationDataList=data;
    //here is where a wxstation is made
    for (int i=0;i<idxCount.size();i++) //loop over the stations
    {
        wxStation subDat;
        subDat.set_stationName(stationDataList[i][0].stationName); //Set the name for each station

        std::string CoordSys=stationDataList[i][0].coordType; //get the coordinate system
        std::string Datum = stationDataList[i][0].datumType; //get the datum type

        if (EQUAL( CoordSys.c_str(), "projcs" ))
        {
            //This has not been tested, I have no idea if this works or not
            CPLDebug("STATION_FETCH","USING PROJCS!");
            subDat.set_location_projected(stationDataList[i][0].lon,stationDataList[i][0].lat,demFile);
//            subDat.set_location_projected(stationDataList[i][0].lat,stationDataList[i][0].lon,demFile);
        }
        else //GEOGCS!
        {
            CPLDebug("STATION_FETCH","USING GEOGCS!");
            subDat.set_location_LatLong(stationDataList[i][0].lat,stationDataList[i][0].lon,
                    demFile,Datum.c_str());
        }

        for (int k=0;k<stationDataList[i].size();k++) //set the weather values for each time step and station
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
/**
 * @brief pointInitialization::interpolateNull
 * If the run is not a timeseries, pass the data thorugh here, and assign it a dummy time
 * with the specified time zone
 * @param demFileName
 * @param vecStations
 * @param timeZone
 * @return
 */
vector<wxStation> pointInitialization::interpolateNull(std::string demFileName,
                                                    vector<vector<preInterpolate> > vecStations,
                                                    std::string timeZone)
{
    CPLDebug("STATION_FETCH", "no interpolation needed");

    vecStations[0][0].cloudCoverUnits=coverUnits::percent;

    vector<wxStation> refinedDat;
    refinedDat=makeWxStation(vecStations,demFileName);
    //fixes time!
    blt::time_zone_ptr timeZonePtr;
    timeZonePtr = globalTimeZoneDB.time_zone_from_region(timeZone);
    bpt::ptime standard = bpt::second_clock::universal_time();
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
    vector<vector<pointInitialization::preInterpolate> > sts,
    std::vector<bpt::ptime> timeList)
{
	
    CPLDebug("STATION_FETCH", "Interpolating Weather Data...");

    vector<vector<preInterpolate> > interpolatedWxData;

    for (int k = 0; k < sts.size(); k++)
    {
        vector<preInterpolate> subInter;

        //Data may be unordered so we search for the first (minIdx) and last (maxIdx) measurements in time
        //----------------------------------------------------
        int minIdx = 0;
        int maxIdx = 0;
        for (int mm = 0; mm < sts[k].size(); mm++)
        {
            if (sts[k][mm].datetime < sts[k][minIdx].datetime) { minIdx = mm; }
            if (sts[k][mm].datetime > sts[k][maxIdx].datetime) { maxIdx = mm; }
        }


        for (int i = 0; i < timeList.size(); i++)
        {
            bpt::time_duration delta0 = bpt::time_duration(bpt::neg_infin);
            bpt::time_duration delta1 = bpt::time_duration(bpt::pos_infin);
            bpt::time_duration zero(0, 0, 0, 0);

            //  Find closest measurement to timeList[i] in the past (idx0) and future (idx1) . If there are none assign the first or last of the existing data respectively.
            //-----------------------------------------------------------------------
            int idx0 = minIdx;
            int idx1 = maxIdx;
            for (int mm = 0; mm < sts[k].size(); mm++)
            {
                bpt::time_duration delta = sts[k][mm].datetime - timeList[i];
                if (delta >= zero && delta <= delta1) { idx1 = mm; delta1 = delta; }
                if (delta <= zero && delta >= delta0) { idx0 = mm; delta0 = delta; }
            }
          
            //Interpolation weight w0 (if timeList[i] is outside range idx0=idx1 and w0=1)
            //------------------------
            double t0 = unixTime(sts[k][idx0].datetime);
            double t1 = unixTime(sts[k][idx1].datetime);
            double w0 = 1;
            if (t1 > t0) { w0 = (t1 - unixTime(timeList[i])) / (t1 - t0); }

            //Interpolate
            //---------------------
            double speed = w0 * sts[k][idx0].speed + (1 - w0) * sts[k][idx1].speed ;
            double temperature = w0 * sts[k][idx0].temperature + (1 - w0) * sts[k][idx1].temperature;
            double cloudCover = w0 * sts[k][idx0].cloudCover + (1 - w0) * sts[k][idx1].cloudCover;
            double xx = w0 * sin(sts[k][idx0].direction * PI / 180.0) + (1 - w0) * sin(sts[k][idx1].direction * PI / 180.0) ;
            double yy = w0 * cos(sts[k][idx0].direction * PI / 180.0) + (1 - w0) * cos(sts[k][idx1].direction * PI / 180.0);
            double angle = atan2(xx, yy) * 180.0 / PI;
            if (angle < 0.0) { angle += 360.0; }


           // Create interpolated data
           //----------------------------
            preInterpolate interpol;
            interpol.datetime = timeList[i];
            interpol.speed = speed;
            interpol.temperature = temperature;
            interpol.cloudCover = cloudCover;
            interpol.direction = angle;
			
            interpol.lat = sts[k][0].lat;
            interpol.lon = sts[k][0].lon;
            interpol.datumType = sts[k][0].datumType;
            interpol.coordType = sts[k][0].coordType;
            interpol.height = sts[k][0].height;
            interpol.heightUnits = sts[k][0].heightUnits;
            interpol.influenceRadius = sts[k][0].influenceRadius;
            interpol.influenceRadiusUnits = sts[k][0].influenceRadiusUnits;
            interpol.stationName = sts[k][0].stationName;
            interpol.inputSpeedUnits = sts[k][0].inputSpeedUnits;
            interpol.tempUnits = sts[k][0].tempUnits;
            interpol.cloudCoverUnits = sts[k][0].cloudCoverUnits;

            subInter.push_back(interpol);
        }

        interpolatedWxData.push_back(subInter);
    }
    return interpolatedWxData;
}
/**
 * @brief pointInitialization::unixTime
 * Converts a ptime object to the number of seconds since January 1st 1970
 * @param time
 * @return
 */
double pointInitialization::unixTime(bpt::ptime time)
{
    bpt::ptime epoch(bg::date(1970,1,1));
    bpt::time_duration::sec_type  dNew= (time - epoch).total_seconds();
    double stepDuration;
    stepDuration = dNew;

    return stepDuration;
}
/**
 * @brief pointInitialization::interpolator
 * Linear Interpolation of station data to fit timeseries
 * Uses seconds since epoch for time rather than doing clock arithmetic
 * @param iPoint - The interpolation time (seconds since epoch)
 * @param lowX - the time before iPoint (unixTime)
 * @param highX - the time after iPoint (unixTime)
 * @param lowY - data point for lowX
 * @param highY - data point for highX
 * @return data point for for timestep
 */
double pointInitialization::interpolator(double iPoint, double lowX, double highX, double lowY, double highY)
{
    double work = 0.0;
    double slope = (highY - lowY) / (highX - lowX);
    double pointS = (iPoint - lowX);
    double result = lowY + pointS * slope;

    //MSVC 2010 is not c++11 compliant-> isnan doesn't work with MSVC2010
    //changing to CPLISNan()

    if(CPLIsNan(result))
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
    //Changing isnan() to CPLIsNan() for MSVC2010 compliance
    if (CPLIsNan(degAverage))
    {
        degAverage=0.0; //Sometimes the interpolation fails, temporary fix here!
        CPLDebug("STATION_FETCH","Direction Interpolation Failed! Zeroing out bad point!");
    }

    return degAverage;
}
/**
 * @brief pointInitialization::BuildTime
 * Combines individual time component strings into one string
 * @param year_0
 * @param month_0
 * @param day_0
 * @param clock_0
 * @param year_1
 * @param month_1
 * @param day_1
 * @param clock_1
 * @return
 */
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

/**
 * @brief pointInitialization::UnifyTime
 * Builds a unified timelist as a vector of strings for use in interpolation and fetching stations
 *
 * this function gets the start and stop time as a vector of strings so that we can fetch a time series
 * from the time list
 * @param timeList
 * @return
 */
vector<std::string> pointInitialization::UnifyTime(vector<bpt::ptime> timeList)
{
    vector<std::string> buildTimes;
    stringstream startstream;
    stringstream endstream;
    bpt::time_facet *facet = new bpt::time_facet("%Y%m%d%H%M");
    bpt::time_duration buffer(1, 0, 0, 0); //add a 1 hour pad to each side

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

/**
 * @brief pointInitialization::fetchMetaData
 * Fetches the Metadata from the DEM, reporting, in a csv,
 * the latitude, longitude, whether or not the station is active,
 * the elevation and its Mesonet ID
 * @param fileName
 * @param demFile
 * @param write
 */
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
        error_msg="Bad metadata in the downloaded file!";
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
/**
 * @brief pointInitialization::BuildMultiUrl
 * Constructs a multiple station API query for a timeseries
 * @param station_ids
 * @param yearx
 * @param monthx
 * @param dayx
 * @param clockx
 * @param yeary
 * @param monthy
 * @param dayy
 * @param clocky
 * @return
 */
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
/**
 * @brief pointInitialization::BuildMultiLatest
 * Constructs a URL for a station name API query for 1 timestep
 * @param station_ids
 * @return
 */
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
/**
 * @brief pointInitialization::BuildBboxUrl
 * Composes the latitude and time components of a timeseries DEM based URL
 * @param lat1
 * @param lon1
 * @param lat2
 * @param lon2
 * @param yearx
 * @param monthx
 * @param dayx
 * @param clockx
 * @param yeary
 * @param monthy
 * @param dayy
 * @param clocky
 * @return
 */
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
/**
 * @brief pointInitialization::BuildBboxLatest
 * Composes the latitude and metadata parts of a bbox latest (1 time step)
 * @param lat1
 * @param lon1
 * @param lat2
 * @param lon2
 * @return
 */

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
/**
 * @brief pointInitialization::BuildUnifiedBbox
 * Constructs a URL for a DEM based timeseries API request
 * @param lat1
 * @param lon1
 * @param lat2
 * @param lon2
 * @param yearx
 * @param monthx
 * @param dayx
 * @param clockx
 * @param yeary
 * @param monthy
 * @param dayy
 * @param clocky
 * @return
 */

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
/**
 * @brief pointInitialization::BuildUnifiedLTBbox
 * Constructs a url for a 1 timestep bounding box API request
 * @param lat1
 * @param lon1
 * @param lat2
 * @param lon2
 * @return
 */
std::string pointInitialization::BuildUnifiedLTBbox(double lat1, double lon1, double lat2, double lon2)
{
    std::string URLat = CPLSPrintf("%.6f", lat2);
    std::string URLon = CPLSPrintf("%.6f", lon2);
    std::string LLLat = CPLSPrintf("%.6f", lat1);
    std::string LLLon = CPLSPrintf("%.6f", lon1);

    std::string URL = BuildBboxLatest(LLLat, LLLon, URLat, URLon);

    return URL;
}
/**
 * @brief pointInitialization::setStationBuffer
 * Sets the distance outside the dem that station data can be fetched
 * @param buffer
 * @param units
 */

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

    CPLDebug("STATION_FETCH","Download Buffer Set to %f",buffer);
    stationBuffer = buffer;
}
/**
 * @brief pointInitialization::getStationBuffer
 * Returns the user set spatial buffer, the maxiumum distance that stations can be fetched
 * outside a dem
 * @return
 */
double pointInitialization::getStationBuffer()
{
        return stationBuffer;
}

vector<std::string> pointInitialization::Split(char* str,const char* delim)
{
    //Splits strings into vectors of strings based on a delimiter, a "," is used for most functions
    char* saveptr;
    //strtok_r is for unix systems only, chang to strtok for cross platform use...
//    char* token = strtok_r(str, delim, &saveptr);
    char* token = strtok(str, delim);

    vector<std::string> result;

    while(token != NULL)
    {
        result.push_back(token);
//        token = strtok_r(NULL, delim, &saveptr);
        token = strtok(NULL, delim);
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
/**
 * @brief pointInitialization::CompareClouds
 * Compares the 3 levels of clouds (low, high, medium) to eachother
 * to determine the total cloud cover reported by a ASOS/Airport Station
 * Sometimes the reported weather data from airport stations is missing a significant
 * portion of data, in which case we set the low clouds to equal the medium clouds.
 * @param low
 * @param med
 * @param high
 * @param countlow
 * @param countmed
 * @param counthigh
 * @return
 */
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

    if (low!=med)
    { /*This is some sort of Bug with the METAR/ASOS stations, the low clouds are always missing 40 data points, and it is only occuring today
        (2/17), earlier times like in january do not have this problem.
        */
        CPLDebug("STATION_FETCH", "That Weird METAR BUG IS HAPPENING AGAIN!, quick fix now...");
        low = med;
    }

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
/**
 * @brief pointInitialization::UnifyClouds
 * ASOS stations provide clouds in the form of octas. This helps clean up these octas
 * into percentages.
 * @param dvCloud
 * @param dwCloud
 * @param dxCloud
 * @param count1
 * @param count2
 * @param count3
 * @param backupcount
 * @return
 */
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

/**
 * @brief pointInitialization::Irradiate
 * Converts solar radiation data to cloud cover based on time, and user location
 *  using solar.getSolarIntensity
 * from solar.cpp
 * Solar Radiation comes from RAWS stations
 * @param solrad
 * @param smallcount
 * @param largecount
 * @param timeZone
 * @param lat
 * @param lon
 * @param times
 * @return
 */
vector<double> pointInitialization::Irradiate(vector<string> solar_radiation, string timeZone, double lat, double lon, char **times)
{
    vector<double> outCloud;

    for (int j=0;j<solar_radiation.size();j++)
    {
        char *trunk=times[j];
        Solar sol;
        bool solar_opt;

        bpt::ptime abs_time;

        bpt::time_input_facet *fig=new bpt::time_input_facet;
        fig->set_iso_extended_format();
        std::istringstream iss(trunk);
        iss.imbue(std::locale(std::locale::classic(),fig));
        iss>>abs_time;

        blt::time_zone_ptr timeZonePtr;
        timeZonePtr = globalTimeZoneDB.time_zone_from_region(timeZone);

        blt::local_date_time startLocal(abs_time,timeZonePtr);

        double zero=0.000000;
        double one=1.0000000;


        solar_opt=sol.compute_solar(startLocal,lat,lon,zero,zero);

        double solar_intensity=sol.get_solarIntensity();
        double solFrac;

        double solrad = atof(solar_radiation[j].c_str());

        solFrac=solrad/solar_intensity;
        if (solFrac<=zero)
        {
            solFrac=one;
        }
        if (solFrac>one)
        {
            solFrac=one;
        }
        //Note that CPLIsNan is required to compile on MSVC2010 c++11's isnan doesn't work
        if (CPLIsNan(solFrac))
        {
            solFrac=one;
        }
        solFrac=one-solFrac;
        solFrac=100*solFrac;
        outCloud.push_back(solFrac);
    }
    return outCloud;
}

/**
 * @brief pointInitialization::fixWindDir
 * downloaded wind direction data from the API is stored as a nonuniform C array of const doubles,
 * this converts it a string that looks better and functions properly
 * @param winddir
 * @param filler
 * @param count
 * @return
 */
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

/**
 * @brief Builds the time list for a pointInitialization run.
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
 * notice that the input simulation time is in local time, the output timeList/datetimes is in utc time.
 */
std::vector<bpt::ptime>
pointInitialization::getTimeList(int startYear, int startMonth, int startDay,
                                 int startHour, int startMinute, int endYear,
                                 int endMonth, int endDay, int endHour, int endMinute,
                                 int nTimeSteps, std::string timeZone)
{
    blt::time_zone_ptr timeZonePtr; // Initialize time zone
    timeZonePtr = globalTimeZoneDB.time_zone_from_region(timeZone); // Get time zone from database
    
    bg::date start_date(startYear,startMonth,startDay); // Generate date object from input time for start time
    bg::date end_date(endYear,endMonth,endDay);         // Generate date object from input time for end time
    bpt::time_duration start_duration(startHour,startMinute,0,0);   // Generate time past the date object from input time for start time
    bpt::time_duration end_duration(endHour,endMinute,0,0);         // Generate time past the date object from input time for end time
    
    // use the time zone pointer to setup the start and end full local_date_time objects
    blt::local_date_time start_local = blt::local_date_time(start_date,start_duration,timeZonePtr,blt::local_date_time::NOT_DATE_TIME_ON_ERROR);
    blt::local_date_time end_local = blt::local_date_time(end_date,end_duration,timeZonePtr,blt::local_date_time::NOT_DATE_TIME_ON_ERROR);
    
    // calculate and output the dst information, super useful for debugging, though may not always match what is expected
    bpt::ptime start_ptime(start_date,start_duration); // Create a ptime for the start date object and start time duration, will be in the same time zone as the input time (in this case, local time)
    bpt::ptime start_dstStartTransition = timeZonePtr->dst_local_start_time(start_date.year()); // Get when DST starts from TZ for start date. Becomes "not-a-date-time" if no DST exists for the TZ
    bpt::ptime start_dstEndTransition = timeZonePtr->dst_local_end_time(start_date.year()); // Get when DST ends from TZ for start date
    CPLDebug("STATION_FETCH","start_ptime: %s",boost::posix_time::to_simple_string(start_ptime).c_str());
    CPLDebug("STATION_FETCH","start_dstStartTransition: %s",boost::posix_time::to_simple_string(start_dstStartTransition).c_str());
    CPLDebug("STATION_FETCH","start_dstEndTransition: %s",boost::posix_time::to_simple_string(start_dstEndTransition).c_str());
    bpt::ptime end_ptime(end_date,end_duration); // Create a ptime for the end date object and end time duration, will be in the same time zone as the input time (in this case, local time)
    bpt::ptime end_dstStartTransition = timeZonePtr->dst_local_start_time(end_date.year()); // Get when DST starts from TZ for end date. Becomes "not-a-date-time" if no DST exists for the TZ
    bpt::ptime end_dstEndTransition = timeZonePtr->dst_local_end_time(end_date.year()); // Get when DST ends from TZ for end date
    CPLDebug("STATION_FETCH","end_ptime: %s",boost::posix_time::to_simple_string(end_ptime).c_str());
    CPLDebug("STATION_FETCH","end_dstStartTransition: %s",boost::posix_time::to_simple_string(end_dstStartTransition).c_str());
    CPLDebug("STATION_FETCH","end_dstEndTransition: %s",boost::posix_time::to_simple_string(end_dstEndTransition).c_str());
    
    // determine if isDST to determine which timezone abbreviation to store
    // only use the start time for these dst time zone comparisons, treat the rest of the times as if they are in the same timezone as the start time
    bool isDST = start_local.is_dst();
    if ( isDST == true )
    {
        CPLDebug("STATION_FETCH", "Time is within DST");
        storeTZAbbrev(timeZonePtr->dst_zone_abbrev());
    } else
    {
        CPLDebug("STATION_FETCH", "Time is outside DST");
        storeTZAbbrev(timeZonePtr->std_zone_abbrev());
    }
    
    // now convert the found local date times (which are now corrected properly for dst) to utc time for output
    bpt::ptime start_UTC = start_local.utc_time();
    bpt::ptime end_UTC = end_local.utc_time();
    
    
    // now warn if the start and stop time cross a daylight savings time transition where time doesn't always behave as expected
    if ( start_dstStartTransition < start_dstEndTransition )
    {
        // normal situation
        // example is PST for Jan to Mar, PDT for Mar to Nov, PST for Nov to Dec
        if ( (start_ptime < start_dstStartTransition && end_ptime > start_dstStartTransition)
          || (start_ptime < start_dstEndTransition && end_ptime > start_dstEndTransition) )
        {
            std::cout << "\nSTATION_FETCH warning: Chosen start and stop times span a daylight savings time transition.\n" << std::endl;
        }
    } else if (start_dstStartTransition > start_dstEndTransition ) // notice that if they are equal is ignored
    {
        // reversed order situation
        // example is NZDT for Jan to Mar, NZST for Mar to Oct, NZDT for Oct to Dec
        if ( (start_ptime < start_dstEndTransition && end_ptime > start_dstEndTransition)
          || (start_ptime < start_dstStartTransition && end_ptime > start_dstStartTransition) )
        {
            std::cout << "\nSTATION_FETCH warning: Chosen start and stop times span a daylight savings time transition.\n" << std::endl;
        }
    }
    
    
    //// do debug output
    CPLDebug("STATION_FETCH","start_local: %s",start_local.to_string().c_str());
    CPLDebug("STATION_FETCH","end_local: %s",end_local.to_string().c_str());
    CPLDebug("STATION_FETCH","tzAbbrev: %s",tzAbbrev.c_str());
    CPLDebug("STATION_FETCH","start_UTC: %s",boost::posix_time::to_simple_string(start_UTC).c_str());
    CPLDebug("STATION_FETCH","end_UTC: %s",boost::posix_time::to_simple_string(end_UTC).c_str());
    
    
    // Sets these for use in the fetch-station functions
    setLocalStartAndStopTimes(start_local,end_local);
    
    
    
    // Get Total Time duration of simulation and divide it into time steps
    bpt::time_duration diffTime = end_UTC - start_UTC;
    bpt::time_duration stepTime;
    if ( nTimeSteps > 1 )
    {
        stepTime = diffTime/(nTimeSteps-1);
    } else
    {
        stepTime = diffTime/nTimeSteps;
    }
    
    std::vector<bpt::ptime> timeOut;
    std::vector<bpt::ptime> timeConstruct;
    std::vector<bpt::ptime> timeList;
    std::vector<bpt::time_duration> timeStorage;
    
    // Create Time Steps by multiplying steps by durations
    // Sets first step to be start time
    // Sets last step to be stop time
    if (nTimeSteps > 1)
    {
        //If there is only one timestep, just use start_UTC
        timeOut.push_back(start_UTC);
        for (int i = 1; i < nTimeSteps-1; i++) //Subtract one to account for indexing beginning early && appending stop/start times
        {
            bpt::time_duration specTime;
            specTime = stepTime*i;
            timeOut.push_back(start_UTC+specTime);
        }
        timeOut.push_back(end_UTC);
    } else
    {
        //if it's a single timestep, run the midpoint of start/end
        timeOut.push_back(start_UTC+diffTime/2);
    }
    timeList = timeOut;
    
    return timeList;
}
/**
 * @brief pointInitialization::generateSingleTimeObject
 * Builds a single ptime object used in the gui and other places
 * for a single step run!
 *
 * @param year
 * @param month
 * @param day
 * @param hour
 * @param minute
 * @param timeZone
 * @return Single datetime in UTC.
 * notice that the input simulation time is in local time, the output time is in utc time.
 */
bpt::ptime pointInitialization::generateSingleTimeObject(int year, int month, int day,
                                                         int hour, int minute,
                                                         string timeZone)
{
    blt::time_zone_ptr timeZonePtr; // Initialize time zone
    timeZonePtr = globalTimeZoneDB.time_zone_from_region(timeZone); // Get time zone from database
    
    bg::date x_date(year,month,day); // Generate date object from input time
    bpt::time_duration x_duration(hour,minute,0,0);  // Generate time past the date object from input time
    
    // use the time zone pointer to setup the full local_date_time object
    blt::local_date_time x_local = blt::local_date_time(x_date,x_duration,timeZonePtr,blt::local_date_time::NOT_DATE_TIME_ON_ERROR);
    
    //// calculate and output the dst information, super useful for debugging, though may not always match what is expected
    bpt::ptime x_ptime(x_date,x_duration); // Create a ptime for the date object and time duration, will be in the same time zone as the input time (in this case, local time)
    bpt::ptime x_dstStartTransition = timeZonePtr->dst_local_start_time(x_date.year()); // Get when DST starts from TZ. Becomes "not-a-date-time" if no DST exists for the TZ
    bpt::ptime x_dstEndTransition = timeZonePtr->dst_local_end_time(x_date.year()); // Get when DST ends from TZ
    CPLDebug("STATION_FETCH","x_ptime: %s",boost::posix_time::to_simple_string(x_ptime).c_str());
    CPLDebug("STATION_FETCH","x_dstStartTransition: %s",boost::posix_time::to_simple_string(x_dstStartTransition).c_str());
    CPLDebug("STATION_FETCH","x_dstEndTransition: %s",boost::posix_time::to_simple_string(x_dstEndTransition).c_str());
    
    // determine if isDST to determine which timezone abbreviation to store
    bool isDST = x_local.is_dst();
    if ( isDST == true )
    {
        CPLDebug("STATION_FETCH", "Time is within DST");
        storeTZAbbrev(timeZonePtr->dst_zone_abbrev());
    } else
    {
        CPLDebug("STATION_FETCH", "Time is outside DST");
        storeTZAbbrev(timeZonePtr->std_zone_abbrev());
    }
    
    // now convert the found local date time (which is now corrected properly for dst) to utc time for output
    bpt::ptime x_UTC = x_local.utc_time();
    
    // now warn if the time becomes not_a_date_time, so far the only cases for this have been at the one hour 
    // of daylight savings time transition when both DST and ST exist.
    boost::posix_time::ptime noTime;    // default constructor should fill it with not_a_date_time as the value
    if ( x_UTC == noTime )
    {
        std::cout << "\nSTATION_FETCH warning: Chosen time is \"not_a_date_time\". This usually happens if the time is right on the daylight savings time transition.\n" << std::endl;
    }
    
    //// do debug output
    CPLDebug("STATION_FETCH","x_local: %s",x_local.to_string().c_str());
    CPLDebug("STATION_FETCH","tzAbbrev: %s",tzAbbrev.c_str());
    CPLDebug("STATION_FETCH","x_UTC: %s",boost::posix_time::to_simple_string(x_UTC).c_str());
    
    return x_UTC;
}
/**
 * @brief pointInitialization::checkFetchTimeDuration
 *
 * Checks the requested download times to see if they are within the accepted limit
 *
 *
 *
 * @param timeList
 * @return
 */
int pointInitialization::checkFetchTimeDuration(std::vector<bpt::ptime> timeList)
{
    if(enforce_limits==true)
    {
        bpt::time_duration diffTime = timeList.back() - timeList.front();

        int tSec = diffTime.total_seconds();
        int max_download_time = 31556926; //One year in seconds

        if(tSec>=max_download_time)//Sanity Check on requested Time Download
        {
            return -2;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}
/**
 * @brief pointInitialization::setCustomAPIKey
 * sets a user specified token to be used in the fetching
 * of station data. See:
 * https://synopticlabs.org/api/mesonet/
 * for information on how to get a key
 * this also removes limits place on the buffer size
 * and number of hours downloadable.
 *
 * @param api_token
 */
void pointInitialization::setCustomAPIKey(string api_token)
{
    if(api_token=="FALSE")
    {
        return;
    }
    else
    {
        CPLDebug("STATION_FETCH","USING API KEY: %s",api_token.c_str());
        if(api_token==backup_token) //means that it is the same as the one that is hard coded in for some reason....
        {
            CPLDebug("STATION_FETCH","PROVIDED TOKEN IS THE SAME AS HARD CODED, IGNORING...");
            enforce_limits=true;
            return;
        }
        if(api_token!=backup_token) //Means they are providing a unique key for us to use
        {
            CPLDebug("STATION_FETCH","AMMENDING TOKEN, REMOVING LIMITS");
            dtoken=api_token;
            enforce_limits=false; //Turn off limits for their key and let them suffer the consequences
        }

    }
}

/**
 * @brief Fetches station data from bounding box.
 * @param demFile Filename/path to the DEM on disk.
 * @param timeList Vector of datetimes in UTC for the simulation.
 */
bool pointInitialization::fetchStationFromBbox(std::string demFile,
                                               std::vector<bpt::ptime> timeList,
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
        error_msg="GDALGetBounds returned false, DEM file is lacking readable data.";
        throw std::runtime_error("GDALGetBounds returned false, DEM file is lacking readable data.");
        return false;

    }

    double buffer;
    buffer=getStationBuffer();

    if(enforce_limits==true) //Generally this is true, unless they provide their own key...
    {
        if(buffer>170000.0)//The Buffer is too big! (170000.0m is ~105 Miles, so its a little bigger)
        {//Sanity Check on Buffer Input
            //Greater than 100 miles
            error_msg="Selected Buffer around DEM is too big! Greater than 100 miles. Use a custom API token to enable larger buffers.";
            throw std::runtime_error("Selected Buffer around DEM is too big! Greater than 100 miles. Use a custom API token to enable larger buffers.");
            return false;
        }
    }


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

    bool fetchGood = fetchStationData(URL, timeZone, latest);

    return fetchGood;
}
/**
 * @brief pointInitialization::fetchStationByName
 * Constructs A URL to query the mesonet API with station names
 * works for both 1 step and time series
 * @param stationList
 * @param timeList
 * @param timeZone
 * @param latest
 * @return
 */

bool pointInitialization::fetchStationByName(std::string stationList,
                                             std::vector<bpt::ptime> timeList,
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

    bool fetchGood = fetchStationData(URL, timeZone, latest);

    return fetchGood;
}
/**
 * @brief pointInitialization::storeFileNames
 * Stores the genrated file names for uses elsewhere in the code
 * These file names are then called when the station data is interpolated
 * after it is written to disk.
 * @param statLoc
 */
void pointInitialization::storeFileNames(vector<std::string> statLoc)
{
    stationFiles=statLoc;
}
/**
 * @brief pointInitialization::storeTZAbbrev
 * Stores the time zone as an abbreviation for naming the files
 * @param tzAbbr
 */
void pointInitialization::storeTZAbbrev(string tzAbbr)
{
    tzAbbrev = tzAbbr;
}
/**
 * @brief pointInitialization::setLocalStartAndStopTimes
 * Sets the start and stop times the static vector
 * indecies are as follows:
 * 0 is start
 * 1 is stop
 * @param start
 * @param stop
 */
void pointInitialization::setLocalStartAndStopTimes(blt::local_date_time start, boost::local_time::local_date_time stop)
{
    start_and_stop_times.push_back(start);
    start_and_stop_times.push_back(stop);
}
/**
 * @brief pointInitialization::writeStationLocationFile
 * writes a csv that points to the weather station csvs
 * this is needed for CLI runs
 * note the comma after the header
 * @param stationPath
 * @param demFile
 * @param current_data
 */
void pointInitialization::writeStationLocationFile(string stationPath, std::string demFile, bool current_data){
    std::string cName;
    stringstream statLen;
    statLen<<stationFiles.size();
    std::string pathName,rootFile;
    std::string baseName(CPLGetBasename(demFile.c_str()));
    pathName = CPLGetPath(demFile.c_str());
    rootFile = CPLFormFilename(pathName.c_str(), baseName.c_str(), NULL);    

//    cName=stationPath+baseName+ "_" + "stations_" + statLen.str() + ".csv";
    std::string nameComponent = baseName+ "_" + "stations_" + statLen.str() + ".csv";
    cName = std::string(CPLFormFilename(stationPath.c_str(),nameComponent.c_str(),".csv"));

    ofstream outFile;
    outFile.open(cName.c_str());
    if(current_data==true)
    {
        outFile<<"Recent_Station_File_List,"<<endl; //note the "," very important
    }
    if(current_data==false)
    {
        outFile<<"Station_File_List,"<<endl;
    }
//    outFile<<"Station_File_List,"<<endl;
    for(int i=0;i<stationFiles.size();i++){
//        outFile<<stationFiles[i]<<endl;
        outFile<<CPLGetFilename(stationFiles[i].c_str())<<endl;
    }
}
/**
 * @brief pointInitialization::parseStationHeight
 * @param name_list
 * uses the name of a station to aid in determining the height of the wind sensor
 * IRAWS stations are at 6ft AGL
 * Permanent RAWS are at 20ft AGL
 *
 * I think all IRAWS stations are named IRAWS
 *
 * @return
 */
double pointInitialization::parseStationHeight(const char* name_list)
{
    double perma_raws = 6.0959; //meters (20 ft above ground for standard raws station)
    double incident_raws = 1.8288; //meters (6 feet above ground for IRAWS station)
    std::string iraws = "IRAWS";

    std::stringstream name_ss;
    name_ss<<name_list;
    std::string nam = name_ss.str();
    std::size_t iraws_found = nam.find(iraws);
    if(iraws_found!=std::string::npos)
    {
        CPLDebug("STATION_FETCH","STATION IS IRAWS, changing height...");
        return incident_raws;
    }
    else
    {
        return perma_raws;
    }
}



/**
 * @brief pointInitialization::outputToVec
 * Sanity check on all incoming API data
 * if the index is -1, means GDAL could not find the variable and we can't use it
 * if the array is NULL, its probably that the data is empty for the request and we need to clean it up
 * Otherwise
 * get the data out of const double* and into a cleaner vector format
 * @param dataArray
 * @param data_idx
 * @param dataCount
 * @param data_name
 * @return
 */

std::vector<std::string> pointInitialization::outputToVec(const double *dataArray,int data_idx, int dataCount, string data_name)
{
    if(data_idx==-1)//Index is invalid, probably the station is missing the sensor we need
    {
        std::vector<std::string> invalid_idx;
        invalid_idx.push_back("NULL_IDX");
        return invalid_idx;
    }
    if(data_idx!=-1 && dataArray==NULL)//The sensor "might" be there but has no data that we can use
    {
        std::vector<std::string>invalid_array;
        invalid_array.push_back("NULL_ARRAY");
        return invalid_array;
    }
    else
    {
        std::vector<std::string> data_vec;
        for(int i=0;i<dataCount;i++)
        {
            std::ostringstream data_ss;
            data_ss<<dataArray[i];

            data_vec.push_back(data_ss.str());

        }
        return data_vec;
    }
}

/**
 * @brief pointInitialization::fixEmptySensor
 * Cleans up a NULL from outputToVec if its wind direction
 * or solar radiation only
 *
 * We don't do this for wind speed or air temperature
 * as they are more important and can't be handled as easily
 * @param data_vec
 * @param data_name
 * @param valid_vec
 * @return
 */
std::vector<std::string> pointInitialization::fixEmptySensor(std::vector<string> data_vec, string data_name, std::vector<string> valid_vec)
{
    std::vector<std::string> corrected_data;
    std::string filler="0";//Fill in the blanks with zeros
    if(data_vec[0]=="NULL_IDX") //Means that we couldn't find the sensor variable
    {
        for(int i=0;i<valid_vec.size();i++)
        {
            corrected_data.push_back(filler);
        }
        return corrected_data;
    }
    if(data_vec[0]=="NULL_ARRAY")//Means that we found the var, and it was empty/failed qc/not there
    {
        for(int i=0;i<valid_vec.size();i++)
        {
            corrected_data.push_back(filler);
        }
        return corrected_data;
    }
    else
    {
        return data_vec;
    }
}

/**
 * @brief pointInitialization::fetchStationData
 * Fetches the data from the specified URL and user
 * specified parameters
 * Saves data to disk
 *
 * Rewritten to handle bad stations more robustly
 *
 * @param URL
 * @param timeZone
 * @param latest
 */

bool pointInitialization::fetchStationData(string URL, string timeZone, bool latest)
{
    OGRDataSourceH hDS;
    OGRLayerH hLayer;
    OGRFeatureH hFeature;
    CPLDebug("STATION_FETCH", "Downloading Data from MesoWest....");
    hDS=OGROpen(URL.c_str(),0,NULL); //open the mesowest url

    if (hDS==NULL) //This is mainly caused by a bad URL, the user enters something wrong
    {
        CPLDebug("STATION_FETCH", "URL: %s", URL.c_str());
        error_msg="OGROpen could not read the station file.\nPossibly no stations exist for the given parameters.";
        return false;
        throw std::runtime_error(error_msg);
    }
    //get the data

    hLayer=OGR_DS_GetLayer(hDS,0);
    OGR_L_ResetReading(hLayer);

    int fCount=OGR_L_GetFeatureCount(hLayer,1); // this is the number of weather stations
    CPLDebug("STATION_FETCH","Found %i Stations...",fCount);
    std::string csvName; //This is a prefix that generally comes from specifying an out directory or something

    vector<int> mnetid; //Vector storing the mesonet codes for each station (options are 1 or 2)
    if (rawStationFilename.substr(rawStationFilename.size()-4,4)==".csv")//This is just incase the user specifies a path with a .csv for some reason
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

    std::vector<std::string> stationCSVNames; //A vector that stores the ccsv names to get pushed into another vector
    std::string header="\"Station_Name\",\"Coord_Sys(PROJCS,GEOGCS)\",\"Datum(WGS84,NAD83,NAD27)\",\"Lat/YCoord\",\"Lon/XCoord\",\"Height\",\"Height_Units(meters,feet)\",\"Speed\",\"Speed_Units(mph,kph,mps,kts)\",\"Direction(degrees)\",\"Temperature\",\"Temperature_Units(F,C)\",\"Cloud_Cover(%)\",\"Radius_of_Influence\",\"Radius_of_Influence_Units(miles,feet,meters,km)\",\"date_time\"";
    std::vector<bool> stationChecks; //stores any stations that fail to produce good data

    for(int ex=0;ex<fCount;ex++) //loop over all the stations based on how many we find
    {
        ofstream outFile;//writing to csv
        int id_idx0=0; //index for getting the mesonet id
        int id_idx1=0; ;//index for getting the stationID/name for file writing purposes

        const char* writeID; //C array for storing station IDs
        bool write_this_station = true; //Assume the data is good until proven otherwise

        hFeature=OGR_L_GetFeature(hLayer,ex);//Cycle through the features

        //Get the type of wxStation and its ID so that we can sort them
        //by unique variables and so that we can create a file
        id_idx0=OGR_F_GetFieldIndex(hFeature,"mnet_id");
        mnetid.push_back(OGR_F_GetFieldAsInteger(hFeature,id_idx0));

        id_idx1=OGR_F_GetFieldIndex(hFeature,"STID");
        writeID=(OGR_F_GetFieldAsString(hFeature,id_idx1));

        stringstream timeStream,timeStream2; //Timestream2 is only needed for timeseries when we have a start and stop time
        std::string tName;
        stringstream idStream;
        stringstream ss;
        bpt::time_facet *facet = new bpt::time_facet("%Y-%m-%d_%H%M");
        timeStream.imbue(locale(timeStream.getloc(),facet));
        std::string timeComponent;

        ss<<ex; //Get the index for more specificity on the file name
        idStream<<writeID;

        if (latest==true)
        {
            bpt::ptime writeTime = bpt::second_clock::local_time();
            timeStream<<writeTime;
            timeComponent = timeStream.str();
        }
        if (latest==false) //If it is a time series we name the file with both the start and stop time
        {
            timeStream2.imbue(locale(timeStream2.getloc(),facet));
            
            timeStream<<start_and_stop_times[0].local_time(); //Name files with Local Times
            timeStream2<<start_and_stop_times[1].local_time();
            timeComponent = timeStream.str()+"-"+timeStream2.str(); //because its local time, add the time zone
        }
        //Generate the filename
        if(csvName!="blank")
        {
            std::string nameComponent = idStream.str() + "-" + timeComponent + "-" + ss.str();
            tName = std::string(CPLFormFilename(csvName.c_str(),nameComponent.c_str(),".csv"));
//            tName = csvName+idStream.str() + "-" + timeComponent + "-" + ss.str() + ".csv";
        }
        else
        {
//            tName=idStream.str() + "-" + timeComponent + "-" + ss.str() + ".csv";
            std::string nameComponent = idStream.str() + "-" + timeComponent + "-" + ss.str();
            tName = std::string(CPLFormFilename(NULL,nameComponent.c_str(),".csv"));
        }

        if(mnetid[ex]==1)
        {
            //first get the datasets
            //For Aiport stations (ASOS/METAR)
            //we get wind speed, dir, temp
            //and then extrapolate cloud cover based on 3 layers reported by the stations
            int wind_count=0;
            int wind_idx = OGR_F_GetFieldIndex(hFeature,"wind_speed");
            const double* wind_data = (OGR_F_GetFieldAsDoubleList(hFeature,wind_idx,&wind_count));
            std::vector<std::string> airport_wind = outputToVec(wind_data,wind_idx,wind_count,"wind_speed");
            if(airport_wind[0]=="NULL_IDX") //We absolutely NEED wind speed info, if its not there, don't write the station
            {
                CPLDebug("STATION_FETCH","No wind data (null idx) found for station, throwing!");
                write_this_station=false;
            }
            if(airport_wind[0]=="NULL_ARRAY")
            {
                CPLDebug("STATION_FETCH","No wind data (null array) found for station, throwing!");
                write_this_station=false;
            }

            int dir_count=0;
            int dir_idx = OGR_F_GetFieldIndex(hFeature,"wind_direction");
            const double* dir_data = (OGR_F_GetFieldAsDoubleList(hFeature,dir_idx,&dir_count));
            std::vector<std::string> airport_dir = outputToVec(dir_data,dir_idx,dir_count,"wind_direction");
            airport_dir = fixEmptySensor(airport_dir,"wind_direction",airport_wind);
            //Wind direction is often omitted by the API if the wind speed = 0 (its calm outside)
            //we population the direction string with 0s so that it is consistent

            int temp_count=0;
            int temp_idx = OGR_F_GetFieldIndex(hFeature,"air_temp");
            const double* temp_data = (OGR_F_GetFieldAsDoubleList(hFeature,temp_idx,&temp_count));
            std::vector<std::string> airport_temp = outputToVec(temp_data,temp_idx,temp_count,"air_temp");
            if(airport_temp[0]=="NULL_IDX") //we absolutely NEED temperature data, don't write if we dont get it
            {
                CPLDebug("STATION_FETCH","No temp data (null idx) found for station, throwing!");
                write_this_station=false;
            }
            if(airport_temp[0]=="NULL_ARRAY")
            {
                CPLDebug("STATION_FETCH","No temp data (null array) found for station, throwing!");
                write_this_station=false;
            }

            int cloud_count_low=0;
            int cloud_count_med=0;
            int cloud_count_high=0;
            //We need some cloud cover data, layer 2 and layer 3 may throw errors but thats okay
            //errors originate from the API not reporting higher codes if the lower code is empty
            int cloud_idx_low=OGR_F_GetFieldIndex(hFeature,"cloud_layer_1_code");
            const double* low_cloud_data=OGR_F_GetFieldAsDoubleList(hFeature,
                                                                    cloud_idx_low,&cloud_count_low);

            CPLPushErrorHandler(&CPLQuietErrorHandler); //Ignore any errors from these two cloud layers
            int cloud_idx_med=OGR_F_GetFieldIndex(hFeature,"cloud_layer_2_code");
            const double* med_cloud_data=OGR_F_GetFieldAsDoubleList(hFeature,
                                                                    cloud_idx_med,&cloud_count_med);

            int cloud_idx_high=OGR_F_GetFieldIndex(hFeature,"cloud_layer_3_code");
            const double* high_cloud_data=OGR_F_GetFieldAsDoubleList(hFeature,
                                                                     cloud_idx_high,&cloud_count_high);
            CPLPopErrorHandler(); //Go back to showing errors

            vector<std::string>cloudkappa;
            cloudkappa=UnifyClouds(low_cloud_data,
                                   med_cloud_data,
                                   high_cloud_data,
                                   cloud_count_low,
                                   cloud_count_med,
                                   cloud_count_high,
                                   temp_count);

            //Now get the station metadata (lat lon etc)
            int lat_idx=OGR_F_GetFieldIndex(hFeature,"latitude");
            double airport_latitude=(OGR_F_GetFieldAsDouble(hFeature,lat_idx));

            int lon_idx=OGR_F_GetFieldIndex(hFeature,"LONGITUDE");
            double airport_longitude=(OGR_F_GetFieldAsDouble(hFeature,lon_idx));

            int stid_idx=OGR_F_GetFieldIndex(hFeature,"STID");
            const char* airport_stid=(OGR_F_GetFieldAsString(hFeature,stid_idx));

            //get the datetimes
            int dt_idx=OGR_F_GetFieldIndex(hFeature,"date_times");
            char** airport_datetime=(OGR_F_GetFieldAsStringList(hFeature,dt_idx));

            if(write_this_station==true)
            {
                CPLDebug("STATION_FETCH","Writing station: %s to file %s",writeID,tName.c_str());
                std::string airport_height="10"; //This may get changed later
                outFile.open(tName.c_str());
                outFile<<header<<endl;
                stationCSVNames.push_back(tName);
                storeFileNames(stationCSVNames);
                if(latest==true)
                {
                    int write_idx=0;
                    outFile<<airport_stid<<",GEOGCS,"<<"WGS84,"<<airport_latitude<<","<<airport_longitude<<","<<airport_height<<","<<"meters,"<<airport_wind[write_idx]<<",mps,"<<airport_dir[write_idx]<<","<<airport_temp[write_idx]<<",C,"<<cloudkappa[write_idx]<<","<<"-1,"<<"km"<<endl;

                }
                else
                {
                    for(int write_idx=0;write_idx<wind_count;write_idx++)
                    {
                        outFile<<airport_stid<<",GEOGCS,"<<"WGS84,"<<airport_latitude<<","<<airport_longitude<<","<<airport_height<<","<<"meters,"<<airport_wind[write_idx]<<",mps,"<<airport_dir[write_idx]<<","<<airport_temp[write_idx]<<",C,"<<cloudkappa[write_idx]<<","<<"-1,"<<"km,"<<airport_datetime[write_idx]<<endl;
                    }
                }
            }
            if(write_this_station==false)
            {
                stationChecks.push_back(write_this_station);
                if(fCount==1)
                {
                    error_msg="ERROR: Station: "+idStream.str()+" is missing required data/sesnors!";
                    CPLDebug("STATION_FETCH","%s failed to return valid data...",writeID);
                    return false;
                    throw std::runtime_error("DATA CHECK FAILED ON ALL STATIONS!");
                }
            }

        }
        if(mnetid[ex]==2)
        {
            //First get all the big sets of data
            //For RAWS Stations we get wind speed, wind direction, air temp
            //and get cloud cover based on solar radiation
            int wind_count=0;
            int wind_idx = OGR_F_GetFieldIndex(hFeature,"wind_speed");
            const double* wind_data = (OGR_F_GetFieldAsDoubleList(hFeature,wind_idx,&wind_count));
            std::vector<std::string> raws_wind = outputToVec(wind_data,wind_idx,wind_count,"wind_speed");
            if(raws_wind[0]=="NULL_IDX")
            {
                CPLDebug("STATION_FETCH","No wind data (null idx) found for station, throwing!");
                write_this_station=false;
            }
            if(raws_wind[0]=="NULL_ARRAY")
            {
                CPLDebug("STATION_FETCH","No wind data (null array) found for station, throwing!");
                write_this_station=false;
            }

            int dir_count=0;
            int dir_idx = OGR_F_GetFieldIndex(hFeature,"wind_direction");
            const double* dir_data = (OGR_F_GetFieldAsDoubleList(hFeature,dir_idx,&dir_count));
            std::vector<std::string> raws_dir = outputToVec(dir_data,dir_idx,dir_count,"wind_direction");
            raws_dir = fixEmptySensor(raws_dir,"wind_direction",raws_wind);

            int temp_count=0;
            int temp_idx = OGR_F_GetFieldIndex(hFeature,"air_temp");
            const double* temp_data = (OGR_F_GetFieldAsDoubleList(hFeature,temp_idx,&temp_count));
            std::vector<std::string> raws_temp = outputToVec(temp_data,temp_idx,temp_count,"air_temp");
            if(raws_temp[0]=="NULL_IDX")
            {
                CPLDebug("STATION_FETCH","No temp data (null idx) found for station, throwing!");
                write_this_station=false;
            }
            if(raws_temp[0]=="NULL_ARRAY")
            {
                CPLDebug("STATION_FETCH","No temp data (null array) found for station, throwing!");
                write_this_station=false;
            }

            int solar_count=0;
            int solar_idx = (OGR_F_GetFieldIndex(hFeature,"solar_radiation"));
            const double* solar_data = OGR_F_GetFieldAsDoubleList(hFeature,solar_idx,&solar_count);
            std::vector<std::string> raws_solar = outputToVec(solar_data,solar_idx,solar_count,"solar_radiation");
            raws_solar = fixEmptySensor(raws_solar,"solar_radiation",raws_wind);

            //Now get the station metadata (lat lon etc)
            int lat_idx=OGR_F_GetFieldIndex(hFeature,"latitude");
            double raws_latitude=(OGR_F_GetFieldAsDouble(hFeature,lat_idx));

            int lon_idx=OGR_F_GetFieldIndex(hFeature,"LONGITUDE");
            double raws_longitude=(OGR_F_GetFieldAsDouble(hFeature,lon_idx));

            int stid_idx=OGR_F_GetFieldIndex(hFeature,"STID");
            const char* raws_stid=(OGR_F_GetFieldAsString(hFeature,stid_idx));

            //get the datetimes
            int dt_idx=OGR_F_GetFieldIndex(hFeature,"date_times");
            char** raws_datetime=(OGR_F_GetFieldAsStringList(hFeature,dt_idx));

            std::vector<double> raws_cloud = Irradiate(raws_solar,
                                                       timeZone,raws_latitude,
                                                       raws_longitude,raws_datetime);

            int name_idx = OGR_F_GetFieldIndex(hFeature,"name");
            const char* raws_name =(OGR_F_GetFieldAsString(hFeature,name_idx));
            double raws_height = parseStationHeight(raws_name);

            if(write_this_station==true)
            {
                CPLDebug("STATION_FETCH","Writing station: %s to file %s",writeID,tName.c_str());
//                std::string raws_height="10"; //This may get changed later
                outFile.open(tName.c_str());
//                cout<<tName<<endl;
                outFile<<header<<endl;
                stationCSVNames.push_back(tName);
                storeFileNames(stationCSVNames);
                if(latest==true)
                {
                    int write_idx=0;
                    outFile<<raws_stid<<",GEOGCS,"<<"WGS84,"<<raws_latitude<<","<<raws_longitude<<","<<raws_height<<","<<"meters,"<<raws_wind[write_idx]<<",mps,"<<raws_dir[write_idx]<<","<<raws_temp[write_idx]<<",C,"<<raws_cloud[write_idx]<<","<<"-1,"<<"km"<<endl;
                }
                else
                {
                    for(int write_idx=0;write_idx<wind_count;write_idx++)
                    {
                        outFile<<raws_stid<<",GEOGCS,"<<"WGS84,"<<raws_latitude<<","<<raws_longitude<<","<<raws_height<<","<<"meters,"<<raws_wind[write_idx]<<",mps,"<<raws_dir[write_idx]<<","<<raws_temp[write_idx]<<",C,"<<raws_cloud[write_idx]<<","<<"-1,"<<"km,"<<raws_datetime[write_idx]<<endl;
                    }
                }
            }
            if(write_this_station==false)
            {
                CPLDebug("STATION_FETCH","%s failed to return valid data...",writeID);
                stationChecks.push_back(write_this_station);
                if(fCount==1)
                {
                    error_msg="ERROR: Station: "+idStream.str()+" is missing required data/sesnors!";
                    return false;
                    throw std::runtime_error("DATA CHECK FAILED ON ALL STATIONS!");
                }
            }
        }
        OGR_F_Destroy(hFeature);
    }
    OGR_DS_Destroy(hDS);
    if(stationChecks.size()>=fCount)
    {
        error_msg="ERROR: Data check failed on all stations, likley stations are missing sensors.";
        CPLDebug("STATION_FETCH","DATA CHECK FAILED ON ALL STATIONS...");
        return false;
    }
    else
    {
        CPLDebug("STATION_FETCH","Data Downloaded and Saved...");
        CPLDebug("STATION_FETCH","STATION-FETCH FINISHED!");
        return true;
    }
}

