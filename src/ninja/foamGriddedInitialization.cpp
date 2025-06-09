/******************************************************************************
*
* $Id:$
*
* Project:  WindNinja
* Purpose:  Initializing with gridded NinjaFOAM simulations for use with diurnal 
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

#include "foamGriddedInitialization.h"

foamGriddedInitialization::foamGriddedInitialization() : initialize()
{
    CPLDebug("NINJA", "Starting a foamGriddedInitialization run.");
    inputVelocityGrid = -9999.0;
    inputAngleGrid = -9999.0;    
}

foamGriddedInitialization::~foamGriddedInitialization()
{
    inputVelocityGrid.deallocate();
    inputAngleGrid.deallocate();
}

void foamGriddedInitialization::initializeFields(WindNinjaInputs &input,
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

void foamGriddedInitialization::setInitializationGrids(WindNinjaInputs &input)
{
    inputVelocityGrid = input.foamVelocityGrid;
    inputAngleGrid = input.foamAngleGrid;

    airTempGrid = input.airTemp;
    setCloudCover(input);

    setWn2dGrids(input);

    //set the u and v initialization grids
    for(int i=0; i<speedInitializationGrid.get_nRows(); i++) {
        for(int j=0; j<speedInitializationGrid.get_nCols(); j++) {
            wind_sd_to_uv(speedInitializationGrid(i,j),
                    dirInitializationGrid(i,j),
                    &(uInitializationGrid)(i,j),
                    &(vInitializationGrid)(i,j));
        }
    }
}

void foamGriddedInitialization::setWn2dGrids(WindNinjaInputs &input)
{
    //Check that the upper right corner is covered by the input grids and buffer if needed
    double corner2_x = input.dem.get_xllCorner() + input.dem.get_nCols() * input.dem.get_cellSize(); //corner 2
    double corner2_y = input.dem.get_yllCorner() + input.dem.get_nRows() * input.dem.get_cellSize();
    
    while( !inputVelocityGrid.check_inBounds(corner2_x, corner2_y) )
    {
        inputVelocityGrid.BufferGridInPlace();
        inputAngleGrid.BufferGridInPlace();
        CPLDebug("NINJA", "Buffering in foamGriddedInitialization...");
    }

    //convert speed/dir to u/v for direction interpolation
    AsciiGrid<double> inputUGrid;
    AsciiGrid<double> inputVGrid;
    inputUGrid.set_headerData(inputVelocityGrid);
    inputVGrid.set_headerData(inputVelocityGrid);

    for(int i=0; i<inputVelocityGrid.get_nRows(); i++) {
        for(int j=0; j<inputVelocityGrid.get_nCols(); j++) {
            wind_sd_to_uv(inputVelocityGrid(i,j), inputAngleGrid(i,j),
                    &(inputUGrid)(i,j), &(inputVGrid)(i,j));
        }
    }

    //Interpolate from input grids to dem coincident grids
    uInitializationGrid.interpolateFromGrid(inputUGrid, AsciiGrid<double>::order1);
    vInitializationGrid.interpolateFromGrid(inputVGrid, AsciiGrid<double>::order1);

    inputUGrid.deallocate();
    inputVGrid.deallocate();

    //fill the speed and dir initialization grids
    for(int i=0; i<speedInitializationGrid.get_nRows(); i++) {
        for(int j=0; j<speedInitializationGrid.get_nCols(); j++) {
            wind_uv_to_sd(uInitializationGrid(i,j), vInitializationGrid(i,j),
                     &(speedInitializationGrid)(i,j), &(dirInitializationGrid)(i,j));
        }
    }
}
