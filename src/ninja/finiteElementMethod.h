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

#ifndef FINITE_ELEMENT_METHOD_H
#define FINITE_ELEMENT_METHOD_H

#include "stability.h"
#include "initialize.h"
#include "preconditioner.h"
#include "volVTK.h"

#define OFFSET(N, incX) ((incX) > 0 ?  0 : ((N) - 1) * (-(incX))) //for cblas_dscal

class FiniteElementMethod
{
    public:
        enum eEquationType{
                conservationOfMassEquation,
                diffusionEquation};

        enum eDiscretizationType{
                centralDifference,
                lumpedCapacitance};

        FiniteElementMethod(eEquationType eqType);
        ~FiniteElementMethod();

        FiniteElementMethod(FiniteElementMethod const& A);
        FiniteElementMethod& operator=(FiniteElementMethod const& A);

        eEquationType GetEquationType(std::string type);
        eDiscretizationType GetDiscretizationType(std::string type);
        void Initialize(const Mesh &mesh, WindNinjaInputs &input, wn_3dVectorField &U0);
        void SetBoundaryConditions();
        void SetStability(WindNinjaInputs &input,
                        AsciiGrid<double> &CloudGrid,
                        boost::shared_ptr<initialize> &init);
        void ComputeUVWField(WindNinjaInputs &input,
                            wn_3dVectorField &U);
        void Discretize();
        void DiscretizeDiffusion();
        bool Solve(WindNinjaInputs &input, int MAXITS, int print_iters, double stop_tol);
        bool SolveMinres(WindNinjaInputs &input, int max_iter, int print_iters, double tol);
        void Write_A_and_b(int NUMNP);
        void SetupSKCompressedRowStorage();
        void Deallocate();
        void SetCurrentDt(boost::posix_time::time_duration dt);
        void SolveDiffusion(wn_3dVectorField &U);

        eEquationType equationType;
        eDiscretizationType diffusionDiscretizationType;

        double *PHI, *PHI_u, *PHI_v, *PHI_w;
        double *DIAG;

        double alphaH; //alpha horizontal from governing equation, weighting for change in horizontal winds
        wn_3dScalarField alphaVfield; //store spatially varying alphaV variable
        boost::posix_time::time_duration currentDt;//current time step size in seconds (can change during simulation)
        bool writePHIandRHS;
        std::string phiOutFilename;
        std::string rhsOutFilename;

    private:
        Mesh const mesh_; //reference to the mesh
        WindNinjaInputs input_; //NOTE: don't use for Com since input.Com is set to NULL in equals operator
        wn_3dVectorField U0_;
        double *RHS, *SK;
        double *xRHS, *yRHS, *zRHS;
        double *C; //transient term in discretized diffusion equation
        double *dUxdt, *dUydt, *dUzdt; //dPHI/dt for diffusion equation
        int *row_ptr, *col_ind;

        void CalculateRcoefficients(element &elem, int j);
        void CalculateHterm(element &elem, int i) ;
        void cblas_dcopy(const int N, const double *X, const int incX, double *Y, const int incY);
        double cblas_ddot(const int N, const double *X, const int incX, const double *Y, const int incY);
        void cblas_daxpy(const int N, const double alpha, const double *X, const int incX, double *Y, const int incY);
        double cblas_dnrm2(const int N, const double *X, const int incX);
        void mkl_dcsrmv(char *transa, int *m, int *k, double *alpha, char *matdescra,
                        double *val, int *indx, int *pntrb, int *pntre, double *x,
                        double *beta, double *y);
        void cblas_dscal(const int N, const double alpha, double *X, const int incX);
        void mkl_trans_dcsrmv(char *transa, int *m, int *k, double *alpha, char *matdescra,
        double *val, int *indx, int *pntrb, int *pntre, double *x, double *beta, double *y);

        wn_3dScalarField heightAboveGround;
        wn_3dScalarField windSpeed;
        wn_3dVectorField windSpeedGradient;

};

#endif	//FINITE_ELEMENT_METHOD_H
