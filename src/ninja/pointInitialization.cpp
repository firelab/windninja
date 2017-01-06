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

 const std::string pointInitialization::dtoken="33e3c8ee12dc499c86de1f2076a9e9d4";
 const std::string pointInitialization::dvar="wind_speed,wind_direction,air_temp,"
                                             "solar_radiation,cloud_layer_1_code";
 const std::string pointInitialization::ndvar="wind_speed,wind_direction,air_temp,"
                                              "solar_radiation,cloud_layer_1_code,"
                                              "cloud_layer_2_code,cloud_layer_3_code";

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
 * @param input WindNinjaInputs object
 * @param mesh associated mesh object
 * @param u0 u component
 * @param v0 v component
 * @param w0 w component
 * @see WindNinjaInputs, Mesh, wn_3dScalarField
 */
void pointInitialization::initializeFields(WindNinjaInputs &input,
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
    profile.profile_switch = windProfile::monin_obukov_similarity;  //switch that detemines what profile is used...
    
    //make sure rough_h is set to zero if profile switch is 0 or 2

    //These are only needed if diurnal is turned on...
    AsciiGrid<double> height;	//height of diurnal flow above "z=0" in log profile
    AsciiGrid<double> uDiurnal;
    AsciiGrid<double> vDiurnal;
    AsciiGrid<double> wDiurnal;
    Aspect aspect;
    Slope slope;
    Shade shade;
    Solar solar;

    if(input.diurnalWinds == true)  //compute values needed for diurnal computations
    {
        //height of diurnal flow above "z=0" in log profile
        height.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(),
                  input.dem.get_xllCorner(), input.dem.get_yllCorner(), 
                  input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);	
        uDiurnal.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(),
                 input.dem.get_xllCorner(), input.dem.get_yllCorner(), 
                 input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);
        vDiurnal.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(),
                input.dem.get_xllCorner(), input.dem.get_yllCorner(), 
                input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);
        wDiurnal.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), 
                input.dem.get_xllCorner(), input.dem.get_yllCorner(), 
                input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);
        aspect.compute_gridAspect(&input.dem, input.numberCPUs);
        slope.compute_gridSlope(&input.dem, input.numberCPUs);
        double aspect_temp = 0;	//just placeholder, basically
        double slope_temp = 0;	//just placeholder, basically
        solar.compute_solar(input.ninjaTime, input.latitude, input.longitude, aspect_temp, slope_temp);
        shade.compute_gridShade(&input.dem, solar.get_theta(), solar.get_phi(), input.numberCPUs);
    }

    AsciiGrid<double> uInitializationGrid(input.dem.get_nCols(), input.dem.get_nRows(), 
                        input.dem.xllCorner, input.dem.yllCorner, input.dem.cellSize, 
                        std::numeric_limits<double>::max(), input.dem.prjString);
    AsciiGrid<double> vInitializationGrid(input.dem.get_nCols(), input.dem.get_nRows(),
                        input.dem.xllCorner, input.dem.yllCorner, input.dem.cellSize, 
                        std::numeric_limits<double>::max(), input.dem.prjString);
    AsciiGrid<double> airTempGrid(input.dem.get_nCols(), input.dem.get_nRows(), 
                        input.dem.xllCorner, input.dem.yllCorner, input.dem.cellSize, 
                        std::numeric_limits<double>::max(), input.dem.prjString);
    AsciiGrid<double> cloudCoverGrid(input.dem.get_nCols(), input.dem.get_nRows(), 
                        input.dem.xllCorner, input.dem.yllCorner, input.dem.cellSize, 
                        -9999.0, input.dem.prjString);

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
        if(input.stationsScratch[ii].get_height() > maxStationHeight){
            maxStationHeight = input.stationsScratch[ii].get_height();
        }
        sd_to_uv(input.stationsScratch[ii].get_speed(), 
                input.stationsScratch[ii].get_direction(), &u[ii], &v[ii]);
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
        throw std::runtime_error("Fill interpolation from the wx stations didn't completely fill the grids.  "
				"To be sure everything is filled, let at least one wx station have an "
                                "infinite influence radius.  This is specified by defining the influence "
                                "radius to be a value less than zero in the wx "
				"station file.");
    }

    cloud = cloudCoverGrid;

    //Monin-Obukhov length, surface friction velocity, and atmospheric boundary layer height
    L.set_headerData(input.dem.get_nCols(), input.dem.get_nRows(), 
                    input.dem.get_xllCorner(), input.dem.get_yllCorner(), 
                    input.dem.get_cellSize(), input.dem.get_noDataValue(), 0.0);
    u_star.set_headerData(input.dem.get_nCols(), input.dem.get_nRows(), 
                    input.dem.get_xllCorner(), input.dem.get_yllCorner(), 
                    input.dem.get_cellSize(), input.dem.get_noDataValue(), 0.0);
    bl_height.set_headerData(input.dem.get_nCols(), input.dem.get_nRows(), 
                    input.dem.get_xllCorner(), input.dem.get_yllCorner(), 
                    input.dem.get_cellSize(), input.dem.get_noDataValue(), -1.0);

    int i_, j_;
    double albedo_, bowen_, cg_, anthropogenic_;
    double U_star;

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
            if(input.dem.check_inBounds(input.stationsScratch[ii].get_projXord(), 
                        input.stationsScratch[ii].get_projYord()))  //if station is in the dem domain
            {   
                input.dem.get_cellIndex(input.stationsScratch[ii].get_projXord(), 
                                        input.stationsScratch[ii].get_projYord(), &i_, &j_);

                profile.Roughness = (input.surface.Roughness)(i_, j_);
                profile.Rough_h = (input.surface.Rough_h)(i_, j_);
                profile.Rough_d = (input.surface.Rough_d)(i_, j_);

                if(input.diurnalWinds == true)	//compute values needed for diurnal computation
                {
                    cDiurnal.initialize(input.stationsScratch[ii].get_projXord(), input.stationsScratch[ii].get_projYord(),
                                        aspect(i_, j_),slope(i_, j_), cloudCoverGrid(i_, j_), airTempGrid(i_, j_),
                                        input.stationsScratch[ii].get_speed(), input.stationsScratch[ii].get_height(),
                                        (input.surface.Albedo)(i_, j_), (input.surface.Bowen)(i_, j_), 
                                        (input.surface.Cg)(i_, j_), (input.surface.Anthropogenic)(i_, j_), 
                                        (input.surface.Roughness)(i_, j_), (input.surface.Rough_h)(i_, j_), 
                                        (input.surface.Rough_d)(i_, j_));

                    cDiurnal.compute_cell_diurnal_parameters(i_, j_,&profile.ObukovLength, &U_star, &profile.ABL_height);
					
                }
                else{	//compute neutral ABL height
					
                    double f;
                    double velocity;

                    //compute f -> Coriolis parameter
                    if(input.latitude<=90.0 && input.latitude>=-90.0)
                    {
                        // f should be about 10^-4 for mid-latitudes
                        f = (1.4544e-4) * sin(pi/180 * input.latitude);	// f = 2 * omega * sin(theta)
                        // (1.4544e-4) here is 2 * omega = 2 * (2 * pi radians) / 24 hours = 1.4544e-4 seconds^-1
                        // obtained from Stull 1988 book
                        if(f<0){
                            f = -f;
                        }
                    }
                    else{
                        f = 1e-4;	//if latitude is not available, set f to mid-latitude value
                    }
				
                    if(f==0.0){	//zero will give division by zero below
                        f = 1e-8;	//if latitude is zero, set f small
                    }

                    //compute neutral ABL height
                    velocity=std::pow(u[ii]*u[ii]+v[ii]*v[ii],0.5);     //Velocity is the velocity magnitude
                    U_star = velocity*0.4/(log((profile.inputWindHeight+profile.Rough_h-profile.Rough_d)/profile.Roughness));
                                        
                    //compute neutral ABL height
                    //from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
                    profile.ABL_height = 0.2 * U_star / f;  
                    profile.ObukovLength = 0.0;
                }
            } 
            else{	//if station is not in dem domain, use grass roughness
                profile.Roughness = 0.01;
                profile.Rough_h = 0.0;
                profile.Rough_d = 0.0;
                albedo_ = 0.25;
                bowen_ = 1.0;
                cg_ = 0.15;
                anthropogenic_ = 0.0;

                if(input.diurnalWinds == true)	//compute values needed for diurnal computation
                {
                    cDiurnal.initialize(input.stationsScratch[ii].get_projXord(), input.stationsScratch[ii].get_projYord(),
                        0.0, 0.0, cloudCoverGrid(i_, j_), airTempGrid(i_, j_), input.stationsScratch[ii].get_speed(),
                        input.stationsScratch[ii].get_height(), albedo_, bowen_, cg_, anthropogenic_, profile.Roughness,
                        profile.Rough_h, profile.Rough_d);

                    cDiurnal.compute_cell_diurnal_parameters(i_, j_,&profile.ObukovLength, &U_star, &profile.ABL_height);
                                    
                }
                else{  //compute neutral ABL height
					
                double f;
                double velocity;

                //compute f -> Coriolis parameter
                if(input.latitude<=90.0 && input.latitude>=-90.0)
                {
                    f = (1.4544e-4) * sin(pi/180 * input.latitude);	// f = 2 * omega * sin(theta)
                    // f should be about 10^-4 for mid-latitudes
                    // (1.4544e-4) here is 2 * omega = 2 * (2 * pi radians) / 24 hours = 1.4544e-4 seconds^-1
                    // obtained from Stull 1988 book
                    if(f<0){
                        f = -f;
                    }
                }
                else{
                    f = 1e-4; //if latitude is not available, set f to mid-latitude value
                }
                            
                if(f==0.0){ //zero will give division by zero below
                    f = 1e-8;	//if latitude is zero, set f small
                }

                //compute neutral ABL height
                velocity=std::pow(u[ii]*u[ii]+v[ii]*v[ii],0.5);     //Velocity is the velocity magnitude
                U_star = velocity*0.4/(log((profile.inputWindHeight+profile.Rough_h-profile.Rough_d)/profile.Roughness));
                                    
                //compute neutral ABL height
                //from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
                profile.ABL_height = 0.2 * U_star / f;  
                profile.ObukovLength = 0.0;
                }
            }

            profile.AGL=maxStationHeight + profile.Rough_h; //this is height above THE GROUND!! (not "z=0" for the log profile)

            wind_sd_to_uv(input.stationsScratch[ii].get_speed(), input.stationsScratch[ii].get_direction(), &u[ii], &v[ii]);
            profile.inputWindSpeed = u[ii];
            u[ii] = profile.getWindSpeed();
            profile.inputWindSpeed = v[ii];
            v[ii] = profile.getWindSpeed();
        }
        else{   //else station is already at 2d interp layer height
            wind_sd_to_uv(input.stationsScratch[ii].get_speed(), input.stationsScratch[ii].get_direction(), &u[ii], &v[ii]);
        }
    }            
	
    uInitializationGrid.interpolateFromPoints(u, X, Y, influenceRadius, input.stationsScratch.size(), dfInvDistWeight);
    vInitializationGrid.interpolateFromPoints(v, X, Y, influenceRadius, input.stationsScratch.size(), dfInvDistWeight);

    input.surface.windSpeedGrid.set_headerData(uInitializationGrid);
    input.surface.windGridExists = true;

    for(i=0;i<input.dem.get_nRows();i++)
    {
        for(j=0;j<input.dem.get_nCols();j++)
        {
            input.surface.windSpeedGrid(i,j) = std::pow((uInitializationGrid(i,j)*
                        uInitializationGrid(i,j)+vInitializationGrid(i,j)*
                        vInitializationGrid(i,j)), 0.5);
        }
    }

    if(u)
    {
            delete[] u;
            u = NULL;
    }
    if(v)
    {
            delete[] v;
            v = NULL;
    }
    if(T)
    {
            delete[] T;
            T = NULL;
    }
    if(cc)
    {
            delete[] cc;
            cc = NULL;
    }
    if(X)
    {
            delete[] X;
            X = NULL;
    }
    if(Y)
    {
            delete[] Y;
            Y = NULL;
    }
    if(influenceRadius)
    {
        delete[] influenceRadius;
        influenceRadius = NULL;
    }

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

    //compute diurnal wind
    if(input.diurnalWinds == true)
    {
        addDiurnal diurnal(&uDiurnal, &vDiurnal, &wDiurnal, &height, &L, &u_star, 
                    &bl_height, &input.dem, &aspect, &slope, &shade, &solar, 
                    &input.surface, &cloudCoverGrid, &airTempGrid, 
                    input.numberCPUs, input.downDragCoeff, input.downEntrainmentCoeff,
                    input.upDragCoeff, input.upEntrainmentCoeff);
    }
    else{	//compute neutral ABL height
        double f, vel;

        //compute f -> Coriolis parameter
        if(input.latitude<=90.0 && input.latitude>=-90.0)
        {
            f = (1.4544e-4) * sin(pi/180 * input.latitude);	// f = 2 * omega * sin(theta)
            // f should be about 10^-4 for mid-latitudes
            // (1.4544e-4) here is 2 * omega = 2 * (2 * pi radians) / 24 hours = 1.4544e-4 seconds^-1
            // obtained from Stull 1988 book
            if(f<0){
                f = -f;
            }
        }
        else{
                f = 1e-4;	//if latitude is not available, set f to mid-latitude value
        }
        if(f==0.0){	//zero will give division by zero below
            f = 1e-8;	//if latitude is zero, set f small
        }

        //compute neutral ABL height
        #pragma omp parallel for default(shared) private(i,j)
        for(i=0;i<input.dem.get_nRows();i++)
        {
            for(j=0;j<input.dem.get_nCols();j++)
            {
                vel = std::pow((uInitializationGrid(i,j)*uInitializationGrid(i,j)+
                            vInitializationGrid(i,j)*vInitializationGrid(i,j)),0.5);
                u_star(i,j) = vel*0.4/(log((input.inputWindHeight+
                            input.surface.Rough_h(i,j)-input.surface.Rough_d(i,j))/
                            input.surface.Roughness(i,j)));
                            
                //compute neutral ABL height
                //from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
                bl_height(i,j) = 0.2 * u_star(i,j) / f;	
            }
        }
    }

    //Initialize u0,v0,w0----------------------------------
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
                //this is height above THE GROUND!! (not "z=0" for the log profile)
                profile.AGL=mesh.ZORD(i, j, k)-input.dem(i,j);
                            
                profile.inputWindSpeed = uInitializationGrid(i,j);
                u0(i, j, k) += profile.getWindSpeed();
                profile.inputWindSpeed = vInitializationGrid(i,j);
                v0(i, j, k) += profile.getWindSpeed();

                profile.inputWindSpeed = 0.0;
                w0(i, j, k) += profile.getWindSpeed();

            }
        }
    }

    //Now add diurnal component if desired
    double AGL=0; //height above top of roughness elements
    if((input.diurnalWinds==true) && (profile.profile_switch==windProfile::monin_obukov_similarity))
    {
        #pragma omp parallel for default(shared) private(i,j,k,AGL)
        for(k=1;k<mesh.nlayers;k++)  //start at 1, not 0 bc ground nodes must be zero for boundary conditions to work properly
        {
            for(i=0;i<mesh.nrows;i++)
            {
                for(j=0;j<mesh.ncols;j++)
                {
                    AGL=mesh.ZORD(i, j, k)-input.dem(i,j);  //this is height above THE GROUND!! (not "z=0" for the log profile)
                    if((AGL - input.surface.Rough_d(i,j) < height(i,j)))
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

vector<wxStation> pointInitialization::interpolateFromDisk(std::string stationFilename,
                                              std::string demFile,
                                              std::vector<boost::posix_time::ptime> timeList,std::string timeZone)
{
    std::string csvFile=stationFilename;
    vector<std::string> stationNames;

    OGRDataSourceH hDS;
    hDS = OGROpen( csvFile.c_str(), FALSE, NULL );

    OGRLayer *poLayer;
    OGRFeature *poFeature;
    OGRFeatureDefn *poFeatureDefn;
    poLayer = (OGRLayer*)OGR_DS_GetLayer( hDS, 0 );

    std::string oStationName;

    OGRLayerH hLayer;
    hLayer=OGR_DS_GetLayer(hDS,0);
    OGR_L_ResetReading(hLayer);

    poLayer->ResetReading();
    while( ( poFeature = poLayer->GetNextFeature() ) != NULL )
    {
        poFeatureDefn = poLayer->GetLayerDefn();

        // get Station name
        oStationName = poFeature->GetFieldAsString( 0 );
        stationNames.push_back(oStationName);
    }

    int statCount;
    statCount=stationNames.size();

    int specCount=statCount;

    vector<int> idxCount;
    int j=0;
    int q=0;

    for (int i=0;i<statCount;i++)
    {
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

    std::vector<pointInitialization::preInterpolate> diskData;
    std::vector<std::vector<pointInitialization::preInterpolate> > wxVector;
    diskData=readDiskLine(stationFilename,demFile);// reads in data

    CPLDebug("STATION_FETCH", "Checking first time step...");

    bool timeCheck;
    timeCheck=timeList[0]>=diskData[0].datetime;

    if (timeCheck==false)
    {
        cout<<"FATAL: Initial time step must match first time step for in CSV!"<<endl;
        cout<<"This error should only appear if you are using your own data file."<<endl;
        cout<<"check inputs and "<<csvFile<<" to ensure that the start time matches the first time in your data"<<endl;
        cout<<"\n"<<endl;

        cout<<"Conflicting TimeSteps:"<<endl;
        cout<<"timeList (start Time) "<<timeList[0]<<endl;
        cout<<"Time on File"<<diskData[0].datetime<<endl;
        exit(1);
    }

    CPLDebug("STATION_FETCH", "First time step check passed...");

    int t=0;
    vector<int> countLimiter;

    for (int ei=1;ei<=idxCount.size();ei++)//organizes data into a vector of vector of data
    {
        int rounder=idxCount.size()-ei;
        int e=std::accumulate(idxCount.begin(),idxCount.end()-rounder,0);
        countLimiter.push_back(e);
    }


    for (int ei=0; ei<idxCount.size(); ei++)
    {
        std::vector<pointInitialization::preInterpolate> sub(&diskData[t],&diskData[countLimiter[ei]]);
        wxVector.push_back(sub);
        t=countLimiter[ei];
    }

    vector<boost::posix_time::ptime> outaTime;
    boost::posix_time::ptime noTime;
    outaTime.push_back(noTime);
    vector<vector<preInterpolate> > interpolatedDataSet;
    vector<wxStation> readyToGo;

    if (wxVector[0][0].datetime==noTime)
    {
        CPLDebug("STATION_FETCH", "noTime");
        readyToGo=interpolateNull(csvFile,demFile,wxVector,timeZone);
    }
    else
    {
        //does all interpolation
        interpolatedDataSet=interpolateTimeData(csvFile,demFile,wxVector,timeList); 
        readyToGo=makeWxStation(interpolatedDataSet,csvFile,demFile);
    }

    for (int i=0;i<readyToGo.size();i++)
    {
        bool a=wxStation::check_station(readyToGo[i]);
        if (a != true)
        {
            cout<<"!!stationcheck failed on #"<<i<<": \""<<readyToGo[i].get_stationName()<<"\" potential for bad data!!"<<endl;
        }
        else
        {
            cout<<"station check passed, station #"<<i<<": \""<<readyToGo[i].get_stationName()<<"\" has good data"<<endl;
        }
    }

    return readyToGo;
}

vector<pointInitialization::preInterpolate> pointInitialization::readDiskLine(string stationFilename, string demFile)
{
    std::string csvFile=stationFilename;
    std::string oErrorString = "";
    preInterpolate oStation;
    std::vector<preInterpolate> oStations;
    preInterpolate work;
    std::vector<preInterpolate> vecwork;
    work.stationName="aaaaaaaa";
    vecwork.push_back(work);

    OGRDataSourceH hDS;
    hDS = OGROpen( csvFile.c_str(), FALSE, NULL );

    if( hDS == NULL )
    {
        oErrorString = "Cannot open csv file: ";
        oErrorString += csvFile;
        throw( std::runtime_error( oErrorString ) );
    }

    OGRFeatureH hFeature;
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

    CPLDebug("STATION_FETCH", "Reading csvName: %s", csvFile.c_str());

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
    }

    OGRFeature::DestroyFeature( poFeature );
    OGR_DS_Destroy( hDS );

    return oStations;
}


vector<wxStation> pointInitialization::makeWxStation(vector<vector<preInterpolate> > data, string csvFile, string demFile)
{
    cout<<"converting Interpolated struct to wxStation..."<<endl;
    vector<std::string> stationNames;
    vector<wxStation> stationData;

    OGRDataSourceH hDS;
    hDS = OGROpen( csvFile.c_str(), FALSE, NULL );

    OGRLayer *poLayer;
    OGRFeature *poFeature;
    OGRFeatureDefn *poFeatureDefn;
    poLayer = (OGRLayer*)OGR_DS_GetLayer( hDS, 0 );

    std::string oStationName;

    OGRLayerH hLayer;
    hLayer=OGR_DS_GetLayer(hDS,0);
    OGR_L_ResetReading(hLayer);

    poLayer->ResetReading();
    while( ( poFeature = poLayer->GetNextFeature() ) != NULL )
    {
    poFeatureDefn = poLayer->GetLayerDefn();

    // get Station name
    oStationName = poFeature->GetFieldAsString( 0 );
    stationNames.push_back(oStationName);
//    cout<<oStationName<<endl;
    }



    int statCount;
    statCount=stationNames.size();

    int specCount=statCount;

    vector<int> idxCount;
    int j=0;
    int q=0;

    for (int i=0;i<statCount;i++)
    {
//        cout<<"looks at: "<<q<<endl;
//        cout<<"starts at: "<<j<<endl;

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
//            cout<<"exiting loop"<<endl;
            break;
        }
    }
    //     cout<<"idxCount size: "<<idxCount.size()<<endl;
    //     for (int ii=0;ii<idxCount.size();ii++)
    //     {
    //         cout<<idxCount[ii]<<endl;
    //     }
         vector<int> countLimiter;

         for (int ei=1;ei<=idxCount.size();ei++)
         {
         //    cout<<ei<<endl;
             int rounder=idxCount.size()-ei;
             int e=std::accumulate(idxCount.begin(),idxCount.end()-rounder,0);
             countLimiter.push_back(e);
         }

    //     for (int i=0;i<idxCount.size();i++)
    //     {
    //         cout<<countLimiter[i]-1<<" "<<stationNames[countLimiter[i]-1]<<endl;
    //     }



    //    cout<<countLimiter[0]-1<<" "<<stationNames[countLimiter[0]-1]<<endl;
    //    cout<<countLimiter[1]-1<<" "<<stationNames[countLimiter[1]-1]<<endl;
    //    cout<<countLimiter[2]-1<<" "<<stationNames[countLimiter[2]-1]<<endl;
    //     vector<wxStationList> Liszt=wxStationList::readStationFetchFile(csvFile,demFile);

         vector<vector<preInterpolate> >stationDataList;
//     if (inputStation[0][0].get_stationName()=="")
//     {
//     vector<vector<wxStationList> >stationDataList=wxStationList::vectorRead(csvFile,demFile);
//     }
//     else

        stationDataList=data;
        //here is where a wxstation is made
        for (int i=0;i<idxCount.size();i++)
        {
            wxStation subDat;
//            subDat.set_stationName(stationDataList[i][0].get_stationName());
            subDat.set_stationName(stationDataList[i][0].stationName);
//            cout<<subDat.get_stationName()<<endl;

            std::string CoordSys=stationDataList[i][0].datumType;

//            int iCT=stationDataList[i][0].get_coordType();
//            cout<<stationDataList[i][0].coordType<<endl;

            if (CoordSys=="projcs")
            {
//             subDat.set_location_projected(stationDataList[i][0].lat,stationDataList[i][0].lon,demFile);
                cout<<"poo"<<endl;
            }
//            if (iCT==1)
//            {
//             stCoordDat="WGS84";
//            }
//            else
//            {
//             cout<<"defaulting to WGS84 "<<endl;
//             stCoordDat="WGS84";
//            }
            else //WGS84!
            {
            const char* stCoorDat=CoordSys.c_str();
//            cout<<stCoorDat<<endl;
            subDat.set_location_LatLong(stationDataList[i][0].lat,stationDataList[i][0].lon,
                    demFile,stCoorDat);
            }

            for (int k=0;k<stationDataList[i].size();k++)
            {
             subDat.set_speed(stationDataList[i][k].speed,stationDataList[i][k].inputSpeedUnits);
             subDat.set_direction(stationDataList[i][k].direction);
             subDat.set_temperature(stationDataList[i][k].temperature,stationDataList[i][k].tempUnits);
             subDat.set_cloudCover(stationDataList[i][k].cloudCover,stationDataList[i][k].cloudCoverUnits);
             subDat.set_influenceRadius(stationDataList[i][k].influenceRadius,stationDataList[i][k].influenceRadiusUnits);
             subDat.set_height(stationDataList[i][k].height,stationDataList[i][k].heightUnits);
             subDat.set_datetime(stationDataList[i][k].datetime);

            }
        //    cout<<subDat.speed.size()<<endl;
        //    cout<<subDat.direction.size()<<endl;
            stationData.push_back(subDat);
        }


    return stationData;
}



/**
 * @brief pointInitialization::interpolateNull
 * Used if the function is the old PointInitialization.
 * @param csvFileName
 * @param demFileName
 * @param vecStations
 * @return
 */


vector<wxStation> pointInitialization::interpolateNull(std::string csvFileName,std::string demFileName,vector<vector<preInterpolate> > vecStations,std::string timeZone)
{
    cout<<"no interpolation needed"<<endl;

    vecStations[0][0].cloudCoverUnits=coverUnits::percent;

    vector<wxStation> refinedDat;
    refinedDat=makeWxStation(vecStations,csvFileName,demFileName);

    //fixes time!
    boost::local_time::tz_database tz_db;
    tz_db.load_from_file( FindDataPath("date_time_zonespec.csv") );
    boost::local_time::time_zone_ptr timeZonePtr;
    timeZonePtr = tz_db.time_zone_from_region(timeZone);
    boost::posix_time::ptime standard = boost::posix_time::second_clock::universal_time();
    for (int i=0;i<refinedDat.size();i++)
    {
        refinedDat[i].datetime.assign(1,standard);
//        cout<<refinedDat[i].get_datetime(0)<<endl;
    }


    return refinedDat;
}


/**
 * @brief interpolates raw data WRT time
 *
 * @param csvFileName: used to verify correct file
 * @param demFileName: used to set coord system in interpolated data
 * @param vecStations: the raw data to be interpolated
 * @param timeList: the desired time and steps
 *
 */



vector<vector<pointInitialization::preInterpolate> > pointInitialization::interpolateTimeData(std::string csvFileName,std::string demFileName,vector<vector<pointInitialization::preInterpolate> > vecStations,std::vector<boost::posix_time::ptime> timeList)
{
    cout<<"Interpolating Time Data..."<<endl;

    boost::posix_time::ptime tempq;
    boost::posix_time::ptime init;


//    cout<<qq<<endl;




    vector<vector<preInterpolate> > Selectify;



//    cout<<"timeList"<<endl;
//    for( int i=0;i<timeList.size();i++)
//    {
//        cout<<timeList[i]<<endl;
//    }
//    cout<<timeList.size()<<endl;
//    cout<<"vecStation[0]"<<endl;
//    for (int i=0;i<qq;i++)
//    {
//        cout<<input.vecStations[0][i].get_datetime()<<endl;
//    }
//    cout<<qq<<endl;

//    cout<<"station[0] first step: "<<input.vecStations[0][0].get_datetime()<<endl;
//    tempq=input.vecStations[0][1].get_datetime();
//    init=timeList[1];
//    cout<<"timelist[0]: "<<timeList[0]<<endl;

//    boost::posix_time::time_duration buffer(1,0,0,0);
    boost::posix_time::time_duration zero(0,0,0,0);
    boost::posix_time::time_duration max(48,0,0,0);
    boost::posix_time::time_duration one(0,1,0,0);

    boost::posix_time::time_duration buffer;
    boost::posix_time::time_duration avgBuffer;
    vector<boost::posix_time::time_duration> avgBufferList;

    boost::posix_time::time_duration bufferSum;

//    buffer=init-tempq;
int totalsize=vecStations.size();


//Creates a vector of time buffers to be used to interpolate the raw data with the timeList
for (int j=0;j<totalsize;j++)
{
    vector<boost::posix_time::time_duration> buffers;
    for (int i=0;i<vecStations[j].size();i++)
    {
        buffer=vecStations[j][i].datetime-vecStations[j][i+1].datetime;
        if (buffer<=zero)
        {
            buffer=buffer.invert_sign();
        }
        if (buffer>=max)
        {
            buffer=buffers[0];
        }
//        cout<<buffer<<endl;
        buffers.push_back(buffer);

    }
    //cout<<buffers.size()<<endl;
    bufferSum=std::accumulate(buffers.begin(),buffers.end(),zero);
    //cout<<bufferSum<<endl;

    avgBuffer=bufferSum/buffers.size();

//    cout<<avgBuffer<<endl;
    avgBufferList.push_back(avgBuffer);
}




////cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
////int k=1;
////for (int j=0;j<timeList.size();j++)
////{
////    int qq;
////    qq=vecStations[k].size();
////    boost::posix_time::ptime comparator;
////    comparator=timeList[j];
////    int counter=0;

////    cout<<"goal: "<<comparator<<endl;

////    for (int i=0;i<qq;i++)
////    {
////        boost::posix_time::time_duration difference;
////        difference=comparator-vecStations[k][i].get_datetime();
////        if (difference<=zero)
////        {
////            difference=difference.invert_sign();
////        }
////        if (difference<=avgBufferList[k])
////        {
////            counter++;

////            if (counter>2)
////            {
////    //                    cout<<"false"<<" "<<counter<<endl;
////                continue;
////            }

////    //                cout<<"true"<<" "<<counter<<endl;
////            cout<<difference<<endl;
////            cout<<vecStations[k][i].get_datetime()<<endl;
////            continue;
////        }
////        if (difference<=avgBufferList[k]+one && counter<2)
////        {
////            cout<<difference<<endl;
////            cout<<vecStations[k][i].get_datetime()<<endl;
////            counter++;
////            continue;
////        }
////        if (difference>avgBufferList[k])
////        {
////    //                cout<<"false"<<endl;
////        }


////    }
////    cout<<" "<<endl;
////}

////cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;

for (int k=0;k<totalsize;k++)
{
    int timesize=0;
    vector<preInterpolate> subSelectify;
    int qq;
    qq=vecStations[k].size();
//    cout<<avgBufferList[k]<<endl;

    for (int j=0;j<timeList.size();j++)
    {
        boost::posix_time::ptime comparator; //robespierre
        comparator=timeList[j];


        int counter=0;
        for (int i=0;i<qq;i++)
        {

            boost::posix_time::time_duration difference;
            difference=comparator-vecStations[k][i].datetime;
            if (difference<=zero)
            {
                difference=difference.invert_sign();
            }
            if (difference<=avgBufferList[k])
            {
                counter++;

                if (counter>2)
                {
//                    cout<<"false"<<" "<<counter<<endl;
                    continue;
                }

//                cout<<"true"<<" "<<counter<<endl;
//                cout<<difference<<endl;
//                cout<<vecStations[k][i].get_datetime()<<endl;
                subSelectify.push_back(vecStations[k][i]);
                continue;
            }
            if (difference<=avgBufferList[k]+one && counter<2)
            {
//                cout<<difference<<endl;
//                cout<<vecStations[k][i].get_datetime()<<endl;
                subSelectify.push_back(vecStations[k][i]);
                counter++;
                continue;
            }
            if (difference>avgBufferList[k])
            {
//                cout<<"false"<<endl;
            }

        }
//        cout<<subSelectify.size()<<endl;

//        cout<<"moving to next timeList[j] \n\n"<<endl;
        timesize++;

    }
//    cout<<timesize<<endl;
//    cout<<timeList.size()<<endl;

    Selectify.push_back(subSelectify);
}



cout<<"Time data Interpolated...\n"<<"Temporally Interpolating wx Data..."<<endl;

vector<vector<preInterpolate> > lowVec;
vector<vector<preInterpolate> > highVec;

for (int j=0;j<Selectify.size();j++)
{
    vector<preInterpolate> lowStations;
    vector<preInterpolate> highstations;

    for (int i=0;i<Selectify[j].size();i+=2)
    {
          lowStations.push_back(Selectify[j][i]);
    }
    for (int k=1;k<Selectify[j].size();k+=2)
    {
        highstations.push_back(Selectify[j][k]);
    }
    lowVec.push_back(lowStations);
    highVec.push_back(highstations);

}

////printf("\n\n");

////cout<<Selectify[0].size()<<endl;



////cout<<" "<<endl;
////cout<<lowVec[0].size()<<endl;
////cout<<timeList.size()<<endl;
////cout<<highVec[0].size()<<endl;

////for (int i=0;i<timeList.size();i++)
////{
////    cout<<lowVec[0][i].get_datetime()<<endl;
////    cout<<timeList[i]<<endl;
////    cout<<highVec[0][i].get_datetime()<<endl;


////}

////exit(1);
////cout<<"highvec size: "<<highVec[2].size()<<endl;
////cout<<"lowvec size: "<<lowVec[2].size()<<endl;

////SETTING WX TIMELIST


vector<vector<preInterpolate> > interpolatedWxData;
for (int ey=0;ey<Selectify.size();ey++)
{
    vector<preInterpolate> subInter;
    for (int ex=0;ex<timeList.size();ex++)
    {
        preInterpolate timeStorage;
        timeStorage.datetime=timeList[ex];
        subInter.push_back(timeStorage);
    }
    interpolatedWxData.push_back(subInter);
}

////wxStation::wxPrinter(vecStations[0][0]);

////SETTING COORD SYS, DATUM, LAT, LON, HEIGH, HU, RADIUS OF INFLUENCE,NAME
for (int k=0;k<Selectify.size();k++)
{
    double latitude;
    double longitude;
    double height;
    double radiusInfluence;
    std::string datum;
    std::string coord;
    std::string stationName;


    latitude=vecStations[k][0].lat;
    longitude=vecStations[k][0].lon;
    height=vecStations[k][0].height;
    radiusInfluence=vecStations[k][0].influenceRadius;
    datum=vecStations[k][0].datumType;
    coord=vecStations[k][0].coordType;
    const char* newdatum="WGS84";
    stationName=vecStations[k][0].stationName;

    std::string demfile=demFileName;


    for (int i=0;i<timeList.size();i++)
    {
////      interpolatedWxData[k][i].set_location_LatLong(latitude,longitude,demfile,newdatum);
        interpolatedWxData[k][i].lat=latitude;
        interpolatedWxData[k][i].lon=longitude;
        interpolatedWxData[k][i].datumType=datum;
        interpolatedWxData[k][i].coordType=coord;
////        interpolatedWxData[k][i].set_height(height,lengthUnits::meters);
        interpolatedWxData[k][i].height=height;
        interpolatedWxData[k][i].heightUnits=lengthUnits::meters;
////        interpolatedWxData[k][i].set_influenceRadius(radiusInfluence,lengthUnits::meters);
        interpolatedWxData[k][i].influenceRadius=radiusInfluence;
        interpolatedWxData[k][i].influenceRadiusUnits=lengthUnits::meters;
        interpolatedWxData[k][i].stationName=stationName;
    }
}



//INTERPOLATING WIND SPEED
for (int k=0;k<Selectify.size();k++)
{
    for (int i=0;i<timeList.size();i++)
    {

    double low;
    double high;
    double inter;

    boost::posix_time::ptime pLow=lowVec[k][i].datetime;
    boost::posix_time::ptime pHigh=highVec[k][i].datetime;
    boost::posix_time::ptime pInter=timeList[i];


    low=unixTime(pLow);
    high=unixTime(pHigh);
    inter=unixTime(pInter);

//    cout<<pLow<<" "<<low<<endl;
//    cout<<pHigh<<" "<<high<<endl;
//    cout<<pInter<<" "<<inter<<endl;

    double speed1;
    double speed2;
    double speedI;

    speed1=lowVec[k][i].speed;
    speed2=highVec[k][i].speed;

//    cout<<speed1<<" "<<speed2<<endl;
    speedI=interpolator(inter,low,high,speed1,speed2);
    if (speedI>113.000)
    {
//        cout<<"windspeed error"<<endl;
        speedI=speed1;

    }
//    printf("%lf",inter);
//    cout<<" "<<endl;
//    printf("%lf",low);
//    cout<<" ";
//    printf("%lf",high);
//    cout<<" "<<endl;
////    cout<<pInter<<endl;
////    cout<<pLow<<" "<<pHigh<<endl;
//    cout<<speed1<<" "<<speed2<<endl;

//    cout<<speedI<<endl;

//    interpolatedWxData[k][i].(speedI,velocityUnits::metersPerSecond);
    interpolatedWxData[k][i].speed=speedI;
    interpolatedWxData[k][i].inputSpeedUnits=vecStations[k][0].inputSpeedUnits;
    }

}

//cout<<interpolatedWxData[1][3].speed<<" "<<interpolatedWxData[1][3].inputSpeedUnits<<endl;


//INTERPOLATING WIND DIRECITON
for (int k=0;k<Selectify.size();k++)
{
    for (int i=0;i<timeList.size();i++)
    {

    double lowDir;
    double highDir;
    double interDir;

    lowDir=lowVec[k][i].direction;
    highDir=highVec[k][i].direction;

//    cout<<lowDir<<" "<<highDir<<endl;

    interDir=interpolateDirection(lowDir,highDir);

//    cout<<interDir<<endl;

    interpolatedWxData[k][i].direction=interDir;
    }

}

//INTERPOLATING TEMPERATURE
for (int k=0;k<Selectify.size();k++)
{
    for (int i=0;i<timeList.size();i++)
    {

    double low;
    double high;
    double inter;

    boost::posix_time::ptime pLow=lowVec[k][i].datetime;
    boost::posix_time::ptime pHigh=highVec[k][i].datetime;
    boost::posix_time::ptime pInter=timeList[i];


    low=unixTime(pLow);
    high=unixTime(pHigh);
    inter=unixTime(pInter);

    double lowTemp;
    double highTemp;
    double interTemp;

    lowTemp=lowVec[k][i].temperature;
    highTemp=highVec[k][i].temperature;
    interTemp=interpolator(inter,low,high,lowTemp,highTemp);
    if (interTemp>57.0)
    {
//        cout<<"temp error"<<endl;
        interTemp=highTemp;
        if (interTemp>57.0)
        {
            interTemp=lowTemp;
        }
        if (interTemp>57.0)
        {
            interTemp=25;
        }
    }


//    cout<<lowTemp<<" "<<highTemp<<endl;
//    cout<<interTemp<<"\n\n"<<endl;

//    interpolatedWxData[k][i].set_temperature(interTemp,temperatureUnits::K);
    interpolatedWxData[k][i].temperature=interTemp;
    interpolatedWxData[k][i].tempUnits=vecStations[k][0].tempUnits;
    }

}

//INTERPOLATING CLOUD COVER
for (int k=0;k<Selectify.size();k++)
{
    for (int i=0;i<timeList.size();i++)
    {

    double low;
    double high;
    double inter;

    boost::posix_time::ptime pLow=lowVec[k][i].datetime;
    boost::posix_time::ptime pHigh=highVec[k][i].datetime;
    boost::posix_time::ptime pInter=timeList[i];


    low=unixTime(pLow);
    high=unixTime(pHigh);
    inter=unixTime(pInter);

    double lowCloud;
    double highCloud;
    double interCloud;

    lowCloud=lowVec[k][i].cloudCover;
    highCloud=highVec[k][i].cloudCover;

    interCloud=interpolator(inter,low,high,lowCloud,highCloud);
    if (interCloud>1.0)
    {
//        cout<<"cloud error"<<endl;
        interCloud=highCloud;
        if (interCloud>1.0)
        {
            interCloud=lowCloud;
        }
    }

//    cout<<lowCloud<<" "<<highCloud<<endl;
//    cout<<interCloud<<"\n\n"<<endl;

//    interpolatedWxData[k][i].set_cloudCover(interCloud,coverUnits::fraction);
    interpolatedWxData[k][i].cloudCover=interCloud;
    interpolatedWxData[k][i].cloudCoverUnits=coverUnits::percent;
    }

}

//cout<<interpolatedWxData.size()<<endl;
//cout<<interpolatedWxData[0].size()<<" , "<<interpolatedWxData[1].size()<<" , "<<interpolatedWxData[2].size()<<endl;
//cout<<"\n\n"<<endl;


    
return interpolatedWxData;
}
double pointInitialization::unixTime(boost::posix_time::ptime time)
{

    boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1));
    boost::posix_time::time_duration::sec_type  dNew= (time - epoch).total_seconds();

    double stepDuration;
    stepDuration=dNew;

//    printf("%lf\n",stepDuration);

    return stepDuration;
}

double pointInitialization::interpolator(double iPoint, double lowX, double highX, double lowY, double highY)
{
    double result;
    double work=0.00;

    double slope;
    slope=(highY-lowY)/(highX-lowX);
    double pointS;
    pointS=(iPoint-lowX);
    result=lowY+pointS*slope;

//    cout<<result<<endl;

    if (result<0.0000)
    {
        result=work;
    }


    return result;
}
//interpolateDirection uses a MEAN OF CIRCULAR QUANTITIES equation, converts from polar to cartesian, averages and then returns degrees
//see this wiki page https://en.wikipedia.org/wiki/Mean_of_circular_quantities
double pointInitialization::interpolateDirection(double lowDir,double highDir)
{
    double work=0.00;
    double one_eighty=180.000;
    double lowRad;
    lowRad=lowDir*PI/one_eighty;

    double highRad;
    highRad=highDir*PI/one_eighty;

    double sinSum;
    double cosSum;
    sinSum=sin(lowRad)+sin(highRad);
    cosSum=cos(lowRad)+cos(highRad);

    double average;
    average=atan2(sinSum,cosSum);

    double degAverage;

    degAverage=average*one_eighty/PI;

    if (degAverage<0)
    {
        degAverage=degAverage+360.000;
    }

    return degAverage;
}

string pointInitialization::BuildTime(std::string year_0,std::string month_0,
                                      std::string day_0,std::string clock_0,
                                      std::string year_1,std::string month_1,
                                      std::string day_1,std::string clock_1)
    {
    //builds the time component of a url
        std::string start;
        std::string end;
        std::string y20;
        std::string m20;
        std::string d20;
        std::string c20;
        std::string y21;
        std::string m21;
        std::string d21;
        std::string c21;
        std::string twofull;
        std::string timemainfull;
        std::string estartfull;
        std::string eendfull;
        std::string startfull;
        std::string endfull;

        start="&start=";
        y20="2016";
        m20="05";
        d20="22";
        c20="1000";
        estartfull=start+y20+m20+d20+c20;
        end="&end=";
        y21="2016";
        m21="05";
        d21="23";
        c21="1000";
        eendfull=end+y21+m21+d21+c21;

        twofull=estartfull+eendfull;

        startfull=start+year_0+month_0+day_0+clock_0;
        endfull=end+year_1+month_1+day_1+clock_1;

        timemainfull=startfull+endfull;

        return timemainfull;
    }
vector<string> pointInitialization::UnifyTime(vector<boost::posix_time::ptime> timeList)
{
    vector<string> buildTimes;
    vector<string> work;
    work.push_back("0");
    stringstream startstream;
    stringstream endstream;
    boost::posix_time::time_facet *facet=new boost::posix_time::time_facet("%Y%m%d%H%M");
    boost::posix_time::time_duration buffer(1,0,0,0);

    startstream.imbue(locale(startstream.getloc(),facet));
    timeList[0]=timeList[0]-buffer;
    startstream<<timeList[0];

    endstream.imbue(locale(endstream.getloc(),facet));
    timeList[timeList.size()-1]=timeList[timeList.size()-1]+buffer;
    endstream<<timeList[timeList.size()-1];

    std::string startString;
    std::string endString;
    startString=startstream.str();
    endString=endstream.str();

    std::string year_0=startString.substr(0,4);
    std::string month_0=startString.substr(4,2);
    std::string day_0=startString.substr(6,2);
    std::string clock_0=startString.substr(8);
    std::string year_1=endString.substr(0,4);
    std::string month_1=endString.substr(4,2);
    std::string day_1=endString.substr(6,2);
    std::string clock_1=endString.substr(8);
//    cout<<year_0<<" , "<<month_0<<" , "<<day_0<<" , "<<clock_0<<endl;
//    cout<<year_1<<" , "<<month_1<<" , "<<day_1<<" , "<<clock_1<<endl;

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

string pointInitialization::IntConvert(int a)
    {
    //converts int to string for "latest" functions
    ostringstream time;
    time<<a;
    return time.str();
    }
//Gets MetaData for stations if turned on
void pointInitialization::fetchMetaData(string fileName, string demFile, bool write)
{
    cout<<"Downloading Station MetaData..."<<endl;
    std::string baseurl="http://api.mesowest.net/v2/stations/metadata?";
    std::string bbox;
    std::string component="&network=1,2&output=geojson";
    std::string url;
    std::string tokfull;

    GDALDataset  *poDS;
    poDS = (GDALDataset *) GDALOpen(demFile.c_str(), GA_ReadOnly );

    double bounds[4];
    bool bRet;

    bRet=GDALGetBounds(poDS,bounds);

    std::string URLat;
    std::string URLon;
    std::string LLLat;
    std::string LLLon;



    URLat=CPLSPrintf("%.6f",bounds[0]);
    URLon=CPLSPrintf("%.6f",bounds[1]);
    LLLat=CPLSPrintf("%.6f",bounds[2]);
    LLLon=CPLSPrintf("%.6f",bounds[3]);
    // LLLAT LLLON URLAT URLON
    // 1 1 2 2

    bbox="&bbox="+LLLon+","+LLLat+","+URLon+","+URLat;
    tokfull="&token="+dtoken;
    url=baseurl+bbox+component+tokfull;

    std::string csvName;
    if (fileName.substr(fileName.size()-4,4)==".csv")
    {
    csvName=fileName;
//    cout<<".csv exists in metaDataFile"<<endl;
    }
    else
    {
    csvName=fileName+".csv";
//    cout<<"adding .csv to metaDataFile"<<endl;
    }
    ofstream outFile;
    if (write==true)
    {
    cout<<"writing MetaData for stations..."<<endl;
    outFile.open(csvName.c_str());
    std::string header="Station_name,STID,Latitude,Longitude,Elevation,Status,MnetID";
    outFile<<header<<endl;
    }


    OGRDataSourceH hDS;
    OGRLayerH hLayer;
    OGRFeatureH hFeature;

    hDS=OGROpen(url.c_str(),0,NULL);
    CPLGetLastErrorMsg();
    if (hDS==NULL)
    {

        cout<<"metadata bad"<<endl;
        exit(1);
    }

    hLayer=OGR_DS_GetLayer(hDS,0);
    OGR_L_ResetReading(hLayer);

    int fCount=OGR_L_GetFeatureCount(hLayer,1);

    int idx1=0;
    int idx2=0;
    int idx3=0;
    int idx4=0;
    int idx5=0;
    int idx6=0;
    int idx7=0;

    double latitude;
    double  longitude;
    const char* stid;
    const char* stationName;
    const char* status;
    int mnetID;
    const char* elevation;


    for (int ex=0; ex<fCount;ex++)
    {
        hFeature=OGR_L_GetFeature(hLayer,ex);

        idx1=OGR_F_GetFieldIndex(hFeature,"STID");
        stid=(OGR_F_GetFieldAsString(hFeature,idx1));

        idx2=OGR_F_GetFieldIndex(hFeature,"name");
        stationName=(OGR_F_GetFieldAsString(hFeature,idx2));

        idx3=OGR_F_GetFieldIndex(hFeature,"latitude");
        latitude=(OGR_F_GetFieldAsDouble(hFeature,idx3));

        idx4=OGR_F_GetFieldIndex(hFeature,"longitude");
        longitude=(OGR_F_GetFieldAsDouble(hFeature,idx4));

        idx5=OGR_F_GetFieldIndex(hFeature,"status");
        status=(OGR_F_GetFieldAsString(hFeature,idx5));

        idx6=OGR_F_GetFieldIndex(hFeature,"mnet_id");
        mnetID=(OGR_F_GetFieldAsInteger(hFeature,idx6));

        idx7=OGR_F_GetFieldIndex(hFeature,"elevation");
        elevation=(OGR_F_GetFieldAsString(hFeature,idx7));

//        cout<<stid<<endl;
//        cout<<stationName<<endl;
//        cout<<latitude<<endl;
//        cout<<longitude<<endl;
//        cout<<status<<endl;
//        cout<<mnetID<<endl;
//        cout<<elevation<<endl;
        if (write==true)
        {
            outFile<<stid<<",\""<<stationName<<"\","<<latitude<<","<<longitude<<","<<elevation<<","<<status<<","<<mnetID<<endl;
        }

    }
    OGR_DS_Destroy(poDS);
    OGR_DS_Destroy(hDS);
    delete stid,stationName,latitude,longitude,status,mnetID,elevation;
}

const char* pointInitialization::BuildSingleUrl(std::string token,
                                                std::string station_id,
                                                std::string svar,
                                                std::string yearx,
                                                std::string monthx,
                                                std::string dayx,
                                                std::string clockx,
                                                std::string yeary,
                                                std::string monthy,
                                                std::string dayy,
                                                std::string clocky)
    {
    //builds a url for a single timeseries station with specific start and stop times
       std::string etoken;
       std::string eburl;
       std::string etokfull;
       std::string estid;
       std::string et01;
       std::string et11;
       std::string esvar;
       std::string eurl;

       // default url

       etoken="33e3c8ee12dc499c86de1f2076a9e9d4";
       eburl="http://api.mesowest.net/v2/stations/timeseries?";
       etokfull="&token="+token;
       estid="stid=kmso";
       et01="&start=201605221000";
       et11="&end=201605231000";
       esvar="&vars=wind_speed";
       eurl=eburl+estid+esvar+et01+et11+etokfull;

       std::string network;
       std::string nEtworkFull;
       network="1,2";
       nEtworkFull="&network="+network;


       const char* a=eurl.c_str();

       std::string url;
       std::string tokfull;
       std::string stidfull;
       std::string svarfull;
       std::string timestart1;
       std::string timestop1;
       std::string timefull;
       std::string timesand;
       std::string output;

       timesand=pointInitialization::BuildTime(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky);

       tokfull="&token="+token;
       stidfull="stid="+station_id;
       svarfull="&vars="+svar;
       output="&output=geojson";


       url=eburl+stidfull+nEtworkFull+svarfull+timesand+output+tokfull;

       const char* charurl=url.c_str();

       return charurl;

    }

const char* pointInitialization::BuildSingleLatest(std::string token, std::string station_id,
                                                   std::string svar,
                                                   int past, bool extendnetwork,
                                                   std::string netids)
    {
    //builds a url for single time series with the latest data for the past n hours
        std::string eburl;
        std::string url;
        std::string tokfull;
        std::string stidfull;
        std::string svarfull;
        std::string timesand;
        std::string output;
        std::string pasthourstr;
        int pasthour;
        int hour=60;

        std::string network;
        std::string nEtworkFull;

        if (extendnetwork==true)
        {
        network="1,2"+netids;
        nEtworkFull="&network="+network;
        }
        else
        {
            network="1,2";
            nEtworkFull="&network="+network;
        }

        pasthour=past*hour;
        pasthourstr=pointInitialization::IntConvert(pasthour);

        eburl="http://api.mesowest.net/v2/stations/timeseries?";
        tokfull="&token="+token;
        stidfull="stid="+station_id;
        svarfull="&vars="+svar;
        output="&output=geojson";
        timesand="&recent="+pasthourstr;

        url=eburl+stidfull+nEtworkFull+svarfull+timesand+output+tokfull;

        const char* charurl=url.c_str();

        return charurl;
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
        std::string eburl;
        std::string url;
        std::string tokfull;
        std::string stidfull;
        std::string svarfull;
        std::string timesand;
        std::string output;

        std::string network;
        std::string nEtworkFull;
        network="1,2";
        nEtworkFull="&network="+network;


        timesand=pointInitialization::BuildTime(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky);

        eburl="http://api.mesowest.net/v2/stations/timeseries?";
        tokfull="&token="+dtoken;
        stidfull="stid="+station_ids;
        svarfull="&vars="+ndvar;
        output="&output=geojson";


        url=eburl+stidfull+nEtworkFull+svarfull+timesand+output+tokfull;

        const char* charurl=url.c_str();

        return url;
    }

std::string pointInitialization::BuildMultiLatest(std::string station_ids)
{
     //builds a url for multiple known stations for the latest n hours
        std::string eburl;
        std::string url;
        std::string tokfull;
        std::string stidfull;
        std::string svarfull;
        std::string timesand;
        std::string output;
        std::string pasthourstr="60";

        std::string network;
        std::string nEtworkFull;
        network="1,2";
        nEtworkFull="&network="+network;


        timesand="&recent="+pasthourstr;
        eburl="http://api.mesowest.net/v2/stations/timeseries?";
        tokfull="&token="+dtoken;
        stidfull="stid="+station_ids;
        svarfull="&vars="+ndvar;
        output="&output=geojson";


        url=eburl+stidfull+nEtworkFull+svarfull+timesand+output+tokfull;

        const char* charurl=url.c_str();

        return url;

    }
const char* pointInitialization::BuildRadiusLatest(std::string token,
                                                   std::string station_id,
                                                   std::string radius,
                                                   std::string limit,
                                                   std::string svar,int past)
    {
    //builds a url fetching all stations within x miles of a known station for last n hours specified

        std::string eburl;
        std::string url;
        std::string tokfull;
        std::string stidfull;
        std::string svarfull;
        std::string timesand;
        std::string output;
        std::string pasthourstr;
        std::string limiter;
        int pasthour;
        int hour=60;

        std::string network;
        std::string nEtworkFull;
        network="1,2";
        nEtworkFull="&network="+network;


        pasthour=past*hour;
        pasthourstr=pointInitialization::IntConvert(pasthour);

        timesand="&recent="+pasthourstr;
        eburl="http://api.mesowest.net/v2/stations/timeseries?";
        limiter="&limit="+limit;
        tokfull="&token="+token;
        stidfull="radius="+station_id+","+radius;
        svarfull="&vars="+svar;
        output="&output=geojson";

        url=eburl+stidfull+nEtworkFull+svarfull+limiter+timesand+output+tokfull;

        const char* charurl=url.c_str();

        return charurl;

    }

const char* pointInitialization::BuildRadiusUrl(std::string token,
                                                std::string station_id,
                                                std::string radius,
                                                std::string limit,
                                                std::string svar,
                                                std::string yearx,
                                                std::string monthx,
                                                std::string dayx,
                                                std::string clockx,
                                                std::string yeary,
                                                std::string monthy,
                                                std::string dayy,
                                                std::string clocky)
    {
    //builds a url for a radius around a known station with a specific start and stop time
        std::string eburl;
        std::string url;
        std::string tokfull;
        std::string stidfull;
        std::string svarfull;
        std::string timesand;
        std::string output;
        std::string limiter;

        std::string network;
        std::string nEtworkFull;
        network="1,2";
        nEtworkFull="&network="+network;


        timesand=pointInitialization::BuildTime(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky);

        eburl="http://api.mesowest.net/v2/stations/timeseries?";
        limiter="&limit="+limit;
        tokfull="&token="+token;
        stidfull="radius="+station_id+","+radius;
        svarfull="&vars="+svar;
        output="&output=geojson";


        url=eburl+stidfull+nEtworkFull+svarfull+limiter+timesand+output+tokfull;

        const char* charurl=url.c_str();

        return charurl;
    }
const char* pointInitialization::BuildLatLonUrl(std::string token,
                                                std::string lat,
                                                std::string lon,
                                                std::string radius,
                                                std::string limit,
                                                std::string svar,
                                                std::string yearx,
                                                std::string monthx,
                                                std::string dayx,
                                                std::string clockx,
                                                std::string yeary,
                                                std::string monthy,
                                                std::string dayy,
                                                std::string clocky)
    {
    //builds a url for a given latitude longitude location and grabs all stations within x miles and limited to y stations.
        std::string eburl;
        std::string url;
        std::string tokfull;
        std::string stidfull;
        std::string svarfull;
        std::string timesand;
        std::string output;
        std::string limiter;

        std::string network;
        std::string nEtworkFull;
        network="1,2";
        nEtworkFull="&network="+network;


        timesand=pointInitialization::BuildTime(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky);

        eburl="http://api.mesowest.net/v2/stations/timeseries?";
        limiter="&limit="+limit;
        tokfull="&token="+token;
        stidfull="&radius="+lat+","+lon+","+radius;
        svarfull="&vars="+svar;
        output="&output=geojson";


        url=eburl+stidfull+nEtworkFull+svarfull+limiter+timesand+output+tokfull;

        const char* charurl=url.c_str();

        return charurl;
    }

const char* pointInitialization::BuildLatLonLatest(std::string token,
                                                   std::string lat,
                                                   std::string lon,
                                                   std::string radius,
                                                   std::string limit,
                                                   std::string svar,
                                                   int past)
    {
    //builds a url for a given lat/lon coordinate for the most recent n Hours
        std::string eburl;
        std::string url;
        std::string tokfull;
        std::string stidfull;
        std::string svarfull;
        std::string timesand;
        std::string output;
        std::string limiter;
        std::string pasthourstr;

        std::string network;
        std::string nEtworkFull;
        network="1,2";
        nEtworkFull="&network="+network;


        int pasthour;
        int hour=60;

        pasthour=past*hour;
        pasthourstr=pointInitialization::IntConvert(pasthour);

        timesand="&recent="+pasthourstr;
        eburl="http://api.mesowest.net/v2/stations/timeseries?";
        limiter="&limit="+limit;
        tokfull="&token="+token;
        stidfull="&radius="+lat+","+lon+","+radius;
        svarfull="&vars="+svar;
        output="&output=geojson";


        url=eburl+stidfull+nEtworkFull+svarfull+limiter+timesand+output+tokfull;

        const char* charurl=url.c_str();

        return charurl;
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
        std::string eburl;
        std::string url;
        std::string tokfull;
        std::string bbox;
        std::string svarfull;
        std::string timesand;
        std::string output;

        std::string network;
        std::string nEtworkFull;
        network="1,2";
        nEtworkFull="&network="+network;


        timesand=pointInitialization::BuildTime(yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky);

        eburl="http://api.mesowest.net/v2/stations/timeseries?";
        tokfull="&token="+dtoken;
        bbox="&bbox="+lon1+","+lat1+","+lon2+","+lat2;
        svarfull="&vars="+ndvar;
        output="&output=geojson";


        url=eburl+bbox+nEtworkFull+svarfull+timesand+output+tokfull;

        const char* charurl=url.c_str();

        return url;
    }

std::string pointInitialization::BuildBboxLatest(std::string lat1,
                                                 std::string lon1,
                                                 std::string lat2,
                                                 std::string lon2)

{
    //builds a url for a bounding box within the latest n hours
        std::string eburl;
        std::string url;
        std::string tokfull;
        std::string bbox;
        std::string svarfull;
        std::string active="&status=active";
        std::string timesand;
        std::string output;
        std::string pasthourstr="60";


        timesand="&recent="+pasthourstr;
        eburl="http://api.mesowest.net/v2/stations/timeseries?";

        std::string network;
        std::string nEtworkFull;
        network="1,2";
        nEtworkFull="&network="+network;

        tokfull="&token="+dtoken;
        bbox="&bbox="+lon1+","+lat1+","+lon2+","+lat2;
        svarfull="&vars="+ndvar;
        output="&output=geojson";
        url=eburl+bbox+nEtworkFull+svarfull+active+timesand+output+tokfull;
//        cout<<url.length()<<endl;

        const char* charurl=url.c_str();

        return url;

    }
std::string pointInitialization::BuildUnifiedBbox(double lat1,double lon1, double lat2,double lon2,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky)
{
    std::string workUrl="WIP";
//    double dURLat=lat2;
//    double dURLon=lon2;
//    double dLLLat=lat1;
//    double dLLLon=lon1;
    std::string URLat;
    std::string URLon;
    std::string LLLat;
    std::string LLLon;

    URLat=CPLSPrintf("%.6f",lat2);
    URLon=CPLSPrintf("%.6f",lon2);
    LLLat=CPLSPrintf("%.6f",lat1);
    LLLon=CPLSPrintf("%.6f",lon1);



//    cout<<buffer<<endl;
//    exit(1);

//    cout<<URLat<<endl<<URLon<<endl<<LLLat<<endl<<LLLon<<endl;

    std::string URL;

    URL=BuildBboxUrl(LLLat,LLLon,URLat,URLon,yearx,monthx,dayx,clockx,yeary,monthy,dayy,clocky);

//    std::string crap;

//    crap=pointInitialization::BuildBboxLatest(LLLat,LLLon,URLat,URLon);

//    cout<<crap<<endl;
//    exit(1);

    return URL;
}

std::string pointInitialization::BuildUnifiedLTBbox(double lat1, double lon1, double lat2, double lon2)
{
    std::string URL;
    std::string URLat;
    std::string URLon;
    std::string LLLat;
    std::string LLLon;

    URLat=CPLSPrintf("%.6f",lat2);
    URLon=CPLSPrintf("%.6f",lon2);
    LLLat=CPLSPrintf("%.6f",lat1);
    LLLon=CPLSPrintf("%.6f",lon1);


    URL=BuildBboxLatest(LLLat,LLLon,URLat,URLon);

    return URL;
}


double pointInitialization::stationBuffer;

void pointInitialization::set_stationBuffer(double buffer, string units)
{

//    cout<<buffer<<endl;
//    cout<<units<<endl;

    if (units=="km" || units=="kilometers" || units=="kilometres")
    {
        lengthUnits::toBaseUnits(buffer,lengthUnits::kilometers);
    }
    if (units=="miles"||units=="mi")
    {
        lengthUnits::toBaseUnits(buffer,lengthUnits::miles);
    }
    if (units=="feet"||units=="ft")
    {
        lengthUnits::toBaseUnits(buffer,lengthUnits::feet);
    }
    if (units=="meters"||units=="m")
    {
        lengthUnits::toBaseUnits(buffer,lengthUnits::meters);
    }

    //    lengthUnits::toBaseUnits(buffer,units);
    stationBuffer=buffer;

}

double pointInitialization::get_stationBuffer()
{
    return stationBuffer;
}

vector<string> pointInitialization::Split(char* str,const char* delim)
    {
    //Splits strings into vectors of strings based on a delimiter, a "," is used for most functions
        char* saveptr;
        char* token = strtok_r(str,delim,&saveptr);

        vector<string> result;

        while(token != NULL)
        {
            result.push_back(token);
            token = strtok_r(NULL,delim,&saveptr);
        }
        return result;
    }
void pointInitialization::stringtoaster(int null,vector<int> vecnull)
{
cout<<null<<endl;
cout<<vecnull[0]<<endl;

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

vector<string> pointInitialization::InterpretCloudData(const double *dbCloud,int counter)
    {
    //converts NWS/FAA cloud information to %cloud cover for a single station


    std::string sa;
    std::string sb;
    std::ostringstream ss;
    std::ostringstream st;

    for(int i=0;i<counter;i++)
    {

        ss<<dbCloud[i]<<",";
        sa=(ss.str());
        //cout<<sa<<endl;
        //sb=sa.substr(sa.size()-2);

    }

    //cout<<sa<<endl;
    const char* delim(",");
    char *cloudstr=&sa[0u];
    vector<string> cloudcata;
    cloudcata=Split(cloudstr,delim);
    //VectorPrinter(cloudcata,"clouds");

    vector<string> cloudcatb;


    for(int i=0;i<counter;i++)
    {
        st<<cloudcata[i][cloudcata[i].size()-1]<<",";
        sb=(st.str());
    }

    //cout<<sb<<endl;
    char *cloudstrb=&sb[0u];
    cloudcatb=Split(cloudstrb,delim);
    //vectorprinter(cloudcatb,"mesowest cat is watching you");

    std::string few="25";
    std::string sct="50";
    std::string bkn="75";
    std::string ovc="100";
    std::string clr="0";
    std::string mesofew="6";
    std::string mesosct="2";
    std::string mesobkn="3";
    std::string mesoovc="4";
    std::string mesoclr="1";
    std::string mesonull="0";

    vector<string> lowclouddat;

    for(int i=0;i<counter;i++)
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



vector<vector<string> > pointInitialization::VectorInterpretCloudData(vector<const double*>dbCloud,
                                                                       int smallcount, int largecount)
        {
    //converts NWS/FAA cloud data to %cloud cover for any given number of stations
    // used by multi, radius, lat/lon,bbox

        vector<vector<string> > newcloudcat;

        for(int jj=0;jj<smallcount;jj++)
        {
            std::string sa;
            std::string sb;
            std::ostringstream ss;
            std::ostringstream st;
            vector<string> lowclouddat;

            for(int j=0;j<largecount;j++)
            {

                ss<<dbCloud[jj][j]<<",";
                sa=(ss.str());
                //cout<<sa<<endl;
                //sb=sa.substr(sa.size()-2);
            }

         //cout<<sa<<endl;
         const char* delim(",");
         char *cloudstr=&sa[0u];
         vector<string> cloudcata;
         cloudcata=Split(cloudstr,delim);

         //VectorPrinter(cloudcata,"clouds");

         std::string clouda;
         vector<string> cloudcatb;
         int counter=(largecount*smallcount);

         for(int k=0;k<largecount;k++)
         {
             st<<cloudcata[k][cloudcata[k].size()-1]<<",";
             sb=(st.str());
         }

         //cout<<sb<<endl;
         char *cloudstrb=&sb[0u];
         cloudcatb=Split(cloudstrb,delim);
         //VectorPrinter(cloudcatb,"mesowest cat is watching you");

         std::string few="25";
         std::string sct="50";
         std::string bkn="75";
         std::string ovc="100";
         std::string clr="0";
         std::string mesofew="6";
         std::string mesosct="2";
         std::string mesobkn="3";
         std::string mesoovc="4";
         std::string mesoclr="1";


         std::ostringstream su;
         std::string sc;

         for(int l=0;l<largecount;l++)
         {

             if (cloudcatb[l]==mesofew)
             {
                 lowclouddat.push_back(few);
             }
             if (cloudcatb[l]==mesosct)
             {
                 lowclouddat.push_back(sct);
             }
             if (cloudcatb[l]==mesobkn)
             {
                 lowclouddat.push_back(bkn);
             }
             if (cloudcatb[l]==mesoovc)
             {
                 lowclouddat.push_back(ovc);
             }
             if (cloudcatb[l]==mesoclr)
             {
                 lowclouddat.push_back(clr);
             }
         }
         //VectorPrinter(lowclouddat,"lowclouds");
         newcloudcat.push_back(lowclouddat);


        }

//        VectorPrinter(newcloudcat[0],"set 1");
//        VectorPrinter(newcloudcat[1],"set 2");

        return newcloudcat;
}

vector<string> pointInitialization::CompareClouds(vector<string>low,vector<string>med,vector<string>high,int countlow,int countmed,int counthigh)
{
    vector<string> work;
    work.push_back("wip");

    vector<int> lowcloudint;
    vector<int> medcloudint;
    vector<int> highcloudint;
    vector<int> overcloud;
    vector<string> totalCloudcat;
    std::string aa;

//    cout<<low.size()<<endl;
//     VectorPrinter(low,"low");
//     VectorPrinter(med,"med");
//     VectorPrinter(high,"high");

    for (int ti=0;ti<countlow;ti++)
    {
        int numa=atoi(low.at(ti).c_str());
        lowcloudint.push_back(numa);
    }
    for (int ti=0;ti<countmed;ti++)
    {
        int numb=atoi(med.at(ti).c_str());
        medcloudint.push_back(numb);
    }
    for (int ti=0;ti<counthigh;ti++)
    {
        int numc=atoi(high.at(ti).c_str());
        highcloudint.push_back(numc);
    }


    for (int ti=0;ti<countlow;ti++)
    {

    if (lowcloudint[ti]>=medcloudint[ti] && lowcloudint[ti]>=highcloudint[ti])
    {
        overcloud.push_back(lowcloudint[ti]);
        continue;
    }
    if (highcloudint[ti]>=medcloudint[ti] && highcloudint[ti]>=lowcloudint[ti])
    {
        overcloud.push_back(highcloudint[ti]);
        continue;
    }
    if (medcloudint[ti]>=lowcloudint[ti] && medcloudint[ti]>=highcloudint[ti])
    {
        overcloud.push_back(medcloudint[ti]);
        continue;
    }

    }
//for (int ti=0;ti<countlow;ti++)
//{
//    printf("%d, ",overcloud[ti]);
//}
int overLen=overcloud.size();

std::string cloudstring;
std::ostringstream cs;

for(int i=0;i<overLen;i++)
{

    cs<<overcloud[i]<<",";
    cloudstring=(cs.str());

}
const char* delim(",");
char *cloudstr=&cloudstring[0u];
totalCloudcat=Split(cloudstr,delim);
//VectorPrinter(totalCloudcat,"total");

return totalCloudcat;
}

vector<string> pointInitialization::UnifyClouds(const double *dvCloud,const double *dwCloud,const double *dxCloud,int count1,int count2,int count3,int backupcount)
{
    vector<string> work;
    work.push_back("wip");
    vector<string> daCloud;
    vector<string> dcCloud;
    vector<string> deCloud;
    vector<string> sCloudData;

    if(count1==0)
    {
       cout<<"no cloud data exists, using air temp count to zero out data"<<endl;
       count1=backupcount;
       if (backupcount==0)
       {
           cout<<"this station is terrible, don't use it, writing data as -9999"<<endl;
           sCloudData.push_back("-9999");
       }
    }
    if(count2==0)
    {
        count2=count1;
    }
    if (count3==0)
    {
        count3=count1;
    }

    if (dvCloud==0)
    {
    for(int ska=0;ska<count1;ska++)
    {
        daCloud.push_back("0");
    }
//            VectorPrinter(daCloud,"1");
    }
    else
    {
        daCloud=InterpretCloudData(dvCloud,count1);
    }
    if (dwCloud==0)
    {
        for(int ska=0;ska<count2;ska++)
        {
            dcCloud.push_back("0");
        }
//            VectorPrinter(dcCloud,"2");
    }
    else
    {
        dcCloud=InterpretCloudData(dwCloud,count2);
    }
    if (dxCloud==0)
    {
        for(int ska=0;ska<count3;ska++)
        {
            deCloud.push_back("0");
        }
//            VectorPrinter(deCloud,"3");
    }
    else
    {
        deCloud=InterpretCloudData(dxCloud,count3);
    }

    sCloudData=CompareClouds(daCloud,dcCloud,deCloud,count1,count2,count3);
//    VectorPrinter(sCloudData,"combo");




return sCloudData;

}

void pointInitialization::StringPrinter(char **stringdat, int counter, std::string name)
    {
    //prints a list of strings as a string with a comma delim
        cout<<name<<endl;
        for(int i=0;i<counter;i++)
        {
            printf("%s, ",stringdat[i]);
        }
        printf("\n");
        cout<<"count of char: "<<counter<<endl;
        printf("\n");


    }


void pointInitialization::FloatPrinter(const double *data, int counter,std::string name)
    {
    //prints a list of floating point numbers with a comma delim
            cout<<name<<endl;
            for(int i=0;i<counter;i++)
            {
                printf("%.3f, ",data[i]);
            }
            printf("\n");
            cout<<"count of doubles: "<<counter<<endl;
            printf("\n");

    }
void pointInitialization::VectorPrinter(std::vector<std::string> stData,std::string name)
{
    //prints a vector of strings with a comma delim
       cout<<name<<endl;
       int i=0;
       for (std::vector<string>::const_iterator i = stData.begin(); i != stData.end(); ++i)
           std::cout << *i << ',';

       printf("\n");
       int veclen;
       veclen=stData.size();
       cout<<"length of vector "<<veclen<<endl;
       printf("\n");
}

void pointInitialization::doubleVectorPrinter(vector<const double*> stData,std::string name, int counter)
{
    //prints a vector of floats witha  comma delimiter
    cout<<name<<endl;
    int i=0;
    int veclen;
    veclen=stData.size();
    for (int k=0;k<veclen;k++)
    {
        const double* data;
        data=stData[k];
        for(int j=0;j<counter;j++)
        {
            printf("%.3f, ",data[j]);
        }
        printf("\n");
        cout<<"set:"<<k<<endl;
        cout<<"count of doubles: "<<counter<<endl;
        printf("\n");
    }



    printf("\n");

    cout<<"length of vector "<<veclen<<endl;
    printf("\n");



}

vector<double> pointInitialization::Irradiate(const double* solrad,int smallcount, int largecount,std::string timeZone,double lat, double lon,char** times)
{
    //will eventually convert solar radiation to cloud cover, this function doesn't work yet.

vector<double> blegh;
vector<double> outCloud;
double work=0.000;
blegh.push_back(work);

//cout<<solrad[0]<<endl;
//cout<<largecount<<endl;
//cout<<lat<<endl;
//cout<<lon<<endl;
//cout<<times[0]<<endl;

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
//        cout<<abs_time<<endl;

//        cout<<timeZone<<endl;

        boost::local_time::tz_database tz_db;
        tz_db.load_from_file( FindDataPath("date_time_zonespec.csv") );
        boost::local_time::time_zone_ptr timeZonePtr;
        timeZonePtr = tz_db.time_zone_from_region(timeZone);

        boost::local_time::local_date_time startLocal(abs_time,timeZonePtr);


//        cout<<startLocal<<endl;

        double zero=0.000000;
        double one=1.0000000;

        silver=sol.compute_solar(startLocal,lat,lon,zero,zero);

        double senor=sol.get_solarIntensity();
        double solFrac;


//        cout<<startLocal<<endl;
////        cout<<" "<<solrad[j]<<" "<<senor<<endl;
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
//        cout<<solFrac<<endl;
//        cout<<"\n"<<endl;
        solFrac=100*solFrac;
        outCloud.push_back(solFrac);
//        cout<<solFrac<<endl;


    }

    //    exit(1);
//    for (int p;p<outCloud.size();p++)
//    {
//        cout<<outCloud[p]<<endl;
//    }


return outCloud;
}
void pointInitialization::UnifyRadiation(vector<double> radiation)
{

}

vector<string> pointInitialization::fixWindDir(const double *winddir,std::string filler,int count)
{
    std::string sa;
    std::ostringstream ss;
    vector<string>direction;

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

void pointInitialization::fetchSingleStation( bool type,
                                              int nHours,
                                              std::string station_id,
                                              std::string yeara,
                                              std::string montha,
                                              std::string daya,
                                              std::string clocka,
                                              std::string yearb,
                                              std::string monthb,
                                              std::string dayb,
                                              std::string clockb)
    {

    const char* lmUrl;

    if(type==true)
    {
        lmUrl=pointInitialization::BuildSingleLatest(dtoken,station_id,
                                                     ndvar,nHours,false,"0");
        //builds url for past n Hours
    }

    else
    {
        //builds url with given time period
        //const char* pszvtry=pointInitialization::BuildSingleUrl(dtoken,altstation,"wind_speed,wind_direction,air_temp,solar_radiation,cloud_layer_1_code","2016","06","01","1200","2016","06","02","1200");
        lmUrl=pointInitialization::BuildSingleUrl(dtoken,station_id,ndvar,
                                                  yeara,montha,daya,clocka,
                                                  yearb,monthb,dayb,clockb);
    }

    //cout<<lmUrl<<endl;

    //printf("\n\n");

    OGRDataSourceH hDS;
    OGRLayerH hLayer;
    OGRFeatureH hFeature;

    hDS=OGROpen(lmUrl,0,NULL);//fetches url using OGR
//    hDS=GDALOpenEx(pszvtry,GDAL_OF_ALL,NULL,NULL,NULL);
    if (hDS==NULL)
    {
        printf("miserable failure \n");
        printf("likely causes:\n station outside network\n no data for station\n mesowest is offline\n");
        exit(1);
    }


//    hLayer = OGR_DS_GetLayerByName(hDS,"OGRGeoJSON");
    hLayer=OGR_DS_GetLayer(hDS,0);
    OGR_L_ResetReading(hLayer);

    const char* csvname="single.csv";

    int idx0=0;
    int idx=0;
    int idx2=0;
    int idx3=0;
    int idx4=0;
    int idx5=0;
    int idx6=0;
    int idx7=0;
    int idx8=0;
    int idx9=0;
    int count1=0;
    int count2=0;
    int count3=0;
    int count4=0;
    int count5=0;

    int idxx1=0;
    int idxx2=0;
    int idxx3=0;
    int countxx1=0;
    int countxx2=0;
    int countxx3=0;
    const double *cloudlow;
    const double *cloudmed;
    const double *cloudhigh;

    const double *windspd;
    const double *winddir;
    const double *airtemp;
    const double *solrad;
    const double *cloud;
    double latitude;
    double longitude;
    std::string station;
    char **datetime;

    int mnetid;



    //saves data to lists of doubles or strings

    while((hFeature=OGR_L_GetNextFeature(hLayer))!=NULL)
    {
        idx0=OGR_F_GetFieldIndex(hFeature,"mnet_id");
        mnetid=(OGR_F_GetFieldAsInteger(hFeature,idx0));

        idx=OGR_F_GetFieldIndex(hFeature,"wind_speed");
        windspd=OGR_F_GetFieldAsDoubleList(hFeature,idx,&count1);

        idx2=OGR_F_GetFieldIndex(hFeature,"wind_direction");
        winddir=OGR_F_GetFieldAsDoubleList(hFeature,idx2,&count2);

        idx3=OGR_F_GetFieldIndex(hFeature,"air_temp");
        airtemp=OGR_F_GetFieldAsDoubleList(hFeature,idx3,&count3);

        if (mnetid==2)
        {
        idx4=OGR_F_GetFieldIndex(hFeature,"solar_radiation");
        solrad=OGR_F_GetFieldAsDoubleList(hFeature,idx4,&count4);
        }
        if (mnetid==1)
        {

        idxx1=OGR_F_GetFieldIndex(hFeature,"cloud_layer_1_code");
        cloudlow=OGR_F_GetFieldAsDoubleList(hFeature,idxx1,&countxx1);

        idxx2=OGR_F_GetFieldIndex(hFeature,"cloud_layer_2_code");
        cloudmed=OGR_F_GetFieldAsDoubleList(hFeature,idxx2,&countxx2);

        idxx3=OGR_F_GetFieldIndex(hFeature,"cloud_layer_3_code");
        cloudhigh=OGR_F_GetFieldAsDoubleList(hFeature,idxx3,&countxx3);
        }

        idx5=OGR_F_GetFieldIndex(hFeature, "LATITUDE");
        latitude=OGR_F_GetFieldAsDouble(hFeature,idx5);

        idx6=OGR_F_GetFieldIndex(hFeature,"LONGITUDE");
        longitude=OGR_F_GetFieldAsDouble(hFeature,idx6);

        idx7=OGR_F_GetFieldIndex(hFeature,"STID");
        station=OGR_F_GetFieldAsString(hFeature,idx7);

        idx8=OGR_F_GetFieldIndex(hFeature,"date_times");
        datetime=OGR_F_GetFieldAsStringList(hFeature,idx8);
        //note: Mesowest data downloaded as GEOJson names date and time data
        //as "date_times", Mewsowest data using
        // Json names time data as date_time!!!!!!


    }
    //printf("\n\n");


//        cout<<mnetid<<endl;


    vector<string> windDirection;
    windDirection=fixWindDir(winddir,"0",count1);



//        FloatPrinter(windspd,count1,"wind_speed");
//        FloatPrinter(winddir,count2,"wind_direction");
//        FloatPrinter(airtemp,count3,"air_temp");

    int dinfluence=-1;
    std::string diu="kilometres";
    cout<<"data saved to: "<<csvname<<endl;
    std::string baddata="-9999";
    vector<string> cloudData;


    ofstream outFile;
    outFile.open(csvname);
    outFile << "Station_ID,Coord_Sys,DATUM(WGS84),Lat/YCoord,Lon/XCoord,Height,Height_Units,Speed,Speed_Units,Direction(degrees),Tempertaure,Temperature_Units,Cloud_Cover(%),Radius_of_influence,Radius_of_influence_Units,date_time"<<endl;

if (mnetid==1)
{

//        solrad2=InterpretCloudData(solrad,count4);
        cloudData=UnifyClouds(cloudlow,cloudmed,cloudhigh,countxx1,countxx2,countxx3,count3);


        if (cloudlow==0)// if airport lacks any cloud data due to bad instrumentation or something
        {
            for(int q=0;q<count1;q++)
            {
                outFile<<station<<",GEOGCS,"<<"WGS84,"<<latitude<<","<<longitude<<",10,"<<"m,"<<windspd[q]<<",m/s,"<<windDirection[q]<<","<<airtemp[q]<<",C,"<<baddata<<","<<dinfluence<<","<<diu<<","<<datetime[q]<<endl;
            }
        }
        else
        {
            for(int q=0;q<count1;q++)
            {
                outFile<<station<<",GEOGCS,"<<"WGS84,"<<latitude<<","<<longitude<<",10,"<<"m,"<<windspd[q]<<",m/s,"<<windDirection[q]<<","<<airtemp[q]<<",C,"<<cloudData[q]<<","<<dinfluence<<","<<diu<<","<<datetime[q]<<endl;
            }
        }
}
if (mnetid==2)
{
        if (solrad==0)// if RAWS lacks any cloud data due to bad instrumentation or something
        {
            for(int q=0;q<count1;q++)
            {
                outFile<<station<<",GEOGCS,"<<"WGS84,"<<latitude<<","<<longitude<<",10,"<<"m,"<<windspd[q]<<",m/s,"<<windDirection[q]<<","<<airtemp[q]<<",C,"<<baddata<<","<<dinfluence<<","<<diu<<","<<datetime[q]<<endl;
            }
        }
        else
        {
            for(int q=0;q<count1;q++)
            {
                outFile<<station<<",GEOGCS,"<<"WGS84,"<<latitude<<","<<longitude<<",10,"<<"m,"<<windspd[q]<<",m/s,"<<windDirection[q]<<","<<airtemp[q]<<",C,"<<solrad[q]<<","<<dinfluence<<","<<diu<<","<<datetime[q]<<endl;
            }
        }
}
}

void pointInitialization::fetchMultiStation(bool type,
                                            int nHours,
                                            std::string station_ids,
                                            std::string yeara,
                                            std::string montha, std::string daya,
                                            std::string clocka,std::string yearb,
                                            std::string monthb,std::string dayb,
                                            std::string clockb)
    {
    //fetches data for multiple known stations
        const char* lmUrl;
        if(type==true)//build url based on desired info
        {
//            lmUrl=pointInitialization::BuildMultiLatest(dtoken,station_ids,ndvar,nHours);
        }

        else
        {
//            lmUrl=pointInitialization::BuildMultiUrl(dtoken,station_ids,
//                                                     ndvar,yeara,montha,
//                                                     daya,clocka,yearb,
//                                                     monthb,dayb,clockb);
        }

        //cout<<lmUrl<<endl;

        OGRDataSourceH hDS;
        OGRLayerH hLayer;
        OGRFeatureH hFeature;

        hDS=OGROpen(lmUrl,0,NULL); //fetches data
        //    hDS=GDALOpenEx(pszvtry,GDAL_OF_ALL,NULL,NULL,NULL);
        if (hDS==NULL)
            printf("I am broken, so very broken \n");
        //printf("\n\n");

        //    hLayer = OGR_DS_GetLayerByName(hDS,"OGRGeoJSON");

        int spam=OGR_DS_GetLayerCount(hDS);
//        cout<<"layercount "<<spam<<endl;
        hLayer=OGR_DS_GetLayer(hDS,0);
        OGR_L_ResetReading(hLayer);

            int idx=0;
            int idx0=0;
            int idx1=0;
            int idx2=0;
            int idx3=0;
            int idx4=0;
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
            int idx17=0;
            int idx18=0;
            int idx19=0;
            int idx20=0;

            int idxx1=0;
            int idxx2=0;
            int idxx3=0;
            const double *cloudlow;
            const double *cloudmed;
            const double *cloudhigh;

            vector<int> mnetid;

            const double* rawsWind;
            const double* rawsDir;
            const double* rawsSolrad;
            const double* rawsTemp;
            double rawsLatitude;
            double rawsLongitude;
            const char* rawsStation;
            char** rawsDateTime;

            const double* metarWind;
            const double* metarDir;
            const double* metarTemp;
            const double* metarCloud;
            double metarLatitude;
            double  metarLongitude;
            const char* metarStation;
            char** metarDateTime;


            const char* csvname="multi.csv";

            const char* newspam=OGR_L_GetName(hLayer);
//            cout<<"layer name: "<<newspam<<endl;
            int fCount=OGR_L_GetFeatureCount(hLayer,1);
//            cout<<"feature count: "<<fCount<<endl;

            //parses data based on mesonet id, 1=NWS/FAA, 2=RAWS

            ofstream outFile;//writing to csv
            outFile.open(csvname);
            cout<<fCount<<" stations "<<"saved to: "<<csvname<<endl;
            outFile << "Station_ID,Coord_Sys,DATUM(WGS84),Lat/YCoord,Lon/XCoord,Height,Height_Units,Speed,Speed_Units,Direction(degrees),Tempertaure,Temperature_Units,Cloud_Cover(%),Radius_of_influence,Radius_of_influence_Units,date_time"<<endl;


            for (int ex=0;ex<fCount;ex++)
            {

                hFeature=OGR_L_GetFeature(hLayer,ex);

                idx=OGR_F_GetFieldIndex(hFeature,"mnet_id");
                mnetid.push_back(OGR_F_GetFieldAsInteger(hFeature,idx));

                if (mnetid[ex]==1) //METAR station uses cloud data
                {
                    int count1=0;
                    int count2=0;
                    int count3=0;
                    int count4=0;
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

                    vector<string>cloudkappa;
                    cloudkappa=UnifyClouds(cloudlow,cloudmed,cloudhigh,countxx1,countxx2,countxx3,count3);
                    vector<string>metarWindDirection;
                    metarWindDirection=fixWindDir(metarDir,"0",count1);


                    for(int ez=0;ez<count1;ez++)
                    {
                     outFile<<metarStation<<",GEOGCS,"<<"WGS84,"<<metarLatitude<<","<<metarLongitude<<",10,"<<"m,"<<metarWind[ez]<<",m/s,"<<metarWindDirection[ez]<<","<<metarTemp[ez]<<",C,"<<cloudkappa[ez]<<","<<"-1,"<<"kilometres,"<<metarDateTime[ez]<<endl;
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

                    const double* aZero;
                    aZero=0;
                    std::string baddata="-9999";
                    vector<string>rawsWindDirection;
                    rawsWindDirection=fixWindDir(rawsDir,"0",count9);


                        for (int ez=0;ez<count9;ez++)
                        {
                        if (rawsSolrad==aZero)
                        {
                            outFile<<rawsStation<<",GEOGCS,"<<"WGS84,"<<rawsLatitude<<","<<rawsLongitude<<",10,"<<"m,"<<rawsWind[ez]<<",m/s,"<<rawsWindDirection[ez]<<","<<rawsTemp[ez]<<",C,"<<baddata<<","<<"-1,"<<"kilometres,"<<rawsDateTime[ez]<<endl;
                        }
                        else
                        {
                            outFile<<rawsStation<<",GEOGCS,"<<"WGS84,"<<rawsLatitude<<","<<rawsLongitude<<",10,"<<"m,"<<rawsWind[ez]<<",m/s,"<<rawsWindDirection[ez]<<","<<rawsTemp[ez]<<",C,"<<rawsSolrad[ez]<<","<<"-1,"<<"kilometres,"<<rawsDateTime[ez]<<endl;
                        }

                        }

        }
        }
    }

void pointInitialization::fetchPointRadiusStation(bool type,
                                                  int nHours, std::string station_id,
                                                  std::string radius,
                                                  std::string limit,
                                                  std::string yeara,
                                                  std::string montha,
                                                  std::string daya,
                                                  std::string clocka,
                                                  std::string yearb,
                                                  std::string monthb,
                                                  std::string dayb,std::string clockb)
    {
    //fetches data for known station and radius around it

        const char* lmUrl;
        if(type==true)//builds url
        {

            lmUrl=pointInitialization::BuildRadiusLatest(dtoken,station_id,radius,limit,ndvar,nHours);
//            lmUrl=pointInitialization::BuildMultiLatest(token,station_id,svar,nHours);

        }

        else
        {
            lmUrl=pointInitialization::BuildRadiusUrl(dtoken,station_id,
                                                      radius,limit,ndvar,
                                                      yeara,montha,daya,
                                                      clocka,yearb,monthb,
                                                      dayb,clockb);
        }

        //cout<<lmUrl<<endl;

        OGRDataSourceH hDS;
        OGRLayerH hLayer;
        OGRFeatureH hFeature;

        const char* csvname="point.csv";


        hDS=OGROpen(lmUrl,0,NULL);//fetches data
        //    hDS=GDALOpenEx(pszvtry,GDAL_OF_ALL,NULL,NULL,NULL);
        if (hDS==NULL)
            printf("I am broken, so very broken \n");
        printf("\n\n");

        //    hLayer = OGR_DS_GetLayerByName(hDS,"OGRGeoJSON");
        hLayer=OGR_DS_GetLayer(hDS,0);
        OGR_L_ResetReading(hLayer);

        int idx=0;
        int idx1=0;
        int idx2=0;
        int idx3=0;
        int idx4=0;
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

        vector<int> mnetid;

        int idxx1=0;
        int idxx2=0;
        int idxx3=0;
        const double *cloudlow;
        const double *cloudmed;
        const double *cloudhigh;

        const double* rawsWind;
        const double* rawsDir;
        const double* rawsSolrad;
        const double* rawsTemp;
        double rawsLatitude;
        double rawsLongitude;
        const char* rawsStation;
        char** rawsDateTime;

        const double* metarWind;
        const double* metarDir;
        const double* metarTemp;
        const double* metarCloud;
        double metarLatitude;
        double  metarLongitude;
        const char* metarStation;
        char** metarDateTime;




        int fCount=OGR_L_GetFeatureCount(hLayer,1);

        ofstream outFile;//writing to csv
        outFile.open(csvname);
        cout<<fCount<<" stations "<<"saved to: "<<csvname<<endl;
        outFile << "Station_ID,Coord_Sys,DATUM(WGS84),Lat/YCoord,Lon/XCoord,Height,Height_Units,Speed,Speed_Units,Direction(degrees),Tempertaure,Temperature_Units,Cloud_Cover(%),Radius_of_influence,Radius_of_influence_Units,date_time"<<endl;

//        cout<<fCount<<endl;
        for (int ex=0;ex<fCount;ex++)
        {

            hFeature=OGR_L_GetFeature(hLayer,ex);

            idx=OGR_F_GetFieldIndex(hFeature,"mnet_id");
            mnetid.push_back(OGR_F_GetFieldAsInteger(hFeature,idx));

            if (mnetid[ex]==1) //METAR station uses cloud data
            {
                int count1=0;
                int count2=0;
                int count3=0;
                int count4=0;
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

                vector<string>cloudkappa;
                cloudkappa=UnifyClouds(cloudlow,cloudmed,cloudhigh,countxx1,countxx2,countxx3,count3);
                vector<string>metarWindDirection;
                metarWindDirection=fixWindDir(metarDir,"0",count1);



                for(int ez=0;ez<count1;ez++)
                {
                 outFile<<metarStation<<",GEOGCS,"<<"WGS84,"<<metarLatitude<<","<<metarLongitude<<",10,"<<"m,"<<metarWind[ez]<<",m/s,"<<metarWindDirection[ez]<<","<<metarTemp[ez]<<",C,"<<cloudkappa[ez]<<","<<"-1,"<<"kilometres,"<<metarDateTime[ez]<<endl;
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

                const double* aZero;
                aZero=0;
                std::string baddata="-9999";
                vector<string>rawsWindDirection;
                rawsWindDirection=fixWindDir(rawsDir,"0",count9);


                    for (int ez=0;ez<count9;ez++)
                    {
                    if (rawsSolrad==aZero)
                    {
                        outFile<<rawsStation<<",GEOGCS,"<<"WGS84,"<<rawsLatitude<<","<<rawsLongitude<<",10,"<<"m,"<<rawsWind[ez]<<",m/s,"<<rawsWindDirection[ez]<<","<<rawsTemp[ez]<<",C,"<<baddata<<","<<"-1,"<<"kilometres,"<<rawsDateTime[ez]<<endl;
                    }
                    else
                    {
                        outFile<<rawsStation<<",GEOGCS,"<<"WGS84,"<<rawsLatitude<<","<<rawsLongitude<<",10,"<<"m,"<<rawsWind[ez]<<",m/s,"<<rawsWindDirection[ez]<<","<<rawsTemp[ez]<<",C,"<<rawsSolrad[ez]<<","<<"-1,"<<"kilometres,"<<rawsDateTime[ez]<<endl;
                    }

                    }

    }
    }
}



void pointInitialization::fetchLatLonStation(bool type,
                                             int nHours, std::string lat,
                                             std::string lon, std::string radius,
                                             std::string limit,
                                             std::string yeara,std::string montha,
                                             std::string daya,std::string clocka,
                                             std::string yearb,std::string monthb,
                                             std::string dayb,std::string clockb)
    {
    //fetches data based on a coordinate and radius

        const char* lmUrl;
        if (type==true)
        {

            lmUrl=pointInitialization::BuildLatLonLatest(dtoken,lat,lon,radius,limit,ndvar,nHours);

        }

        else
        {
            lmUrl=pointInitialization::BuildLatLonUrl(dtoken,lat,lon,radius,limit,ndvar,yeara,montha,daya,clocka,yearb,monthb,dayb,clockb);
        }

        //cout<<lmUrl<<endl;

        OGRDataSourceH hDS;
        OGRLayerH hLayer;
        OGRFeatureH hFeature;

        const char* csvname="latlon.csv";

        hDS=OGROpen(lmUrl,0,NULL);//reading data
        //    hDS=GDALOpenEx(pszvtry,GDAL_OF_ALL,NULL,NULL,NULL);
        if (hDS==NULL)
            printf("I am broken, so very broken \n");
        printf("\n\n");

        //    hLayer = OGR_DS_GetLayerByName(hDS,"OGRGeoJSON");
        hLayer=OGR_DS_GetLayer(hDS,0);
        OGR_L_ResetReading(hLayer);

        int idx=0;
        int idx1=0;
        int idx2=0;
        int idx3=0;
        int idx4=0;
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

        vector<int> mnetid;

        int idxx1=0;
        int idxx2=0;
        int idxx3=0;
        const double *cloudlow;
        const double *cloudmed;
        const double *cloudhigh;

        const double* rawsWind;
        const double* rawsDir;
        const double* rawsSolrad;
        const double* rawsTemp;
        double rawsLatitude;
        double rawsLongitude;
        const char* rawsStation;
        char** rawsDateTime;

        const double* metarWind;
        const double* metarDir;
        const double* metarTemp;
        const double* metarCloud;
        double metarLatitude;
        double  metarLongitude;
        const char* metarStation;
        char** metarDateTime;

        int fCount=OGR_L_GetFeatureCount(hLayer,1);

        //parsing

        ofstream tsetseWrite;//writing to csv
        tsetseWrite.open(csvname);
        cout<<fCount<<" stations "<<"saved to: "<<csvname<<endl;
        tsetseWrite << "Station_ID,Coord_Sys,DATUM(WGS84),Lat/YCoord,Lon/XCoord,Height,Height_Units,Speed,Speed_Units,Direction(degrees),Tempertaure,Temperature_Units,Cloud_Cover(%),Radius_of_influence,Radius_of_influence_Units,date_time"<<endl;


        for (int ex=0;ex<fCount;ex++)
        {

            hFeature=OGR_L_GetFeature(hLayer,ex);

            idx=OGR_F_GetFieldIndex(hFeature,"mnet_id");
            mnetid.push_back(OGR_F_GetFieldAsInteger(hFeature,idx));

            if (mnetid[ex]==1) //METAR station uses cloud data
            {
                int count1=0;
                int count2=0;
                int count3=0;
                int count4=0;
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

                vector<string>cloudkappa;
                cloudkappa=UnifyClouds(cloudlow,cloudmed,cloudhigh,countxx1,countxx2,countxx3,count3);
                vector<string>metarWindDirection;
                metarWindDirection=fixWindDir(metarDir,"0",count1);


                for(int ez=0;ez<count1;ez++)
                {
                 tsetseWrite<<metarStation<<",GEOGCS,"<<"WGS84,"<<metarLatitude<<","<<metarLongitude<<",10,"<<"m,"<<metarWind[ez]<<",m/s,"<<metarWindDirection[ez]<<","<<metarTemp[ez]<<",C,"<<cloudkappa[ez]<<","<<"-1,"<<"kilometres,"<<metarDateTime[ez]<<endl;
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

                const double* aZero;
                aZero=0;
                std::string baddata="-9999";
                vector<string>rawsWindDirection;
                rawsWindDirection=fixWindDir(rawsDir,"0",count9);


                    for (int ez=0;ez<count9;ez++)
                    {
                    if (rawsSolrad==aZero)
                    {
                        tsetseWrite<<rawsStation<<",GEOGCS,"<<"WGS84,"<<rawsLatitude<<","<<rawsLongitude<<",10,"<<"m,"<<rawsWind[ez]<<",m/s,"<<rawsWindDirection[ez]<<","<<rawsTemp[ez]<<",C,"<<baddata<<","<<"-1,"<<"kilometres,"<<rawsDateTime[ez]<<endl;
                    }
                    else
                    {
                        tsetseWrite<<rawsStation<<",GEOGCS,"<<"WGS84,"<<rawsLatitude<<","<<rawsLongitude<<",10,"<<"m,"<<rawsWind[ez]<<",m/s,"<<rawsWindDirection[ez]<<","<<rawsTemp[ez]<<",C,"<<rawsSolrad[ez]<<","<<"-1,"<<"kilometres,"<<rawsDateTime[ez]<<endl;
                    }

                    }

        }
    }
}


void pointInitialization::fetchManualBboxStation(bool type,int nHours,
                                           std::string lat1,
                                           std::string lon1,
                                           std::string lat2,
                                           std::string lon2,
                                           std::string yeara,
                                           std::string montha,
                                           std::string daya,
                                           std::string clocka,
                                           std::string yearb,
                                           std::string monthb,
                                           std::string dayb,std::string clockb)
    {
    //fetches data based on a bounding box
    //the bounding box is specified by
    // lower left (lon,lat) to upper right (lon,lat)

        const char* lmUrl;
        std::string tempUrl;
        if (type==true)
        {
            tempUrl=pointInitialization::BuildBboxLatest(lat1,lon1,lat2,lon2);
            lmUrl=tempUrl.c_str();
        }
        else
        {
            tempUrl=pointInitialization::BuildBboxUrl(lat1,lon1,lat2,lon2,
                                                    yeara,montha,daya,clocka,
                                                    yearb,monthb,dayb,clockb);
            lmUrl=tempUrl.c_str();
        }
//        cout<<lmUrl<<endl;
        OGRDataSourceH hDS;
        OGRLayerH hLayer;
        OGRFeatureH hFeature;

        const char* csvname="box.csv";


        hDS=OGROpen(lmUrl,0,NULL);
        CPLGetLastErrorMsg();
        //    hDS=GDALOpenEx(pszvtry,GDAL_OF_ALL,NULL,NULL,NULL);
        if (hDS==NULL)
            printf("i be broken mon \n");
        printf("\n\n");

        //    hLayer = OGR_DS_GetLayerByName(hDS,"OGRGeoJSON");
        hLayer=OGR_DS_GetLayer(hDS,0);
        OGR_L_ResetReading(hLayer);

        int idx=0;
        int idx1=0;
        int idx2=0;
        int idx3=0;
        int idx4=0;
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

        vector<int> mnetid;

        int idxx1=0;
        int idxx2=0;
        int idxx3=0;
        const double *cloudlow;
        const double *cloudmed;
        const double *cloudhigh;

        const double* rawsWind;
        const double* rawsDir;
        const double* rawsSolrad;
        const double* rawsTemp;
        double rawsLatitude;
        double rawsLongitude;
        const char* rawsStation;
        char** rawsDateTime;

        const double* metarWind;
        const double* metarDir;
        const double* metarTemp;
        const double* metarCloud;
        double metarLatitude;
        double  metarLongitude;
        const char* metarStation;
        char** metarDateTime;

        int fCount=OGR_L_GetFeatureCount(hLayer,1);

        ofstream tsetseWrite;//writing to csv
        tsetseWrite.open(csvname);
        cout<<fCount<<" stations "<<"saved to: "<<csvname<<endl;
        tsetseWrite << "Station_ID,Coord_Sys,DATUM(WGS84),Lat/YCoord,Lon/XCoord,Height,Height_Units,Speed,Speed_Units,Direction(degrees),Tempertaure,Temperature_Units,Cloud_Cover(%),Radius_of_influence,Radius_of_influence_Units,date_time"<<endl;

        for (int ex=0;ex<fCount;ex++)
        {

            hFeature=OGR_L_GetFeature(hLayer,ex);

            idx=OGR_F_GetFieldIndex(hFeature,"mnet_id");
            mnetid.push_back(OGR_F_GetFieldAsInteger(hFeature,idx));

            if (mnetid[ex]==1) //METAR station uses cloud data
            {
                int count1=0;
                int count2=0;
                int count3=0;
                int count4=0;
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

                vector<string>cloudkappa;
                cloudkappa=UnifyClouds(cloudlow,cloudmed,cloudhigh,countxx1,countxx2,countxx3,count3);
                vector<string>metarWindDirection;
                metarWindDirection=fixWindDir(metarDir,"0",count1);


                for(int ez=0;ez<count1;ez++)
                {
                 tsetseWrite<<metarStation<<",GEOGCS,"<<"WGS84,"<<metarLatitude<<","<<metarLongitude<<",10,"<<"meters,"<<metarWind[ez]<<",mps,"<<metarWindDirection[ez]<<","<<metarTemp[ez]<<",C,"<<cloudkappa[ez]<<","<<"-1,"<<"km,"<<metarDateTime[ez]<<endl;
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

                const double* aZero;
                aZero=0;
                std::string baddata="-9999";
                vector<string>rawsWindDirection;
                rawsWindDirection=fixWindDir(rawsDir,"0",count9);


                    for (int ez=0;ez<count9;ez++)
                    {
                    if (rawsSolrad==aZero)
                    {

                        tsetseWrite<<rawsStation<<",GEOGCS,"<<"WGS84,"<<rawsLatitude<<","<<rawsLongitude<<",10,"<<"meters,"<<rawsWind[ez]<<",mps,"<<rawsWindDirection[ez]<<","<<rawsTemp[ez]<<",C,"<<baddata<<","<<"-1,"<<"km,"<<rawsDateTime[ez]<<endl;
                    }
                    else
                    {
                        tsetseWrite<<rawsStation<<",GEOGCS,"<<"WGS84,"<<rawsLatitude<<","<<rawsLongitude<<",10,"<<"meters,"<<rawsWind[ez]<<",mps,"<<rawsWindDirection[ez]<<","<<rawsTemp[ez]<<",C,"<<rawsSolrad[ez]<<","<<"-1,"<<"km,"<<rawsDateTime[ez]<<endl;
                    }

                    }

        }
    }


}

int pointInitialization::storeHour(int nHours)
{
    int bla=nHours;
    return bla;
}




void pointInitialization::newAuto(AsciiGrid<double> &dem)
{
    double dz=dem.get_maxValue();
    cout<<"test"<<endl;



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
pointInitialization::getTimeList( int startYear, int startMonth, int startDay,
                                        int startHour, int startMinute, int endYear,
                                        int endMonth, int endDay, int endHour, int endMinute,
                                        int nTimeSteps, std::string timeZone )
{


    boost::local_time::tz_database tz_db;
    tz_db.load_from_file( FindDataPath("date_time_zonespec.csv") );
    boost::local_time::time_zone_ptr timeZonePtr;
    timeZonePtr = tz_db.time_zone_from_region(timeZone);
    
    endHour=endHour;
    startHour=startHour;
    
//    boost::posix_time::ptime startTime(boost::gregorian::date(startYear,startMonth,startDay),boost::posix_time::time_duration(startHour,startMinute,0,0));
    boost::gregorian::date dStart(startYear,startMonth,startDay);
    boost::posix_time::time_duration dStartTime(startHour,startMinute,0,0);

    boost::local_time::local_date_time startLocal(dStart,dStartTime,timeZonePtr,true);


//    boost::posix_time::ptime endTime(boost::gregorian::date(endYear,endMonth,endDay),boost::posix_time::time_duration(endHour,endMinute,0,0));
    boost::gregorian::date dEnd(endYear,endMonth,endDay);
    boost::posix_time::time_duration dEndTime(endHour,endMinute,0,0);

    boost::local_time::local_date_time endLocal(dEnd,dEndTime,timeZonePtr,true);




    boost::posix_time::ptime startUtc=startLocal.utc_time();
    boost::posix_time::ptime endUtc=endLocal.utc_time();
//    cout<<startUtc<<endl;
//    cout<<endUtc<<endl;
    printf("\n");


//    cout<<endTime-startTime<<endl;

    boost::posix_time::time_duration diffTime=endUtc-startUtc;

//    cout<<diffTime<<endl;

    boost::posix_time::time_duration stepTime=diffTime/nTimeSteps;
////    cout<<stepTime<<endl;

    std::vector<boost::posix_time::ptime> timeOut;
    std::vector<boost::posix_time::ptime> timeConstruct;
    std::vector<boost::posix_time::ptime> timeList;
    std::vector<boost::posix_time::time_duration> timeStorage;


    timeOut.push_back(startUtc);
//    cout<<timeOut[0]<<endl;
    for (int i=1;i<nTimeSteps;i++)
    {
        boost::posix_time::time_duration specTime;
        specTime=stepTime*i;
        timeOut.push_back(startUtc+specTime);
//        cout<<timeOut[i]<<endl;
    }
    timeOut.push_back(endUtc);
//    cout<<timeOut[nTimeSteps]<<endl;
//    cout<<timeOut.size()<<endl;
    timeList=timeOut;

    return timeList;
}

vector<boost::posix_time::ptime> pointInitialization::getSingleTimeList(string timeZone)
{
    vector<boost::posix_time::ptime> timeList;
    boost::posix_time::ptime standard = boost::posix_time::second_clock::universal_time();
    timeList.push_back(standard);
//    cout<<standard<<endl;
    return timeList;
}


/**@brief Fetches station data from bounding box.
 *
 * @param stationFilename Filename/path where the station csv will be written.
 * @param demFile Filename/path to the DEM on disk.
 * @param timeList Vector of datetimes in UTC for the simulation.
 */
bool pointInitialization::fetchStationFromBbox(std::string stationFilename,
                                    std::string demFile,
                                    std::vector<boost::posix_time::ptime> timeList, std::string timeZone, bool latest)
{

//    cout<<timeUTC[0]<<endl;//year_0
//    cout<<timeUTC[1]<<endl;//month_0
//    cout<<timeUTC[2]<<endl;//day_0
//    cout<<timeUTC[3]<<endl;//clock_0
//    cout<<timeUTC[4]<<endl;//year_1
//    cout<<timeUTC[5]<<endl;//month_1
//    cout<<timeUTC[6]<<endl;//day_1
//    cout<<timeUTC[7]<<endl;//clock_1

    GDALDataset  *poDS;
    poDS = (GDALDataset *) GDALOpen(demFile.c_str(), GA_ReadOnly );
    if (poDS==NULL)
    {
        cout<<"Could not read DEM file for station fetching"<<endl;
        cout<<false<<endl;
        return false;
    }

    double bounds[4];
    bool bRet;

    bRet=GDALGetBounds(poDS,bounds);
    if (bRet==false)
    {
        cout<<"GDALGetBounds returned false, DEM file is lacking readable data.";
        cout<<false<<endl;       
        throw std::runtime_error("GDALGetBounds returned false, DEM file is lacking readable data.");
        return false;

    }
//    cout<<bounds[0]<<endl;//UR Lat
//    cout<<bounds[1]<<endl;// UR Lon
//    cout<<bounds[2]<<endl;//LL Lat
//    cout<<bounds[3]<<endl;//LL Lon
    double buffer;
    buffer=get_stationBuffer();
    cout<<"Adding "<<buffer<<" m to DEM for station fetching..."<<endl;

    double projxL=bounds[2];
    double projyL=bounds[3];
    double projxR=bounds[0];
    double projyR=bounds[1];
//    cout<<"lowerLeft"<<endl;
//    cout<<projxL<<","<<projyL<<endl;

//    cout<<"UpperRight"<<endl;
//    cout<<projxR<<","<<projyR<<endl;

    GDALPointFromLatLon(projyL,projxL,poDS,"WGS84"); //LowerLeft


    GDALPointFromLatLon(projyR,projxR,poDS,"WGS84"); //Upper Right


    projxL=projxL-buffer;
    projyL=projyL-buffer;

    projxR=projxR+buffer;
    projyR=projyR+buffer;

//    cout<<"------------------------------"<<endl;

    GDALPointToLatLon(projyL,projxL,poDS,"WGS84");
//    cout<<"lowerLeft"<<endl;
//    cout<<projxL<<","<<projyL<<endl;
    GDALPointToLatLon(projyR,projxR,poDS,"WGS84");
//    cout<<"upperRight"<<endl;
//    cout<<projxR<<","<<projyR<<endl;

    bounds[2]=projxL;
    bounds[3]=projyL;
    bounds[0]=projxR;
    bounds[1]=projyR;

//        cout<<bounds[0]<<endl;//UR Lat
//        cout<<bounds[1]<<endl;// UR Lon
//        cout<<bounds[2]<<endl;//LL Lat
//        cout<<bounds[3]<<endl;//LL Lon




//    exit(1)
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
    cout<<"WxData URL: "<<endl;
    cout<<URL<<endl;

    std::string csvName;
    if (stationFilename.substr(stationFilename.size()-4,4)==".csv")
    {
    csvName=stationFilename;
    cout<<".csv exists in stationFilename..."<<endl;
    }
    else
    {
    csvName=stationFilename+".csv";
    cout<<"Adding .csv to stationFilename..."<<endl;
    }

    OGRDataSourceH hDS;
    OGRLayerH hLayer;
    OGRFeatureH hFeature;

    hDS=OGROpen(URL.c_str(),0,NULL);
    CPLGetLastErrorMsg();
    if (hDS==NULL)
    {
        cout<<URL<<endl;
        printf("OGROpen could not read file\n try running again ");
        cout<<"if persists: check URL, possibly no stations exist for given parameters."<<endl;
        cout<<"if ERROR IS HTTP, Check Internet Connection and server status";
        cout<<false<<endl;
        throw std::runtime_error("OGROpen could not read file\nif persists:check URL, possibly not stations\nexist for given parameters,\nIf ERROR is HTTP, check internet connection and server status.");
        return false;
    }

    //    hLayer = OGR_DS_GetLayerByName(hDS,"OGRGeoJSON");
    hLayer=OGR_DS_GetLayer(hDS,0);
    OGR_L_ResetReading(hLayer);

    int idx=0;
    int idx1=0;
    int idx2=0;
    int idx3=0;
    int idx4=0;
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

    vector<int> mnetid;

    int idxx1=0;
    int idxx2=0;
    int idxx3=0;
    const double *cloudlow;
    const double *cloudmed;
    const double *cloudhigh;

    const double* rawsWind;
    const double* rawsDir;
    const double* rawsSolrad;
    const double* rawsTemp;
    double rawsLatitude;
    double rawsLongitude;
    const char* rawsStation;
    char** rawsDateTime;

    const double* metarWind;
    const double* metarDir;
    const double* metarTemp;
    double metarLatitude;
    double  metarLongitude;
    const char* metarStation;
    char** metarDateTime;

    int fCount=OGR_L_GetFeatureCount(hLayer,1);

    ofstream outFile;//writing to csv
    outFile.open(csvName.c_str());
    cout<<fCount<<" stations "<<"saved to: "<<csvName<<endl;
    cout<<"Downloading Data from MesoWest...."<<endl;
//    outFile << "Station_ID,Coord_Sys,DATUM(WGS84),Lat/YCoord,Lon/XCoord,Height,Height_Units,Speed,Speed_Units,Direction(degrees),Tempertaure,Temperature_Units,Cloud_Cover(%),Radius_of_influence,Radius_of_influence_Units,date_time"<<endl;
    std::string header="\"Station_Name\",\"Coord_Sys(PROJCS,GEOGCS)\",\"Datum(WGS84,NAD83,NAD27)\",\"Lat/YCoord\",\"Lon/XCoord\",\"Height\",\"Height_Units(meters,feet)\",\"Speed\",\"Speed_Units(mph,kph,mps)\",\"Direction(degrees)\",\"Temperature\",\"Temperature_Units(F,C)\",\"Cloud_Cover(%)\",\"Radius_of_Influence\",\"Radius_of_Influence_Units(miles,feet,meters,km)\",\"date_time\"";
    outFile<<header<<endl;
    
    
    for (int ex=0;ex<fCount;ex++)
    {

        hFeature=OGR_L_GetFeature(hLayer,ex);

        idx=OGR_F_GetFieldIndex(hFeature,"mnet_id");
        mnetid.push_back(OGR_F_GetFieldAsInteger(hFeature,idx));


        if (mnetid[ex]==1) //METAR station uses cloud data
        {
            int count1=0;
            int count2=0;
            int count3=0;
            int count4=0;
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
            
//            FloatPrinter(metarWind,count1,"metarwind");
//            FloatPrinter(metarDir,count2,"metardir");
//            FloatPrinter(metarTemp,count3,"metartemp");
//            FloatPrinter(cloudlow,countxx1,"cloudll");
//            FloatPrinter(cloudmed,countxx2,"cloudmm");
//            FloatPrinter(cloudhigh,countxx3,"cloudhh");

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


//            FloatPrinter(rawsWind,count9,"rawswind");
//            FloatPrinter(rawsDir,count10,"rawsdir");
//            FloatPrinter(rawsTemp,count11,"rawstemp");
//            FloatPrinter(rawsSolrad,count12,"rawssol");

            int aZero;
            aZero=0;
            std::string baddata="-9999";
            vector<string>rawsWindDirection;
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
    }
    cout<<"Data downloaded and saved...."<<endl;
    OGR_DS_Destroy(poDS);
    OGR_DS_Destroy(hDS);
    delete cloudhigh;
    delete cloudlow,rawsWind,rawsDir,rawsSolrad,rawsTemp,rawsLatitude,rawsLongitude,rawsStation,rawsDateTime;
    delete metarWind,metarDir,metarTemp,metarLatitude,metarLongitude,metarStation,metarDateTime;

//    const double *cloudlow;
//    const double *cloudmed;
//    const double *cloudhigh;

//    const double* rawsWind;
//    const double* rawsDir;
//    const double* rawsSolrad;
//    const double* rawsTemp;
//    double rawsLatitude;
//    double rawsLongitude;
//    const char* rawsStation;
//    char** rawsDateTime;

//    const double* metarWind;
//    const double* metarDir;
//    const double* metarTemp;
//    double metarLatitude;
//    double  metarLongitude;
//    const char* metarStation;
//    char** metarDateTime;



    cout<<"Fetch Station returned true: "<<true<<endl;

    return true;
}

bool pointInitialization::fetchStationByName(string stationFilename, string stationList,
                                             std::vector<boost::posix_time::ptime> timeList,
                                             string timeZone, bool latest)
{
    bool test=true;
    std::string URL;

    if (latest==true)
    {
        URL=BuildMultiLatest(stationList);
        cout<<URL<<endl;
    }
    if (latest==false)
    {
        vector<std::string>timeUTC;
        timeUTC=UnifyTime(timeList);
        URL=BuildMultiUrl(stationList,timeUTC[0],timeUTC[1],timeUTC[2],
                timeUTC[3],timeUTC[4],timeUTC[5],
                timeUTC[6],timeUTC[7]);

    }
    std::string csvName;
    if (stationFilename.substr(stationFilename.size()-4,4)==".csv")
    {
    csvName=stationFilename;
    cout<<".csv exists in stationFilename..."<<endl;
    }
    else
    {
    csvName=stationFilename+".csv";
    cout<<"Adding .csv to stationFilename..."<<endl;
    }

    OGRDataSourceH hDS;
    OGRLayerH hLayer;
    OGRFeatureH hFeature;

    hDS=OGROpen(URL.c_str(),0,NULL);
    CPLGetLastErrorMsg();
    if (hDS==NULL)
    {
        cout<<URL<<endl;
        printf("OGROpen could not read file\n try running again ");
        cout<<"if persists: check URL, possibly no stations exist for given parameters."<<endl;
        cout<<"if ERROR IS HTTP, Check Internet Connection and server status";
        cout<<false<<endl;
        throw std::runtime_error("OGROpen could not read file\nif persists:check URL, possibly not stations\nexist for given parameters,\nIf ERROR is HTTP, check internet connection and server status.");
        return false;
    }

    //    hLayer = OGR_DS_GetLayerByName(hDS,"OGRGeoJSON");
    hLayer=OGR_DS_GetLayer(hDS,0);
    OGR_L_ResetReading(hLayer);

    int idx=0;
    int idx1=0;
    int idx2=0;
    int idx3=0;
    int idx4=0;
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

    vector<int> mnetid;

    int idxx1=0;
    int idxx2=0;
    int idxx3=0;
    const double *cloudlow;
    const double *cloudmed;
    const double *cloudhigh;

    const double* rawsWind;
    const double* rawsDir;
    const double* rawsSolrad;
    const double* rawsTemp;
    double rawsLatitude;
    double rawsLongitude;
    const char* rawsStation;
    char** rawsDateTime;

    const double* metarWind;
    const double* metarDir;
    const double* metarTemp;
    double metarLatitude;
    double  metarLongitude;
    const char* metarStation;
    char** metarDateTime;

    int fCount=OGR_L_GetFeatureCount(hLayer,1);

    ofstream outFile;//writing to csv
    outFile.open(csvName.c_str());
    cout<<fCount<<" stations "<<"saved to: "<<csvName<<endl;
    cout<<"Downloading Data from MesoWest...."<<endl;
//    outFile << "Station_ID,Coord_Sys,DATUM(WGS84),Lat/YCoord,Lon/XCoord,Height,Height_Units,Speed,Speed_Units,Direction(degrees),Tempertaure,Temperature_Units,Cloud_Cover(%),Radius_of_influence,Radius_of_influence_Units,date_time"<<endl;
    std::string header="\"Station_Name\",\"Coord_Sys(PROJCS,GEOGCS)\",\"Datum(WGS84,NAD83,NAD27)\",\"Lat/YCoord\",\"Lon/XCoord\",\"Height\",\"Height_Units(meters,feet)\",\"Speed\",\"Speed_Units(mph,kph,mps)\",\"Direction(degrees)\",\"Temperature\",\"Temperature_Units(F,C)\",\"Cloud_Cover(%)\",\"Radius_of_Influence\",\"Radius_of_Influence_Units(miles,feet,meters,km)\",\"date_time\"";
    outFile<<header<<endl;


    for (int ex=0;ex<fCount;ex++)
    {

        hFeature=OGR_L_GetFeature(hLayer,ex);

        idx=OGR_F_GetFieldIndex(hFeature,"mnet_id");
        mnetid.push_back(OGR_F_GetFieldAsInteger(hFeature,idx));


        if (mnetid[ex]==1) //METAR station uses cloud data
        {
            int count1=0;
            int count2=0;
            int count3=0;
            int count4=0;
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

//            FloatPrinter(metarWind,count1,"metarwind");
//            FloatPrinter(metarDir,count2,"metardir");
//            FloatPrinter(metarTemp,count3,"metartemp");
//            FloatPrinter(cloudlow,countxx1,"cloudll");
//            FloatPrinter(cloudmed,countxx2,"cloudmm");
//            FloatPrinter(cloudhigh,countxx3,"cloudhh");

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


//            FloatPrinter(rawsWind,count9,"rawswind");
//            FloatPrinter(rawsDir,count10,"rawsdir");
//            FloatPrinter(rawsTemp,count11,"rawstemp");
//            FloatPrinter(rawsSolrad,count12,"rawssol");

            int aZero;
            aZero=0;
            std::string baddata="-9999";
            vector<string>rawsWindDirection;
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
    }
    cout<<"Data downloaded and saved...."<<endl;
    OGR_DS_Destroy(hDS);
    delete cloudhigh;
    delete cloudlow,rawsWind,rawsDir,rawsSolrad,rawsTemp,rawsLatitude,rawsLongitude,rawsStation,rawsDateTime;
    delete metarWind,metarDir,metarTemp,metarLatitude,metarLongitude,metarStation,metarDateTime;


    cout<<"Fetch Station returned true: "<<true<<endl;

    return true;
}


//FOR TESTING MW LATEST!!!
void pointInitialization::fetchTest(std::string stationFilename,
                                    std::string demFile,
                                    std::vector<boost::posix_time::ptime> timeList, std::string timeZone, bool latest)
    {

    cout<<"inactive"<<endl;
    exit(1);

    }

void pointInitialization::stationCliCaller(bool station_fetch, std::string station_id, int nHours, bool btype,
                                           std::string fetcher,std::string radius,
                                           std::string limit, std::string pLat,
                                           std::string pLon, std::string LLLat,
                                           std::string LLLon, std::string URLat,
                                           std::string URLon , std::string yeara,
                                           std::string montha, std::string daya,
                                           std::string clocka,std::string yearb,
                                           std::string monthb,std::string dayb,
                                           std::string clockb)

    {
    if(station_fetch==true)
    {
        cout<<"true"<<endl;
//        fetchSingleStation(true,24,"kmso","2016","06","01","1200","2016","06","02","1200");
//        fetchMultiStation(true,24,"kmso,TR266","2016","06","01","1200","2016","06","02","1200");
//        fetchPointRadiusStation(true,24,"kmso","30","30","2016","06","01","1200","2016","06","02","1200");
//        fetchLatLonStation(true,24,"46.92","-114.12","30","30","2016","06","01","1200","2016","06","02","1200");
//        fetchBboxStation(true,24,"45.7511","-115.2578","47.1486","-112.7591",ndvar,"2016","06","01","1200","2016","06","02","1200");
//        fetchTest("kmso",24,ndvar);


    }
    else
    {
        exit(1);
        {

        if (fetcher=="single")
        {
            //FetchSingleStation(dtoken,btype,nHours,station_id,"wind_speed,wind_direction,air_temp,solar_radiation,cloud_layer_1_code","2016","06","01","1200","2016","06","02","1200");
            fetchSingleStation(btype,nHours,station_id,
                                  yeara,montha,daya,clocka,yearb,monthb,dayb,clockb);
        }
        if (fetcher=="multi")
        {
            fetchMultiStation(btype,nHours,station_id,
                              yeara,montha,daya,clocka,yearb,monthb,dayb,clockb);
        }
        if (fetcher=="point")
        {
//            fetchPointRadiusStation(btype,nHours,station_id,
//                                    radius,limit,
//                                    yeara,montha,daya,clocka,yearb,monthb,dayb,clockb);
        }
        if (fetcher=="latlon")
        {
//            fetchLatLonStation(btype,nHours,pLat,pLon,radius,limit,yeara,montha,
//                               daya,clocka,yearb,monthb,dayb,clockb);

        }
        if (fetcher=="box")
        {
//            fetchBboxStation(btype,nHours,LLLat,LLLon,URLat,URLon,
//                             yeara,montha,daya,clocka,yearb,monthb,dayb,clockb);
            //bbox is the following
            // lower left to upper right
        }
        if (fetcher=="auto")
        {
    //        auto_bbox_fetch(dtoken,btype,nHours,LLLat,LLLon,URLat,URLon,dvar,yeara,montha,daya,clocka,yearb,monthb,dayb,clockb);
        }

        if (fetcher=="test")

        {
    //        fetchTest(dtoken,true,nHours,station_id,dvar);
        }
        }
    }
    cout<<"WIP!"<<endl;
    }
