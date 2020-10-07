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

#ifdef NINJAFOAM
/**
 * Sets input speed and direction from input grids.
 * @param input WindNinjaInputs object storing necessary input information.
 */
void griddedInitialization::ninjaFoamInitializeFields(WindNinjaInputs &input,
                                                    AsciiGrid<double> &cloud)
{
    setGridHeaderData(input, cloud);

    setInitializationGrids(input);

    //set average speed
    input.inputSpeed = speedInitializationGrid.get_meanValue();

    //average u and v components
    double meanU;
    double meanV;
    meanU = uInitializationGrid.get_meanValue();
    meanV = vInitializationGrid.get_meanValue();

    double meanSpd;
    double meanDir;

    wind_uv_to_sd(meanU, meanV, &meanSpd, &meanDir);

    //set average direction
    input.inputDirection = meanDir;

    cloud = cloudCoverGrid;
}
#endif //NINJAFOAM

void griddedInitialization::setInitializationGrids(WindNinjaInputs &input)
{
    //set initialization grids
    airTempGrid = input.airTemp;
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

    //Check that the initialization grids completely overlap the DEM
    if(!inputVelocityGrid.CheckForGridOverlap(input.dem) || !inputAngleGrid.CheckForGridOverlap(input.dem))
    {
        throw std::runtime_error("The input speed and direction grids do not completely overlap the DEM.");
    }

    //convert to base units
    velocityUnits::toBaseUnits(inputVelocityGrid, input.inputSpeedUnits);

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

    CPLDebug("NINJA", "check for coincident grids: u = %d", speedInitializationGrid.checkForCoincidentGrids(uInitializationGrid));
}
