/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Linear algebra operations 
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

#ifndef LINEAR_ALGEBRA_H
#define LINEAR_ALGEBRA_H

#include "preconditioner.h"
#include "WindNinjaInputs.h"

#define OFFSET(N, incX) ((incX) > 0 ?  0 : ((N) - 1) * (-(incX))) //for cblas_dscal

class LinearAlgebra
{
    public:

        LinearAlgebra(int NUMNP, int nLayers, int nRows, int nCols);
        ~LinearAlgebra();
        LinearAlgebra(LinearAlgebra const& A);
        LinearAlgebra& operator=(LinearAlgebra const& A);

        bool SolveConjugateGradient(WindNinjaInputs &input);
        bool SolveMinres(WindNinjaInputs &input);
        void Write_A_and_b();

    private:
        int *row_ptr, *col_ind;
        int NUMNP, nLayers, nRows, nCols;
        double *RHS, *PHI, *SK;
        void cblas_dcopy(const int N, const double *X, const int incX, double *Y, const int incY);
        double cblas_ddot(const int N, const double *X, const int incX, const double *Y, const int incY);
        void cblas_daxpy(const int N, const double alpha, const double *X, const int incX,
                        double *Y, const int incY);
        double cblas_dnrm2(const int N, const double *X, const int incX);
        void mkl_dcsrmv(char *transa, int *m, int *k, double *alpha, char *matdescra,
                        double *val, int *indx, int *pntrb, int *pntre, double *x,
                        double *beta, double *y);
        void cblas_dscal(const int N, const double alpha, double *X, const int incX);
        void mkl_trans_dcsrmv(char *transa, int *m, int *k, double *alpha, char *matdescra,
                        double *val, int *indx, int *pntrb, int *pntre, double *x, double *beta, double *y);
};

#endif	//FINITE_ELEMENT_METHOD_H
