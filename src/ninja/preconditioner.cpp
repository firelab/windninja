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

#include "preconditioner.h"

Preconditioner::Preconditioner()
{
	NUMNP = 0;
	D = NULL;
	Lt = NULL;
	U = NULL;
	scratch = NULL;
	L_row_ptr = NULL;
	L_col_ind = NULL;
	w = 1.0;

	//stuff for sparse BLAS solve
	one=1.E0;
	zero=0.E0;

	L_transa='t';		//solve using transpose y := alpha*inv(A')*x
	L_matdescra[0]='t';	//triangular
	L_matdescra[1]='u';	//upper triangle
	L_matdescra[2]='u';	//unit diagonal
	L_matdescra[3]='c';	//zero based indexing

	U_transa='n';		//solve using regular matrix (not transpose) y := alpha*inv(A)*x
	U_matdescra[0]='t';	//triangular
	U_matdescra[1]='u';	//upper triangle
	U_matdescra[2]='n';	//not unit diagonal
	U_matdescra[3]='c';	//zero based indexing
}

Preconditioner::~Preconditioner()
{
	if(D)
		delete[] D;

	if(Lt)
		delete[] Lt;
	if(U)
		delete[] U;

	if(scratch)
		delete[] scratch;
	if(L_row_ptr)
		delete[] L_row_ptr;
	//if(U_row_ptr)
	//	delete U_row_ptr;
	if(L_col_ind)
		delete[] L_col_ind;
	//if(U_col_ind)
	//	delete U_col_ind;
}

bool Preconditioner::initialize(int numnp, double *A, int *row_ptr, int *col_ind, int preconditionerType, char *matdescra)
{	
	
	if(preconditionerType == none)
	{
		preConditionerType = preconditionerType;
		NUMNP = numnp;
	}else if(preconditionerType == Jacobi)
	{
		preConditionerType = preconditionerType;
		NUMNP = numnp;
		
		
		D = new double[NUMNP];
        if(matdescra[0] == 's'){  //if A is symmetric
		for(int i=0; i<NUMNP; i++)
			D[i] = 1./A[row_ptr[i]];	//D is really stored as M^(-1)
        }
        
        if(matdescra[0] == 'g'){  //if A is not symmetric
            int j;
            for(int i=0; i<NUMNP; i++){ //iterate over rows
                j = 0;
                while(col_ind[row_ptr[i] + j] < i){ // interate through cols until col = row (the diagonal)
                    j++;
                }
                //std::cout<<"D[i] = "<<A[row_ptr[i] + j]<<std::endl;
                D[i] = 1./A[row_ptr[i] + j];	// store location of diagonal
                //std::cout<<"D[i] = "<<D[i]<<std::endl;
            }
        }
        
		return true;
		
	}else if(preconditionerType == SSOR)
	{
		preConditionerType = preconditionerType;
		NUMNP = numnp;

		int i, j;

		int count=0;
		for(i=0; i<NUMNP; i++)	//count size of matrix A
		{
			for(j=row_ptr[i]; j<row_ptr[i+1]; j++)
				count++;
		}
		
		//Allocate matrix storage;

		Lt = new double[count-NUMNP];
		U = new double[count];

		scratch = new double[NUMNP];
		L_row_ptr = new int[NUMNP+1]; //+1 since we need to store the location of the end of Lt also (in mkl_dcsrsv() need pntre)
		//U_row_ptr = new int[NUMNP];
		L_col_ind = new int[count-NUMNP];
		//U_col_ind = new int[count];
		

		count=0;
		for(i=0; i<NUMNP; i++)	//loop over matrix A, setting up storage for Lt (not storing diagonal since it's 1)
		{
			L_row_ptr[i] = count;
			for(j=row_ptr[i]+1; j<row_ptr[i+1]; j++)
			{
				if(i != col_ind[j])	//if not on diagonal
				{
					L_col_ind[count] = col_ind[j];
					count++;
				}
			}
		}
		L_row_ptr[NUMNP] = count;

		//--------------------------------------------------------------------------------------------------
		//	Make L and U matrices --> L*U = M
		//		For SSOR:
		//			L = (D-wE)*D^-1 = I-wED^-1
		//			U = D-wF
		//		where:
		//			D = diagonal of A
		//			-E = strict lower triangular part of A
		//			-F = strict upper triangular part of A
		//			w = parameter in SOR method; should be 0<w<2 (if w=1, this is the Symmetric Gauss-Seidel (SGS) preconditioner
		//---------------------------------------------------------------------------------------------------

		for(i=0; i<NUMNP; i++)	//make L and U matrix (*NOTE: actually storing L^t)
		{
			U[row_ptr[i]] = A[row_ptr[i]];	//fill in the diagonal...
			count = 0;
			for(j=(row_ptr[i]+1); j<row_ptr[i+1]; j++)	//loop over strict upper triangle of A
			{
				U[j] = w*A[j];
				Lt[L_row_ptr[i]+count] = w*A[j]*(1./A[row_ptr[i]]);
				count++;
			}
		}
	}

	return true;
}

bool Preconditioner::solve(double *r, double *z, int *row_ptr, int *col_ind)
{	//solves M*z=r;  ie z=M^(-1)*r

	if(preConditionerType == none)
	{
		cblas_dcopy(NUMNP, r, 1, z, 1);
	}else if(preConditionerType == Jacobi)
	{
		for(int i=0; i<NUMNP; i++)
			z[i] = D[i]*r[i];

		return true;
	}else if(preConditionerType == SSOR)
	{
		//--------------------------------------------------
		//solves M*z = r
		//	-->  LU*z = r
		//	-->	 L(U*z) = r
		//	-->  L(scratch) = r
		//	-->  U*z = scratch
		//--------------------------------------------------

		mkl_dcsrsv(&L_transa, &NUMNP, &one, L_matdescra, Lt, L_col_ind, L_row_ptr, &L_row_ptr[1], r, scratch);
		mkl_dcsrsv(&U_transa, &NUMNP, &one, U_matdescra, U, col_ind, row_ptr, &row_ptr[1], scratch, z);

		return true;
	}

	return false;
}

void Preconditioner::mkl_dcsrsv(char *transa, int *m, double *alpha, char *matdescra, double *val, int *indx, int *pntrb, int *pntre, double *x, double *y)
{	// My version of the mkl_dcsrsv() function; solves val*y=x
	// Only works for my specific settings
	//		Case 1:
	//			transa='t';			//solve using transpose y := alpha*inv(A')*x
	//			matdescra[0]='t';	//triangular
	//			matdescra[1]='u';	//upper triangle
	//			matdescra[2]='u';	//unit diagonal
	//			matdescra[3]='c';	//zero based indexing
	//		Case 2:
	//			transa='n';			//solve using regular matrix (not transpose) y := alpha*inv(A)*x
	//			matdescra[0]='t';	//triangular
	//			matdescra[1]='u';	//upper triangle
	//			matdescra[2]='n';	//not unit diagonal
	//			matdescra[3]='c';	//zero based indexing
	int i, j;

	//Case 1:
	if(*transa=='t' && matdescra[0]=='t' && matdescra[1]=='u' && matdescra[2]=='u' && matdescra[3]=='c')
	{
		for(i=0; i<*m; i++)
			y[i] = x[i];
		for(i=0; i<*m; i++)
		{
							// normally would have x[i]/diagonal of val[i,i] here, but val[i,i] is unit (=1)
			for(j=pntrb[i]; j<pntre[i]; j++)
			{
				y[indx[j]] -=  y[i]*val[j];
			}
		}
	//Case 2:
	}else if(*transa=='n' && matdescra[0]=='t' && matdescra[1]=='u' && matdescra[2]=='n' && matdescra[3]=='c')
	{
		for(i=*m-1; i>=0; i--)	//loop up rows
			y[i] = x[i];
		y[*m-1] /= val[pntrb[*m-1]];
		for(i=*m-2; i>=0; i--)	//loop up rows
		{
			for(j=pntrb[i]+1; j<pntre[i]; j++)	//don't include diagonal in loop
				y[i] -= val[j]*y[indx[j]];
			y[i] /= val[pntrb[i]];
		}


	}else
		throw std::logic_error("ERROR IN PRECONDITIONER: TRIANGULAR SOLVER FAILED");
}

void Preconditioner::cblas_dcopy(const int N, const double *X, const int incX, double *Y, const int incY)
{	// My version of cblas_dcopy, only works for incX==1 and incY==1
	int i;
	for(i=0; i<N; i++)
		Y[i] = X[i];
}
