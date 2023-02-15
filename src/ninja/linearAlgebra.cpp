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
#include "linearAlgebra.h"

LinearAlgebra::LinearAlgebra()
{
    numRows = 0;
    p = NULL;
    z = NULL;
    q = NULL;
    r = NULL;
    R = NULL;
    Z = NULL;
    U = NULL;
    V = NULL;
    W = NULL;
    UOLD = NULL;
    VOLD = NULL;
    WOLD = NULL;
    WOOLD = NULL;
    alpha = 0.0;
    beta = 0.0;
    rho = 0.0;
    rho_1 = 0.0;
    normb = 0.0;
    resid = 0.0;
    residual_percent_complete = 0.0;
    residual_percent_complete_old = 0.0;
    time_percent_complete = 0.0;
    start_resid = 0.0;
}
/**
 * Copy constructor.
 * @param A Copied value.
 */

LinearAlgebra::LinearAlgebra(LinearAlgebra const& RHS)
{
    numRows = RHS.numRows;

    deepCopyDoubleArray(p, RHS.p, RHS.numRows);
    deepCopyDoubleArray(z, RHS.z, RHS.numRows);
    deepCopyDoubleArray(q, RHS.q, RHS.numRows);
    deepCopyDoubleArray(r, RHS.r, RHS.numRows);

    deepCopyDoubleArray(R, RHS.R, RHS.numRows);
    deepCopyDoubleArray(Z, RHS.Z, RHS.numRows);
    deepCopyDoubleArray(U, RHS.U, RHS.numRows);
    deepCopyDoubleArray(V, RHS.V, RHS.numRows);
    deepCopyDoubleArray(W, RHS.W, RHS.numRows);
    deepCopyDoubleArray(UOLD, RHS.UOLD, RHS.numRows);
    deepCopyDoubleArray(VOLD, RHS.VOLD, RHS.numRows);
    deepCopyDoubleArray(WOLD, RHS.WOLD, RHS.numRows);
    deepCopyDoubleArray(WOOLD, RHS.WOOLD, RHS.numRows);

    alpha = RHS.alpha;
    beta = RHS.beta;
    rho = RHS.rho;
    rho_1 = RHS.rho_1;
    normb = RHS.normb;
    resid = RHS.resid;
    residual_percent_complete = RHS.residual_percent_complete;
    residual_percent_complete_old = RHS.residual_percent_complete_old;
    time_percent_complete = RHS.time_percent_complete;
    start_resid = RHS.start_resid;
}

/**
 * Equals operator.
 * @param A Value to set equal to.
 * @return a copy of an object
 */

LinearAlgebra& LinearAlgebra::operator=(LinearAlgebra const& RHS)
{
    if(&RHS != this){
        numRows = RHS.numRows;

        deepCopyDoubleArray(p, RHS.p, RHS.numRows);
        deepCopyDoubleArray(z, RHS.z, RHS.numRows);
        deepCopyDoubleArray(q, RHS.q, RHS.numRows);
        deepCopyDoubleArray(r, RHS.r, RHS.numRows);

        deepCopyDoubleArray(R, RHS.R, RHS.numRows);
        deepCopyDoubleArray(Z, RHS.Z, RHS.numRows);
        deepCopyDoubleArray(U, RHS.U, RHS.numRows);
        deepCopyDoubleArray(V, RHS.V, RHS.numRows);
        deepCopyDoubleArray(W, RHS.W, RHS.numRows);
        deepCopyDoubleArray(UOLD, RHS.UOLD, RHS.numRows);
        deepCopyDoubleArray(VOLD, RHS.VOLD, RHS.numRows);
        deepCopyDoubleArray(WOLD, RHS.WOLD, RHS.numRows);
        deepCopyDoubleArray(WOOLD, RHS.WOOLD, RHS.numRows);

        alpha = RHS.alpha;
        beta = RHS.beta;
        rho = RHS.rho;
        rho_1 = RHS.rho_1;
        normb = RHS.normb;
        resid = RHS.resid;
        residual_percent_complete = RHS.residual_percent_complete;
        residual_percent_complete_old = RHS.residual_percent_complete_old;
        time_percent_complete = RHS.time_percent_complete;
        start_resid = RHS.start_resid;
    }
    return *this;
}

LinearAlgebra::~LinearAlgebra()
{
    Deallocate();
}

/**
 * Function to initialize the LinearAlgebra class.  Mostly allocates memory.
 * @param numberOfRows Number of rows in the A matrix.
 * @return Nothing.
 */
void LinearAlgebra::InitializeConjugateGradient(int numberOfRows)
{
    Deallocate();
    numRows = numberOfRows;
    p=new double[numRows];
    z=new double[numRows];
    q=new double[numRows];
    r=new double[numRows];
}


/**
 * Function to initialize the LinearAlgebra class.  Mostly allocates memory.
 * @param numberOfRows Number of rows in the A matrix.
 * @return Nothing.
 */
void LinearAlgebra::InitializeMinres(int numberOfRows)
{
    Deallocate();
    numRows = numberOfRows;
    R=new double[numRows];
    Z=new double[numRows];
    U=new double[numRows];
    V=new double[numRows];
    W=new double[numRows];
    UOLD=new double[numRows];
    VOLD=new double[numRows];
    WOLD=new double[numRows];
    WOOLD=new double[numRows];
}

/**
 * Function to deallocate memory held by the LinearAlgebra class.
 * @return Nothing.
 */
void LinearAlgebra::Deallocate()
{
    //Conjugate Gradient arrays-----------
    if(p!=NULL){
        delete[] p;
        p = NULL;
    }
    if(z!=NULL){
        delete[] z;
        z = NULL;
    }
    if(q!=NULL){
        delete[] q;
        q = NULL;
    }
    if(r!=NULL){
        delete[] r;
        r = NULL;
    }

    //Minres arrays------------------------
    if(R!=NULL){
        delete[] R;
        R = NULL;
    }
    if(Z!=NULL){
        delete[] Z;
        Z = NULL;
    }
    if(U!=NULL){
        delete[] U;
        U = NULL;
    }
    if(V!=NULL){
        delete[] V;
        V = NULL;
    }
    if(W!=NULL){
        delete[] W;
        W = NULL;
    }
    if(UOLD!=NULL){
        delete[] UOLD;
        UOLD = NULL;
    }
    if(VOLD!=NULL){
        delete[] VOLD;
        VOLD = NULL;
    }
    if(WOLD!=NULL){
        delete[] WOLD;
        WOLD = NULL;
    }
    if(WOOLD!=NULL){
        delete[] WOOLD;
        WOOLD = NULL;
    }
}

//  CG solver
//    This solver is fastest, but is not monotonic convergence (residual oscillates a bit up and down)
//    If this solver diverges, try MINRES from PetSc below...
/**
 * This is a congugate gradient solver to solve the matrix equation Ax=b.  It requires all the matrices/vectors
 * to have memory properly allocated and owned by some outside caller.
 * It seems to be the fastest, but is not monotonic convergence (residual oscillates a bit up and down).
 * If this solver diverges, try the MINRES from PetSc, a version is included below...
 * A is the stiffness matrix in Ax=b matrix equation.
 * Storage is symmetric compressed sparse row storage.
 * b is the right hand side of matrix equation.
 * x is the vector to store solution in.
 * row_ptr is a vector used to index to a row in A.
 * col_ind is a vector storing the column number of corresponding value in A.
 *
 * @param input Reference to WindNinjaInputs (used for ninjaCom)
 * @param A The "A" matrix in Ax=b matrix equation.  Must be stored in symmetric compressed sparse row storage.
 * @param x The vector where the solution to Ax=b is to be stored.
 * @param b The right-hand-side vector for the Ax=b matrix equation.
 * @param row_ptr A vector used to index to a row in A.
 * @param col_ind A vector storing the column number of the corresponding value in A.
 * @return Returns true if solver converges and completes properly.
 */
bool LinearAlgebra::SolveConjugateGradient(WindNinjaInputs &input, double* A, double* x, double* b, int *row_ptr, int *col_ind)
{
    //note these values can get altered in the loop below
    int max_iter = 100000;    //maximum number of iterations in the solver
    double tol = 1.E-1; //stopping criteria for iterations (2-norm of residual)
    int print_iters = 10;   //Iterations to print out

    //stuff for sparse BLAS MV multiplication
    char transa='n';
    double one=1.E0, zero=0.E0;
    char matdescra[6];
    matdescra[0]='s';	//symmetric
    matdescra[1]='u';	//upper triangle stored
    matdescra[2]='n';	//non-unit diagonal
    matdescra[3]='c';	//c-style array (ie 0 is index of first element, not 1 like in Fortran)

    residual_percent_complete = 0.0;
    residual_percent_complete_old = -1.;

    Preconditioner M;
    if(M.initialize(numRows, A, row_ptr, col_ind, M.SSOR, matdescra)==false)
    {
        input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Initialization of SSOR preconditioner failed, trying Jacobi preconditioner...");
        if(M.initialize(numRows, A, row_ptr, col_ind, M.Jacobi, matdescra)==false)
            throw std::runtime_error("Initialization of Jacobi preconditioner failed.");
    }

//#define NINJA_DEBUG_VERBOSE
#ifdef NINJA_DEBUG_VERBOSE
    if((convergence_history = fopen ("convergence_history.txt", "w")) == NULL)
        throw std::runtime_error("A convergence_history file to write to cannot be created.\nIt may be in use by another program.");

    fprintf(convergence_history,"\nIteration\tResidual\tResidual_check");
#endif //NINJA_DEBUG_VERBOSE

    //matrix vector multiplication A*x=Ax
    mkl_dcsrmv(&transa, &numRows, &numRows, &one, matdescra, A, col_ind, row_ptr, &row_ptr[1], x, &zero, r);

    for(int i=0;i<numRows;i++){
        r[i]=b[i]-r[i]; //calculate the initial residual
    }

    normb = cblas_dnrm2(numRows, b, 1); //calculate the 2-norm of RHS

    if (normb == 0.0)
        normb = 1.;

    //compute 2 norm of r
    resid = cblas_dnrm2(numRows, r, 1) / normb;

    if (resid <= tol)
    {
        tol = resid;
        max_iter = 0;
        return true;
    }

    //start iterating---------------------------------------------------------------------------------------
    for (int i = 1; i <= max_iter; i++)
    {
        M.solve(r, z, row_ptr, col_ind);	//apply preconditioner
        rho = cblas_ddot(numRows, z, 1, r, 1);

        if (i == 1)
        {
            cblas_dcopy(numRows, z, 1, p, 1);
        }else {
            beta = rho / rho_1;

#pragma omp parallel for
            for(int j=0; j<numRows; j++)
                p[j] = z[j] + beta*p[j];
        }

        //matrix vector multiplication!!!		q = A*p;
        mkl_dcsrmv(&transa, &numRows, &numRows, &one, matdescra, A, col_ind, row_ptr, &row_ptr[1], p, &zero, q);

        alpha = rho / cblas_ddot(numRows, p, 1, q, 1);
        cblas_daxpy(numRows, alpha, p, 1, x, 1);	//PHI = PHI + alpha * p;
        cblas_daxpy(numRows, -alpha, q, 1, r, 1);	//r = r - alpha * q;
        resid = cblas_dnrm2(numRows, r, 1) / normb;	//compute resid

        if(i==1)
            start_resid = resid;

        if((i%print_iters)==0)
        {

#ifdef NINJA_DEBUG_VERBOSE
            input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Iteration = %d\tResidual = %lf\ttol = %lf", i, resid, tol);
            fprintf(convergence_history,"\n%ld\t%lf\t%lf",i,resid,tol);
#endif //NINJA_DEBUG_VERBOSE

            residual_percent_complete=100-100*((resid-tol)/(start_resid-tol));
            if(residual_percent_complete<residual_percent_complete_old)
                residual_percent_complete=residual_percent_complete_old;
            if(residual_percent_complete<0.)
                residual_percent_complete=0.;
            else if(residual_percent_complete>100.)
                residual_percent_complete=100.0;

            time_percent_complete=1.8*exp(0.0401*residual_percent_complete);
            if(time_percent_complete >= 99.0)
                time_percent_complete = 99.0;
            residual_percent_complete_old=residual_percent_complete;
            //fprintf(convergence_history,"\n%ld\t%lf\t%lf",i,residual_percent_complete, time_percent_complete);
#ifdef NINJAFOAM
            //If its a foam+diurnal run, progressWeight==0.80, which means we need to modify the
            //diurnal time_percent_complete
            if(input.Com->progressWeight!=1.0)
            {
                time_percent_complete = (input.Com->progressWeight*100)+
                    (1.0-input.Com->progressWeight)*time_percent_complete;
            }
#endif //NINJAFOAM
            //Tell the GUI what the percentage to complete for the ninja is
            input.Com->ninjaCom(ninjaComClass::ninjaSolverProgress, "%d",(int) (time_percent_complete+0.5));        }

        if (resid <= tol)	//check residual against tolerance
        {
            break;
        }

        rho_1 = rho;

    }//end iterations---------------------------------------------------------------------------------------

#ifdef NINJA_DEBUG_VERBOSE
    fclose(convergence_history);
#endif //NINJA_DEBUG_VERBOSE

    if(resid>tol)
    {
        throw std::runtime_error("Solution did not converge.\nMAXITS reached.");
    }else{
        time_percent_complete = 100; //When the solver finishes, set it to 100
        input.Com->ninjaCom(ninjaComClass::ninjaSolverProgress, "%d",(int) (time_percent_complete+0.5));
        return true;
    }
}

//
//    This solver seems to be monotonic in its convergence (residual always goes down), but slower than congugate gradient method
//    Could use this if CG diverges, but haven't seen divergence yet...
/**
 * This is the MINRES solver to solve the matrix equation Ax=b.  It requires all the matrices/vectors
 * to have memory properly allocated and owned by some outside caller.
 * It was obtained from PetSc (found in google code search).
 * This solver seems to be monotonic in its convergence (residual always goes down), but slower than congugate gradient method
 * At this point, use MINRES if CG diverges.
 * A is the stiffness matrix in Ax=b matrix equation.
 * Storage is symmetric compressed sparse row storage.
 * b is the right hand side of matrix equation.
 * x is the vector to store solution in.
 * row_ptr is a vector used to index to a row in A.
 * col_ind is a vector storing the column number of corresponding value in A.
 *
 * @param input Reference to WindNinjaInputs (used for ninjaCom)
 * @param A The "A" matrix in Ax=b matrix equation.  Must be stored in symmetric compressed sparse row storage.
 * @param x The vector where the solution to Ax=b is to be stored.
 * @param b The right-hand-side vector for the Ax=b matrix equation.
 * @param row_ptr A vector used to index to a row in A.
 * @param col_ind A vector storing the column number of the corresponding value in A.
 * @return Returns true if solver converges and completes properly.
 */
bool LinearAlgebra::SolveMinres(WindNinjaInputs &input, double* A, double* x, double* b, int *row_ptr, int *col_ind)
{
    //note these values can get altered in the loop below
    int max_iter = 100000;    //maximum number of iterations in the solver
    double tol = 1E-1; //stopping criteria for iterations (2-norm of residual)
    int print_iters = 10;   //Iterations to print out

  int i,j, n;
  double alpha,malpha,beta,mbeta,ibeta,betaold,eta,c=1.0,ceta,cold=1.0,coold,s=0.0,sold=0.0,soold;
  double rho0,rho1,irho1,rho2,mrho2,rho3,mrho3,dp = 0.0;
  double rnorm, bnorm;
  double np;	//this is the residual
  double residual_percent_complete_old, residual_percent_complete, time_percent_complete, start_resid;

  n = numRows;

  //stuff for sparse BLAS MV multiplication
  char transa='n';
  double one=1.E0, zero=0.E0;
  char matdescra[6];
  //matdescra[0]='s'; //s = symmetric
  matdescra[0]='g'; //g = generic
  matdescra[1]='u';
  matdescra[2]='n';
  matdescra[3]='c';

  residual_percent_complete_old = -1.;

  FILE *convergence_history;

  if((convergence_history = fopen ("convergence_history.txt", "w")) == NULL)
  {
	   input.Com->ninjaCom(ninjaComClass::ninjaFailure, "A convergence_history file to write to cannot be created.\nIt may be in use by another program.");
	   exit(0);
  }
  fprintf(convergence_history,"\nIteration\tResidual\tResidual_check");

  Preconditioner precond;

  if(precond.initialize(n, A, row_ptr, col_ind, precond.Jacobi, matdescra)==false)
  {
	  input.Com->ninjaCom(ninjaComClass::ninjaWarning, "Initialization of Jacobi preconditioner failed, trying SSOR preconditioner...");
      if(precond.initialize(numRows, A, row_ptr, col_ind, precond.SSOR, matdescra)==false) // SSOR does not work for full asymmetric matrix !!
	  {
		  input.Com->ninjaCom(ninjaComClass::ninjaFailure, "Initialization of SSOR preconditioner failed, CANNOT SOLVE FOR WINDFLOW!!!");
		  return false;
	  }
  }

  //ksp->its = 0;

  for(int j=0;j<n;j++)	UOLD[j] = 0.0;	//  u_old  <-   0
  cblas_dcopy(n, UOLD, 1, VOLD, 1);	//	v_old  <-   0
  cblas_dcopy(n, UOLD, 1, W, 1);	//	w      <-   0
  cblas_dcopy(n, UOLD, 1, WOLD, 1);	//	w_old  <-   0

  mkl_dcsrmv(&transa, &n, &n, &one, matdescra, A, col_ind, row_ptr, &row_ptr[1], x, &zero, R); // r <- b - A*x

  for(int j=0;j<n;j++)	R[j] = b[j] - R[j];

  bnorm = cblas_dnrm2(n, b, 1);

  precond.solve(R, Z, row_ptr, col_ind);	//apply preconditioner    M*z = r

  //KSP_PCApply(ksp,R,Z);

  dp = cblas_ddot(n, R, 1, Z, 1);
  if(dp<0.0)
  {
	input.Com->ninjaCom(ninjaComClass::ninjaFailure, "ERROR IN SOLVER!!! SQUARE ROOT OF A NEGATIVE NUMBER...");
	return false;
  }

  dp = std::sqrt(dp);
  beta = dp;		//  beta <- sqrt(r'*z)
  eta = beta;

  //  VecCopy(R,V)   V = R
  cblas_dcopy(n, R, 1, V, 1);
  cblas_dcopy(n, Z, 1, U, 1);
  ibeta = 1.0/beta;
  cblas_dscal(n, ibeta, V, 1);	//  v <- r/beta
  cblas_dscal(n, ibeta, U, 1);	//  u <- z/beta

  np = cblas_dnrm2(n, Z, 1);	//  np <- ||z||

  rnorm = np;

  // TEST FOR CONVERGENCE HERE!!!!

  i = 0;

  do{

	  //Lanczos

	  mkl_dcsrmv(&transa, &n, &n, &one, matdescra, A, col_ind, row_ptr, &row_ptr[1], U, &zero, R); // r <- A*x

	  alpha = cblas_ddot(n, U, 1, R, 1);	//  alpha <- r'*u
	  precond.solve(R, Z, row_ptr, col_ind);	//apply preconditioner    M*z = r

	  malpha = -alpha;
	  cblas_daxpy(n, malpha, V, 1, R, 1);	//  r <- r - alpha*v
	  cblas_daxpy(n, malpha, U, 1, Z, 1);	//  z <- z - alpha*u
	  mbeta = -beta;
	  cblas_daxpy(n, mbeta, VOLD, 1, R, 1);	//  r <- r - beta*v_old
	  cblas_daxpy(n, mbeta, UOLD, 1, Z, 1);	//  z <- z - beta*u_old

	  betaold = beta;

	  dp = cblas_ddot(n, R, 1, Z, 1);

	  beta = std::sqrt(dp);		//  beta <- sqrt(r'*z)

	  coold = cold; cold = c; soold = sold; sold = s;

	  rho0 = cold * alpha - coold * sold * betaold;
      rho1 = std::sqrt(rho0*rho0 + beta*beta);
      rho2 = sold * alpha + coold * cold * betaold;
      rho3 = soold * betaold;

	  //  Givens rotation

	  c = rho0 / rho1;
      s = beta / rho1;

	  //  Update

	  cblas_dcopy(n, WOLD, 1, WOOLD, 1);	//  w_oold <- w_old
	  cblas_dcopy(n, W, 1, WOLD, 1);	//  w_old <- w

	  cblas_dcopy(n, U, 1, W, 1);	//  w <- U
	  mrho2 = -rho2;
	  cblas_daxpy(n, mrho2, WOLD, 1, W, 1);	//  w <- w - rho2 w_old
	  mrho3 = -rho3;
	  cblas_daxpy(n, mrho3, WOOLD, 1, W, 1);	//  w <- w - rho3 w_oold
	  irho1 = 1.0 / rho1;
	  cblas_dscal(n, irho1, W, 1);	//  w <- w/rho1

	  ceta = c * eta;
	  cblas_daxpy(n, ceta, W, 1, x, 1);	//  x <- x - ceta w
	  eta = - s * eta;

	  cblas_dcopy(n, V, 1, VOLD, 1);	//  v_old <- v
	  cblas_dcopy(n, U, 1, UOLD, 1);	//  u_old <- u
	  cblas_dcopy(n, R, 1, V, 1);	//  v <- r
	  cblas_dcopy(n, Z, 1, U, 1);	//  u <- z
	  ibeta = 1.0 / beta;
      cblas_dscal(n, ibeta, V, 1);	//  v <- r / beta
	  cblas_dscal(n, ibeta, U, 1);	//  u <- z / beta

	  np = rnorm * fabs(s);

	  rnorm = np;

	  //  Test for convergence
	  //if(np<tol)	break;

	  //if(rnorm<tol*bnorm)   break;

	  //cout<<"residual = "<<np<<endl;

        if(i==1)
            start_resid = np;
	
        if(np<tol)   break;

	  i++;


	  if((i%print_iters)==0)
      {

	    #ifdef NINJA_DEBUG_VERBOSE
		input.Com->ninjaCom(ninjaComClass::ninjaDebug, "solver: n=%d iterations = %d residual norm %12.4e", n,i,rnorm/bnorm);
		//input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Iteration = %d\tResidual = %lf\ttol = %lf", i, resid, tol);
		fprintf(convergence_history,"\n%ld\t%lf\t%lf",i,rnorm/bnorm,tol);
		#endif //NINJA_DEBUG_VERBOSE

		input.Com->ninjaCom(ninjaComClass::ninjaDebug, "solver: n=%d iterations = %d residual norm %12.4e", n,i,rnorm/bnorm);
		fprintf(convergence_history,"\n%d\t%lf\t%lf",i,rnorm/bnorm,tol);


	    residual_percent_complete=100-100*((np-tol)/(start_resid-tol));
		if(residual_percent_complete<residual_percent_complete_old)
			residual_percent_complete=residual_percent_complete_old;
		if(residual_percent_complete<0.)
			 residual_percent_complete=0.;
		else if(residual_percent_complete>100.)
			residual_percent_complete=100.0;

		time_percent_complete=1.8*exp(0.0401*residual_percent_complete);
		residual_percent_complete_old=residual_percent_complete;
                //fprintf(convergence_history,"\n%ld\t%lf\t%lf",i,residual_percent_complete, time_percent_complete);
		input.Com->ninjaCom(ninjaComClass::ninjaSolverProgress, "%d",(int) (time_percent_complete+0.5));
	  }

  }while(i<max_iter);

  if(i >= max_iter)
  {
		input.Com->ninjaCom(ninjaComClass::ninjaFailure, "SOLVER DID NOT CONVERGE WITHIN THE SPECIFIED MAXIMUM NUMBER OF ITERATIONS!!");
		return false;
  }else{
		//input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Solver done.");
		return true;
  }

	fclose(convergence_history);

  return true;
}

/**@brief Performs the calculation X = alpha * X.
 * X is an N-element vector and alpha is a scalar.
 * A limited version of the BLAS function dscal().
 * @param N Size of vectors.
 * @param X Source vector.
 * @param incX Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 */
void LinearAlgebra::cblas_dscal(const int N, const double alpha, double *X, const int incX)
{
     int i, ix;

     if (incX <= 0) return;

     ix = OFFSET(N, incX);

     for (i = 0; i < N; i++) {
         X[ix] *= alpha;
         ix    += incX;
     }
}

/**Copies values from the X vector to the Y vector.
 * A limited version of the BLAS function dcopy().
 * @param N Size of vectors.
 * @param X Source vector.
 * @param incX Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 * @param Y Target vector to copy values to.
 * @param incY Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 */
void LinearAlgebra::cblas_dcopy(const int N, const double *X, const int incX, double *Y, const int incY)
{
	int i;
	for(i=0; i<N; i++)
		Y[i] = X[i];
}

/**Performs the dot product X*Y.
 * A limited version of the BLAS function ddot().
 * @param N Size of vectors.
 * @param X Vector of size N.
 * @param incX Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 * @param Y Vector of size N.
 * @param incY Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 * @return Dot product X*Y value.
 */
double LinearAlgebra::cblas_ddot(const int N, const double *X, const int incX, const double *Y, const int incY)
{
	double val=0.0;

	#pragma omp parallel for reduction(+:val)
    for(int i=0;i<N;i++)
		val += X[i]*Y[i];

	return val;
}

/**Performs the calculation Y = Y + alpha * X.
 * A limited version of the BLAS function daxpy().
 * @param N Size of vectors.
 * @param alpha
 * @param X Vector of size N.
 * @param incX Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 * @param Y Vector of size N.
 * @param incY Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 */
void LinearAlgebra::cblas_daxpy(const int N, const double alpha, const double *X, const int incX, double *Y, const int incY)
{
    #pragma omp parallel for
    for(int i=0; i<N; i++)
		Y[i] += alpha*X[i];
}

/**Computes the 2-norm of X.
 * A limited version of the BLAS function dnrm2().
 * @param N Size of X.
 * @param X Vector of size N.
 * @param incX Number of values to skip.  MUST BE 1 FOR THIS VERSION!!
 * @return Value of the 2-norm of X.
 */
double LinearAlgebra::cblas_dnrm2(const int N, const double *X, const int incX)
{
	double val=0.0;

	//#pragma omp parallel for reduction(+:val)
    for(int i=0;i<N;i++)
		val += X[i]*X[i];
	val = std::sqrt(val);

	return val;
}

/**
 * @brief Computes the vector-matrix product A*x=y.
 *
 * This is a limited version of the BLAS function dcsrmv().
 * It is limited in that the matrix must be in compressed sparse row
 * format, and symmetric with only the upper triangular part stored.
 * ALPHA=1 and BETA=0 must also be true.
 *
 * @note My version of MKL's compressed sparse row (CSR) matrix vector product function
 * MINE ONLY WORKS FOR A SYMMETRICALLY STORED, UPPER TRIANGULAR MATRIX!!!!!!
 * AND ALPHA==1 AND BETA==0
 *
 * @param transa Not used here, but included to stay with the BLAS.
 * @param m Number of rows in "A" matrix. Must equal k in this implementation.
 * @param k Number of columns in "A" matrix. Must equal m in this implementation.
 * @param alpha Must be one for this implementation.
 * @param matdescra Not used here, but included to stay with the BLAS.
 * @param val This is the "A" array.  Must be in compressed sparse row format, and symmetric with only the upper triangular part stored.
 * @param indx An array describing the column index of "A" (sometimes called "col_ind").
 * @param pntrb A pointer containing indices of "A" of the starting locations of the rows.
 * @param pntre A pointer containg indices of "A" of the ending locations of the rows.
 * @param x Vector of size m (and k) in the A*x=y computation.
 * @param beta Not used here, but included to stay with the BLAS.
 * @param y Vector of size m (and k) in the A*x=y computation.
 */
void LinearAlgebra::mkl_dcsrmv(char *transa, int *m, int *k, double *alpha, char *matdescra, double *val, int *indx, int *pntrb, int *pntre, double *x, double *beta, double *y)
{	// My version of MKL's compressed sparse row (CSR) matrix vector product function
	// MINE ONLY WORKS FOR A SYMMETRICALLY STORED, UPPER TRIANGULAR MATRIX!!!!!!
	// AND ALPHA==1 AND BETA==0

		//function multiplies a sparse matrix "val" times a vector "x", result is stored in "y"
		//		ie. Ax = y
		//"m" and "k" are equal to the number of rows and columns in "A" (must be equal in mine)
		//"indx" is an array describing the column index of "A" (sometimes called "col_ind")
		//"pntrb" is a pointer containing indices of "A" of the starting locations of the rows
		//"pntre" is a pointer containg indices of "A" of the ending locations of the rows
		int i,j,N;
		N=*m;

    #pragma omp parallel private(i,j)
    {
        #pragma omp for
        for(i=0;i<N;i++)
            y[i]=0.0;

        #pragma omp for
        for(i=0;i<N;i++)
        {
            y[i] += val[pntrb[i]]*x[i];	// diagonal
            for(j=pntrb[i]+1;j<pntre[i];j++)
            {
                y[i] += val[j]*x[indx[j]];
            }
        }
    }	//end parallel region

    for(i=0;i<N;i++)
    {
        for(j=pntrb[i]+1;j<pntre[i];j++)
        {
            {
                y[indx[j]] += val[j]*x[i];
            }
        }
    }
}

/**
 * @brief Computes the vector-matrix product A^T*x=y.
 *
 * This is a modified version of mkl_dcsrmv().
 * It is modiefied to compute the product of the traspose of the matrix and
 * the x vector. ALPHA=1 and BETA=0 must be true.
 *
 * @note My version of MKL's compressed sparse row (CSR) matrix vector product function
 * THIS FUNCTION ONLY WORKS FOR A FULLY STORED MATRIX!!!!!!
 * AND ALPHA==1 AND BETA==0
 *
 * @param transa Not used here, but included to stay with the BLAS.
 * @param m Number of rows in "A" matrix. Must equal k in this implementation.
 * @param k Number of columns in "A" matrix. Must equal m in this implementation.
 * @param alpha Must be one for this implementation.
 * @param matdescra Not used here, but included to stay with the BLAS.
 * @param val This is the "A" array.  Must be in compressed sparse row format, and symmetric with only the upper triangular part stored.
 * @param indx An array describing the column index of "A" (sometimes called "col_ind").
 * @param pntrb A pointer containing indices of "A" of the starting locations of the rows.
 * @param pntre A pointer containg indices of "A" of the ending locations of the rows.
 * @param x Vector of size m (and k) in the A*x=y computation.
 * @param beta Not used here, but included to stay with the BLAS.
 * @param y Vector of size m (and k) in the A*x=y computation.
 */
void LinearAlgebra::mkl_trans_dcsrmv(char *transa, int *m, int *k, double *alpha, char *matdescra, double *val, int *indx, int *pntrb, int *pntre, double *x, double *beta, double *y)
{
		//function multiplies the traspose of a sparse matrix "val" times a vector "x", result is stored in "y"
		//		ie. A^Tx = y
		//"m" and "k" are equal to the number of rows and columns in "A" (must be equal in mine)
		//"indx" is an array describing the column index of "A" (sometimes called "col_ind")
		//"pntrb" is a pointer containing indices of "A" of the starting locations of the rows
		//"pntre" is a pointer containg indices of "A" of the ending locations of the rows
		int i,j,N;
		N=*m;
		double *temp;

		temp = new double[N];  //vector to store (A[j] * x[j]) for each row in A (rows are really cols in A^T)

		for(int i = 0; i < N; i++){
            temp[i] = 0.0;
		}

    #pragma omp parallel private(i,j)
    {

        #pragma omp for
        for(i=0;i<N;i++){
            y[i]=0.0;
        }

        #pragma omp for
        for(i=0;i<N;i++)
        {
            // calculates A^Tx "in place" (A^T does not physically exist)
            if(matdescra[0] == 'g'){
                for(j=pntrb[i];j<pntre[i];j++) // iterate over all columns
                {
                    temp[indx[j]] += val[j]*x[i];  // for entire row: add element[j] * x[i] to element[j] in temp vector
                }
            }
        }

        cblas_dcopy(N, temp, 1, y, 1); // y <- temp

    }	//end parallel region

    if(temp)
        delete[] temp;
}

/**Function used to write the A matrix and b vector to a file.
 * Used for debugging purposes.
 * @param A "A" matrix in A*x=b computation. Stored in compressed row storage format.
 * @param col_ind Column index vector for compressed row stored matrix A.
 * @param row_ptr Row pointer vector for compressed row stored matrix A.
 * @param b "b" vector in A*x=b computation.  Size is numRows.
 */
void LinearAlgebra::Write_A_and_b(double* A, double* b, int *row_ptr, int *col_ind)
{
    FILE *A_file;
    FILE *b_file;

    A_file = fopen("A.txt", "w");
	b_file = fopen("b.txt", "w");

    for(int i=0;i<numRows;i++)
	{
        for(int j=row_ptr[i];j<row_ptr[i+1];j++)
			fprintf(A_file,"%d = %lf , %d\n", j, A[j], col_ind[j]);
		fprintf(b_file,"%d = %lf\n", i, b[i]);
	}

	fclose(A_file);
	fclose(b_file);
}

/**
 * @brief Deep copies a c-style array.
 *
 * The function does a deep copy of a c-style array.  This means it deallocates and then
 * allocates the proper memory, then copies the contents.
 *
 * @param destination Destination double pointer.
 * @param source Pointer to c-style array that already exists.
 * @param numValues Number of values to copy to the destination.
 */
void LinearAlgebra::deepCopyDoubleArray(double* destination, double* source, const int& numValues)
{
    if(destination!=NULL)
        delete[] destination;
    destination = new double[numValues];
    std::memcpy(destination, source, numValues*sizeof(double));
}
