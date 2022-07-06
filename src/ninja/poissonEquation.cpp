/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Poisson's equation operations
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
#include "poissonEquation.h"

PoissonEquation::PoissonEquation()
{
    //Pointers to dynamically allocated memory
    mesh_=NULL;
    input_=NULL;
    PHI=NULL;
    RHS=NULL;
    SK=NULL;
    row_ptr=NULL;
    col_ind=NULL;
    isBoundaryNode=NULL;
    phiOutFilename="!set";
    rhsOutFilename="!set";
    stabilityUsingAlphasFlag=0;
    alphaH=1.0;
}

/**
 * Copy constructor.
 * @param A Copied value.
 */

PoissonEquation::PoissonEquation(PoissonEquation const& A)
: alphaVfield(A.alphaVfield)
, U_(A.U_)
, mesh_(A.mesh_)
, input_(A.input_)
, U0_(A.U0_)
, fem(A.fem)
, matrixEquation(A.matrixEquation)
{
    PHI=A.PHI;
    RHS=A.RHS;
    SK=A.SK;
    row_ptr=A.row_ptr;
    col_ind=A.col_ind;
    isBoundaryNode=A.isBoundaryNode;
    phiOutFilename=A.phiOutFilename;
    rhsOutFilename=A.rhsOutFilename;
    stabilityUsingAlphasFlag=A.stabilityUsingAlphasFlag;
    alphaH=A.alphaH;
}

/**
 * Equals operator.
 * @param A Value to set equal to.
 * @return a copy of an object
 */

PoissonEquation& PoissonEquation::operator=(PoissonEquation const& A)
{
    if(&A != this) {
        PHI=A.PHI;
        RHS=A.RHS;
        SK=A.SK;
        row_ptr=A.row_ptr;
        col_ind=A.col_ind;
        isBoundaryNode=A.isBoundaryNode;
        phiOutFilename=A.phiOutFilename;
        rhsOutFilename=A.rhsOutFilename;
        stabilityUsingAlphasFlag=A.stabilityUsingAlphasFlag;
        alphaH=A.alphaH;

        alphaVfield=A.alphaVfield;
        U_=A.U_;
        mesh_=A.mesh_;
        input_=A.input_;
        U0_=A.U0_;
        fem=A.fem;
        matrixEquation=A.matrixEquation;
    }
    return *this;
}

PoissonEquation::~PoissonEquation()      //destructor
{
    Deallocate();
}

void PoissonEquation::Discretize() 
{
    //The governing equation to solve is
    //
    //    d        dPhi      d        dPhi      d        dPhi
    //   ---- ( Rx ---- ) + ---- ( Ry ---- ) + ---- ( Rz ---- ) + H = 0.0
    //    dx        dx       dy        dy       dz        dz
    //
    //
    //    where, for conservation of mass:
    //                    1                          1
    //    Rx = Ry =  ------------          Rz = ------------
    //                2*alphaH^2                 2*alphaV^2
    //
    //         du0     dv0     dz0
    //    H = ----- + ----- + -----
    //         dx      dy      dz
    //

    fem.Discretize(SK, RHS, col_ind, row_ptr, U0_, alphaH, alphaVfield);
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
void PoissonEquation::SetBoundaryConditions()
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
}

void PoissonEquation::Deallocate()
{
    if(PHI)
    {	
        delete[] PHI;
        PHI=NULL;
    }
    if(SK)
    {	
        delete[] SK;
        SK=NULL;
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
    if(isBoundaryNode)
    {
        delete[] isBoundaryNode;
        isBoundaryNode=NULL;
    }
    if(RHS)
    {	
        delete[] RHS;
        RHS=NULL;
    }

    alphaVfield.deallocate();
}

/**
 * @brief Sets up compressed row storage for the SK array.
 *
 */
void PoissonEquation::SetupSKCompressedRowStorage()
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
        PHI[i]=0.;
        RHS[i]=0.;
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
 * \brief Sets the 3-D stability field for the projection equation object.
 *
 * \param input A reference to the WindNinja inputs.
 * \param CloudGrid A reference to the cloud grid.
 * \param init A reference to the initialization object for this run.
 *
 * \return void
 */
void PoissonEquation::SetAlphaCoefficients(WindNinjaInputs &input,
                AsciiGrid<double> &CloudGrid,
                boost::shared_ptr<initialize> &init)
{
    CPLDebug("STABILITY", "input.initializationMethod = %i\n", input.initializationMethod);
    CPLDebug("STABILITY", "input.stabilityFlag = %i\n", input.stabilityFlag);
    Stability stb(input);
    alphaVfield.allocate(mesh_);

    if(stabilityUsingAlphasFlag==0) // if stabilityFlag not set
    {
        for(unsigned int k=0; k<mesh_->nlayers; k++)
        {
            for(unsigned int i=0; i<mesh_->nrows;i++)
            {
                for(unsigned int j=0; j<mesh_->ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/1.0;
                }
            }
        }
    }
    else if(stabilityUsingAlphasFlag==1 && input.alphaStability!=-1) // if the alpha was specified directly in the CLI
    {
        for(unsigned int k=0; k<mesh_->nlayers; k++)
        {
            for(unsigned int i=0; i<mesh_->nrows; i++)
            {
                for(unsigned int j=0; j<mesh_->ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/input.alphaStability;
                }
            }
        }
    }
    else if(stabilityUsingAlphasFlag==1 &&
            input.initializationMethod==WindNinjaInputs::domainAverageInitializationFlag) //it's a domain-average run
    {
        stb.SetDomainAverageAlpha(input, *mesh_);  //sets alpha based on incident solar radiation
        for(unsigned int k=0; k<mesh_->nlayers; k++)
        {
            for(unsigned int i=0; i<mesh_->nrows; i++)
            {
                for(unsigned int j=0;j<input.dem.get_nCols();j++)
                {
                    alphaVfield(i,j,k) = alphaH/stb.alphaField(i,j,k);
                }
            }
        }
    }
    else if(stabilityUsingAlphasFlag==1 &&
            input.initializationMethod==WindNinjaInputs::pointInitializationFlag) //it's a point-initialization run
    {
        stb.SetPointInitializationAlpha(input, *mesh_);
        for(unsigned int k=0; k<mesh_->nlayers; k++)
        {
            for(unsigned int i=0; i<mesh_->nrows; i++)
            {
                for(unsigned int j=0; j<mesh_->ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/stb.alphaField(i,j,k);
                }
            }
        }
    }
    //If the run is a 2D WX model run
    else if(stabilityUsingAlphasFlag==1 &&
            input.initializationMethod==WindNinjaInputs::wxModelInitializationFlag &&
            init->getForecastIdentifier()!="WRF-3D") //it's a 2D wx model run
    {
        stb.Set2dWxInitializationAlpha(input, *mesh_, CloudGrid);

        for(unsigned int k=0; k<mesh_->nlayers; k++)
        {
            for(unsigned int i=0; i<mesh_->nrows; i++)
            {
                for(unsigned int j=0; j<mesh_->ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/stb.alphaField(i,j,k);
                }
            }
        }
    }
    //If the run is a WX model run where the full WX vertical profile is used and
    //the potential temp (theta) is available, then use the method below
    else if(stabilityUsingAlphasFlag==1 &&
            input.initializationMethod==WindNinjaInputs::wxModelInitializationFlag &&
            init->getForecastIdentifier()=="WRF-3D") //it's a 3D wx model run
    {
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Calculating stability...");

        stb.Set3dVariableAlpha(input, *mesh_, init->air3d, U0_);
        init->air3d.deallocate();

        for(unsigned int k=0; k<mesh_->nlayers; k++)
        {
            for(unsigned int i=0; i<mesh_->nrows; i++)
            {
                for(unsigned int j=0; j<mesh_->ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/stb.alphaField(i,j,k);
                }
            }
        }
    }
    else{
        throw logic_error( string("Can't determine how to set atmospheric stability.") );
    }

    CPLDebug("STABILITY", "alphaVfield(0,0,0) = %lf\n", alphaVfield(0,0,0));

    stb.alphaField.deallocate();
}

/**
 * \brief Write PHI and RHS for debugging.
 *
 * \return void
 */
void PoissonEquation::WritePHIandRHS()
{
    wn_3dScalarField phiField;
    wn_3dScalarField rhsField;
    phiField.allocate(mesh_);
    rhsField.allocate(mesh_);
    int _NPK;
    for(unsigned int k=0; k<mesh_->nlayers; k++)
    {
        for(unsigned int i=0; i<mesh_->nrows;i++)
        {
            for(unsigned int j=0; j<mesh_->ncols; j++)
            {
                _NPK=k*input_->dem.get_nCols()*input_->dem.get_nRows()+i*input_->dem.get_nCols()+j; //NPK is the global row number (also the node # we're on)
                phiField(i,j,k) = PHI[_NPK];
                rhsField(i,j,k) = RHS[_NPK];
            }
        }
    }
    volVTK VTKphi(phiField, mesh_->XORD, mesh_->YORD, mesh_->ZORD, 
                input_->dem.get_nCols(), input_->dem.get_nRows(), mesh_->nlayers, phiOutFilename);
    volVTK VTKrhs(rhsField, mesh_->XORD, mesh_->YORD, mesh_->ZORD, 
                input_->dem.get_nCols(), input_->dem.get_nRows(), mesh_->nlayers, rhsOutFilename);
    phiField.deallocate();
    rhsField.deallocate();
}

/**
 * @brief Computes the u,v,w 3d volume wind field.
 *
 * Calculate @f$u@f$, @f$v@f$, and @f$w@f$ from derivitives of @f$\Phi@f$. Where:
 *
 * @f$ u = u_{0} + \frac{1}{2} * \frac{d\Phi}{dx} @f$
 *
 * @f$ v = v_{0} + \frac{1}{2} * \frac{d\Phi}{dy} @f$
 *
 * @f$ w = w_{0} + \frac{\alpha^{2}}{2} * \frac{d\Phi}{dz} @f$
 *
 * @note Since the derivatives cannot be directly calculated because they are
 * located at the nodal points(the derivatives across element boundaries are
 * discontinuous), another method must be used.  The method used here is that
 * used in Thompson's book on page 228 called "stress smoothing".  It is
 * basically an inverse-distance weighted average from the gauss points of the
 * surrounding cells.
 *
 * This is the result of the @f$ A*x=b @f$ calculation.
 *
 * @return wn_3dVectorField The compute the 3d volume wind field
 */
wn_3dVectorField PoissonEquation::ComputeUVWField()
{
    /*-----------------------------------------------------*/
    /*      Calculate u,v, and w from derivatives of PHI   */
    /*                     1         d PHI                 */
    /*     u =  u  +  -----------  * ----                  */
    /*           0     2*alphaH^2     dx                   */
    /*                                                     */
    /*                     1         d PHI                 */
    /*     v =  v  +  -----------  * ----                  */
    /*           0     2*alphaH^2     dy                   */
    /*                                                     */
    /*                     1         d PHI                 */
    /*     w =  w  +  -----------  * ----                  */
    /*           0     2*alphaV^2     dz                   */
    /*                                                     */
    /*     Since the derivatives cannot be directly        */
    /*     calculated because they are located at the      */
    /*     nodal points(the derivatives across element     */
    /*     boundaries are discontinuous), another method   */
    /*     must be used.  The method used here is that     */
    /*     used in Thompson's book on page 228 called      */
    /*     "stress smoothing".  It is basically an inverse-*/
    /*     distance weighted average from the gauss points */
    /*     of the surrounding cells.                       */
    /*-----------------------------------------------------*/

    //Initialize u,v, and w
    U_ = 0.0;

    fem.ComputeGradientField(PHI, U_);

    double alphaV = 1.0;

    for(int i=0;i<mesh_->NUMNP;i++)
    {
        //calculate u,v,w
        alphaV = alphaVfield(i); //set alphaV for stability
        
        //Remember, dPHI/dx is stored in U
        U_.vectorData_x(i)=U0_.vectorData_x(i)+1.0/(2.0*alphaH*alphaH)*U_.vectorData_x(i);
        U_.vectorData_y(i)=U0_.vectorData_y(i)+1.0/(2.0*alphaH*alphaH)*U_.vectorData_y(i);
        U_.vectorData_z(i)=U0_.vectorData_z(i)+1.0/(2.0*alphaV*alphaV)*U_.vectorData_z(i);
    }

    //set ground to zero
    for(int i=0; i<U_.vectorData_x.mesh_->nrows; i++)
    {
        for(int j=0; j<U_.vectorData_x.mesh_->ncols; j++)
        {
            U_.vectorData_x(i,j,0) = 0.0;
            U_.vectorData_y(i,j,0) = 0.0;
            U_.vectorData_z(i,j,0) = 0.0;
        }
    }

    return U_;
}

void PoissonEquation::SetInitialVelocity(wn_3dVectorField &U)
{
   U0_ = U; 
}

/**
 * \brief Initialize the poisson equation object.
 *
 * \param mesh A reference to the mesh.
 * \param input A reference to the WindNinja inputs.
 *
 * \return void
 */
void PoissonEquation::Initialize(const Mesh &mesh, const WindNinjaInputs &input)
{
    mesh_ = &mesh;
    input_ = &input; //NOTE: don't use for Com since input.Com is set to NULL in equals operator

    if(PHI == NULL)
        PHI=new double[mesh_->NUMNP];
    else
        throw std::runtime_error("Error allocating PHI field.");
    if(isBoundaryNode == NULL)
        isBoundaryNode=new bool[mesh_->NUMNP]; //flag to specify if it's a boundary node
    else
        throw std::runtime_error("Error allocating isBoundaryNode field.");

    RHS=new double[mesh_->NUMNP]; //This is the final right hand side (RHS) matrix

    SetupSKCompressedRowStorage();

    stabilityUsingAlphasFlag = input.stabilityFlag;

    fem.Initialize(mesh_, input_);

    U_.allocate(&mesh);
}

void PoissonEquation::Solve(WindNinjaInputs &input)
{
//#define WRITE_A_B
#ifdef WRITE_A_B	//used for debugging...
    matrixEquation.Write_A_and_b(SK, RHS, row_ptr, col_ind);
#endif

    //if the CG solver diverges, try the minres solver
    matrixEquation.InitializeConjugateGradient(mesh_->NUMNP);
    if(matrixEquation.SolveConjugateGradient(input, SK, PHI, RHS, row_ptr, col_ind)==false)
    {
        matrixEquation.InitializeMinres(mesh_->NUMNP);
        if(matrixEquation.SolveMinres(input, SK, PHI, RHS, row_ptr, col_ind)==false)
        {
            throw std::runtime_error("Solver returned false.");
        }
    }
}
