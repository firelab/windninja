/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Abstract base class for initializing WindNinja wind fields
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

#include "initialize.h"

initialize::initialize()
{
    //make sure rough_h is set to zero if profile switch is 0 or 2
    //switch that detemines what profile is used...
    profile.profile_switch = windProfile::monin_obukov_similarity;
}

initialize::~initialize()
{	
    height.deallocate();
    uDiurnal.deallocate();
    vDiurnal.deallocate();
    wDiurnal.deallocate();
    uInitializationGrid.deallocate();
    vInitializationGrid.deallocate();
    airTempGrid.deallocate();
    cloudCoverGrid.deallocate();
    speedInitializationGrid.deallocate();
    dirInitializationGrid.deallocate();
}

void initialize::initializeWindToZero( Mesh const& mesh,
                                    wn_3dScalarField& u0,
                                    wn_3dScalarField& v0,
                                    wn_3dScalarField& w0)
{
    int i, j, k;

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

}

void initialize::initializeWindFromProfile(WindNinjaInputs &input,
                                const Mesh& mesh,
                                wn_3dScalarField& u0,
                                wn_3dScalarField& v0,
                                wn_3dScalarField& w0)
{
    int i, j, k;
//#pragma omp parallel for default(shared) firstprivate(profile) private(i,j,k)
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
}

void initialize::initializeBoundaryLayer(WindNinjaInputs& input)
{
    //Set windspeed grid for diurnal computation
    input.surface.set_windspeed(speedInitializationGrid);

    //compute diurnal wind, Monin-Obukhov length, surface friction velocity, and ABL height
    if(input.diurnalWinds == true)
    {
        double aspect_temp = 0;	//just placeholder, basically
        double slope_temp = 0;	//just placeholder, basically

        Solar solar(input.ninjaTime, input.latitude, input.longitude, aspect_temp, slope_temp);
        Aspect aspect(&input.dem, input.numberCPUs);
        Slope slope(&input.dem, input.numberCPUs);
        Shade shade(&input.dem, solar.get_theta(), solar.get_phi(), input.numberCPUs);

        addDiurnal(input, &aspect, &slope, &shade, &solar);

    }else{	//compute neutral ABL height

        int i, j, k;

        double f;

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
                u_star(i,j) = speedInitializationGrid(i,j)*0.4/
                            (log((input.inputWindHeight+input.surface.Rough_h(i,j)-
                                  input.surface.Rough_d(i,j))/input.surface.Roughness(i,j)));

                //compute neutral ABL height
                //from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
                bl_height(i,j) = 0.2 * u_star(i,j) / f;	
            }
        }
    }
}

void initialize::addDiurnal(WindNinjaInputs& input, Aspect const* asp, Slope const* slp,
                            Shade const* shd, Solar *inSolar) 
{
	cellDiurnal cDiurnal(&input.dem, shd, inSolar, input.downDragCoeff,
                             input.downEntrainmentCoeff, input.upDragCoeff,
                             input.upEntrainmentCoeff);

	double u_, v_, w_, height_, L_, U_star_, BL_height_, Xord, Yord, WindSpeed;
	int i,j;

	// DO THE WORK
//	#pragma omp parallel for default(none) private(i,j,u_,v_,w_,height_,L_,U_star_,BL_height_, Xord, Yord, WindSpeed, Z) firstprivate(cDiurnal) shared(input.dem,uDiurnal,vDiurnal,wDiurnal,height,L,u_star,bl_height, airTempGrid, asp, cloudCoverGrid, slp, input.surface)
    for(i = 0; i < input.dem.get_nRows(); i++)
    {
        for(j = 0; j < input.dem.get_nCols(); j++)
        {
            WindSpeed = speedInitializationGrid(i,j);

            input.dem.get_cellPosition(i, j, &Xord, &Yord);

            cDiurnal.initialize(Xord, Yord, (*asp)(i,j),(*slp)(i,j),
                    cloudCoverGrid(i,j), airTempGrid(i,j), WindSpeed, input.surface.Z,
                    input.surface.Albedo(i,j), input.surface.Bowen(i,j),
                    input.surface.Cg(i,j), input.surface.Anthropogenic(i,j),
                    input.surface.Roughness(i,j), input.surface.Rough_h(i,j),
                    input.surface.Rough_d(i,j));

            cDiurnal.compute_cell_diurnal_wind(i, j, &u_, &v_, &w_,
                    &height_, &L_, &U_star_, &BL_height_);

            uDiurnal.set_cellValue(i, j, u_);
            vDiurnal.set_cellValue(i, j, v_);
            wDiurnal.set_cellValue(i, j, w_);
            height.set_cellValue(i, j, height_);
            L.set_cellValue(i, j, L_);
            u_star.set_cellValue(i, j, U_star_);
            bl_height.set_cellValue(i, j, BL_height_);
        }
    }
}

void initialize::addDiurnalComponent(WindNinjaInputs &input,
                                    const Mesh& mesh,
                                    wn_3dScalarField& u0,
                                    wn_3dScalarField& v0,
                                    wn_3dScalarField& w0)
{
    int i, j, k;
    double AGL=0; //height above top of roughness elements
#pragma omp parallel for default(shared) private(i,j,k,AGL)
    //start at 1, not zero because ground nodes must be zero for boundary conditions to work properly
    for(k=1;k<mesh.nlayers;k++)	
    {
        for(i=0;i<mesh.nrows;i++)
        {
            for(j=0;j<mesh.ncols;j++)
            {
                //this is height above THE GROUND!! (not "z=0" for the log profile)
                AGL=mesh.ZORD(i, j, k)-input.dem(i,j);

                if((AGL - input.surface.Rough_d(i,j)) < height(i,j))
                {
                    u0(i, j, k) += uDiurnal(i,j);
                    v0(i, j, k) += vDiurnal(i,j);
                    w0(i, j, k) += wDiurnal(i,j);
                }
            }
        }
    }
}

void initialize::setCloudCover(WindNinjaInputs &input)
{
    //Set cloud grid
    cloudCoverGrid = input.cloudCover;
}

void initialize::setUniformCloudCover(WindNinjaInputs &input,
                                    AsciiGrid<double> cloud)
{
    //Set 1x1 cloud grid
    int longEdge = input.dem.get_nRows();
    if(input.dem.get_nRows() < input.dem.get_nCols())
        longEdge = input.dem.get_nCols();

    double tempCloudCover;
    if(input.cloudCover < 0)
        tempCloudCover = 0.0;
    else
        tempCloudCover = input.cloudCover;

    cloud.set_headerData(1, 1, input.dem.get_xllCorner(),
                        input.dem.get_yllCorner(),
                        (longEdge * input.dem.cellSize),
                        -9999.0, tempCloudCover, input.dem.prjString);
}

void initialize::setGridHeaderData(WindNinjaInputs& input, AsciiGrid<double>& cloud)
{
    L.set_headerData(input.dem);
    u_star.set_headerData(input.dem);
    bl_height.set_headerData(input.dem);
    height.set_headerData(input.dem);
    uDiurnal.set_headerData(input.dem);
    vDiurnal.set_headerData(input.dem);
    wDiurnal.set_headerData(input.dem);
    airTempGrid.set_headerData(input.dem);
    cloudCoverGrid.set_headerData(input.dem);
    cloud.set_headerData(input.dem);
    speedInitializationGrid.set_headerData(input.dem);
    dirInitializationGrid.set_headerData(input.dem);
    uInitializationGrid.set_headerData(input.dem);
    vInitializationGrid.set_headerData(input.dem);
}
