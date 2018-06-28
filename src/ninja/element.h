/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for finite element operations on elements
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

#ifndef ELEMENT_H
#define ELEMENT_H

#include "ninjaException.h"
#include "mesh.h"

class Mesh;
class element
{
	public:
		element(Mesh const* m);						 //Default constructor
		~element();                              // Destructor


		//element(element const& e);               // Copy constructor
		//element& operator= (element const& e);   // Assignment operator

		void deallocate();

		void initializeQuadPtArrays();

		void get_xyz(const int &elementNum, const double &u, const double &v, const double &w, double &x, double &y, double &z);
		void computeJacobianEtc(int &elementNum, const double &u, const double &v, const double &w, double &x, double &y, double &z);
		//void computeElementStiffnessMatrix(const int &elementNum, const wn_3dScalarField &u0, const wn_3dScalarField &v0, const wn_3dScalarField &w0, const double &alpha);	//note that computeJacobianInfo() must have been run for this element first!
		void computeJacobianQuadraturePoint(const int &localQuadPointNum, const int &elementNum, double &x, double &y, double &z);
		void computeJacobianQuadraturePoint(const int &localQuadPointNum, const int &elementNum);

        void get_ij(double const& x,double const& y,
                     int& cell_i, int& cell_j); // given (x,y) returns cell (i,j)

		void get_uv(double const& x,double const& y,
                    int& cell_i, int& cell_j,
                    double& u, double &v);

		void get_uvw(double const& x,double const& y, double const& z,
		         int& cell_i, int& cell_j, int& cell_k,
				 double& u, double &v, double& w);	//given (x,y,z), this function locates the cell (i,j,k) that the point is in AND
	                                                //    the internal "parent" local cell coordinates (u,v,w) for use in interpolation in
	                                                //    functions such as wn_3dScalarField::interpolate()
		void get_uvw(double const& x,double const& y, double const& z,
		         double& u, double &v, double& w);	//Given (x,y,z), this function locates the cell (i,j,k) that the point is in AND
	                                                //    the internal "parent" local cell coordinates (u,v,w) for use in interpolation in
	                                                //    functions such as wn_3dScalarField::interpolate().

		double SFNV(const double &u, const double &v, const double &w, const int &n);

		Mesh const* mesh_;	//view of mesh for obtaining number of nodes in element, etc.
		int NUMQPTV;               //number of quadrature points used in the volume quadrature (can be 1, 8, or 27)
		double DETJ;		//determinant of Jacobian matrix
		double *RJACV, *RJACVI;	//Jacobian matrix and its inverse
		int node0, NPK, KNP;
		double *QE, *S;
		double *DNDX, *DNDY, *DNDZ;
		double *SFV;
		double *QPTV;

		double A1;            //quadrature point
		double WT;            //Gaussian quadrature weight
		double WT1;           //Gaussian weights for 3 point quadrature
		double WT2;
		double WT3;
		double WT4;
		double HVJ, RX, RY, RZ, DV;

		#ifdef SCALAR
		double BX, BY, BZ;    //coefficients for advection terms in scalar transport govering equation
        #endif

	private:

		double iterativeInterpTol;

	    double SFNVu(const double &u, const double &v, const double &w, const int &n);
	    double SFNVv(const double &u, const double &v, const double &w, const int &n);
	    double SFNVw(const double &u, const double &v, const double &w, const int &n);
	    double SFNS(const double &u, const double &v, const int &n);
	    double SFNSu(const double &u, const double &v, const int &n);
	    double SFNSv(const double &u, const double &v, const int &n);

		void interpLocalCoords_xy(const double &x,const double &y,
                                  const int& cell_i, const int& cell_j,
                                  double& u, double &v);

		void interpLocalCoords(const double &x,const double &y, const double &z,
		         const int& cell_i, const int& cell_j, const int& cell_k,
				 double& u, double &v, double& w);	//given (x,y,z) and cell (i,j,k), this function returns the internal
	                                                //    "parent" local cell coordinates (u,v,w)
};

#endif /* ELEMENT_H */
