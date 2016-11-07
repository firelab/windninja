/******************************************************************************
*
* $Id:$
*
* Project:  WindNinja
* Purpose:  Initializing with domain-average NinjaFOAM simulations for use with diurnal 
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

#include "foamDomainAverageInitialization.h"

foamDomainAverageInitialization::foamDomainAverageInitialization() : initialize()
{
    CPLDebug("NINJA", "Starting a foamDomainAverageInitialization run.");
    inputVelocityGrid = -9999.0;
    inputAngleGrid = -9999.0;
}

foamDomainAverageInitialization::~foamDomainAverageInitialization()
{
    inputVelocityGrid.deallocate();
    inputAngleGrid.deallocate();
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
void foamDomainAverageInitialization::initializeFields(WindNinjaInputs &input,
	Mesh const& mesh,
	wn_3dScalarField& u0,
	wn_3dScalarField& v0,
	wn_3dScalarField& w0,
	AsciiGrid<double>& cloud,
        AsciiGrid<double>& L,
        AsciiGrid<double>& u_star,
        AsciiGrid<double>& bl_height)
{
    setGridHeaderData(input, cloud, L, u_star, bl_height, airTempGrid);

    setUniformCloudCover(input, cloud);

    //switch that detemines what profile is used...	
    //make sure rough_h is set to zero if profile switch is 0 or 2
    profile.profile_switch = windProfile::monin_obukov_similarity;	
	
    setWn2dGrids(input);

    //Check that the upper right corner is covered by the input grids and buffer if needed
    double corner2_x = input.dem.get_xllCorner() + input.dem.get_nCols() * input.dem.get_cellSize(); //corner 2
    double corner2_y = input.dem.get_yllCorner() + input.dem.get_nRows() * input.dem.get_cellSize();
    
    while( !inputVelocityGrid.check_inBounds(corner2_x, corner2_y) )
    {
        inputVelocityGrid.BufferGridInPlace();
        inputAngleGrid.BufferGridInPlace();
        CPLDebug("NINJA", "Buffering in foamDomainAverageInitialization...");
    }

    //Interpolate from input grids to dem coincident grids
    speedInitializationGrid.interpolateFromGrid(inputVelocityGrid, AsciiGrid<double>::order0);
    dirInitializationGrid.interpolateFromGrid(inputAngleGrid, AsciiGrid<double>::order0);
    
    CPLDebug("NINJA", "check for coincident grids: speedInitializationGrid = %d",
            speedInitializationGrid.checkForCoincidentGrids(input.dem));
   
    int i, j;
    //set the u and v initialization grids
    for(int i=0; i<speedInitializationGrid.get_nRows(); i++) {
        for(int j=0; j<speedInitializationGrid.get_nCols(); j++) {
            wind_sd_to_uv(speedInitializationGrid(i,j),
                    dirInitializationGrid(i,j),
                    &(uInitializationGrid)(i,j),
                    &(vInitializationGrid)(i,j));
        }
    }

    initializeWindToZero(mesh, u0, v0, w0);

    initializeDiurnal(input, cloud, L, u_star, bl_height, airTempGrid);
    
    initializeWindFromProfile(input, mesh, L, bl_height, u0, v0, w0);

    if((input.diurnalWinds==true) && (profile.profile_switch==windProfile::monin_obukov_similarity))
    {
        addDiurnalComponent(input, mesh, u0, v0, w0);
    }
}

void foamDomainAverageInitialization::setWn2dGrids(WindNinjaInputs &input)
{
    inputVelocityGrid.set_headerData(input.dem);
    inputAngleGrid.set_headerData(input.dem);

    //Check that the upper right corner is covered by the input grids and buffer if needed
    double corner2_x = input.dem.get_xllCorner() + input.dem.get_nCols() * input.dem.get_cellSize(); //corner 2
    double corner2_y = input.dem.get_yllCorner() + input.dem.get_nRows() * input.dem.get_cellSize();
    
    while( !inputVelocityGrid.check_inBounds(corner2_x, corner2_y) )
    {
        inputVelocityGrid.BufferGridInPlace();
        inputAngleGrid.BufferGridInPlace();
        CPLDebug("NINJA", "Buffering in foamDomainAverageInitialization...");
    }

    //Interpolate from input grids to dem coincident grids
    speedInitializationGrid.interpolateFromGrid(inputVelocityGrid, AsciiGrid<double>::order0);
    dirInitializationGrid.interpolateFromGrid(inputAngleGrid, AsciiGrid<double>::order0);
    
    CPLDebug("NINJA", "check for coincident grids: speedInitializationGrid = %d",
            speedInitializationGrid.checkForCoincidentGrids(input.dem));
}
