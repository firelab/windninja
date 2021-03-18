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

    discretizationType = DiffusionEquation::lumpedCapacitance;
    //discretizationType = DiffusionEquation::centralDifference;

    //Pointers to dynamically allocated memory
    PHI=NULL;
    RHS=NULL;
    xRHS=NULL;
    yRHS=NULL;
    zRHS=NULL;
    SK=NULL;
    CL=NULL;
    writePHIandRHS=false;
    phiOutFilename="!set";
    rhsOutFilename="!set";
    stabilityUsingAlphasFlag=0;
    currentDt = boost::posix_time::seconds(0);
}

/**
 * Copy constructor.
 * @param A Copied value.
 */

DiffusionEquation::DiffusionEquation(DiffusionEquation const& A)
{
    PHI=A.PHI;
    RHS=A.RHS;
    xRHS=A.xRHS;
    yRHS=A.yRHS;
    zRHS=A.zRHS;
    SK=A.SK;
    CL=A.CL;
    currentDt = A.currentDt;
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
        RHS=A.RHS;
        xRHS=A.xRHS;
        yRHS=A.yRHS;
        zRHS=A.zRHS;
        SK=A.SK;
        CL=A.CL;
        currentDt = A.currentDt;
        writePHIandRHS=A.writePHIandRHS;
        phiOutFilename=A.phiOutFilename;
        rhsOutFilename=A.rhsOutFilename;
    }
    return *this;
}

DiffusionEquation::~DiffusionEquation()      //destructor
{
    Deallocate();
}

void DiffusionEquation::Diffusion() 
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


}


void DiffusionEquation::SetBoundaryConditions()
{

}


void DiffusionEquation::CalculateHterm(element &elem, int i)
{
    elem.HVJ=0.0;
}

/**
 * @brief Computes the R coefficients in the governing equations.
 *
 * @param elem A reference to the element which R is to be calculated in
 * @param j The quadrature point index
 *
 */
void DiffusionEquation::CalculateRcoefficients(element &elem, int j)
{
    /*
     * calculate diffusivities
     * windSpeedGradient.vectorData_z is the 3-d array with dspeed/dz
     * Rz = 0.4 * heightAboveGround * du/dz
     * 
     * TODO: Investigate other parameterizations for Rz.
     * See Stull p. 209 for several options used in the 
     * literature. Based on limited testing, the one commented out below
     * with the von Karman constant and height squared seems to 
     * produce too much diffusion.
     */

    //calculate elem.RZ, RX, RY for current element.
    double height = 0.;
    double speed = 0.;
    for(int k=0;k<mesh_.NNPE;k++) //Start loop over nodes in the element
    {
        height=height+elem.SFV[0*mesh_.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j]*
            heightAboveGround(elem.NPK);
        speed=speed+elem.SFV[0*mesh_.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j]*
            windSpeedGradient.vectorData_z(elem.NPK);
    }
    //0.41 is the von Karman constant
    //elem.RZ = 0.41*0.41 * height*height * fabs(speed);
    elem.RZ = 0.41 * height * fabs(speed);
    elem.RX = elem.RZ;
    elem.RY = elem.RZ;
    elem.RC = 1.;
}

DiffusionEquation::eDiscretizationType DiffusionEquation::GetDiscretizationType(std::string type)
{
    if(type == "centralDifference")
            return centralDifference;
    else if(type == "lumpedCapacitance")
        return lumpedCapacitance;
    else
        throw std::runtime_error(std::string("Cannot determine type in DiffusionEquation::GetDiscretizationType()."));
}

void DiffusionEquation::Initialize(const Mesh &mesh, WindNinjaInputs &input, wn_3dVectorField &U0)
{
    mesh_ = mesh;
    input_ = input; //NOTE: don't use for Com since input.Com is set to NULL in equals operator
    U0_ = U0;

    if(PHI == NULL)
        PHI=new double[mesh_.NUMNP];
    else
        throw std::runtime_error("Error allocating PHI field.");

    if(CL == NULL)
    {
        CL = new double[mesh_.NUMNP]; //lumped capacitence matrix for transient term in discretized diffusion equation
    }
    else
        throw std::runtime_error("Error allocating C field.");
    if(xRHS == NULL)
        xRHS=new double[mesh_.NUMNP]; //This is the final right hand side (RHS) matrix
    else
        throw std::runtime_error("Error allocating xRHS field.");
    if(yRHS == NULL)
        yRHS=new double[mesh_.NUMNP]; //This is the final right hand side (RHS) matrix
    else
        throw std::runtime_error("Error allocating yRHS field.");
    if(zRHS == NULL)
        zRHS=new double[mesh_.NUMNP]; //This is the final right hand side (RHS) matrix
    else
        throw std::runtime_error("Error allocating zRHS field.");

    heightAboveGround.allocate(&mesh_);
    windSpeed.allocate(&mesh_);
    windSpeedGradient.allocate(&mesh_);

    for(int i = 0; i < mesh_.nrows; i++){
        for(int j = 0; j < mesh_.ncols; j++){
            for(int k = 0; k < mesh_.nlayers; k++){
                //find distance to ground at each node in mesh and write to wn_3dScalarField
                heightAboveGround(i,j,k) = mesh_.ZORD(i,j,k) - mesh_.ZORD(i,j,0);
            }
        }
    }
    //The lumped-capacitance solver does not solve an Ax=b equation, so the SK matrix is not needed 
    if(!(diffusionDiscretizationType == GetDiscretizationType("lumpedCapacitance")))
    {
        SetupSKCompressedRowStorage();
    }
}

void DiffusionEquation::UpdateTimeVaryingValues(boost::posix_time::time_duration dt,
        wn_3dVectorField &U0)
{
    U0_ = U0;
    currentDt = dt;

    for(int i = 0; i < mesh_.nrows; i++){
        for(int j = 0; j < mesh_.ncols; j++){
            for(int k = 0; k < mesh_.nlayers; k++){
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

/**
 * @brief Solver for diffusion 
 *
 * @param U Field to dump diffusion results into
 * @param input Reference to WindNinjaInputs
 */
void DiffusionEquation::Solve(wn_3dVectorField &U, WindNinjaInputs &input)
{
    int NPK;

    if(diffusionDiscretizationType == GetDiscretizationType("lumpedCapacitance"))
    {
        for(int k=0;k<mesh_.nlayers;k++)
        {
            for(int i=0;i<input_.dem.get_nRows();i++)
            {
                for(int j=0;j<input_.dem.get_nCols();j++) //loop over nodes using i,j,k notation
                {
                    //get the node point number
                    NPK = k*input_.dem.get_nCols()*input_.dem.get_nRows()+i*input_.dem.get_nCols()+j;

                    //don't diffuse if it's an inlet
                    if(U0_.isOnGround(i,j,k)){
                        U.vectorData_x(NPK) = U0_.vectorData_x(NPK);
                        U.vectorData_y(NPK) = U0_.vectorData_y(NPK);
                        U.vectorData_z(NPK) = U0_.vectorData_z(NPK);
                    }
                    else{
                        U0_.vectorData_x(NPK) += xRHS[NPK]/CL[NPK]*(currentDt.total_microseconds()/1000000.0);
                        U0_.vectorData_y(NPK) += yRHS[NPK]/CL[NPK]*(currentDt.total_microseconds()/1000000.0);
                        U0_.vectorData_z(NPK) += zRHS[NPK]/CL[NPK]*(currentDt.total_microseconds()/1000000.0);

                        U.vectorData_x(NPK) = U0_.vectorData_x(NPK);
                        U.vectorData_y(NPK) = U0_.vectorData_y(NPK);
                        U.vectorData_z(NPK) = U0_.vectorData_z(NPK);
                    }
                }
            }
        }
    }
    else if(diffusionDiscretizationType == GetDiscretizationType("centralDifference"))
    {
        RHS = xRHS;
        //if(Solve(input)==false)
        //    throw std::runtime_error("Solver returned false.");
        for(int k=0;k<mesh_.nlayers;k++)
        {
            for(int i=0;i<input_.dem.get_nRows();i++)
            {
                for(int j=0;j<input_.dem.get_nCols();j++) //loop over nodes using i,j,k notation
                {
                    //get the node point number
                    NPK = k*input_.dem.get_nCols()*input_.dem.get_nRows()+i*input_.dem.get_nCols()+j;
                    U.vectorData_x(NPK) = PHI[NPK];
                }
            }
        }

        RHS = yRHS;
        //if(Solve(input)==false)
        //    throw std::runtime_error("Solver returned false.");
        for(int k=0;k<mesh_.nlayers;k++)
        {
            for(int i=0;i<input_.dem.get_nRows();i++)
            {
                for(int j=0;j<input_.dem.get_nCols();j++) //loop over nodes using i,j,k notation
                {
                    //get the node point number
                    NPK = k*input_.dem.get_nCols()*input_.dem.get_nRows()+i*input_.dem.get_nCols()+j;
                    U.vectorData_y(NPK) = PHI[NPK];
                }
            }
        }

        RHS = zRHS;
        //if(Solve(input)==false)
        //    throw std::runtime_error("Solver returned false.");
        for(int k=0;k<mesh_.nlayers;k++)
        {
            for(int i=0;i<input_.dem.get_nRows();i++)
            {
                for(int j=0;j<input_.dem.get_nCols();j++) //loop over nodes using i,j,k notation
                {
                    //get the node point number
                    NPK = k*input_.dem.get_nCols()*input_.dem.get_nRows()+i*input_.dem.get_nCols()+j;
                    U.vectorData_z(NPK) = PHI[NPK];
                }
            }
        }

        RHS = NULL;
    }
}
