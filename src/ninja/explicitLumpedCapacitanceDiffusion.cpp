/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Explicit lumped-capacitance diffusion operations
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
#include "explicitLumpedCapacitanceDiffusion.h"

ExplicitLumpedCapacitanceDiffusion::ExplicitLumpedCapacitanceDiffusion() : DiffusionEquation()
{
   CL=NULL;
}

/**
 * Copy constructor.
 * @param A Copied value.
 */

ExplicitLumpedCapacitanceDiffusion::ExplicitLumpedCapacitanceDiffusion(ExplicitLumpedCapacitanceDiffusion const& A)
: DiffusionEquation(A)
{
    CL=A.CL;
}

/**
 * Equals operator.
 * @param A Value to set equal to.
 * @return a copy of an object
 */

ExplicitLumpedCapacitanceDiffusion& ExplicitLumpedCapacitanceDiffusion::operator=(ExplicitLumpedCapacitanceDiffusion const& A)
{
    if(&A != this) {
        DiffusionEquation::operator=(A);
        CL=A.CL;
    }
    return *this;
}

ExplicitLumpedCapacitanceDiffusion::~ExplicitLumpedCapacitanceDiffusion()      //destructor
{
    Deallocate();
}

void ExplicitLumpedCapacitanceDiffusion::Discretize() 
{
    //The governing equation to solve for diffusion of the velocity field is:
    //
    //    d        dPhi      d        dPhi      d        dPhi            dPhi
    //   ---- ( Rx ---- ) + ---- ( Ry ---- ) + ---- ( Rz ---- ) + H - Rc ---- = 0.0
    //    dx        dx       dy        dy       dz        dz              dt
    //
    //    where:
    //
    //    Phi = Ux, Uy, Uz --> the current velocity field
    //    Rz = 0.4 * heightAboveGround * du/dz
    //    Rx = Ry = Rz
    //    H = source term, 0 for now
    //    Rc = 1
    //
    //    There are two discretization schemes available for diffusion: lumped-capacitance (see p. 195
    //    in Thompson book) and central difference (see eq. 10.26 on p. 194 and p. 203-209 in 
    //    Thompson book). 
    //        

        fem.DiscretizeLumpedCapacitenceDiffusion(U0_, xRHS, yRHS, zRHS, CL, heightAboveGround,
                windSpeedGradient);
}

void ExplicitLumpedCapacitanceDiffusion::Initialize(const Mesh *mesh, WindNinjaInputs *input)
{
    mesh_ = mesh;
    input_ = input; //NOTE: don't use for Com since input.Com is set to NULL in equals operator
    U0_.allocate(mesh_);
    fem.Initialize(mesh_, input_); //initialize the FEM object

    if(PHI == NULL)
        PHI=new double[mesh_->NUMNP];
    else
        throw std::runtime_error("Error allocating PHI field.");
    if(CL == NULL)
    {
        CL = new double[mesh_->NUMNP]; //lumped capacitence matrix for transient term in discretized diffusion equation
    }
    else
        throw std::runtime_error("Error allocating CL field.");
    if(xRHS == NULL)
        xRHS=new double[mesh_->NUMNP]; //This is the final right hand side (RHS) matrix
    else
        throw std::runtime_error("Error allocating xRHS field.");
    if(yRHS == NULL)
        yRHS=new double[mesh_->NUMNP]; //This is the final right hand side (RHS) matrix
    else
        throw std::runtime_error("Error allocating yRHS field.");
    if(zRHS == NULL)
        zRHS=new double[mesh_->NUMNP]; //This is the final right hand side (RHS) matrix
    else
        throw std::runtime_error("Error allocating zRHS field.");

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
 * @brief Solver for explicit lumped-capacitance diffusion 
 *
 * @param U1 Field to diffuse
 * @param U Field to dump diffusion results into
 * @param dt The current time step size
 *
 */
void ExplicitLumpedCapacitanceDiffusion::Solve(wn_3dVectorField &U1, wn_3dVectorField &U, 
                              boost::posix_time::time_duration dt)
{
    U0_ = U1; //update the initial velocity field
    Discretize(); //discretize CL matrix and RHS terms

    int NPK;

    CPLDebug("SEMI_LAGRANGIAN", "Solving diffusion with lumped capacitence....");
    for(int k=0;k<mesh_->nlayers;k++)
    {
        for(int i=0;i<input_->dem.get_nRows();i++)
        {
            for(int j=0;j<input_->dem.get_nCols();j++) //loop over nodes using i,j,k notation
            {
                //get the node point number
                NPK = k*input_->dem.get_nCols()*input_->dem.get_nRows()+i*input_->dem.get_nCols()+j;

                //don't diffuse if it's an inlet
                if(U0_.isOnGround(i,j,k)){
                    U.vectorData_x(NPK) = U0_.vectorData_x(NPK);
                    U.vectorData_y(NPK) = U0_.vectorData_y(NPK);
                    U.vectorData_z(NPK) = U0_.vectorData_z(NPK);
                }
                else{
                    U0_.vectorData_x(NPK) += xRHS[NPK]/CL[NPK]*(dt.total_microseconds()/1000000.0);
                    U0_.vectorData_y(NPK) += yRHS[NPK]/CL[NPK]*(dt.total_microseconds()/1000000.0);
                    U0_.vectorData_z(NPK) += zRHS[NPK]/CL[NPK]*(dt.total_microseconds()/1000000.0);

                    U.vectorData_x(NPK) = U0_.vectorData_x(NPK);
                    U.vectorData_y(NPK) = U0_.vectorData_y(NPK);
                    U.vectorData_z(NPK) = U0_.vectorData_z(NPK);
                }
            }
        }
    }
}

void ExplicitLumpedCapacitanceDiffusion::Deallocate()
{
    if(PHI)
    {	
        delete[] PHI;
        PHI=NULL;
    }
    if(xRHS)
    {	
        delete[] xRHS;
        xRHS=NULL;
    }
    if(yRHS)
    {	
        delete[] yRHS;
        yRHS=NULL;
    }
    if(zRHS)
    {	
        delete[] zRHS;
        zRHS=NULL;
    }
    if(CL)
    {	
        delete[] CL;
        CL=NULL;
    }
}
