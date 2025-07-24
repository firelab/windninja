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
    input.inputDirection = wrap0to360( meanDir + input.dem.getAngleFromNorth() ); //convert FROM projected TO geographic coordinates

    CPLDebug( "WINDNINJA", "griddedInitialization::ninjaFoamInitializeFields()" );
    CPLDebug( "WINDNINJA", "input.inputSpeed = %lf", input.inputSpeed );
    CPLDebug( "WINDNINJA", "input.inputDirection (geographic coordinates) = %lf", input.inputDirection );
    CPLDebug( "WINDNINJA", "angleFromNorth (N_to_dem) = %lf", input.dem.getAngleFromNorth() );
    CPLDebug( "WINDNINJA", "input.inputDirection (projection coordinates) = %lf", meanDir );

    initializeBoundaryLayer(input);

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

    inputVelocityGrid.GDALReadGrid( input.speedInitGridFilename.c_str() );
    inputAngleGrid.GDALReadGrid( input.dirInitGridFilename.c_str() );

    // check that the initialization grids have same projection
    OGRSpatialReferenceH hSpeedSRS, hDirSRS;
    hSpeedSRS = OSRNewSpatialReference(inputVelocityGrid.prjString.c_str());
    hDirSRS = OSRNewSpatialReference(inputAngleGrid.prjString.c_str());
    if( hSpeedSRS == NULL )
    {
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error reading the input speed grid." );
        throw std::runtime_error("Could not make a spatial reference for input speed grid.");
    }
    if( hDirSRS == NULL )
    {
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error reading the input direction grid." );
        throw std::runtime_error("Could not make a spatial reference for input direction grid.");
    }

    if( !OSRIsSameEx( hSpeedSRS, hDirSRS, NULL ) )
    {
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error reading the input speed and direction grids." );
        throw std::runtime_error("The input speed grid does not have the same spatial reference as the input direction grid.");
    }

    // if the initialization grids have a different projection than the dem, need to warp them to the dem
    // with a corresponding coordinateTransformationAngle FROM the projection of the initialization grids TO the projection of the dem
    OGRSpatialReferenceH hDemSRS;
    hDemSRS = OSRNewSpatialReference(input.dem.prjString.c_str());
    if( hDemSRS == NULL )
    {
        throw std::runtime_error("Could not make a spatial reference for the dem.");
    }

    if( !OSRIsSameEx( hDirSRS, hDemSRS, NULL ) )
    {
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "warping the input speed and direction grids to the projection of the dem." );

        const char *pszDstWkt = input.dem.prjString.c_str();

        // compute the coordinateTransformationAngle, the angle between the y coordinate grid lines of the pre-warped and warped datasets,
        // going FROM the y coordinate grid line of the pre-warped dataset TO the y coordinate grid line of the warped dataset
        // in this case, going FROM initialization grid projection coordinates TO dem projection coordinates
        double coordinateTransformationAngle = 0.0;
        if( CSLTestBoolean(CPLGetConfigOption("DISABLE_ANGLE_FROM_NORTH_CALCULATION", "FALSE")) == false )
        {
            GDALDatasetH hDirDS = inputAngleGrid.ascii2GDAL();
            // direct calculation of FROM input_grid TO dem, already has the appropriate sign
            if(!GDALCalculateCoordinateTransformationAngle_FROM_src_TO_dst( hDirDS, coordinateTransformationAngle, pszDstWkt ))  // this is FROM input_grid TO dem
            {
                printf("Warning: Unable to calculate coordinate transform angle for the gridded initialization input speed and direction grids to dem coordinates.");
            }
            GDALClose(hDirDS);
        }

        // need an intermediate u and v set of ascii grids, for the warp
        // always warp u and v NOT speed and dir, to avoid angle interpolation issues (angles are periodic)
        AsciiGrid<double> uGrid;
        AsciiGrid<double> vGrid;
        uGrid.set_headerData(inputAngleGrid);
        vGrid.set_headerData(inputAngleGrid);
        for(int i=0; i<inputAngleGrid.get_nRows(); i++)
        {
            for(int j=0; j<inputAngleGrid.get_nCols(); j++)
            {
                wind_sd_to_uv(inputVelocityGrid(i,j), inputAngleGrid(i,j), &(uGrid)(i,j), &(vGrid)(i,j));
            }
        }

        GDALDatasetH uGrid_hDS = uGrid.ascii2GDAL();
        GDALDatasetH vGrid_hDS = vGrid.ascii2GDAL();

        GDALDatasetH uGrid_hVrtDS;
        GDALDatasetH vGrid_hVrtDS;

        // warp the u and v grids FROM initialization grid projection coordinates TO dem projection coordinates
        if(!GDALWarpToWKT_GDALAutoCreateWarpedVRT( uGrid_hDS, 1, uGrid_hVrtDS, pszDstWkt ))
        {
            throw std::runtime_error("Could not warp the input speed and direction grids (uGrid).");
        }
        if(!GDALWarpToWKT_GDALAutoCreateWarpedVRT( vGrid_hDS, 1, vGrid_hVrtDS, pszDstWkt ))
        {
            throw std::runtime_error("Could not warp the input speed and direction grids (vGrid).");
        }

        AsciiGrid<double> warped_uGrid;
        AsciiGrid<double> warped_vGrid;
        GDAL2AsciiGrid( (GDALDataset*)uGrid_hVrtDS, 1, warped_uGrid );
        GDAL2AsciiGrid( (GDALDataset*)vGrid_hVrtDS, 1, warped_vGrid );

        // update the inputVelocityGrid and inputAngleGrid to the new warped grid size and values
        // and use the coordinateTransformationAngle to correct each spd,dir, u,v dataset from the warp
        inputVelocityGrid.set_headerData(warped_uGrid);
        inputAngleGrid.set_headerData(warped_uGrid);
        for(int i=0; i<warped_uGrid.get_nRows(); i++)
        {
            for(int j=0; j<warped_uGrid.get_nCols(); j++)
            {
                // fill the grids with the raw values before the angle correction
                wind_uv_to_sd(warped_uGrid(i,j), warped_vGrid(i,j), &(inputVelocityGrid)(i,j), &(inputAngleGrid)(i,j));

                inputAngleGrid(i,j) = wrap0to360( inputAngleGrid(i,j) - coordinateTransformationAngle ); //convert FROM input_grid projection coordinates TO dem projected coordinates
                // always recalculate the u and v grids from the corrected dir grid, the changes need to go together
                // however, these u and v grids are not actually being used past this point
                //wind_sd_to_uv(inputVelocityGrid(i,j), inputAngleGrid(i,j), &(warped_uGrid)(i,j), &(warped_vGrid)(i,j));
            }
        }

        // close the gdal datasets
        GDALClose(uGrid_hVrtDS);
        GDALClose(vGrid_hVrtDS);
        GDALClose(uGrid_hDS);
        GDALClose(vGrid_hDS);

        // cleanup the intermediate grids
        warped_uGrid.deallocate();
        warped_vGrid.deallocate();
        uGrid.deallocate();
        vGrid.deallocate();
    }

    OSRDestroySpatialReference(hSpeedSRS);
    OSRDestroySpatialReference(hDirSRS);
    OSRDestroySpatialReference(hDemSRS);

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
