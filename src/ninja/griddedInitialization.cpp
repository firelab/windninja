/******************************************************************************
*
* $Id:$
*
* Project:  WindNinja
* Purpose:  Initializing with gridded speed and direction 
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

#include "griddedInitialization.h"

griddedInitialization::griddedInitialization() : initialize()
{

}

griddedInitialization::~griddedInitialization()
{
    CPLDebug("NINJA", "Starting a griddedInitialization run.");
	
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
void griddedInitialization::initializeFields(WindNinjaInputs &input,
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

    CPLDebug("NINJA", "input.speedInitGridFilename = %s", input.speedInitGridFilename.c_str());
    CPLDebug("NINJA", "input.dirInitGridFilename = %s", input.dirInitGridFilename.c_str());
        
    AsciiGrid<double> inputVelocityGrid, inputAngleGrid;
        
    GDALDatasetH hSpeedDS, hDirDS;
    hSpeedDS = GDALOpen( input.speedInitGridFilename.c_str(), GA_ReadOnly );
    if( hSpeedDS == NULL )
    {
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error reading the input speed grid." );
        throw std::runtime_error("Can't open input speed grid.");
    }
    hDirDS = GDALOpen( input.dirInitGridFilename.c_str(), GA_ReadOnly );
    if( hDirDS == NULL )
    {
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error reading the input direction grid." );
        throw std::runtime_error("Can't open input direction grid.");
    }

    GDAL2AsciiGrid( (GDALDataset *)hSpeedDS, 1, inputVelocityGrid );
    GDAL2AsciiGrid( (GDALDataset *)hDirDS, 1, inputAngleGrid );
        
    GDALClose(hSpeedDS);
    GDALClose(hDirDS);
        
    //Check that the upper right corner covered by the input grids and buffer if needed
    //NOTE: Right now this is only for input grids with same prj and llcorner as DEM
    double corner2_x = input.dem.get_xllCorner() + input.dem.get_nCols() * input.dem.get_cellSize(); //corner 2
    double corner2_y = input.dem.get_yllCorner() + input.dem.get_nRows() * input.dem.get_cellSize();
    while( !inputVelocityGrid.check_inBounds(corner2_x, corner2_y) ){
        inputVelocityGrid.BufferGridInPlace();
        inputAngleGrid.BufferGridInPlace();
    }

    //Interpolate from input grids to dem coincident grids
    speedInitializationGrid.interpolateFromGrid(inputVelocityGrid, AsciiGrid<double>::order0);
    dirInitializationGrid.interpolateFromGrid(inputAngleGrid, AsciiGrid<double>::order0);
    
    //Set windspeed grid for diurnal computation
    input.surface.set_windspeed(speedInitializationGrid);
    
    CPLDebug("NINJA", "check for coincident grids: u = %d", speedInitializationGrid.checkForCoincidentGrids(uInitializationGrid));

    //set the u and v initialization grids
    for(int i=0; i<speedInitializationGrid.get_nRows(); i++) {
        for(int j=0; j<speedInitializationGrid.get_nCols(); j++) {
            wind_sd_to_uv(speedInitializationGrid(i,j), dirInitializationGrid(i,j), &(uInitializationGrid)(i,j), &(vInitializationGrid)(i,j));
        }
    }

    initializeWindToZero(mesh, u0, v0, w0);

    initializeDiurnal(input, cloud, L, u_star, bl_height, airTempGrid, speedInitializationGrid);
    
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

