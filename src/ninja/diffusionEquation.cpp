/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Diffusion equation operations
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
#include "diffusionEquation.h"

DiffusionEquation::DiffusionEquation()
{

}

DiffusionEquation::DiffusionEquation(const Mesh *mesh, WindNinjaInputs *input)
{
    //Pointers to dynamically allocated memory
    PHI=NULL;
    xRHS=NULL;
    yRHS=NULL;
    zRHS=NULL;
    writePHIandRHS=false;
    phiOutFilename="!set";
    rhsOutFilename="!set";

    mesh_ = mesh;
    input_ = input; //NOTE: don't use for Com since input.Com is set to NULL in equals operator
    U0_.allocate(mesh_);
    fem.Initialize(mesh_, input_); //initialize the FEM object

    if(PHI == NULL)
        PHI=new double[mesh_->NUMNP];
    else
        throw std::runtime_error("Error allocating PHI field.");

    for(int i=0; i<mesh_->NUMNP; i++)
    {
        PHI[i]=0.;
    }

    heightAboveGround.allocate(mesh_);
    windSpeed.allocate(mesh_);
    windSpeedGradient.allocate(mesh_);

    for(int i = 0; i < mesh_->nrows; i++){
        for(int j = 0; j < mesh_->ncols; j++){
            for(int k = 0; k < mesh_->nlayers; k++){
                //find distance to ground at each node in mesh and write to wn_3dScalarField
                heightAboveGround(i,j,k) = mesh_->ZORD(i,j,k) - mesh_->ZORD(i,j,0);
            }
        }
    }
}

/**
 * Copy constructor.
 * @param A Copied value.
 */

DiffusionEquation::DiffusionEquation(DiffusionEquation const& A)
{
    PHI=A.PHI;
    xRHS=A.xRHS;
    yRHS=A.yRHS;
    zRHS=A.zRHS;
    writePHIandRHS=A.writePHIandRHS;
    phiOutFilename=A.phiOutFilename;
    rhsOutFilename=A.rhsOutFilename;
}

/**
 * Equals operator.
 * @param A Value to set equal to.
 * @return a copy of an object
 */

DiffusionEquation& DiffusionEquation::operator=(DiffusionEquation const& A)
{
    if(&A != this) {
        PHI=A.PHI;
        xRHS=A.xRHS;
        yRHS=A.yRHS;
        zRHS=A.zRHS;
        writePHIandRHS=A.writePHIandRHS;
        phiOutFilename=A.phiOutFilename;
        rhsOutFilename=A.rhsOutFilename;
    }
    return *this;
}

void DiffusionEquation::CalculateGradientsForDiffusion()
{
    for(int i = 0; i < mesh_->nrows; i++){
        for(int j = 0; j < mesh_->ncols; j++){
            for(int k = 0; k < mesh_->nlayers; k++){
                //compute and store wind speed at each node
                windSpeed(i,j,k) = std::sqrt(U0_.vectorData_x(i,j,k) * U0_.vectorData_x(i,j,k) +
                        U0_.vectorData_y(i,j,k) * U0_.vectorData_y(i,j,k));
            }
        }
    }

    //calculate and store dspeed/dx, dspeed/dy, dspeed/dz
    windSpeed.ComputeGradient(windSpeedGradient.vectorData_x,
                            windSpeedGradient.vectorData_y,
                            windSpeedGradient.vectorData_z);
}

DiffusionEquation::~DiffusionEquation()      //destructor
{

}

