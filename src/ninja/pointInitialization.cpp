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
		AsciiGrid<double>& cloud,
        AsciiGrid<double>& L,
        AsciiGrid<double>& u_star,
        AsciiGrid<double>& bl_height)
{
	//Set cloud grid
//	int longEdge = input.dem.get_nRows();
//	if(input.dem.get_nRows() < input.dem.get_nCols())
//		longEdge = input.dem.get_nCols();
//	cloud.set_headerData(1, 1, input.dem.get_xllCorner(), input.dem.get_yllCorner(), (longEdge * input.dem.cellSize), -9999.0, 0, input.dem.prjString);

	
	int i, j, k;

	windProfile profile;
	profile.profile_switch = windProfile::monin_obukov_similarity;	//switch that detemines what profile is used...
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

	if(input.diurnalWinds == true)	//compute values needed for diurnal computations
	{
		height.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);	//height of diurnal flow above "z=0" in log profile
		uDiurnal.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);
		vDiurnal.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);
		wDiurnal.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);
		aspect.compute_gridAspect(&input.dem, input.numberCPUs);
		slope.compute_gridSlope(&input.dem, input.numberCPUs);
		double aspect_temp = 0;	//just placeholder, basically
		double slope_temp = 0;	//just placeholder, basically
		solar.compute_solar(input.ninjaTime, input.latitude, input.longitude, aspect_temp, slope_temp);
		shade.compute_gridShade(&input.dem, solar.get_theta(), solar.get_phi(), input.numberCPUs);
		//shade.write_Grid("C:\\01_JASON_L14\\WindNinja_legacy\\trunk\\source\\shadeTest.asc", 2);
	}

	AsciiGrid<double> uInitializationGrid(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.xllCorner, input.dem.yllCorner, input.dem.cellSize, std::numeric_limits<double>::max(), input.dem.prjString);
	AsciiGrid<double> vInitializationGrid(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.xllCorner, input.dem.yllCorner, input.dem.cellSize, std::numeric_limits<double>::max(), input.dem.prjString);
	AsciiGrid<double> airTempGrid(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.xllCorner, input.dem.yllCorner, input.dem.cellSize, std::numeric_limits<double>::max(), input.dem.prjString);
	AsciiGrid<double> cloudCoverGrid(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.xllCorner, input.dem.yllCorner, input.dem.cellSize, -9999.0, input.dem.prjString);

	//input.stations = wxStation::readStationFile(input.wxStationFilename, input.dem.fileName);

	double *u, *v, *T, *cc, *X, *Y, *influenceRadius;
	u = new double [input.stationsScratch.size()];
	v = new double [input.stationsScratch.size()];
	T = new double [input.stationsScratch.size()];
	cc = new double [input.stationsScratch.size()];
	X = new double [input.stationsScratch.size()];
	Y = new double [input.stationsScratch.size()];
	influenceRadius = new double [input.stationsScratch.size()];
	double maxStationHeight = -1;	//height above ground of highest station, used as height of 2d layer to interpolate horizontally to

	for(unsigned int ii = 0; ii<input.stationsScratch.size(); ii++)
	{
		if(input.stationsScratch[ii].get_height() > maxStationHeight)
			maxStationHeight = input.stationsScratch[ii].get_height();
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
	//airTempGrid.write_Grid("C:\\01_JASON_L14\\WindNinja_legacy\\trunk\\source\\airTempTest.asc", 2);
	cloudCoverGrid.interpolateFromPoints(cc, X, Y, influenceRadius, input.stationsScratch.size(), dfInvDistWeight);

	//Check one grid to be sure that the interpolation completely filled the grid
	if(cloudCoverGrid.checkForNoDataValues())
	{
		throw std::runtime_error("Fill interpolation from the wx stations didn't completely fill the grids.  " \
				"To be sure everything is filled, let at least one wx station have an infinite influence radius.  " \
				"This is specified by defining the influence radius to be a value less than zero in the wx " \
				"station file.");
	}

	cloud = cloudCoverGrid;

	//Monin-Obukhov length, surface friction velocity, and atmospheric boundary layer height
	L.set_headerData(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0.0);
	u_star.set_headerData(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0.0);
	bl_height.set_headerData(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), -1.0);

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
		if(input.stationsScratch[ii].get_height() != maxStationHeight)	//if station is not at the 2d interp layer height of maxStationHeight, interpolate vertically using profile to this height
		{	
			profile.inputWindHeight = input.stationsScratch[ii].get_height();
			//get surface properties
			if(input.dem.check_inBounds(input.stationsScratch[ii].get_projXord(), input.stationsScratch[ii].get_projYord()))	//if station is in the dem domain
			{
				input.dem.get_cellIndex(input.stationsScratch[ii].get_projXord(), input.stationsScratch[ii].get_projYord(), &i_, &j_);

				profile.Roughness = (input.surface.Roughness)(i_, j_);
				profile.Rough_h = (input.surface.Rough_h)(i_, j_);
				profile.Rough_d = (input.surface.Rough_d)(i_, j_);

				if(input.diurnalWinds == true)	//compute values needed for diurnal computation
				{
					cDiurnal.initialize(input.stationsScratch[ii].get_projXord(), input.stationsScratch[ii].get_projYord(),
							aspect(i_, j_),slope(i_, j_), cloudCoverGrid(i_, j_), airTempGrid(i_, j_),
							input.stationsScratch[ii].get_speed(), input.stationsScratch[ii].get_height(),
							(input.surface.Albedo)(i_, j_), (input.surface.Bowen)(i_, j_), (input.surface.Cg)(i_, j_),
							(input.surface.Anthropogenic)(i_, j_), (input.surface.Roughness)(i_, j_),
							(input.surface.Rough_h)(i_, j_), (input.surface.Rough_d)(i_, j_));


					cDiurnal.compute_cell_diurnal_parameters(i_, j_,&profile.ObukovLength, &U_star, &profile.ABL_height);
					
				}else{	//compute neutral ABL height
					
					double f;
					double velocity;

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
					velocity=std::pow(u[ii]*u[ii]+v[ii]*v[ii],0.5);     //Velocity is the velocity magnitude
					U_star = velocity*0.4/(log((profile.inputWindHeight+profile.Rough_h-profile.Rough_d)/profile.Roughness));
					
					//compute neutral ABL height
					profile.ABL_height = 0.2 * U_star / f;	//from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
					profile.ObukovLength = 0.0;
				}

			}else{	//if station is not in dem domain, use grass roughness
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
					
				}else{	//compute neutral ABL height
					
					double f;
					double velocity;

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
					velocity=std::pow(u[ii]*u[ii]+v[ii]*v[ii],0.5);     //Velocity is the velocity magnitude
					U_star = velocity*0.4/(log((profile.inputWindHeight+profile.Rough_h-profile.Rough_d)/profile.Roughness));
					
					//compute neutral ABL height
					profile.ABL_height = 0.2 * U_star / f;	//from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
					profile.ObukovLength = 0.0;
				}
			}

			profile.AGL=maxStationHeight + profile.Rough_h;			//this is height above THE GROUND!! (not "z=0" for the log profile)

			wind_sd_to_uv(input.stationsScratch[ii].get_speed(), input.stationsScratch[ii].get_direction(), &u[ii], &v[ii]);
			profile.inputWindSpeed = u[ii];	
			u[ii] = profile.getWindSpeed();
			profile.inputWindSpeed = v[ii];
			v[ii] = profile.getWindSpeed();
		}else{	//else station is already at 2d interp layer height
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
			input.surface.windSpeedGrid(i,j) = std::pow((uInitializationGrid(i,j)*uInitializationGrid(i,j)+vInitializationGrid(i,j)*vInitializationGrid(i,j)), 0.5);
		}
	}

	//input.surface.windSpeedGrid.write_Grid("C:\\01_JASON_L14\\WindNinja_legacy\\trunk\\source\\speedGrid.asc", 2);

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
		addDiurnal diurnal(&uDiurnal, &vDiurnal, &wDiurnal, &height, &L, &u_star, &bl_height, &input.dem, &aspect, &slope, &shade, &solar, &input.surface, &cloudCoverGrid, &airTempGrid, input.numberCPUs);
		
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
		//	   (*diurnalVelocityGrid)[i][j]=std::pow(((uDiurnal)[i][j]*(uDiurnal)[i][j]+(vDiurnal)[i][j]*(vDiurnal)[i][j]),0.5);       //calculate velocity magnitude (in x,y plane; I decided to include z here so the wind is the total magnitude wind)
		//         
		//             if ((uDiurnal)[i][j]==0.0 && (vDiurnal)[i][j]==0.0)
		//                  interMedVal=0.0;
		//             else
		//  		          interMedVal=-atan2((uDiurnal)[i][j], -(vDiurnal)[i][j]);
		//             if(interMedVal<0)addDiurnal
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
		
		double f, vel;

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
				vel = std::pow((uInitializationGrid(i,j)*uInitializationGrid(i,j)+vInitializationGrid(i,j)*vInitializationGrid(i,j)),0.5);
				u_star(i,j) = vel*0.4/(log((input.inputWindHeight+input.surface.Rough_h(i,j)-input.surface.Rough_d(i,j))/input.surface.Roughness(i,j)));
				
				//compute neutral ABL height
				bl_height(i,j) = 0.2 * u_star(i,j) / f;	//from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
			}
		}
	}

	//bl_height.write_Grid("C:\\01_JASON_L14\\WindNinja_legacy\\trunk\\source\\blHeightTest.asc", 2);
	//L.write_Grid("C:\\01_JASON_L14\\WindNinja_legacy\\trunk\\source\\LTest.asc", 2);

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
				profile.AGL=mesh.ZORD(i, j, k)-input.dem(i,j);			//this is height above THE GROUND!! (not "z=0" for the log profile)
				
				
				profile.inputWindSpeed = uInitializationGrid(i,j);
				u0(i, j, k) += profile.getWindSpeed();
				//if(i==45 && j==13)
				//{
				//	printf("%i\t%lf\t%lf\t%lf\t%lf\n", k, profile.AGL, profile.ABL_height, u0(i, j, k), profile.ObukovLength);
				//}
				profile.inputWindSpeed = vInitializationGrid(i,j);
				v0(i, j, k) += profile.getWindSpeed();

				profile.inputWindSpeed = 0.0;
				w0(i, j, k) += profile.getWindSpeed();

			}
		}
	}

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
    
    
    
    /*
    //write out grids, kml, for testing
    
    AsciiGrid<double> *temporaryVelocityGrid, *temporaryAngleGrid;
    temporaryVelocityGrid=NULL;
    temporaryAngleGrid=NULL;
    
    KmlVector temporaryKmlFiles;
    //temporaryKmlFiles.com = input.Com;
    
    temporaryAngleGrid = new AsciiGrid<double> ();
    temporaryAngleGrid->set_headerData(input.dem);
    temporaryVelocityGrid = new AsciiGrid<double> ();
    temporaryVelocityGrid->set_headerData(input.dem);
    
    double interMedVal;
    
    //Change from u,v components to speed and direction
    for(int i=0; i<temporaryVelocityGrid->get_nRows(); i++)
    {
        for(int j=0; j<temporaryVelocityGrid->get_nCols(); j++)
    	{
    		
            (*temporaryVelocityGrid)(i,j) = std::pow(((uInitializationGrid)(i,j)*(uInitializationGrid)(i,j)+(vInitializationGrid)(i,j)*(vInitializationGrid)(i,j)),0.5);       //calculate velocity magnitude (in x,y plane; I decided to include z here so the wind is the total magnitude wind)
             
                 if ((uInitializationGrid)(i,j)==0.0 && (vInitializationGrid)(i,j)==0.0)
                      interMedVal=0.0;
                 else
                     interMedVal=-atan2((uInitializationGrid)(i,j), -(vInitializationGrid)(i,j));
                 if(interMedVal<0)
                      interMedVal+=2.0*pi;
                 (*temporaryAngleGrid)(i,j)=(180.0/pi*interMedVal);
    	}
    }
    
    temporaryVelocityGrid->write_Grid("/home/jforthofer/Desktop/waldo/temporarySpeed.asc", 2);
    temporaryVelocityGrid->set_prjString(input.dem.prjString);
    temporaryAngleGrid->set_prjString(input.dem.prjString);
    
       
    temporaryKmlFiles.setKmlFile("/home/jforthofer/Desktop/waldo/temporary.kml");
    temporaryKmlFiles.setKmzFile("/home/jforthofer/Desktop/waldo/temporary.kmz");
    temporaryKmlFiles.setDemFile(input.dem.fileName);
    temporaryKmlFiles.setLegendFile("/home/jforthofer/Desktop/waldo/temporaryLegend.txt");
    temporaryKmlFiles.setDateTimeLegendFile("/home/jforthofer/Desktop/waldo/temporaryDateTimeLegend.txt", input.ninjaTime);    
    temporaryKmlFiles.setSpeedGrid(*temporaryVelocityGrid, input.outputSpeedUnits);
    temporaryKmlFiles.setDirGrid(*temporaryAngleGrid);
    if(temporaryKmlFiles.writeKml(input.googSpeedScaling))
    {
    	if(temporaryKmlFiles.makeKmz())
    		temporaryKmlFiles.removeKmlFile();
    }	
    if(temporaryAngleGrid)
    {
    	delete temporaryAngleGrid;
    	temporaryAngleGrid=NULL;
    }
    if(temporaryVelocityGrid)
    {
    	delete temporaryVelocityGrid;
    	temporaryVelocityGrid=NULL;
    }
    */
    
}
