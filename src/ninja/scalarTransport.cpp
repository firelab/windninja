/******************************************************************************
 *
 * $Id:$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing scalar transport information.
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

#include "scalarTransport.h"

scalarTransport::scalarTransport()
{

}


scalarTransport::~scalarTransport()   // Destructor
{
    
}


/**
 * @brief Allocates storage for scalar transport variables
 *
 * @param mesh WindNinja computational mesh
 *
 */
void scalarTransport::allocate(Mesh const& mesh)
{
	Rx.allocate(&mesh);
	Ry.allocate(&mesh);
	Rz.allocate(&mesh);
	
	heightAboveGround.allocate(&mesh);
	windVelocity.allocate(&mesh);

}

/**
 * @brief Deallocates storage for scalar transport variables
 */
void scalarTransport::deallocate()
{
	Rx.deallocate();
	Ry.deallocate();
	Rz.deallocate();
	
	heightAboveGround.deallocate();
	windVelocity.deallocate();

}

/**
 * @brief Computes eddy diffusivities for scalar transport model
 *
 * @param input WindNinja inputs
 * @param mesh WindNinja computational mesh
 * @param u 3-d array of u-component of wind
 * @param v 3-d array of v-component of wind
 *
 */
 
void scalarTransport::computeDiffusivity(WindNinjaInputs &input,
                                         const Mesh &mesh, 
                                         const wn_3dScalarField &u,
                                         const wn_3dScalarField &v)
{
    
    for(int i = 0; i < mesh.nrows; i++){
        for(int j = 0; j < mesh.ncols; j++){
            for(int k = 0; k < mesh.nlayers; k++){
                
                //find distance to ground at each node in mesh and write to wn_3dScalarField
                heightAboveGround(i,j,k) = mesh.ZORD(i,j,k) - mesh.ZORD(i,j,0);
                
                //compute and store wind velocity at each node
                windVelocity(i,j,k) = std::sqrt(u(i,j,k) * u(i,j,k) + v(i,j,k) * v(i,j,k));
                
            }
        }
    }
    
    /*
     * calculate diffusivities
     * velocityGradients.vectorData_z is the 3-d array with dspeed/dz
     * Rz = 0.4 * heightAboveGround * du/dz
     */
     
    windVelocity.ComputeGradient(input, velocityGradients); //calculates and stores dspeed/dx, dspeed/dy, dspeed/dz
    
    ///Transform gradient vectors to be normal to ground surface (as in dust::ComputeUstar()) instead of along 
    ///usual z-coordinate ??

    for(int i = 0; i < mesh.nrows; i++){
        for(int j = 0; j < mesh.ncols; j++){
            for(int k = 0; k < mesh.nlayers; k++){
                
                Rz(i,j,k) = 0.4 * heightAboveGround(i,j,k) * (*velocityGradients.vectorData_z)(i,j,k);
                Rx(i,j,k) = 2 * Rz(i,j,k);
                Ry(i,j,k) = 2 * Rz(i,j,k);
                
            }
        }
    }
    
    // Send Rz, Rx, Ry to elem.RX, elem.RY, elem.RZ ?
    // units are fine.
    //gradients should be normal to surface? Need to compute facet normals here if so...
    
    // testing:
    std::string filename;
    AsciiGrid<double> testGrid;
    /*testGrid.set_headerData(mesh.ncols, mesh.nrows,
                            mesh.XORD(0,0,0), mesh.YORD(0,0,0),
                            mesh.meshResolution, -9999.0, 0.0,
                            input.dem.prjString.c_str());*/
    testGrid.set_headerData(input.dem);
    testGrid.set_noDataValue(-9999.0);
        
        for(int k = 0; k < mesh.nlayers; k++){
        for(int i = 0; i <mesh.nrows; i++){
            for(int j = 0; j < mesh.ncols; j++ ){
                testGrid(i,j) = Rz(i,j,k);
                filename = "Rz_" + boost::lexical_cast<std::string>(k);
            }
        }
        if(k == 10){
            testGrid.write_Grid(filename.c_str(), 2);
            
            std::string outFilename = "Rz.png";
            std::string scalarLegendFilename = "Rz";
            std::string legendTitle = "Rz";
            std::string legendUnits = "(m2/s)";
            bool writeLegend = TRUE;
	
            testGrid.replaceNan( -9999.0 );
            testGrid.ascii2png( outFilename, scalarLegendFilename, legendUnits, legendTitle, writeLegend );
            
        }
        }
    testGrid.deallocate();
    //------------------------------------------end testing*/

}
