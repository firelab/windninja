/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Calculates diurnal wind in a cell of a spatial grid
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

#include "cellDiurnal.h"

cellDiurnal::cellDiurnal()
{

}

cellDiurnal::cellDiurnal(Elevation const* incomingDem, Shade const* shd, Solar *solarInput)
{
	#ifdef CELL_DIURNAL_DEBUG
		Stopwatch fxTimer;
		fxTimer.start();
		std::cout << "Construct class cellDiurnal using cellDiurnal(class Dem dem)" << std::endl;
	#endif
	
	dem = incomingDem;
	shade = shd;
	diurnalSolar = *solarInput;

	xord = 0.0;
	yord = 0.0;

	airTemp = 0.0;
	CloudCover = 0.0;
	z = 0.0;
	albedo = 0.0;
	bowen = 0.0;
	cg = 0.0;
	anthropogenic = 0.0;
	roughness = 0.0;
	rough_h = 0.0;
	rough_d = 0.0;

	i = 0;
	j = 0;		
	Qsw = 0;
	Qh = 0;
	diurnal_wind = false;
	up_down = 0;
	hillValleyDist = 0;
	flow_thickness_ratio = 0.05;
	elev_change = 0;
	sinAlphaLocal = 0;
	S = 0;
	cellDist_shadeFlag = true;
    g = 9.81;

	Cd_downslope = 0.0001;
	entrainment_coeff_downslope = 0.01;
	Cd_upslope = 0.2;
	entrainment_coeff_upslope = 0.2;

     epsilon = dem->get_cellSize()/1000.0;
     c1 = 5.31e-13;
	c2 = 60;
	c3 = 0.12;
     a1 = 990;
	a2 = -30;
	b1 = -0.75;
	b2 = 3.4;
//	air.read_fluidProperties("air.prp");
//	K = 0.4;
	gamma = 4.7;
	stop_crit = 0.01;

	l = 1e6;	//just make Monin-Obukov length large

	inputWindspeed = -1.0;


	#ifdef CELL_DIURNAL_DEBUG
		std::cout << "End construct class cellDiurnal using cellDiurnal(class Dem dem)\tTime Elapsed: " << fxTimer.stop() << std::endl;
	#endif
}

cellDiurnal::~cellDiurnal()
{
	
}

cellDiurnal::cellDiurnal(cellDiurnal &c)
{
	#ifdef CELL_DIURNAL_DEBUG
		Stopwatch fxTimer;
		fxTimer.start();
		std::cout << "Construct class cellDiurnal using cellDiurnal(class Dem dem)" << std::endl;
	#endif
	
	dem = c.dem;
	aspect = c.aspect;
	slope = c.slope;
	shade = c.shade;
	diurnalSolar = c.diurnalSolar;

	xord = c.xord;
	yord = c.yord;
	airTemp = c.airTemp;
	CloudCover = c.CloudCover;
	z = c.z;
	albedo = c.albedo;
	bowen = c.bowen;
	cg = c.cg;
	anthropogenic = c.anthropogenic;
	roughness = c.roughness;
	rough_h = c.rough_h;
	rough_d = c.rough_d;

	i = c.i;
	j = c.j;
	CloudCover = c.CloudCover;
	airTemp = c.airTemp;		
	Qsw = c.Qsw;
	Qh = c.Qh;
	diurnal_wind = c.diurnal_wind;
	up_down = c.up_down;
	hillValleyDist = c.hillValleyDist;
	flow_thickness_ratio = c.flow_thickness_ratio;
	elev_change = c.elev_change;
	sinAlphaLocal = c.sinAlphaLocal;
	S = c.S;
	cellDist_shadeFlag = c.cellDist_shadeFlag;
    g = c.g;
	Cd_downslope = c.Cd_downslope;
	entrainment_coeff_downslope = c.entrainment_coeff_downslope;
	Cd_upslope = c.Cd_upslope;
	entrainment_coeff_upslope = c.entrainment_coeff_upslope;
	epsilon = c.epsilon;
	c1 = c.c1;
	c2 = c.c2;
	c3 = c.c3;
	a1 = c.a1;
	a2 = c.a2;
	b1 = c.b1;
	b2 = c.b2;
//	air.read_fluidProperties("air.prp");
//	K = 0.4;
	gamma = c.gamma;
	stop_crit = c.stop_crit;

	inputWindspeed = c.inputWindspeed;

	l = c.l;

	#ifdef CELL_DIURNAL_DEBUG
		std::cout << "End construct class cellDiurnal using cellDiurnal(class Dem dem)\tTime Elapsed: " << fxTimer.stop() << std::endl;
	#endif
}

/** 
 * Initializae a cellDiurnal
 * 
 * @param incomingDem dem to use
 * @param shd Shade input
 * @param solarInput solar input
 * 
 */
void cellDiurnal::create(Elevation const* incomingDem, Shade const* shd, Solar *solarInput)
{
#ifdef CELL_DIURNAL_DEBUG
    Stopwatch fxTimer;
    fxTimer.start();
    std::cout << "Construct class cellDiurnal using cellDiurnal(class Dem dem)" << std::endl;
#endif
	
    dem = incomingDem;
    shade = shd;
    diurnalSolar = *solarInput;

    xord = 0.0;
    yord = 0.0;

    airTemp = 0.0;
    CloudCover = 0.0;
    z = 0.0;
    albedo = 0.0;
    bowen = 0.0;
    cg = 0.0;
    anthropogenic = 0.0;
    roughness = 0.0;
    rough_h = 0.0;
    rough_d = 0.0;

    i = 0;
    j = 0;		
    Qsw = 0;
    Qh = 0;
    diurnal_wind = false;
    up_down = 0;
    hillValleyDist = 0;
    flow_thickness_ratio = 0.05;
    elev_change = 0;
    sinAlphaLocal = 0;
    S = 0;
    cellDist_shadeFlag = true;
    g = 9.81;

    Cd_downslope = 0.0001;
    entrainment_coeff_downslope = 0.01;
    Cd_upslope = 0.2;
    entrainment_coeff_upslope = 0.2;

    epsilon = dem->get_cellSize()/1000.0;
    c1 = 5.31e-13;
    c2 = 60;
    c3 = 0.12;
    a1 = 990;
    a2 = -30;
    b1 = -0.75;
    b2 = 3.4;
    //	air.read_fluidProperties("air.prp");
    //	K = 0.4;
    gamma = 4.7;
    stop_crit = 0.01;

    l = 1e6;	//just make Monin-Obukov length large

    inputWindspeed = -1.0;


#ifdef CELL_DIURNAL_DEBUG
    std::cout << "End construct class cellDiurnal using cellDiurnal(class Dem dem)\tTime Elapsed: " << fxTimer.stop() << std::endl;
#endif
}

void cellDiurnal::initialize(double Xord, double Yord, double asp, double slp, double cloudCover, double airTemperature, double WindSpeed, double Z, double Albedo, double Bowen, double Cg, double Anthropogenic, double Roughness, double Rough_h, double Rough_d)
{
	#ifdef CELL_DIURNAL_DEBUG
		Stopwatch fxTimer;
		fxTimer.start();
		std::cout << "Construct class cellDiurnal using cellDiurnal(class Dem dem)" << std::endl;
	#endif
	
	xord = Xord;
	yord = Yord;
	aspect = asp;
	slope = slp;

	CloudCover = cloudCover;
	airTemp = airTemperature;
	inputWindspeed = WindSpeed;
	z = Z;
	albedo = Albedo;
	bowen = Bowen;
	cg = Cg;
	anthropogenic = Anthropogenic;
	roughness = Roughness;
	rough_h = Rough_h;
	rough_d = Rough_d;


	#ifdef CELL_DIURNAL_DEBUG
		std::cout << "End construct class cellDiurnal using cellDiurnal(class Dem dem)\tTime Elapsed: " << fxTimer.stop() << std::endl;
	#endif
}

void cellDiurnal::compute_solarIntensity()
{
	diurnalSolar.set_aspect(aspect);	//Set aspect and slope for this cell in the diurnalSolar class BEFORE we run it to compute solar intensity!
	diurnalSolar.set_slope(slope);
	diurnalSolar.call_solPos();
}

void cellDiurnal::compute_Qsw()	//compute incident shortwave radiation to cell based on Solpos and Holtslag and Ulden 1983 to correct for
							//cloud cover, water vapor, and dust (function also includes shadows)
{
	if((*shade)(i,j) == true)	//if shaded, set sinPsi to 0.0
	{
	    sinPsi = 0.0;
		//cout<<"shade == true"<<endl;
	}	
	else
	{
	    sinPsi = diurnalSolar.get_solarIntensity() / 1353.0;
		//cout<<"shade == false"<<endl;
	}	

	Qsw = (a1 * sinPsi + a2) * (1.0 + b1 * std::pow(CloudCover, b2));
	
}

void cellDiurnal::compute_Qh()	//compute sensible heat flux
{
/*	switch(veg)
	{
		case 0:	//grass
			A = 0.25;
			B = 1.0;
			cg = 0.15;
			Qf = 0.0;compute_Qsw
			break;
		case 1:	//brush
			A = 0.25;
			B = 1.0;
			cg = 0.15;
			Qf = 0.0;
			break;
		case 2:	//trees
			A = 0.10;
			B = 1.0;
			cg = 0.15;
			Qf = 0.0;
			break;
		default:
			A = 0.25;
			B = 1.0;
			cg = 0.15;
			Qf = 0.0;
			break;
	}
*/
	Qstar = ((1.0 - albedo) * Qsw + c1 * std::pow(airTemp,6) - 5.67e-8 * std::pow(airTemp,4) + c2 * CloudCover)/(1.0 + c3);

	Qh = (bowen / (1.0 + bowen)) * (Qstar * (1.0 - cg) + anthropogenic);
	
	if(inputWindspeed < 1.788)	//Check if windspeed is small (less than 4 mph or 1.788 m/s)
		inputWindspeed = 1.788;	//if it is, make 4 mph so diurnal computation of Qh heat flux is closer to correct at night (ie not very small because of small windspeed-->friction velocity)
						//this is my arbitrary number, but chosen as a typical diurnal windspeed

	//Determine which direction the wind should be
	if(Qh == 0)		//no diurnal wind
	{
		diurnal_wind = false;
		l = 1e6;	//just make Monin-Obukhov length large
		zm = z + rough_h - rough_d;	//Compute height above zero-displacement plane in log profile
		u_star = (K*inputWindspeed) / (log(zm/roughness) - stability_function(0.0, 1.0) + stability_function(0.0, 1.0));

	}else if(Qh > 0)	//upslope wind, now compute friction velocity and Monin-Obukov length using iterative procedure from CALMET
	{
		diurnal_wind = true;
		up_down = 0;
		
		//compute initial guess for friction velocity (assuming Monin-Obukov length is infinite, ie. neutral ABL)
		zm = z + rough_h - rough_d;	//Compute height above zero-displacement plane in log profile
		u_star = (K*inputWindspeed) / (log(zm/roughness) - stability_function(0.0, 1.0) + stability_function(0.0, 1.0));
		do
		{
			u_star_old = u_star;
			l = (-air.get_rho(airTemp)*air.get_cSubP(airTemp)*airTemp*u_star_old*u_star_old*u_star_old)/(K*g*Qh);
			u_star = (K*inputWindspeed) / (log(zm/roughness) - stability_function(zm/l, l) + stability_function(roughness/l, l));

		}while((fabs(1 - u_star/u_star_old)) > stop_crit);
	}
	else				//downslope wind, need to recompute Qh using computation for nightime and compute friction velocity and Monin-Obukov length using Van Ulden and Holtslag, 1985
	{
		diurnal_wind = true;
		up_down = 1;
		
		zm = z + rough_h - rough_d;	//Compute height above zero-displacement plane in log profile
		
/*		//Compute theta_star, u_star, and l using CALMET (has problem if windspeed is zero, get division by zero)
		Cdn = K/(log(zm/roughness));
		theta_star1 = 0.09*(1-0.5*CloudCover*CloudCover);
		theta_star2 = (airTemp*Cdn*inputWindspeed*inputWindspeed)/(4.0*gamma*zm*g);
		if(theta_star1 <= theta_star2)
		{
			theta_star = theta_star1;
		}else{
			theta_star = theta_star2;
		}
		u02 = (gamma*zm*g*theta_star)/(airTemp);
		C = 1 - (4*u02)/(Cdn*inputWindspeed*inputWindspeed);
		u_star = 0.5*Cdn*inputWindspeed*(1.0+std::pow(C,0.5));
		
		Qh = -air.get_rho(airTemp)*air.get_cSubP(airTemp)*u_star*theta_star;

		l = (-air.get_rho(airTemp)*air.get_cSubP(airTemp)*airTemp*u_star*u_star*u_star)/(K*g*Qh);
*/

		//Compute theta_star, u_star, and l using Van Ulden and Holtslag, 1985 (iteratively)
		//Compute initial u_star assuming neutral (ie. L is infinite)
		if(inputWindspeed == 0.0)	//if windspeed is 0, this gives somewhat of a problem in the rest of the computations, ie division by zero, so treat this seperately
		{
			u_star = 0.0;
			l = 1e6;	//just make Monin-Obukov length large, ie neutral ABL since doesn't really matter
			Qh = 0.0;	//heat flux is zero
		}else{
			u_star = (K*inputWindspeed) / (log(zm/roughness) - stability_function(0.0, 1.0) + stability_function(0.0, 1.0));
			d3 = (-Qsw*(1.0 - albedo) + 96.0 - 60.0*CloudCover)/2870;
			do
			{
				u_star_old = u_star;
				v_star = u_star_old/50.0;
				theta_star = airTemp*(std::pow((std::pow((15.0*v_star*v_star + 6600*v_star*v_star*v_star),2.0) + d3*v_star*v_star + 1.55*v_star*v_star*v_star),0.5) - 15.0*v_star*v_star - 6600*v_star*v_star*v_star);
				l = (u_star_old*u_star_old)/((K*g*theta_star)/airTemp);
				u_star = (K*inputWindspeed) / (log(zm/roughness) - stability_function(zm/l, l) +stability_function(roughness/l, l));
			}while((fabs(1 - u_star/u_star_old)) > stop_crit);
			Qh = -air.get_rho(airTemp)*air.get_cSubP(airTemp)*u_star*theta_star;
		}
	}
}

void cellDiurnal::compute_Bl_height()
{
	double f;	//Coriolis parameter
	double neutral_ABL_height = 0.0;
	
	if(diurnalSolar.get_latitude()<=90.0 && diurnalSolar.get_latitude()>=-90.0)
	{
		f = (1.4544e-4) * sin(pi/180 * diurnalSolar.get_latitude());	// f = 2 * omega * sin(theta)
															// f should be about 10^-4 for mid-latitudes
															// (1.4544e-4) here is 2 * omega = 2 * (2 * pi radians) / 24 hours = 1.4544e-4 seconds^-1
															// obtained from Stull 1988 book
		if(f<0)
				f = -f;
	}else{
		f = 1e-4;	//if latitude is not available, set f to mid-latitude value
	}
	
	//compute neutral ABL height
	neutral_ABL_height = 0.2 * u_star / f;	//from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)

	if(Qh >= 0) //unstable case (and neutral)
	{
		bl_height = neutral_ABL_height;	//Note that here I am assuming the convective boundary layer (CBL)
										// is just the height of a neutral ABL.  What could be done for a more
										// accurate height is to integrate over time starting in the morning.
										// The neutral ABL height used here should be an OK estimate for now??

	}else{	//stable case
		bl_height = 0.4 * std::pow((u_star * fabs(l) / f), 0.5);	//stable ABL height from Van Ulden and Holtslag 1985 (originally Zilitinkevich 1972)
		if(bl_height > neutral_ABL_height)	//limit height to be less than neutral ABL height (also from Van Ulden and Holtslag 1985)
			bl_height = neutral_ABL_height;
	}
}

void cellDiurnal::compute_cellHillDist()		//Computes distance to hilltop (downslope flow) or valley bottom (upslope flow) for ONE CELL;
{												// if:   up_down =		0 => upslope flow (compute distance to valley bottom)
												//					1 => downslope flow (compute distance to hill top)
	
	stepMultiplier = 1.5;		//indicates number of cell distances moved per step upslope or downslope along tracking path
								//BE SURE STEPMULTIPLIER IS GREATER THAN 1.41421... (SQUARE ROOT OF 2) OR TRACKING CORNER TO CORNER ON A CELL WON'T MAKE IT ACROSS THE CELL
	interp_order = AsciiGrid<double>::order1;		//indicates the order of interpolation used in the interpolation function
	
	dem->get_cellIndex(xord, yord, &i, &j);
	
	elevOld = (*dem)(i,j);
	elevNew = elevOld;

	if(slope == 0)	//if slope is zero, return distance of zero
		hillValleyDist = 0.0;
	
	xComponent = cos(n_to_xy(aspect)*pi/180.0);	//calculate vector in downhill direction (used for UPSLOPE FLOWS)
	yComponent = sin(n_to_xy(aspect)*pi/180.0);
	if(up_down == 1)	//if DOWNSLOPE FLOW, calculate the vector in the opposite direction
	{
		xComponent = xComponent*(-1.0);
		yComponent = yComponent*(-1.0);
	}
	
	dem->get_cellPosition(i, j, &xStart, &yStart);	//set initial X,Y and xStart,yStart to current i,j cell center
	X = xStart;
	Y = yStart;

	if(up_down == 0)	//if UPSLOPE FLOW
	{
		do	//track along path downhill
		{
			X = X + (stepMultiplier * xComponent * dem->get_cellSize());
			Y = Y + (stepMultiplier * yComponent * dem->get_cellSize());
			elevOld = elevNew;
			if(!dem->check_inBounds(X, Y))	//if we ran off the grid, break  out of loop
				break;
			elevNew = dem->interpolateGrid(X, Y, interp_order);
			if(cellDist_shadeFlag == true)
			{
				if(shade->interpolateGrid(X, Y, AsciiGrid<short>::order0) == shade->shaded)
					break;
			}
		}while(elevOld > elevNew);

	}else	//if DOWNSLOPE FLOW
	{
		do	//track along path uphill
		{
			X = X + (stepMultiplier * xComponent * dem->get_cellSize());
			Y = Y + (stepMultiplier * yComponent * dem->get_cellSize());
			elevOld = elevNew;
			if(!dem->check_inBounds(X, Y))	//if we ran off the grid, break  out of loop
				break;
			elevNew = dem->interpolateGrid(X, Y, interp_order);
			if(cellDist_shadeFlag == true)
			{
				if(shade->interpolateGrid(X, Y, AsciiGrid<short>::order0) == shade->unshaded)
					break;
			}
		}while(elevOld < elevNew);
	}

	X = X - (stepMultiplier * xComponent * dem->get_cellSize());	//step back to last position to get distance
	Y = Y - (stepMultiplier * yComponent * dem->get_cellSize());
	elev_change = fabs(elevOld - (*dem)(i,j));
    hillValleyDist = std::sqrt((X - xStart)*(X - xStart) + (Y - yStart)*(Y - yStart) + (elevOld - ((*dem)(i,j)))*(elevOld - (*dem)(i,j)));

	if(up_down == 1)	//if DOWNSLOPE FLOW, compute local sin(alpha)
	{
		X = xStart + (stepMultiplier * xComponent * dem->get_cellSize());
		Y = yStart + (stepMultiplier * yComponent * dem->get_cellSize());
		if(!dem->check_inBounds(X, Y))	//if we ran off the grid, let local sin(alpha) = 0
				sinAlphaLocal = 0.0;
		else		//if not off grid compute local sin(alpha)
		{
			dist = std::sqrt((X - xStart)*(X - xStart) + (Y - yStart)*(Y - yStart));
			sinAlphaLocal = sin(atan((fabs(dem->interpolateGrid(X, Y, interp_order) - (*dem)(i,j))) / dist));
		}
	}
}

void cellDiurnal::compute_S()
{
	if((diurnal_wind == false) || ((hillValleyDist - epsilon) < 0.0))
		S = 0;
	else if(up_down == 0)	//upslope wind
	{
		//S = std::pow(((Qh * g * elev_change) / (Cd_upslope + entrainment_coeff_upslope) * (air.get_rho(airTemp) * air.get_cSubP(airTemp) * airTemp)), 1.0/3.0);
		S = std::pow(((Qh * g * elev_change) / ((Cd_upslope + entrainment_coeff_upslope) * (air.get_rho(airTemp) * air.get_cSubP(airTemp) * airTemp))), 1.0/3.0);

	}else				//downslope wind
	{
		z_over_x = elev_change / hillValleyDist;
		if(z_over_x <= sinAlphaLocal)
			sinAlpha = z_over_x;
		else
			sinAlpha = sinAlphaLocal;
		
		Le = (0.05 * elev_change) / (Cd_downslope + entrainment_coeff_downslope);
          //if(Le == 0.0)
          //     std::cout << "stopped" << endl;

		S = std::pow(((-Qh * g * hillValleyDist * sinAlpha) / ((air.get_rho(airTemp) * air.get_cSubP(airTemp) * airTemp) *(Cd_downslope + entrainment_coeff_downslope))), 1.0/3.0)
			* std::pow((1.0 - std::pow(2.71828, -1.0 * hillValleyDist / Le)), 1.0/3.0);
	}
}

void cellDiurnal::compute_UVW()
{
	if(S == 0)
	{
		u = 0;
		v = 0;
		w = 0;
	}else
	{
		if(up_down == 0)	//upslope
		{
			theta = aspect - 180.0;
			if(theta < 0)
				theta += 360.0;
			phi = slope;
		}else			//downslope
		{
			theta = aspect;
			phi = -slope;
		}

		rThetaPhi_to_uvw(S, theta, phi, &u, &v, &w);	//compute (u,v,w) components from (r,theta,phi)
	}
}

double cellDiurnal::stability_function(double z_over_L, double L_switch)
{	//function that computes the stability function for the vertical wind profile (similarity theory)
	//z_over_L is the height divided by the Monin-Obukov length
	//L_switch indicates the sign of the Monin-Obukov length (either positive value for stable or negative value for unstable)
	
	if(L_switch >= 0.0)	//stable conditions
	{
//		value = -5.0*z_over_L;	//usual computation
		value = -17.0*(1.0 - exp(-0.29*z_over_L));	//improved computation from Van Ulden and Holtslag, 1985

	}else{	//unstable conditions
		xxx = std::pow((1.0-16.0*z_over_L),0.25);									//usual computation
		value = 2.0*log((1.0+xxx)/2.0) + log((1+xxx*xxx)/2.0) - 2.0*atan(xxx) + pi/2.0;
//		value = std::pow((1.0-16.0*z_over_L),0.25) - 1.0;	//improved computation from Van Ulden and Holtslag, 1985
	}

	return value;
}

void cellDiurnal::compute_cell_diurnal_wind(int I, int J, double *U, double *V, double *W, double *height, double *L, double *U_star, double *BL_height)	//Function computes diurnal (u,v,w) wind components for cell (i,j)
																			//	Note: other variables such as vegetation, cloud cover, etc. are specified at construction of cellDiurnal class or changed using set functions
{
	i = I;	//Set i,j of current cell
	j = J;

/*	if(slope == 0.0)	//if slope is zero, don't compute a diurnal wind
	{
		*U = 0.0;
		*V = 0.0;
		*W = 0.0;
		return;
	}
*/	

	compute_solarIntensity();
	compute_Qsw();
	compute_Qh();
	compute_Bl_height();

	if(diurnal_wind == false || slope == 0.0)
	{
		*U = 0.0;
		*V = 0.0;
		*W = 0.0;
		*L = l;
		*U_star = u_star;
		*BL_height = bl_height;
		*height = flow_thickness_ratio*elev_change;	//assumed to be height above "z=0" for log profile???
	}else
	{
		compute_cellHillDist();
		compute_S();
		compute_UVW();

		*U = u;
		*V = v;
		*W = w;
		*L = l;
		*U_star = u_star;
		*BL_height = bl_height;
		*height = flow_thickness_ratio*elev_change;	//assumed to be height above "z=0" for log profile???
	}
}

void cellDiurnal::compute_cell_diurnal_parameters(int I, int J, double *L, double *U_star, double *BL_height)
{
	i = I;	//Set i,j of current cell
	j = J;

/*	if(slope == 0.0)	//if slope is zero, don't compute a diurnal wind
	{
		*U = 0.0;
		*V = 0.0;
		*W = 0.0;
		return;
	}
*/	

	compute_solarIntensity();
	compute_Qsw();
	compute_Qh();
	compute_Bl_height();

	if(diurnal_wind == false || slope == 0.0)
	{
		*L = l;
		*U_star = u_star;
		*BL_height = bl_height;
	}else
	{
		*L = l;
		*U_star = u_star;
		*BL_height = bl_height;
	}

}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
