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

void griddedInitialization::setInitializationGrids(WindNinjaInputs &input)
{
    //set initialization grids
    setCloudCover(input);

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
    
    CPLDebug("NINJA", "check for coincident grids: u = %d", speedInitializationGrid.checkForCoincidentGrids(uInitializationGrid));

    int i, j, k;
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
