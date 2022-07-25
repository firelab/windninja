/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Implicit backward difference diffusion 
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
#include "implicitBackwardDifferenceDiffusion.h"

ImplicitBackwardDifferenceDiffusion::ImplicitBackwardDifferenceDiffusion() : DiffusionEquation()
{
    RHS=NULL;
    SK=NULL;
    row_ptr=NULL;
    col_ind=NULL;
    isBoundaryNode=NULL;
}

/**
 * Copy constructor.
 * @param A Copied value.
 */

ImplicitBackwardDifferenceDiffusion::ImplicitBackwardDifferenceDiffusion(ImplicitBackwardDifferenceDiffusion const& A)
: DiffusionEquation(A)
, U_(A.U_)
, matrixEquation(A.matrixEquation)
, scalarField(A.scalarField)
{
    RHS=A.RHS;
    SK=A.SK;
    row_ptr=A.row_ptr;
    col_ind=A.col_ind;
    isBoundaryNode=A.isBoundaryNode;
}

/**
 * Equals operator.
 * @param A Value to set equal to.
 * @return a copy of an object
 */

ImplicitBackwardDifferenceDiffusion& ImplicitBackwardDifferenceDiffusion::operator=(ImplicitBackwardDifferenceDiffusion const& A)
{
    if(&A != this) {
        DiffusionEquation::operator=(A);
        RHS=A.RHS;
        SK=A.SK;
        row_ptr=A.row_ptr;
        col_ind=A.col_ind;
        isBoundaryNode=A.isBoundaryNode;

        U_=A.U_;
        matrixEquation=A.matrixEquation;
        scalarField=A.scalarField;
    }
    return *this;
}

ImplicitBackwardDifferenceDiffusion::~ImplicitBackwardDifferenceDiffusion()      //destructor
{
    Deallocate();
}

void ImplicitBackwardDifferenceDiffusion::Discretize() 
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
    //    There are two implicit discretization schemes available for diffusion: backward difference (see p. 193
    //    in Thompson book) and central difference (see eq. 10.26 on p. 194 and p. 203-209 in 
    //    Thompson book). 
    
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

    cout<<"scalarField(25) = "<<scalarField(25)<<endl;
    fem.DiscretizeBackwardDifferenceDiffusion(SK, PHI, col_ind, row_ptr, scalarField, RHS, currentDt,
                                              heightAboveGround, windSpeedGradient);
}

void ImplicitBackwardDifferenceDiffusion::Initialize(const Mesh *mesh, WindNinjaInputs *input)
{
    mesh_ = mesh;
    input_ = input; //NOTE: don't use for Com since input.Com is set to NULL in equals operator

    if(PHI == NULL)
        PHI=new double[mesh_->NUMNP];
    else
        throw std::runtime_error("Error allocating PHI field.");
    if(RHS == NULL)
        RHS=new double[mesh_->NUMNP]; //This is the final right hand side (RHS) matrix
    else
        throw std::runtime_error("Error allocating RHS field.");
    if(isBoundaryNode == NULL)
        isBoundaryNode=new bool[mesh_->NUMNP]; //flag to specify if it's a boundary node
    else
        throw std::runtime_error("Error allocating isBoundaryNode field.");

    //allocate SK, row_ptr, col_ind and initialize col_ind, row_ptr
    SetupSKCompressedRowStorage();

    fem.Initialize(mesh_, input_);

    for(int i=0; i<mesh_->NUMNP; i++)
    {
        PHI[i]=0.;
        RHS[i]=0.;
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

void ImplicitBackwardDifferenceDiffusion::SetupSKCompressedRowStorage()
{
    int interrows=input_->dem.get_nRows()-2;
    int intercols=input_->dem.get_nCols()-2;
    int interlayers=mesh_->nlayers-2;
    int i, ii, j, jj, k, kk, l;

    //NZND is the # of nonzero elements in the SK stiffness array that are stored
    int NZND=(8*8)+(intercols*4+interrows*4+interlayers*4)*12+
         (intercols*interlayers*2+interrows*interlayers*2+intercols*interrows*2)*18+
         (intercols*interrows*interlayers)*27;

    //this is because we will only store the upper half of the SK matrix since it's symmetric
    NZND = (NZND - mesh_->NUMNP)/2 + mesh_->NUMNP;	

    //This is the final global stiffness matrix in Compressed Row Storage (CRS) and symmetric 
    SK = new double[NZND];

    //This holds the global column number of the corresponding element in the CRS storage
    col_ind=new int[NZND];

    //This holds the element number in the SK array (CRS) of the first non-zero entry for the
    //global row (the "+1" is so we can use the last entry to quit loops; ie. so we know how 
    //many non-zero elements are in the last node)
    row_ptr=new int[mesh_->NUMNP+1];

    int type; //This is the type of node (corner, edge, side, internal)
    int temp, temp1;

#pragma omp parallel for default(shared) private(i)
    for(i=0; i<mesh_->NUMNP; i++)
    {
        row_ptr[i]=0;
    }

#pragma omp parallel for default(shared) private(i)
    for(i=0; i<NZND; i++)
    {
        col_ind[i]=0;
        SK[i]=0.;
    }

    int row, col;

    //Set up Compressed Row Storage (CRS) format (only store upper triangular SK matrix)
    //temp stores the location (in the SK and col_ind arrays) where the first non-zero element
    //for the current row is located
    temp=0;
    for(k=0;k<mesh_->nlayers;k++)
    {
        for(i=0;i<input_->dem.get_nRows();i++)
        {
            for(j=0;j<input_->dem.get_nCols();j++) //Looping over all nodes using i,j,k notation
            {
                type=mesh_->get_node_type(i,j,k);
                if(type==0) //internal node
                {
                    row = k*input_->dem.get_nCols()*input_->dem.get_nRows()+i*input_->dem.get_nCols()+j;
                    row_ptr[row]=temp;
                    temp1=temp;
                    for(kk=-1;kk<2;kk++)
                    {
                        for(ii=-1;ii<2;ii++)
                        {
                            for(jj=-1;jj<2;jj++)
                            {
                                col = (k+kk)*input_->dem.get_nCols()*input_->dem.get_nRows()+
                                    (i+ii)*input_->dem.get_nCols()+(j+jj);
                                if(col >= row)	//only do if we're on the upper triangular part of SK
                                {
                                    col_ind[temp1]=col;
                                    temp1++;
                                }
                            }
                        }
                    }
                    temp=temp1;
                }
                else if(type==1) //face node
                {
                    row = k*input_->dem.get_nCols()*input_->dem.get_nRows()+i*input_->dem.get_nCols()+j;
                    row_ptr[row]=temp;
                    temp1=temp;
                    for(kk=-1;kk<2;kk++)
                    {
                        for(ii=-1;ii<2;ii++)
                        {
                            for(jj=-1;jj<2;jj++)
                            {
                                if(((i+ii)<0)||((i+ii)>(input_->dem.get_nRows()-1)))
                                    continue;
                                if(((j+jj)<0)||((j+jj)>(input_->dem.get_nCols()-1)))
                                    continue;
                                if(((k+kk)<0)||((k+kk)>(mesh_->nlayers-1)))
                                    continue;

                                col = (k+kk)*input_->dem.get_nCols()*input_->dem.get_nRows()+
                                    (i+ii)*input_->dem.get_nCols()+(j+jj);
                                if(col >= row) //only do if we're on the upper triangular part of SK
                                {
                                    col_ind[temp1]=col;
                                    temp1++;
                                }
                            }
                        }
                    }
                    temp=temp1;
                }
                else if(type==2) //edge node
                {
                    row = k*input_->dem.get_nCols()*input_->dem.get_nRows()+i*input_->dem.get_nCols()+j;
                    row_ptr[row]=temp;
                    temp1=temp;
                    for(kk=-1;kk<2;kk++)
                    {
                        for(ii=-1;ii<2;ii++)
                        {
                            for(jj=-1;jj<2;jj++)
                            {
                                if(((i+ii)<0)||((i+ii)>(input_->dem.get_nRows()-1)))
                                    continue;
                                if(((j+jj)<0)||((j+jj)>(input_->dem.get_nCols()-1)))
                                    continue;
                                if(((k+kk)<0)||((k+kk)>(mesh_->nlayers-1)))
                                    continue;

                                col = (k+kk)*input_->dem.get_nCols()*input_->dem.get_nRows()+
                                    (i+ii)*input_->dem.get_nCols()+(j+jj);
                                if(col >= row) //only do if we're on the upper triangular part of SK
                                {
                                    col_ind[temp1]=col;
                                    temp1++;
                                }
                            }
                        }
                    }
                    //temp=temp+12;
                    temp=temp1;
                }
                else if(type==3) //corner node
                {
                    row = k*input_->dem.get_nCols()*input_->dem.get_nRows()+i*input_->dem.get_nCols()+j;
                    row_ptr[row]=temp;
                    temp1=temp;
                    for(kk=-1;kk<2;kk++)
                    {
                        for(ii=-1;ii<2;ii++)
                        {
                            for(jj=-1;jj<2;jj++)
                            {
                                if(((i+ii)<0)||((i+ii)>(input_->dem.get_nRows()-1)))
                                    continue;
                                if(((j+jj)<0)||((j+jj)>(input_->dem.get_nCols()-1)))
                                    continue;
                                if(((k+kk)<0)||((k+kk)>(mesh_->nlayers-1)))
                                    continue;

                                col = (k+kk)*input_->dem.get_nCols()*input_->dem.get_nRows()+
                                    (i+ii)*input_->dem.get_nCols()+(j+jj);
                                if(col >= row) //only do if we're on the upper triangular part of SK
                                {
                                    col_ind[temp1]=col;
                                    temp1++;
                                }
                            }
                        }
                    }
                    temp=temp1;
                }
                else
                    throw std::logic_error("Error arranging SK array. Exiting...");
            }
        }
    }
    row_ptr[mesh_->NUMNP]=temp; //Set last value of row_ptr, so we can use "row_ptr+1" to use to index to in loops
}

/**
 * \brief Sets the boundary conditions.
 *
 *   Specify known values of PHI.
 *   This is done by replacing the particular node equation (row) with all zeros except a "1" on the 
 *   diagonal of SK[]. Then the corresponding row in RHS[] is replaced with the value of the known PHI.
 *   The other nodes that are "connected" to the known node need to have their equations adjusted accordingly.
 *   This is done by moving the term with the known value to the RHS.
 *
 * \return void
 */
void ImplicitBackwardDifferenceDiffusion::SetBoundaryConditions()
{
    int NPK, KNP;
    int i, j, k, l;

#pragma omp parallel default(shared) private(i,j,k,l,NPK,KNP)
    {
#pragma omp for
    for(k=0;k<mesh_->nlayers;k++)
    {
        for(i=0;i<input_->dem.get_nRows();i++)
        {
            for(j=0;j<input_->dem.get_nCols();j++) //loop over nodes using i,j,k notation
            {
                if(j==0||j==(input_->dem.get_nCols()-1)||i==0||i==(input_->dem.get_nRows()-1)||k==(mesh_->nlayers-1))  //Check the node to see if it is a boundary node
                    isBoundaryNode[k*input_->dem.get_nCols()*input_->dem.get_nRows()+i*input_->dem.get_nCols()+j]=true;
                else
                    isBoundaryNode[k*input_->dem.get_nCols()*input_->dem.get_nRows()+i*input_->dem.get_nCols()+j]=false;
            }
        }
    }
#pragma omp for
    for(k=0;k<mesh_->nlayers;k++)
    {
        for(i=0;i<input_->dem.get_nRows();i++)
        {
            for(j=0;j<input_->dem.get_nCols();j++) //loop over nodes using i,j,k notation
            {
                NPK=k*input_->dem.get_nCols()*input_->dem.get_nRows()+i*input_->dem.get_nCols()+j; //NPK is the global row number (also the node # we're on)
                for(l=row_ptr[NPK];l<row_ptr[NPK+1];l++) //loop through all non-zero elements for row NPK
                {
                    KNP=col_ind[l]; //KNP is the global column number we're on
                    if(isBoundaryNode[KNP]==true)
                        SK[l]=0.; //Set the value on the known PHI's column to zero
                    if(isBoundaryNode[NPK]==true)
                    {
                        if(KNP==NPK) //Check if we're at the diagonal
                            SK[l]=1.; //If so, set the value to 1
                        else
                            SK[l]=0.; //Set the value on the known PHI's row to zero
                    }
                }
                if(isBoundaryNode[NPK]==true)
                    RHS[NPK]=0.; //Phi is zero on "flow through boundaries"
            }
        }
    }
    }	//end parallel region
    if(isBoundaryNode)
    {
        delete[] isBoundaryNode;
        isBoundaryNode=NULL;
    }

}

/**
 * @brief Solver for diffusion 
 *
 * @param U1 Field to diffuse
 * @param U Field to dump diffusion results into
 * @param dt Current timestep size
 *
 * TODO: Fix calls to SolveConjugateGradient and SolveMinres
 *       double check how RHS, SK, and PHI are being used here
 *
 */
void ImplicitBackwardDifferenceDiffusion::Solve(wn_3dVectorField &U1, wn_3dVectorField &U, 
                              boost::posix_time::time_duration dt)
{
    U0_ = U1;
    currentDt = dt;
    int NPK;

    //set scalar fied to be diffused
    scalarField = U0_.vectorData_x;
    Discretize();

    //if the CG solver diverges, try the minres solver
    matrixEquation.InitializeConjugateGradient(mesh_->NUMNP);
    if(matrixEquation.SolveConjugateGradient(*input_, SK, PHI, RHS, row_ptr, col_ind)==false)
    {
        matrixEquation.InitializeMinres(mesh_->NUMNP);
        if(matrixEquation.SolveMinres(*input_, SK, PHI, RHS, row_ptr, col_ind)==false)
        {
            throw std::runtime_error("Solver returned false.");
        }
    }

    for(int k=0;k<mesh_->nlayers;k++)
    {
        for(int i=0;i<input_->dem.get_nRows();i++)
        {
            for(int j=0;j<input_->dem.get_nCols();j++) //loop over nodes using i,j,k notation
            {
                //get the node point number
                NPK = k*input_->dem.get_nCols()*input_->dem.get_nRows()+i*input_->dem.get_nCols()+j;
                U.vectorData_x(NPK) = PHI[NPK];
            }
        }
    }

    //set scalar fied to be diffused
    scalarField = U0_.vectorData_y;
    Discretize();

    //if the CG solver diverges, try the minres solver
    matrixEquation.InitializeConjugateGradient(mesh_->NUMNP);
    if(matrixEquation.SolveConjugateGradient(*input_, SK, PHI, RHS, row_ptr, col_ind)==false)
    {
        matrixEquation.InitializeMinres(mesh_->NUMNP);
        if(matrixEquation.SolveMinres(*input_, SK, PHI, RHS, row_ptr, col_ind)==false)
        {
            throw std::runtime_error("Solver returned false.");
        }
    }
    for(int k=0;k<mesh_->nlayers;k++)
    {
        for(int i=0;i<input_->dem.get_nRows();i++)
        {
            for(int j=0;j<input_->dem.get_nCols();j++) //loop over nodes using i,j,k notation
            {
                //get the node point number
                NPK = k*input_->dem.get_nCols()*input_->dem.get_nRows()+i*input_->dem.get_nCols()+j;
                U.vectorData_y(NPK) = PHI[NPK];
            }
        }
    }

    //set scalar fied to be diffused
    scalarField = U0_.vectorData_z;
    Discretize();

    //if the CG solver diverges, try the minres solver
    matrixEquation.InitializeConjugateGradient(mesh_->NUMNP);
    if(matrixEquation.SolveConjugateGradient(*input_, SK, PHI, RHS, row_ptr, col_ind)==false)
    {
        matrixEquation.InitializeMinres(mesh_->NUMNP);
        if(matrixEquation.SolveMinres(*input_, SK, PHI, RHS, row_ptr, col_ind)==false)
        {
            throw std::runtime_error("Solver returned false.");
        }
    }
    for(int k=0;k<mesh_->nlayers;k++)
    {
        for(int i=0;i<input_->dem.get_nRows();i++)
        {
            for(int j=0;j<input_->dem.get_nCols();j++) //loop over nodes using i,j,k notation
            {
                //get the node point number
                NPK = k*input_->dem.get_nCols()*input_->dem.get_nRows()+i*input_->dem.get_nCols()+j;
                U.vectorData_z(NPK) = PHI[NPK];
            }
        }
    }
}

void ImplicitBackwardDifferenceDiffusion::Deallocate()
{
    if(PHI)
    {	
        delete[] PHI;
        PHI=NULL;
    }
    if(RHS)
    {	
        delete[] RHS;
        RHS=NULL;
    }
    if(col_ind)
    {	
        delete[] col_ind;
        col_ind=NULL;
    }
    if(row_ptr)
    {
        delete[] row_ptr;
        row_ptr=NULL;
    }
    if(SK)
    {	
        delete[] SK;
        SK=NULL;
    }
    if(isBoundaryNode)
    {	
        delete[] isBoundaryNode;
        isBoundaryNode=NULL;
    }

    scalarField.deallocate();
}
