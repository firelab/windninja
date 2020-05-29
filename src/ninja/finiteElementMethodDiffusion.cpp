/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Finite Element Method operations for diffusion
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
#include "finiteElementMethodDiffusion.h"

FiniteElementMethodDiffusion::FiniteElementMethodDiffusion(const Mesh &mesh,
        wn_3dVectorField &U0) : FiniteElementMethod()
{
    heightAboveGround.allocate(&mesh);
    windSpeed.allocate(&mesh);
    windSpeedGradient.allocate(&mesh);

    for(int i = 0; i < mesh.nrows; i++){
        for(int j = 0; j < mesh.ncols; j++){
            for(int k = 0; k < mesh.nlayers; k++){
                                                    
            //find distance to ground at each node in mesh and write to wn_3dScalarField
            heightAboveGround(i,j,k) = mesh.ZORD(i,j,k) - mesh.ZORD(i,j,0);
                                
            //compute and store wind speed at each node
            windSpeed(i,j,k) = std::sqrt(U0.vectorData_x(i,j,k) * U0.vectorData_x(i,j,k) +
                    U0.vectorData_y(i,j,k) * U0.vectorData_y(i,j,k));
            }
        }
    }

    //calculates and stores dspeed/dx, dspeed/dy, dspeed/dz
    windSpeed.ComputeGradient(windSpeedGradient.vectorData_x,
                            windSpeedGradient.vectorData_y,
                            windSpeedGradient.vectorData_z);
}


FiniteElementMethodDiffusion::~FiniteElementMethodDiffusion()      //destructor
{
    heightAboveGround.deallocate();
    windSpeed.deallocate();
    windSpeedGradient.deallocate();

}

void FiniteElementMethodDiffusion::SetBoundaryConditions(const Mesh &mesh, WindNinjaInputs &input)
{

}

void FiniteElementMethodDiffusion::CalculateRcoefficients(const Mesh &mesh, element &elem)
{
//    wn_3dScalarField heightAboveGround;
//    wn_3dScalarField windSpeed;
//    wn_3dVectorField windSpeedGradient;
//    heightAboveGround.allocate(&mesh);
//    windSpeed.allocate(&mesh);
//    windSpeedGradient.allocate(&mesh);

//    for(int i = 0; i < mesh.nrows; i++){
//        for(int j = 0; j < mesh.ncols; j++){
//            for(int k = 0; k < mesh.nlayers; k++){
//                                                    
//            //find distance to ground at each node in mesh and write to wn_3dScalarField
//            heightAboveGround(i,j,k) = mesh.ZORD(i,j,k) - mesh.ZORD(i,j,0);
//                                
//            //compute and store wind speed at each node
//            windSpeed(i,j,k) = std::sqrt(U0.vectorData_x(i,j,k) * U0.vectorData_x(i,j,k) +
//                    U0.vectorData_y(i,j,k) * U0.vectorData_y(i,j,k));
//            }
//        }
//    }

    /*
     * calculate diffusivities
     * windSpeedGradient.vectorData_z is the 3-d array with dspeed/dz
     * Rz = 0.4 * heightAboveGround * du/dz
     */

    //TODO:
    //elem.RZ, RX, RY need to be calcluated for current element. Need to get mesh info for current
    //element to grab correct windSpeedGradient and heightAboveGround.

    //calculates and stores dspeed/dx, dspeed/dy, dspeed/dz
//    windSpeed.ComputeGradient(windSpeedGradient.vectorData_x,
//                            windSpeedGradient.vectorData_y,
//                            windSpeedGradient.vectorData_z);
//
//    for(int i = 0; i < mesh.nrows; i++){
//        for(int j = 0; j < mesh.ncols; j++){
//            for(int k = 0; k < mesh.nlayers; k++){
//                elem.RZ(i,j,k) = 0.4 * heightAboveGround(i,j,k) * windSpeedGradient.vectorData_z(i,j,k);
//                elem.RX(i,j,k) = 2 * elem.RZ(i,j,k);
//                elem.RY(i,j,k) = 2 * elem.RZ(i,j,k);
//            }
//        }
//    }


    int elemNum = get_elemNum(elem_i, elem_j, elem_k);

    elem.RZ = 0.4 * heightAboveGround(i,j,k) * windSpeedGradient.vectorData_z(i,j,k);
    elem.RX = 2 * elem.RZ;
    elem.RY = 2 * elem.RZ;

}

void FiniteElementMethodDiffusion::CalculateHterm(const Mesh &mesh, element &elem, wn_3dVectorField &U0, int i)
{

}