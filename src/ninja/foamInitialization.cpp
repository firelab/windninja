/******************************************************************************
*
* $Id:$
*
* Project:  WindNinja
* Purpose:  Initializing with NinjaFOAM simulations for use with diurnal 
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

#include "foamInitialization.h"

foamInitialization::foamInitialization() : initialize()
{

}

foamInitialization::~foamInitialization()
{
    CPLDebug("NINJA", "Starting a foamInitialization run.");
    inputVelocityGrid = -9999.0;
    inputAngleGrid = -9999.0;    
}

/**
 * This function initializes the 3d mesh wind field with initial velocity values
 * based on surface (2D) output from a NinjaFOAM soluation.
 * This 2D output is interpolated to the WindNinja grid using bilinear
 * interpolation, then, the diurnal components are added.
 * @param input WindNinjaInputs object
 * @param mesh associated mesh object
 * @param u0 u component
 * @param v0 v component
 * @param w0 w component
 * @see WindNinjaInputs, Mesh, wn_3dScalarField
 */
void foamInitialization::initializeFields(WindNinjaInputs &input,
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
    profile.profile_switch = windProfile::monin_obukov_similarity;	//switch that detemines what profile is used...	
    //make sure rough_h is set to zero if profile switch is 0 or 2
	
    AsciiGrid<double> speedInitializationGrid;
    speedInitializationGrid.set_headerData(input.dem);
    AsciiGrid<double> dirInitializationGrid;
    dirInitializationGrid.set_headerData(input.dem);
    AsciiGrid<double> uInitializationGrid;
    uInitializationGrid.set_headerData(input.dem);
    AsciiGrid<double> vInitializationGrid;
    vInitializationGrid.set_headerData(input.dem);
    
    cloud.set_headerData(input.dem);
    cloud = input.cloudCover;

    AsciiGrid<double> airTempGrid(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.xllCorner, input.dem.yllCorner, input.dem.cellSize, input.dem.get_noDataValue(), input.airTemp);
	AsciiGrid<double> cloudCoverGrid(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.xllCorner, input.dem.yllCorner, input.dem.cellSize, input.dem.get_noDataValue(), input.cloudCover);

    //Check that the upper right corner is covered by the input grids and buffer if needed
    double corner2_x = input.dem.get_xllCorner() + input.dem.get_nCols() * input.dem.get_cellSize(); //corner 2
    double corner2_y = input.dem.get_yllCorner() + input.dem.get_nRows() * input.dem.get_cellSize();
    
    while( !inputVelocityGrid.check_inBounds(corner2_x, corner2_y) )
    {
        inputVelocityGrid.BufferGridInPlace();
        inputAngleGrid.BufferGridInPlace();
        CPLDebug("NINJA", "Buffering in foamInitialization...");
    }

    //Interpolate from input grids to dem coincident grids
    speedInitializationGrid.interpolateFromGrid(inputVelocityGrid, AsciiGrid<double>::order0);
    dirInitializationGrid.interpolateFromGrid(inputAngleGrid, AsciiGrid<double>::order0);
    
    CPLDebug("NINJA", "check for coincident grids: speedInitializationGrid = %d", speedInitializationGrid.checkForCoincidentGrids(input.dem));
   
    //Set windspeed grid for diurnal computation
    input.surface.set_windspeed(speedInitializationGrid);

    //set the u and v initialization grids
    for(int i=0; i<speedInitializationGrid.get_nRows(); i++) {
        for(int j=0; j<speedInitializationGrid.get_nCols(); j++) {
            wind_sd_to_uv(speedInitializationGrid(i,j), dirInitializationGrid(i,j), &(uInitializationGrid)(i,j), &(vInitializationGrid)(i,j));
        }
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

    //Monin-Obukhov length, surface friction velocity, and atmospheric boundary layer height
    L.set_headerData(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0.0);
    u_star.set_headerData(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0.0);
    bl_height.set_headerData(input.dem.get_nCols(), input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), -1.0);

    //These are only needed if diurnal is turned on...
    AsciiGrid<double> height;	//height of diurnal flow above "z=0" in log profile
    AsciiGrid<double> uDiurnal;
    AsciiGrid<double> vDiurnal;
    AsciiGrid<double> wDiurnal;

    //compute diurnal wind, Monin-Obukhov length, surface friction velocity, and ABL height
    if(input.diurnalWinds == true)
    {
        height.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);	//height of diurnal flow above "z=0" in log profile
        uDiurnal.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);
        vDiurnal.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);
        wDiurnal.set_headerData(input.dem.get_nCols(),input.dem.get_nRows(), input.dem.get_xllCorner(), input.dem.get_yllCorner(), input.dem.get_cellSize(), input.dem.get_noDataValue(), 0);
        
        double aspect_temp = 0;	//just placeholder, basically
        double slope_temp = 0;	//just placeholder, basically

        Solar solar(input.ninjaTime, input.latitude, input.longitude, aspect_temp, slope_temp);
        Aspect aspect(&input.dem, input.numberCPUs);
        Slope slope(&input.dem, input.numberCPUs);
        Shade shade(&input.dem, solar.get_theta(), solar.get_phi(), input.numberCPUs);

        addDiurnal diurnal(&uDiurnal, &vDiurnal, &wDiurnal, &height, &L,
                        &u_star, &bl_height, &input.dem, &aspect, &slope,
                        &shade, &solar, &input.surface, &cloud,
                        &airTempGrid, input.numberCPUs, input.downDragCoeff,
                        input.downEntrainmentCoeff, input.upDragCoeff,
                        input.upEntrainmentCoeff);

    }else{	//compute neutral ABL height

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
        }
    else{
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
            u_star(i,j) = speedInitializationGrid(i,j)*0.4/(log((input.inputWindHeight+input.surface.Rough_h(i,j)-input.surface.Rough_d(i,j))/input.surface.Roughness(i,j)));

            //compute neutral ABL height
            bl_height(i,j) = 0.2 * u_star(i,j) / f;	//from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
            }
        }
    }
    
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
}

