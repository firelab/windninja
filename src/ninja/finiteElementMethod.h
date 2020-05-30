/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Base class for Finite Element Method operations 
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

#define OFFSET(N, incX) ((incX) > 0 ?  0 : ((N) - 1) * (-(incX))) //for cblas_dscal

class FiniteElementMethod
{
    public:
        enum eEquationType{
                diffusionEquation,
                projectionEquation,
                conservationOfMassEquation};

        FiniteElementMethod(eEquationType eqType);
        ~FiniteElementMethod();

        FiniteElementMethod(FiniteElementMethod const& A);
        FiniteElementMethod& operator=(FiniteElementMethod const& A);

        void SetBoundaryConditions(const Mesh &mesh, WindNinjaInputs &input);
        void SetStability(const Mesh &mesh, WindNinjaInputs &input,
                        wn_3dVectorField &U0,
                        AsciiGrid<double> &CloudGrid,
                        boost::shared_ptr<initialize> &init);
        void ComputeUVWField(const Mesh &mesh, WindNinjaInputs &input,
                            wn_3dVectorField &U0,
                            wn_3dVectorField &U);

        void CalculateRcoefficients(const Mesh &mesh, element &elem, int j);
        void CalculateHterm(const Mesh &mesh, element &elem, wn_3dVectorField &U0, int i) ;
        
        void Discretize(const Mesh &mesh, WindNinjaInputs &input, 
                    wn_3dVectorField &U0);
        bool Solve(WindNinjaInputs &input, int NUMNP, int MAXITS, int print_iters, double stop_tol);
        bool SolveMinres(WindNinjaInputs &input, int NUMNP, int max_iter, int print_iters, double tol);
        void Write_A_and_b(int NUMNP);
        void SetupSKCompressedRowStorage(const Mesh &mesh, WindNinjaInputs &input);
        void Deallocate();

        eEquationType equationType;

        double *PHI;
        double *DIAG;

        double alphaH; //alpha horizontal from governing equation, weighting for change in horizontal winds
        wn_3dScalarField alphaVfield; //store spatially varying alphaV variable

    private:
        int NUMNP;
        double *RHS, *SK;
        int *row_ptr, *col_ind;

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
