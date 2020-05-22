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

FiniteElementMethodDiffusion::FiniteElementMethodDiffusion() : FiniteElementMethod()
{

}


FiniteElementMethodDiffusion::~FiniteElementMethodDiffusion()      //destructor
{

}

void FiniteElementMethodDiffusion::SetBoundaryConditions(const Mesh &mesh, WindNinjaInputs &input)
{

}

void CalculateRcoefficients(const Mesh &mesh, element &elem, int j)
{

//    for(int i = 0; i < mesh.nrows; i++){
//        for(int j = 0; j < mesh.ncols; j++){
//            for(int k = 0; k < mesh.nlayers; k++){
//                                                    
//            //find distance to ground at each node in mesh and write to wn_3dScalarField
//            heightAboveGround(i,j,k) = mesh.ZORD(i,j,k) - mesh.ZORD(i,j,0);
//                                
//            //compute and store wind velocity at each node
//            windVelocity(i,j,k) = std::sqrt(u(i,j,k) * u(i,j,k) + v(i,j,k) * v(i,j,k));
//                                                                                
//            }
//        }
//    }
//
//    /*
//     * calculate diffusivities
//     * velocityGradients.vectorData_z is the 3-d array with dspeed/dz
//     * Rz = 0.4 * heightAboveGround * du/dz
//     */
//
//    //calculates and stores dspeed/dx, dspeed/dy, dspeed/dz
//    windVelocity.ComputeGradient(input, velocityGradients);
//    for(int i = 0; i < mesh.nrows; i++){
//        for(int j = 0; j < mesh.ncols; j++){
//            for(int k = 0; k < mesh.nlayers; k++){
//                Rz(i,j,k) = 0.4 * heightAboveGround(i,j,k) * (*velocityGradients.vectorData_z)(i,j,k);
//                Rx(i,j,k) = 2 * Rz(i,j,k);
//                Ry(i,j,k) = 2 * Rz(i,j,k);
//            }
//        }
//    }

}

void CalculateHterm(const Mesh &mesh, element &elem, wn_3dVectorField &U0, int i)
{

}
