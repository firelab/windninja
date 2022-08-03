/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Finite Element Method operations
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
#include "finiteElementMethod.h"

FiniteElementMethod::FiniteElementMethod()
{
    DIAG=NULL;
}

/**
 * Copy constructor.
 * @param A Copied value.
 */

FiniteElementMethod::FiniteElementMethod(FiniteElementMethod const& A)
: elementArray(A.elementArray)
, mesh_(A.mesh_)
, input_(A.input_)
{
    DIAG=A.DIAG;
}

/**
 * Equals operator.
 * @param A Value to set equal to.
 * @return a copy of an object
 */

FiniteElementMethod& FiniteElementMethod::operator=(FiniteElementMethod const& A)
{
    if(&A != this) {
        DIAG=A.DIAG;
        elementArray=A.elementArray;
        mesh_=A.mesh_;
        input_=A.input_;
    }
    return *this;
}

FiniteElementMethod::~FiniteElementMethod()      //destructor
{
    Deallocate();
}

void FiniteElementMethod::DiscretizeBackwardDifferenceDiffusion(double* SK, double* PHI, 
                            int* col_ind, int* row_ptr, wn_3dScalarField &scalarField, double* RHS,
                            boost::posix_time::time_duration &currentDt, wn_3dScalarField &heightAboveGround,
                            wn_3dVectorField &windSpeedGradient) 
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
    //    There are three discretization schemes available for diffusion: lumped-capacitance (see p. 195
    //    in Thompson book), backward difference (see p. 193), and central difference (see eq. 10.26 on 
    //    p. 194 and p. 203-209 in Thompson book).
    //        
    //    For backward difference, the equation to solve is (p. 193 in Thompson book):
    //    
    //    [1/dt[C]+[K]]{PHI}sub(t+dt) = {Q}sub(t+dt) + 1/dt[C]{PHI}sub(t)
    //
    //    Currently, for us C (transient coefficient) is 1 and Q (the volumetric source) is 0.
    //    for Ax=b --> A=[1/dt[C]+[K]], x={PHI}sub(t+dt), b={Q}sub(t+dt)+1/dt[C]{PHI}sub(t)
    cout<<"currentDt............... = "<<currentDt<<endl;
    cout<<"scalarField(25) = "<<scalarField(25)<<endl;
    
    int i, j, k, l;

    int interrows=input_->dem.get_nRows()-2;
    int intercols=input_->dem.get_nCols()-2;
    int interlayers=mesh_->nlayers-2;
    //NZND is the # of nonzero elements in the SK stiffness array that are stored
    int NZND=(8*8)+(intercols*4+interrows*4+interlayers*4)*12+
         (intercols*interlayers*2+interrows*interlayers*2+intercols*interrows*2)*18+
         (intercols*interrows*interlayers)*27;

    //this is because we will only store the upper half of the SK matrix since it's symmetric
    NZND = (NZND - mesh_->NUMNP)/2 + mesh_->NUMNP;	

#pragma omp parallel default(shared) private(i,j,k,l)
    {
        int pos;  
        int ii, jj, kk;

#pragma omp for
        for(i=0; i<mesh_->NUMEL; i++) //Start loop over elements
        {
            for(j=0; j<mesh_->NNPE; j++)
            {
                elementArray[i].QE[j]=0.0;
                //just set the transient term to 1 for now --> not sure if this is right
                elementArray[i].C[j]=1.0;
                for(int k=0; k<mesh_->NNPE; k++)
                {
                    elementArray[i].S[j*mesh_->NNPE+k]=0.0;
                }
            }
        }

#pragma omp for
        for(i=0; i<mesh_->NUMNP; i++)
        {
            RHS[i]=0.;
        }

#pragma omp for 
        for(i=0; i<NZND; i++)
        {
            SK[i]=0.;
        }

cout<<"Going into loop over elements............"<<endl;
#pragma omp for
        for(i=0;i<mesh_->NUMEL;i++) //Start loop over elements
        {
            /*-----------------------------------------------------*/
            /*      NO SURFACE QUADRATURE NEEDED SINCE NONE OF     */
            /*      THE BOUNDARY CONDITIONS HAVE A NON-ZERO FLUX   */
            /*      SPECIFICATION:                                 */
            /*      Flow through =>  Phi = 0                       */
            /*      Ground       =>  normal flux = 0               */
            /*-----------------------------------------------------*/

            //Begin quadrature for current element
            elementArray[i].node0=mesh_->get_node0(i); //get the global nodal number of local node 0 of element i

            for(j=0;j<elementArray[i].NUMQPTV;j++) //Start loop over quadrature points in the element
            {
                if(elementArray[i].NUMQPTV==27)
                {
                    if(j<=7)
                    {
                        elementArray[i].WT=elementArray[i].WT1;
                    }
                    else if(j<=19)
                    {
                        elementArray[i].WT=elementArray[i].WT2;
                    }
                    else if(j<=25)
                    {
                        elementArray[i].WT=elementArray[i].WT3;
                    }
                    else
                    {
                        elementArray[i].WT=elementArray[i].WT4;
                    }
                }

                //calculates elem.HVJ
                //CalculateHterm(i, U0);
                //just set the source term to 0 for now --> no soure term for diffusion
                elementArray[i].HVJ=0.0;

                //calculates elem.RX, elem.RY, elem.RZ
                CalculateDiffusionRcoefficients(i, j, heightAboveGround, windSpeedGradient);

                //Create element stiffness matrix---------------------------------------------
                for(k=0;k<mesh_->NNPE;k++) //Start loop over nodes in the element
                {
                    elementArray[i].QE[k]=elementArray[i].QE[k]+elementArray[i].WT*elementArray[i].SFV[0*mesh_->NNPE*elementArray[i].NUMQPTV+k*elementArray[i].NUMQPTV+j]*elementArray[i].HVJ*elementArray[i].DV;
                    for(l=0;l<mesh_->NNPE;l++)
                    {
                        elementArray[i].S[k*mesh_->NNPE+l]=elementArray[i].S[k*mesh_->NNPE+l]+elementArray[i].WT*(elementArray[i].DNDX[k]*elementArray[i].RX*elementArray[i].DNDX[l]+
                                elementArray[i].DNDY[k]*elementArray[i].RY*elementArray[i].DNDY[l]+elementArray[i].DNDZ[k]*elementArray[i].RZ*elementArray[i].DNDZ[l])*elementArray[i].DV;
                    }
                } //End loop over nodes in the element
            } //End loop over quadrature points in the element

            //Place completed element matrix in global SK and Q matrices
            for(j=0;j<mesh_->NNPE;j++) //Start loop over nodes in the element (also, it is the row # in S[])
            {
                //elem.NPK is the global row number of the element stiffness matrix
                elementArray[i].NPK=mesh_->get_global_node(j, i);

#pragma omp atomic
                //Q is currently 0 (no volumetric source)
                RHS[elementArray[i].NPK] += elementArray[i].QE[j] + 
                                            elementArray[i].C[j]*scalarField(elementArray[i].NPK);  

                for(k=0;k<mesh_->NNPE;k++) //k is the local column number in S[]
                {
                    elementArray[i].KNP=mesh_->get_global_node(k, i);

                    if(elementArray[i].KNP >= elementArray[i].NPK) //do only if we're on the upper triangular region of SK[]
                    {
                        pos=-1; //pos is the position # in SK[] to place S[j*mesh.NNPE+k]

                        //l increments through col_ind[] starting from where row_ptr[] says until
                        //we find the column number we're looking for
                        l=0;
                        do
                        {
                            if(col_ind[row_ptr[elementArray[i].NPK]+l]==elementArray[i].KNP) //Check if we're at the correct position
                                 pos=row_ptr[elementArray[i].NPK]+l; //If so, save that position in pos
                            l++;
                        }
                        while(pos<0);
#pragma omp atomic
                        //Here is the final global stiffness matrix in symmetric storage
                        //What is [C]?
                        //[SK] = [A] = 1/dt[C] + [K]
                        SK[pos] += elementArray[i].S[j*mesh_->NNPE+k] +
                                   1/currentDt.total_microseconds()/1000000.0/2. * elementArray[i].C[j];
                    }
                }
            } //End loop over nodes in the element
        } //End loop over elements
    } //End parallel region
}

void FiniteElementMethod::DiscretizeCentralDifferenceDiffusion(double* SK, double* PHI, int* col_ind, int* row_ptr,
                            wn_3dScalarField &scalarField, double* RHS, boost::posix_time::time_duration currentDt) 
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
    //    For central difference, the equation to solve is (p. 194 in Thompson book):
    //    
    //    [CPK]{PHI}sub(t+dt) = {dQ} + [CMK]{PHI}sub(t)
    //
    //    where:
    //    [CPK] = [C] + (dt/2)[K]
    //    [CMK] = [C] - (dt/2)[K]
    //    {dQ} = dt{Q}sub(t+dt/2)
    //    
    //    TODO:
    //    [C] and [K] are evaluated at t+dt/2.
    //
    //    Currently, for us C (transient coefficient) is 1 and Q (the volumetric source) is 0.
    //    for Ax=b --> A=[CPK], x={PHI}sub(t+dt), b={dQ}+[CMK]{PHI}sub(t)
    //    now since Q is 0.
    
    int i, j, k, l;

    for(i=0; i<mesh_->NUMNP; i++)
    {
        PHI[i]=0.;
        RHS[i]=0.;
    }

    //compute [CMK]
    //CMK[

    //we can omit {dQ} for now since Q is currently 0 (no volumetric source)
    RHS[elementArray[i].NPK] += elementArray[i].C[j];  

#pragma omp parallel default(shared) private(i,j,k,l)
    {
        int pos;

#pragma omp for
        for(k=0;k<mesh_->NNPE;k++) //k is the local column number in S[]
        {
            elementArray[i].KNP=mesh_->get_global_node(k, i);
            RHS[elementArray[i].NPK] -= (currentDt.total_microseconds()/1000000.0)/2.*
                            elementArray[i].S[j*mesh_->NNPE+k]*
                            scalarField(elementArray[i].KNP);
        }

#pragma omp for
        for(k=0;k<mesh_->NNPE;k++) //k is the local column number in S[]
        {
            elementArray[i].KNP=mesh_->get_global_node(k, i);

            if(elementArray[i].KNP >= elementArray[i].NPK) //do only if we're on the upper triangular region of SK[]
            {
                pos=-1; //pos is the position # in SK[] to place S[j*mesh.NNPE+k]

                //l increments through col_ind[] starting from where row_ptr[] says until
                //we find the column number we're looking for
                l=0;
                do
                {
                    if(col_ind[row_ptr[elementArray[i].NPK]+l]==elementArray[i].KNP) //Check if we're at the correct position
                         pos=row_ptr[elementArray[i].NPK]+l; //If so, save that position in pos
                    l++;
                }
                while(pos<0);
#pragma omp atomic
                //Here is the final global stiffness matrix in symmetric storage
                //SK=[CPK]
                SK[pos] += elementArray[i].C[j*mesh_->NNPE+k] +
                           (currentDt.total_microseconds()/1000000.0)/2*elementArray[i].S[j*mesh_->NNPE+k];
            }
        }
    }
}

void FiniteElementMethod::DiscretizeLumpedCapacitenceDiffusion(wn_3dVectorField &U0, double* xRHS,
                            double* yRHS, double* zRHS, double* CL, wn_3dScalarField &heightAboveGround,
                            wn_3dVectorField &windSpeedGradient) 
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

    int i, j, k, l;

#pragma omp parallel default(shared) private(i,j,k,l)
    {
        int ii, jj, kk;
        int pos;  

#pragma omp for
        for(i=0; i<mesh_->NUMEL; i++) //Start loop over elements
        {
            for(j=0; j<mesh_->NNPE; j++)
            {
                elementArray[i].QE[j]=0.0;
                for(k=0; k<mesh_->NNPE; k++)
                {
                    elementArray[i].S[j*mesh_->NNPE+k]=0.0;
                    elementArray[i].C[j*mesh_->NNPE+k]=0.0;
                }
            }
        }

#pragma omp for
        for(i=0; i<mesh_->NUMNP; i++)
        {
            CL[i]=0.;
            xRHS[i]=0.;
            yRHS[i]=0.;
            zRHS[i]=0.;
        }

#pragma omp for
        for(i=0;i<mesh_->NUMEL;i++) //Start loop over elements
        {
            //Begin quadrature for current element
            elementArray[i].node0=mesh_->get_node0(i); //get the global nodal number of local node 0 of element i

            for(j=0;j<elementArray[i].NUMQPTV;j++) //Start loop over quadrature points in the element
            {
                if(elementArray[i].NUMQPTV==27)
                {
                    if(j<=7)
                    {
                        elementArray[i].WT=elementArray[i].WT1;
                    }
                    else if(j<=19)
                    {
                        elementArray[i].WT=elementArray[i].WT2;
                    }
                    else if(j<=25)
                    {
                        elementArray[i].WT=elementArray[i].WT3;
                    }
                    else
                    {
                        elementArray[i].WT=elementArray[i].WT4;
                    }
                }

                //calculates elem.HVJ
                //CalculateHterm(elementArray[i], i);
                //just set the source term to 0 for now --> no soure term for diffusion
                elementArray[i].HVJ=0.0;

                //calculates elem.RX, elem.RY, elem.RZ
                CalculateDiffusionRcoefficients(i, j, heightAboveGround, windSpeedGradient);

                //Create element stiffness matrix---------------------------------------------
                for(k=0;k<mesh_->NNPE;k++) //Start loop over nodes in the element
                {
                    //elementArray[i].QE is currently just 0 since elementArray[i].HVJ is 0 (no source term)
                    elementArray[i].QE[k]=elementArray[i].QE[k]+elementArray[i].WT*
                        elementArray[i].SFV[0*mesh_->NNPE*elementArray[i].NUMQPTV+k*elementArray[i].NUMQPTV+j]*
                        elementArray[i].HVJ*elementArray[i].DV;
                    for(l=0;l<mesh_->NNPE;l++)
                    {
                        elementArray[i].C[k*mesh_->NNPE+l]=elementArray[i].C[k*mesh_->NNPE+l]+
                            elementArray[i].WT*elementArray[i].SFV[0*mesh_->NNPE*elementArray[i].NUMQPTV+k*elementArray[i].NUMQPTV+j]*elementArray[i].RC*
                            elementArray[i].SFV[0*mesh_->NNPE*elementArray[i].NUMQPTV+l*elementArray[i].NUMQPTV+j]*elementArray[i].DV;

                        elementArray[i].S[k*mesh_->NNPE+l]=elementArray[i].S[k*mesh_->NNPE+l]+elementArray[i].WT*(elementArray[i].DNDX[k]*elementArray[i].RX*elementArray[i].DNDX[l]+
                                elementArray[i].DNDY[k]*elementArray[i].RY*elementArray[i].DNDY[l]+elementArray[i].DNDZ[k]*elementArray[i].RZ*elementArray[i].DNDZ[l])*elementArray[i].DV;
                    }
                } //End loop over nodes in the element
            } //End loop over quadrature points in the element

            //Place completed element matrix in global matrices
            for(j=0;j<mesh_->NNPE;j++) //Start loop over nodes in the element (also, it is the row # in S[])
            {
                //elementArray[i].NPK is the global row number of the element stiffness matrix
                elementArray[i].NPK=mesh_->get_global_node(j, i);

#pragma omp atomic
                xRHS[elementArray[i].NPK] += elementArray[i].QE[j];
                yRHS[elementArray[i].NPK] += elementArray[i].QE[j];
                zRHS[elementArray[i].NPK] += elementArray[i].QE[j];

                for(k=0;k<mesh_->NNPE;k++) //k is the local column number in S[]
                {
                    elementArray[i].KNP=mesh_->get_global_node(k, i);
#pragma omp atomic
                    CL[elementArray[i].NPK] += elementArray[i].C[j*mesh_->NNPE+k];
                    xRHS[elementArray[i].NPK] -= elementArray[i].S[j*mesh_->NNPE+k]*
                        U0.vectorData_x(elementArray[i].KNP);
                    yRHS[elementArray[i].NPK] -= elementArray[i].S[j*mesh_->NNPE+k]*
                        U0.vectorData_y(elementArray[i].KNP);
                    zRHS[elementArray[i].NPK] -= elementArray[i].S[j*mesh_->NNPE+k]*
                        U0.vectorData_z(elementArray[i].KNP);
                }
            } //End loop over nodes in the element
        } //End loop over elements
    } //End parallel region
}

/**
 * \brief Discretize the conservation of mass/projection equations.
 *
 *
 *  The governing equation to solve is
 *
 *    d        dPhi      d        dPhi      d        dPhi
 *   ---- ( Rx ---- ) + ---- ( Ry ---- ) + ---- ( Rz ---- ) + H = 0.0
 *    dx        dx       dy        dy       dz        dz
 *
 *
 *    where, for conservation of mass:
 *                    1                          1
 *    Rx = Ry =  ------------          Rz = ------------
 *                2*alphaH^2                 2*alphaV^2
 *
 *         du0     dv0     dz0
 *    H = ----- + ----- + -----
 *         dx      dy      dz
 *
 *
 * \param SK A pointer to the SK array (A in the Ax=b equation).
 * \param RHS A pointer to the RHS vector (b in the Ax=b equation).
 * \param col_ind A pointer to the column index in SK.
 * \param row_ptr A pointer to the row index in SK.
 * \param U0 A reference to the initial velocity field.
 * \param alphaH The horizontal alpha term (Rx=Ry=1/(2*alphaH^2) in the governing equation).
 * \param alphaVfield A reference to the 3-D alphaV field (Rz=1/(2*alphaV^2) in the governing equation).
 *
 * \return void
 */
void FiniteElementMethod::Discretize(double* SK, double* RHS, int* col_ind, int* row_ptr,
        wn_3dVectorField& U0, double alphaH, wn_3dScalarField& alphaVfield) 
{
    int i, j, k, l;

    int interrows=input_->dem.get_nRows()-2;
    int intercols=input_->dem.get_nCols()-2;
    int interlayers=mesh_->nlayers-2;
    //NZND is the # of nonzero elements in the SK stiffness array that are stored
    int NZND=(8*8)+(intercols*4+interrows*4+interlayers*4)*12+
         (intercols*interlayers*2+interrows*interlayers*2+intercols*interrows*2)*18+
         (intercols*interrows*interlayers)*27;

    //this is because we will only store the upper half of the SK matrix since it's symmetric
    NZND = (NZND - mesh_->NUMNP)/2 + mesh_->NUMNP;	
    
#pragma omp parallel default(shared) private(i,j,k,l)
    {
        int pos;  
        int ii, jj, kk;

#pragma omp for
        for(i=0; i<mesh_->NUMEL; i++) //Start loop over elements
        {
            for(j=0; j<mesh_->NNPE; j++)
            {
                elementArray[i].QE[j]=0.0;
                for(int k=0; k<mesh_->NNPE; k++)
                {
                    elementArray[i].S[j*mesh_->NNPE+k]=0.0;
                }
            }
        }

#pragma omp for
        for(i=0; i<mesh_->NUMNP; i++)
        {
            RHS[i]=0.;
        }

#pragma omp for 
        for(i=0; i<NZND; i++)
        {
            SK[i]=0.;
        }

#pragma omp for
        for(i=0;i<mesh_->NUMEL;i++) //Start loop over elements
        {
            /*-----------------------------------------------------*/
            /*      NO SURFACE QUADRATURE NEEDED SINCE NONE OF     */
            /*      THE BOUNDARY CONDITIONS HAVE A NON-ZERO FLUX   */
            /*      SPECIFICATION:                                 */
            /*      Flow through =>  Phi = 0                       */
            /*      Ground       =>  normal flux = 0               */
            /*-----------------------------------------------------*/

            //Begin quadrature for current element
            elementArray[i].node0=mesh_->get_node0(i); //get the global nodal number of local node 0 of element i

            for(j=0;j<elementArray[i].NUMQPTV;j++) //Start loop over quadrature points in the element
            {
                if(elementArray[i].NUMQPTV==27)
                {
                    if(j<=7)
                    {
                        elementArray[i].WT=elementArray[i].WT1;
                    }
                    else if(j<=19)
                    {
                        elementArray[i].WT=elementArray[i].WT2;
                    }
                    else if(j<=25)
                    {
                        elementArray[i].WT=elementArray[i].WT3;
                    }
                    else
                    {
                        elementArray[i].WT=elementArray[i].WT4;
                    }
                }

                //calculates elem.HVJ
                CalculateHterm(i, U0);

                //calculates elem.RX, elem.RY, elem.RZ
                CalculateRcoefficients(i, j, alphaH, alphaVfield);

                //Create element stiffness matrix---------------------------------------------
                for(k=0;k<mesh_->NNPE;k++) //Start loop over nodes in the element
                {
                    elementArray[i].QE[k]=elementArray[i].QE[k]+elementArray[i].WT*elementArray[i].SFV[0*mesh_->NNPE*elementArray[i].NUMQPTV+k*elementArray[i].NUMQPTV+j]*elementArray[i].HVJ*elementArray[i].DV;
                    for(l=0;l<mesh_->NNPE;l++)
                    {
                        elementArray[i].S[k*mesh_->NNPE+l]=elementArray[i].S[k*mesh_->NNPE+l]+elementArray[i].WT*(elementArray[i].DNDX[k]*elementArray[i].RX*elementArray[i].DNDX[l]+
                                elementArray[i].DNDY[k]*elementArray[i].RY*elementArray[i].DNDY[l]+elementArray[i].DNDZ[k]*elementArray[i].RZ*elementArray[i].DNDZ[l])*elementArray[i].DV;
                    }
                } //End loop over nodes in the element
            } //End loop over quadrature points in the element

            //Place completed element matrix in global SK and Q matrices
            for(j=0;j<mesh_->NNPE;j++) //Start loop over nodes in the element (also, it is the row # in S[])
            {
                //elem.NPK is the global row number of the element stiffness matrix
                elementArray[i].NPK=mesh_->get_global_node(j, i);

#pragma omp atomic
                RHS[elementArray[i].NPK] += elementArray[i].QE[j];

                for(k=0;k<mesh_->NNPE;k++) //k is the local column number in S[]
                {
                    elementArray[i].KNP=mesh_->get_global_node(k, i);

                    if(elementArray[i].KNP >= elementArray[i].NPK) //do only if we're on the upper triangular region of SK[]
                    {
                        pos=-1; //pos is the position # in SK[] to place S[j*mesh.NNPE+k]

                        //l increments through col_ind[] starting from where row_ptr[] says until
                        //we find the column number we're looking for
                        l=0;
                        do
                        {
                            if(col_ind[row_ptr[elementArray[i].NPK]+l]==elementArray[i].KNP) //Check if we're at the correct position
                                 pos=row_ptr[elementArray[i].NPK]+l; //If so, save that position in pos
                            l++;
                        }
                        while(pos<0);
#pragma omp atomic
                        //Here is the final global stiffness matrix in symmetric storage
                        SK[pos] += elementArray[i].S[j*mesh_->NNPE+k];
                    }
                }
            } //End loop over nodes in the element
        } //End loop over elements
    } //End parallel region
}

void FiniteElementMethod::Deallocate()
{
    if(DIAG)
    {	
        delete[] DIAG;
        DIAG=NULL;
    }
    for(int i=0; i<mesh_->NUMEL; i++) //Start loop over elements
    {
        elementArray[i].deallocate();
    }
}

/**
 * @brief Computes the gradient of a scalar in the x, y, and z directions.
 *
 * @note Since the derivatives cannot be directly calculated because they are
 * located at the nodal points(the derivatives across element boundaries are
 * discontinuous), another method must be used.  The method used here is that
 * used in Thompson's book on page 228 called "stress smoothing".  It is
 * basically an inverse-distance weighted average from the gauss points of the
 * surrounding cells.
 *
 */
void FiniteElementMethod::ComputeGradientField(double *scalar, wn_3dVectorField &U)
{
     /*-----------------------------------------------------*/
     /*      Calculate the derivatives of PHI               */
     /*                                                     */
     /*                d PHI   d PHI   d PHI                */
     /*                ----,   ----,   ----                 */
     /*                 dx      dy      dz                  */
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

    int i, j, k;

    for(i=0;i<mesh_->NUMNP;i++) //Initialize u,v, and w
    {
        U.vectorData_x(i)=0.;
        U.vectorData_y(i)=0.;
        U.vectorData_z(i)=0.;
        DIAG[i]=0.;
    }

#pragma omp parallel default(shared) private(i,j,k)
    {
        double DPHIDX, DPHIDY, DPHIDZ;
        double XJ, YJ, ZJ;
        double wght, XK, YK, ZK;
        double *uScratch, *vScratch, *wScratch, *DIAGScratch;
        uScratch=NULL;
        vScratch=NULL;
        wScratch=NULL;
        DIAGScratch=NULL;

        uScratch=new double[mesh_->nlayers*input_->dem.get_nRows()*input_->dem.get_nCols()];
        vScratch=new double[mesh_->nlayers*input_->dem.get_nRows()*input_->dem.get_nCols()];
        wScratch=new double[mesh_->nlayers*input_->dem.get_nRows()*input_->dem.get_nCols()];
        DIAGScratch=new double[mesh_->nlayers*input_->dem.get_nRows()*input_->dem.get_nCols()];

        for(i=0;i<mesh_->NUMNP;i++)
        {
            uScratch[i]=0.;
            vScratch[i]=0.;
            wScratch[i]=0.;
            DIAGScratch[i]=0.;
        }

#pragma omp for
        for(i=0;i<mesh_->NUMEL;i++) //Start loop over elements
        {
            elementArray[i].node0 = mesh_->get_node0(i); //get the global node number of local node 0 of element i
            for(j=0;j<elementArray[i].NUMQPTV;j++) //Start loop over quadrature points in the element
            {
                DPHIDX=0.0; //Set DPHI/DX, etc. to zero for the new quad point
                DPHIDY=0.0;
                DPHIDZ=0.0;

                elementArray[i].computeJacobianQuadraturePoint(j, i, XJ, YJ, ZJ);

                //Calculate dN/dx, dN/dy, dN/dz (Remember we're using the transpose of the inverse!)
                for(k=0;k<mesh_->NNPE;k++)
                {
                    elementArray[i].NPK=mesh_->get_global_node(k, i); //NPK is the global node number

                    //Calculate the DPHI/DX, etc. for the quad point we are on
                    DPHIDX=DPHIDX+elementArray[i].DNDX[k]*scalar[elementArray[i].NPK];
                    DPHIDY=DPHIDY+elementArray[i].DNDY[k]*scalar[elementArray[i].NPK];
                    DPHIDZ=DPHIDZ+elementArray[i].DNDZ[k]*scalar[elementArray[i].NPK];
                }

                //Now we know DPHI/DX, etc. for quad point j. 
                //We will distribute this inverse distance weighted average 
                //to each nodal point for the cell we're on
                for(k=0;k<mesh_->NNPE;k++) //Start loop over nodes in the element
                {   //Calculate the Jacobian at the quad point
                    elementArray[i].NPK=mesh_->get_global_node(k, i); //NPK is the global nodal number

                    XK=mesh_->XORD(elementArray[i].NPK); //Coodinates of the nodal point
                    YK=mesh_->YORD(elementArray[i].NPK);
                    ZK=mesh_->ZORD(elementArray[i].NPK);

                    wght=std::pow((XK-XJ),2)+std::pow((YK-YJ),2)+std::pow((ZK-ZJ),2);
                    wght=1.0/(std::sqrt(wght));
                    //c				    #pragma omp critical
                    {
                        //Here we store the summing values of DPHI/DX, etc.
                        //in the u,v,w arrays for later use (to actually calculate u,v,w)
                        uScratch[elementArray[i].NPK]=uScratch[elementArray[i].NPK]+wght*DPHIDX;
                        vScratch[elementArray[i].NPK]=vScratch[elementArray[i].NPK]+wght*DPHIDY;
                        wScratch[elementArray[i].NPK]=wScratch[elementArray[i].NPK]+wght*DPHIDZ;
                        //Store the sum of the weights for the node
                        DIAGScratch[elementArray[i].NPK]=DIAGScratch[elementArray[i].NPK]+wght;
                    }
                } //End loop over nodes in the element
            } //End loop over quadrature points in the element
        } //End loop over elements

#pragma omp critical
        {
            for(i=0;i<mesh_->NUMNP;i++)
            {
                //Dividing by the DIAG[NPK] gives the value of DPHI/DX,
                //etc. (still stored in the u,v,w arrays)
                //Remember each thread is storing its full copy of *Scratch
                //and adding the thread's amount to each node
                U.vectorData_x(i) += uScratch[i];
                U.vectorData_y(i) += vScratch[i];
                U.vectorData_z(i) += wScratch[i];
                DIAG[i] += DIAGScratch[i];
            }
        } //end critical

        if(uScratch)
        {
            delete[] uScratch;
            uScratch=NULL;
        }
        if(vScratch)
        {
            delete[] vScratch;
            vScratch=NULL;
        }
        if(wScratch)
        {
            delete[] wScratch;
            wScratch=NULL;
        }
        if(DIAGScratch)
        {
            delete[] DIAGScratch;
            DIAGScratch=NULL;
        }

#pragma omp barrier

#pragma omp for
        for(i=0;i<mesh_->NUMNP;i++)
        {
            //Dividing by the DIAG[NPK] gives the value of DPHI/DX, etc. (still stored in the u,v,w arrays)
            U.vectorData_x(i)=U.vectorData_x(i)/DIAG[i];
            U.vectorData_y(i)=U.vectorData_y(i)/DIAG[i];
            U.vectorData_z(i)=U.vectorData_z(i)/DIAG[i];
        }
    }//end parallel section
}

void FiniteElementMethod::CalculateHterm(int i,
        wn_3dVectorField& U0)
{
    //Calculate the coefficient H 
    //This is what drives the flow, this is the source term
    //for PHI
    //
    //           d u0   d v0   d w0
    //     H = ( ---- + ---- + ---- )
    //           d x    d y    d z
    //

    elementArray[i].HVJ=0.0;

    for(int k=0;k<mesh_->NNPE;k++) //Start loop over nodes in the element
    {
        elementArray[i].NPK=mesh_->get_global_node(k, i); //NPK is the global nodal number

        elementArray[i].HVJ=elementArray[i].HVJ+((elementArray[i].DNDX[k]*U0.vectorData_x(elementArray[i].NPK))+
                (elementArray[i].DNDY[k]*U0.vectorData_y(elementArray[i].NPK))+
                (elementArray[i].DNDZ[k]*U0.vectorData_z(elementArray[i].NPK)));
    } //End loop over nodes in the element
}

/**
 * @brief Computes the R coefficients in the projection/conservation of mass governing equation.
 *
 * @param elem A reference to the element which R is to be calculated in
 * @param j The quadrature point index
 *
 */
void FiniteElementMethod::CalculateRcoefficients(int i, int j, double alphaH,
        wn_3dScalarField& alphaVfield)
{
    //                    1                          1
    //    Rx = Ry =  ------------          Rz = ------------
    //                2*alphaH^2                 2*alphaV^2

    double alphaV = 0.;
    for(int k=0;k<mesh_->NNPE;k++) //Start loop over nodes in the element
    {
        alphaV=alphaV+elementArray[i].SFV[0*mesh_->NNPE*elementArray[i].NUMQPTV+k*elementArray[i].NUMQPTV+j]*alphaVfield(elementArray[i].NPK);

    } //End loop over nodes in the element

    elementArray[i].RX = 1.0/(2.0*alphaH*alphaH);
    elementArray[i].RY = 1.0/(2.0*alphaH*alphaH);
    elementArray[i].RZ = 1.0/(2.0*alphaV*alphaV);
}

/**
 * @brief Computes the R coefficients in the diffusion equation.
 *
 * @param elem A reference to the element which R is to be calculated in
 * @param j The quadrature point index
 *
 */
void FiniteElementMethod::CalculateDiffusionRcoefficients(int i, int j, 
        wn_3dScalarField &heightAboveGround, wn_3dVectorField &windSpeedGradient)
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
    for(int k=0;k<mesh_->NNPE;k++) //Start loop over nodes in the element
    {
        height=height+elementArray[i].SFV[0*mesh_->NNPE*elementArray[i].NUMQPTV+k*elementArray[i].NUMQPTV+j]*
            heightAboveGround(elementArray[i].NPK);
        speed=speed+elementArray[i].SFV[0*mesh_->NNPE*elementArray[i].NUMQPTV+k*elementArray[i].NUMQPTV+j]*
            windSpeedGradient.vectorData_z(elementArray[i].NPK);
    }
    //0.41 is the von Karman constant
    //elementArray[i].RZ = 0.41*0.41 * height*height * fabs(speed);
    elementArray[i].RZ = 0.41 * height * fabs(speed);
    elementArray[i].RX = elementArray[i].RZ;
    elementArray[i].RY = elementArray[i].RZ;
    elementArray[i].RC = 1.;
}

/**
 * \brief Initialize the finite element method object.
 *
 * Sets up a vector of elements (all elements in the mesh) for the simulation.
 * Several element class members (quad point arrays, Jacobian quad points, DV, S) are initialized
 * for each element object. DIAG is also allocated here.
 *
 * \param mesh A reference to the mesh.
 * \param input A reference to the WindNinja inputs.
 *
 * \return void
 */
void FiniteElementMethod::Initialize(const Mesh *mesh, const WindNinjaInputs *input)
{
    mesh_ = mesh;
    input_ = input; //NOTE: don't use for Com since input.Com is set to NULL in equals operator

    elementArray.resize(mesh_->NUMEL);
    for(int i=0; i<elementArray.size(); i++)
    {
        elementArray[i] = element(mesh_);
    }

    for(int i=0; i<mesh_->NUMEL; i++) //Start loop over elements
    {
        if(elementArray[i].SFV == NULL)
            elementArray[i].initializeQuadPtArrays();

        for(int j=0; j<elementArray[i].NUMQPTV; j++) //Start loop over quadrature points in the element
        {
            elementArray[i].computeJacobianQuadraturePoint(j, i);

            //DV is the DV for the volume integration (could be eliminated and just use DETJ everywhere)
            elementArray[i].DV=elementArray[i].DETJ;
        }
    }

    //DIAG is the sum of the weights at each nodal point; eventually,
    //dPHI/dx, etc. are divided by this value to get the "smoothed" (or averaged)
    //value of dPHI/dx at each node point
    if(DIAG == NULL)
        DIAG=new double[mesh_->nlayers*input_->dem.get_nRows()*input_->dem.get_nCols()];
    else
        throw std::runtime_error("Error allocating DIAG field.");
}

