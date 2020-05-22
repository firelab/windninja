/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Finite Element Method operations for conservation of mass
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
#include "finiteElementMethodMassConservation.h"

//The governing equation to solve is
//
//    d        dPhi      d        dPhi      d        dPhi
//   ---- ( Rx ---- ) + ---- ( Ry ---- ) + ---- ( Rz ---- ) + H = 0.0
//    dx        dx       dy        dy       dz        dz
//
//        where
//
//                    1                          1
//    Rx = Ry =  ------------          Rz = ------------
//                2*alphaH^2                 2*alphaV^2
//
//         du0     dv0     dz0
//    H = ----- + ----- + -----
//         dx      dy      dz

FiniteElementMethodMassConservation::FiniteElementMethodMassConservation() : FiniteElementMethod()
{
    alphaH=1.0;
}


FiniteElementMethodMassConservation::~FiniteElementMethodMassConservation()      //destructor
{

}

void FiniteElementMethodMassConservation::SetBoundaryConditions(const Mesh &mesh, WindNinjaInputs &input)
{
    //Specify known values of PHI
    //This is done by replacing the particular node equation (row) with all zeros except a "1" on the diagonal of SK[].
    //Then the corresponding row in RHS[] is replaced with the value of the known PHI.
    //The other nodes that are "connected" to the known node need to have their equations adjusted accordingly.
    //This is done by moving the term with the known value to the RHS.

    int NPK, KNP;
    int i, j, k, l;
    bool *isBoundaryNode;
    isBoundaryNode=new bool[mesh.NUMNP]; //flag to specify if it's a boundary node

#pragma omp parallel default(shared) private(i,j,k,l,NPK,KNP)
    {
#pragma omp for
    for(k=0;k<mesh.nlayers;k++)
    {
        for(i=0;i<input.dem.get_nRows();i++)
        {
            for(j=0;j<input.dem.get_nCols();j++) //loop over nodes using i,j,k notation
            {
                if(j==0||j==(input.dem.get_nCols()-1)||i==0||i==(input.dem.get_nRows()-1)||k==(mesh.nlayers-1))  //Check the node to see if it is a boundary node
                    isBoundaryNode[k*input.dem.get_nCols()*input.dem.get_nRows()+i*input.dem.get_nCols()+j]=true;
                else
                    isBoundaryNode[k*input.dem.get_nCols()*input.dem.get_nRows()+i*input.dem.get_nCols()+j]=false;
            }
        }
    }
#pragma omp for
    for(k=0;k<mesh.nlayers;k++)
    {
        for(i=0;i<input.dem.get_nRows();i++)
        {
            for(j=0;j<input.dem.get_nCols();j++) //loop over nodes using i,j,k notation
            {
                NPK=k*input.dem.get_nCols()*input.dem.get_nRows()+i*input.dem.get_nCols()+j; //NPK is the global row number (also the node # we're on)
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

void FiniteElementMethodMassConservation::SetStability(const Mesh &mesh, 
        WindNinjaInputs &input,
        wn_3dVectorField &U0,
        AsciiGrid<double> &CloudGrid,
        boost::shared_ptr<initialize> &init)
{
    CPLDebug("STABILITY", "input.initializationMethod = %i\n", input.initializationMethod);
    CPLDebug("STABILITY", "input.stabilityFlag = %i\n", input.stabilityFlag);
    Stability stb(input);
    alphaVfield.allocate(&mesh);

    if(input.stabilityFlag==0) // if stabilityFlag not set
    {
        for(unsigned int k=0; k<mesh.nlayers; k++)
        {
            for(unsigned int i=0; i<mesh.nrows;i++)
            {
                for(unsigned int j=0; j<mesh.ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/1.0;
                }
            }
        }
    }
    else if(input.stabilityFlag==1 && input.alphaStability!=-1) // if the alpha was specified directly in the CLI
    {
        for(unsigned int k=0; k<mesh.nlayers; k++)
        {
            for(unsigned int i=0; i<mesh.nrows; i++)
            {
                for(unsigned int j=0; j<mesh.ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/input.alphaStability;
                }
            }
        }
    }
    else if(input.stabilityFlag==1 &&
            input.initializationMethod==WindNinjaInputs::domainAverageInitializationFlag) //it's a domain-average run
    {
        stb.SetDomainAverageAlpha(input, mesh);  //sets alpha based on incident solar radiation
        for(unsigned int k=0; k<mesh.nlayers; k++)
        {
            for(unsigned int i=0; i<mesh.nrows; i++)
            {
                for(unsigned int j=0;j<input.dem.get_nCols();j++)
                {
                    alphaVfield(i,j,k) = alphaH/stb.alphaField(i,j,k);
                }
            }
        }
    }
    else if(input.stabilityFlag==1 &&
            input.initializationMethod==WindNinjaInputs::pointInitializationFlag) //it's a point-initialization run
    {
        stb.SetPointInitializationAlpha(input, mesh);
        for(unsigned int k=0; k<mesh.nlayers; k++)
        {
            for(unsigned int i=0; i<mesh.nrows; i++)
            {
                for(unsigned int j=0; j<mesh.ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/stb.alphaField(i,j,k);
                }
            }
        }
    }
    //If the run is a 2D WX model run
    else if(input.stabilityFlag==1 &&
            input.initializationMethod==WindNinjaInputs::wxModelInitializationFlag &&
            init->getForecastIdentifier()!="WRF-3D") //it's a 2D wx model run
    {
        stb.Set2dWxInitializationAlpha(input, mesh, CloudGrid);

        for(unsigned int k=0; k<mesh.nlayers; k++)
        {
            for(unsigned int i=0; i<mesh.nrows; i++)
            {
                for(unsigned int j=0; j<mesh.ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/stb.alphaField(i,j,k);
                }
            }
        }
    }
    //If the run is a WX model run where the full WX vertical profile is used and
    //the potential temp (theta) is available, then use the method below
    else if(input.stabilityFlag==1 &&
            input.initializationMethod==WindNinjaInputs::wxModelInitializationFlag &&
            init->getForecastIdentifier()=="WRF-3D") //it's a 3D wx model run
    {
        input.Com->ninjaCom(ninjaComClass::ninjaNone, "Calculating stability...");

        stb.Set3dVariableAlpha(input, mesh, init->air3d, U0);
        init->air3d.deallocate();

        for(unsigned int k=0; k<mesh.nlayers; k++)
        {
            for(unsigned int i=0; i<mesh.nrows; i++)
            {
                for(unsigned int j=0; j<mesh.ncols; j++)
                {
                    alphaVfield(i,j,k) = alphaH/stb.alphaField(i,j,k);
                }
            }
        }
    }
    else{
        throw logic_error( string("Can't determine how to set atmophseric stability.") );
    }

    CPLDebug("STABILITY", "alphaVfield(0,0,0) = %lf\n", alphaVfield(0,0,0));

    stb.alphaField.deallocate();
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
 */
void FiniteElementMethodMassConservation::ComputeUVWField(const Mesh &mesh, WindNinjaInputs &input,
                                wn_3dVectorField &U0,
                                wn_3dVectorField &U)
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

    int i, j, k;

    //u is positive toward East
    //v is positive toward North
    //w is positive up
    U.allocate(&mesh);

    //DIAG is the sum of the weights at each nodal point; eventually,
    //dPHI/dx, etc. are divided by this value to get the "smoothed" (or averaged)
    //value of dPHI/dx at each node point
    if(DIAG == NULL)
        DIAG=new double[mesh.nlayers*input.dem.get_nRows()*input.dem.get_nCols()];

    for(i=0;i<mesh.NUMNP;i++) //Initialize u,v, and w
    {
        U.vectorData_x(i)=0.;
        U.vectorData_y(i)=0.;
        U.vectorData_z(i)=0.;
        DIAG[i]=0.;
    }

#pragma omp parallel default(shared) private(i,j,k)
    {
        element elem(&mesh);

        double DPHIDX, DPHIDY, DPHIDZ;
        double XJ, YJ, ZJ;
        double wght, XK, YK, ZK;
        double *uScratch, *vScratch, *wScratch, *DIAGScratch;
        uScratch=NULL;
        vScratch=NULL;
        wScratch=NULL;
        DIAGScratch=NULL;

        uScratch=new double[mesh.nlayers*input.dem.get_nRows()*input.dem.get_nCols()];
        vScratch=new double[mesh.nlayers*input.dem.get_nRows()*input.dem.get_nCols()];
        wScratch=new double[mesh.nlayers*input.dem.get_nRows()*input.dem.get_nCols()];
        DIAGScratch=new double[mesh.nlayers*input.dem.get_nRows()*input.dem.get_nCols()];

        for(i=0;i<mesh.NUMNP;i++)
        {
            uScratch[i]=0.;
            vScratch[i]=0.;
            wScratch[i]=0.;
            DIAGScratch[i]=0.;
        }

#pragma omp for
        for(i=0;i<mesh.NUMEL;i++) //Start loop over elements
        {
            elem.node0 = mesh.get_node0(i); //get the global node number of local node 0 of element i
            for(j=0;j<elem.NUMQPTV;j++) //Start loop over quadrature points in the element
            {
                DPHIDX=0.0; //Set DPHI/DX, etc. to zero for the new quad point
                DPHIDY=0.0;
                DPHIDZ=0.0;

                elem.computeJacobianQuadraturePoint(j, i, XJ, YJ, ZJ);

                //Calculate dN/dx, dN/dy, dN/dz (Remember we're using the transpose of the inverse!)
                for(k=0;k<mesh.NNPE;k++)
                {
                    elem.NPK=mesh.get_global_node(k, i); //NPK is the global node number

                    //Calculate the DPHI/DX, etc. for the quad point we are on
                    DPHIDX=DPHIDX+elem.DNDX[k]*PHI[elem.NPK];
                    DPHIDY=DPHIDY+elem.DNDY[k]*PHI[elem.NPK];
                    DPHIDZ=DPHIDZ+elem.DNDZ[k]*PHI[elem.NPK];
                }

                //Now we know DPHI/DX, etc. for quad point j. 
                //We will distribute this inverse distance weighted average 
                //to each nodal point for the cell we're on
                for(k=0;k<mesh.NNPE;k++) //Start loop over nodes in the element
                {   //Calculate the Jacobian at the quad point
                    elem.NPK=mesh.get_global_node(k, i); //NPK is the global nodal number

                    XK=mesh.XORD(elem.NPK); //Coodinates of the nodal point
                    YK=mesh.YORD(elem.NPK);
                    ZK=mesh.ZORD(elem.NPK);

                    wght=std::pow((XK-XJ),2)+std::pow((YK-YJ),2)+std::pow((ZK-ZJ),2);
                    wght=1.0/(std::sqrt(wght));
                    //c				    #pragma omp critical
                    {
                        //Here we store the summing values of DPHI/DX, etc.
                        //in the u,v,w arrays for later use (to actually calculate u,v,w)
                        uScratch[elem.NPK]=uScratch[elem.NPK]+wght*DPHIDX;
                        vScratch[elem.NPK]=vScratch[elem.NPK]+wght*DPHIDY;
                        wScratch[elem.NPK]=wScratch[elem.NPK]+wght*DPHIDZ;
                        //Store the sum of the weights for the node
                        DIAGScratch[elem.NPK]=DIAGScratch[elem.NPK]+wght;
                    }
                } //End loop over nodes in the element
            } //End loop over quadrature points in the element
        } //End loop over elements

#pragma omp critical
        {
            for(i=0;i<mesh.NUMNP;i++)
            {
                //Dividing by the DIAG[NPK] gives the value of DPHI/DX,
                //etc. (still stored in the u,v,w arrays)
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

        double alphaV = 1.0;

#pragma omp for

        for(i=0;i<mesh.NUMNP;i++)
        {
            //Dividing by the DIAG[NPK] gives the value of DPHI/DX, etc. (still stored in the u,v,w arrays)

            U.vectorData_x(i)=U.vectorData_x(i)/DIAG[i];
            U.vectorData_y(i)=U.vectorData_y(i)/DIAG[i];
            U.vectorData_z(i)=U.vectorData_z(i)/DIAG[i];

            //Finally, calculate u,v,w
            alphaV = alphaVfield(i); //set alphaV for stability

            U.vectorData_x(i)=U0.vectorData_x(i)+
            1.0/(2.0*alphaH*alphaH)*U.vectorData_x(i); //Remember, dPHI/dx is stored in u
            U.vectorData_y(i)=U0.vectorData_y(i)+
            1.0/(2.0*alphaH*alphaH)*U.vectorData_y(i);
            U.vectorData_z(i)=U0.vectorData_z(i)+1.0/(2.0*alphaV*alphaV)*U.vectorData_z(i);
        }
    } //end parallel section

    alphaVfield.deallocate();

    // testing
    /*std::string filename;
    AsciiGrid<double> testGrid;
    testGrid.set_headerData(input.dem);
    testGrid.set_noDataValue(-9999.0);

    for(int k = 0; k < mesh.nlayers; k++){
        for(int i = 0; i <mesh.nrows; i++){
            for(int j = 0; j < mesh.ncols; j++ ){
                testGrid(i,j) = u(i,j,k);
                filename = "u" + boost::lexical_cast<std::string>(k);
            }
        }
        testGrid.write_Grid(filename.c_str(), 2);
    }
    testGrid.deallocate();*/
}

void FiniteElementMethodMassConservation::CalculateRcoefficients(const Mesh &mesh, element &elem, int j)
{
    //                    1                          1
    //    Rx = Ry =  ------------          Rz = ------------
    //                2*alphaH^2                 2*alphaV^2

    double alphaV = 0;

    for(int k=0;k<mesh.NNPE;k++) //Start loop over nodes in the element
    {
        alphaV=alphaV+elem.SFV[0*mesh.NNPE*elem.NUMQPTV+k*elem.NUMQPTV+j]*alphaVfield(elem.NPK);
    } //End loop over nodes in the element

    elem.RX = 1.0/(2.0*alphaH*alphaH);
    elem.RY = 1.0/(2.0*alphaH*alphaH);
    elem.RZ = 1.0/(2.0*alphaV*alphaV);
}

void FiniteElementMethodMassConservation::CalculateHterm(const Mesh &mesh, 
        element &elem, wn_3dVectorField &U0, int i)
{
    //Calculate the coefficient H 
    //
    //           d u0   d v0   d w0
    //     H = ( ---- + ---- + ---- )
    //           d x    d y    d z
    //

    elem.HVJ=0.0;

    for(int k=0;k<mesh.NNPE;k++) //Start loop over nodes in the element
    {
        elem.NPK=mesh.get_global_node(k, i); //NPK is the global nodal number

        elem.HVJ=elem.HVJ+((elem.DNDX[k]*U0.vectorData_x(elem.NPK))+
                (elem.DNDY[k]*U0.vectorData_y(elem.NPK))+
                (elem.DNDZ[k]*U0.vectorData_z(elem.NPK)));
    } //End loop over nodes in the element
}
