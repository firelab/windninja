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
}

