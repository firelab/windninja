/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Preconditioner for the solver
 * Author:   Jason Forthofer <jforthofer@gmail.com>
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

#ifndef PRECONDITIONER_H
#define PRECONDITIONER_H

#include <stdio.h>
#include <stdlib.h>
   //#include <conio.h>
#include <new>

#include <iostream>
	

#include "ninjaException.h"


#ifdef _OPENMP
#include <omp.h>
#endif

class Preconditioner
{

public:
	Preconditioner();
	~Preconditioner();

	enum precondType{
		none,
		Jacobi,
		SSOR
	};
    
    bool initialize(int numnp, double *A, int *row_ptr, int *col_ind, int preconditionerType, char *matdescra);
	bool solve(double *r, double *z, int *row_ptr, int *col_ind);

private:
	
	int NUMNP;
	int preConditionerType;
	double *D;	//This is the inverse of the diagonal for Jacobi preconditioning, ie M^(-1)
	double *Lt, *U;	//These are the upper and lower triangular matrices for the SSOR preconditioner
	double *scratch;	//This is a vector used for intermediate computations in the SSOR preconditioner
	int *L_row_ptr, *L_col_ind;
	//int *U_row_ptr, *U_col_ind;
	double w;	//omega used in the SSOR preconditioner
	
	//stuff for sparse BLAS triangular solve in SSOR preconditioner
	double one, zero;
	char L_transa;	//solve using transpose y := alpha*inv(A')*x
	char L_matdescra[6];
	char U_transa;	//solve using regular matrix (not transpose) y := alpha*inv(A)*x
	char U_matdescra[6];

	void mkl_dcsrsv(char *transa, int *m, double *alpha, char *matdescra, double *val, int *indx, int *pntrb, int *pntre, double *x, double *y);
	void cblas_dcopy(const int N, const double *X, const int incX, double *Y, const int incY);
};

#endif	//PRECONDITIONER_H



