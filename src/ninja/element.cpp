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

#include "element.h"

element::element(Mesh const* m)
{
	mesh_ = m;

	NUMQPTV=1;               //number of quadrature points used in the volume quadrature (can be 1, 8, or 27)
	iterativeInterpTol = 1E-6;

	SFV=NULL;
	QPTV=NULL;
	QE=NULL;
	S=NULL;
	DNDX=NULL;
	DNDY=NULL;
	DNDZ=NULL;
	RJACV=NULL;
	RJACVI=NULL;
}

element::~element()
{
	deallocate();
}

void element::initializeQuadPtArrays()
{
	SFV=new double[4*mesh_->NNPE*NUMQPTV];   //SF array for the volume quadrature (either(0=N,1=dN/du,2=dN/dv,3=dN/dw),local nodal point,quadrature point)
	QPTV=new double[NUMQPTV*3];        //QPTV stores the u, v, and w coordinates of the quadrature points
	QE=new double[mesh_->NNPE];
	S=new double[mesh_->NNPE*mesh_->NNPE];
	DNDX=new double[mesh_->NNPE];
	DNDY=new double[mesh_->NNPE];
	DNDZ=new double[mesh_->NNPE];
	RJACV=new double[9];                    //Jacobian matrix for volume quadrature
	RJACVI=new double[9];                   //Jacobian matrix inverse for volume quadrature


	if(NUMQPTV==1)        //This corresponds to one point quadrature
    {
        A1=0;
        WT=8.0;
        QPTV[0]=0;
        QPTV[1]=0;
        QPTV[2]=0;

    }else if(NUMQPTV==8)  //This is 2 point quadrature (in one dimension), or 8 points in 3 dimensions
    {
        A1=0.5773502692;
        WT=1.0;
        QPTV[0*3+0]=-A1;
        QPTV[0*3+1]=-A1;
        QPTV[0*3+2]=-A1;
        QPTV[1*3+0]=A1;
        QPTV[1*3+1]=-A1;
        QPTV[1*3+2]=-A1;
        QPTV[2*3+0]=A1;
        QPTV[2*3+1]=A1;
        QPTV[2*3+2]=-A1;
        QPTV[3*3+0]=-A1;
        QPTV[3*3+1]=A1;
        QPTV[3*3+2]=-A1;
        QPTV[4*3+0]=-A1;
        QPTV[4*3+1]=-A1;
        QPTV[4*3+2]=A1;
        QPTV[5*3+0]=A1;
        QPTV[5*3+1]=-A1;
        QPTV[5*3+2]=A1;
        QPTV[6*3+0]=A1;
        QPTV[6*3+1]=A1;
        QPTV[6*3+2]=A1;
        QPTV[7*3+0]=-A1;
        QPTV[7*3+1]=A1;
        QPTV[7*3+2]=A1;
    }else if(NUMQPTV==27)  //This is 3 point quadrature (in one dimension), or 27 points in 3 dimensions
    {
        A1=std::pow(0.6,0.5);
        WT1=125.0/729.0;
        WT2=200.0/729.0;
        WT3=320.0/729.0;
        WT4=512.0/729.0;
        QPTV[0*3+0]=-A1;
        QPTV[0*3+1]=-A1;
        QPTV[0*3+2]=-A1;
        QPTV[1*3+0]=A1;
        QPTV[1*3+1]=-A1;
        QPTV[1*3+2]=-A1;
        QPTV[2*3+0]=-A1;
        QPTV[2*3+1]=A1;
        QPTV[2*3+2]=-A1;
        QPTV[3*3+0]=A1;
        QPTV[3*3+1]=A1;
        QPTV[3*3+2]=-A1;
        QPTV[4*3+0]=-A1;
        QPTV[4*3+1]=-A1;
        QPTV[4*3+2]=A1;
        QPTV[5*3+0]=A1;
        QPTV[5*3+1]=-A1;
        QPTV[5*3+2]=A1;
        QPTV[6*3+0]=-A1;
        QPTV[6*3+1]=A1;
        QPTV[6*3+2]=A1;
        QPTV[7*3+0]=A1;
        QPTV[7*3+1]=A1;
        QPTV[7*3+2]=A1;
        QPTV[8*3+0]=-A1;
        QPTV[8*3+1]=0;
        QPTV[8*3+2]=-A1;
        QPTV[9*3+0]=A1;
        QPTV[9*3+1]=0;
        QPTV[9*3+2]=-A1;
        QPTV[10*3+0]=0;
        QPTV[10*3+1]=-A1;
        QPTV[10*3+2]=-A1;
        QPTV[11*3+0]=0;
        QPTV[11*3+1]=A1;
        QPTV[11*3+2]=-A1;
        QPTV[12*3+0]=-A1;
        QPTV[12*3+1]=-A1;
        QPTV[12*3+2]=0;
        QPTV[13*3+0]=A1;
        QPTV[13*3+1]=-A1;
        QPTV[13*3+2]=0;
        QPTV[14*3+0]=-A1;
        QPTV[14*3+1]=A1;
        QPTV[14*3+2]=0;
        QPTV[15*3+0]=A1;
        QPTV[15*3+1]=A1;
        QPTV[15*3+2]=0;
        QPTV[16*3+0]=-A1;
        QPTV[16*3+1]=0;
        QPTV[16*3+2]=A1;
        QPTV[17*3+0]=A1;
        QPTV[17*3+1]=0;
        QPTV[17*3+2]=A1;
        QPTV[18*3+0]=0;
        QPTV[18*3+1]=-A1;
        QPTV[18*3+2]=A1;
        QPTV[19*3+0]=0;
        QPTV[19*3+1]=A1;
        QPTV[19*3+2]=A1;
        QPTV[20*3+0]=0;
        QPTV[20*3+1]=0;
        QPTV[20*3+2]=-A1;
        QPTV[21*3+0]=-A1;
        QPTV[21*3+1]=0;
        QPTV[21*3+2]=0;
        QPTV[22*3+0]=A1;
        QPTV[22*3+1]=0;
        QPTV[22*3+2]=0;
        QPTV[23*3+0]=0;
        QPTV[23*3+1]=-A1;
        QPTV[23*3+2]=0;
        QPTV[24*3+0]=0;
        QPTV[24*3+1]=A1;
        QPTV[24*3+2]=0;
        QPTV[25*3+0]=0;
        QPTV[25*3+1]=0;
        QPTV[25*3+2]=A1;
        QPTV[26*3+0]=0;
        QPTV[26*3+1]=0;
        QPTV[26*3+2]=0;
    }else{
        throw std::domain_error("The number of quadrature points (NUMQPTV) must be 1, 8, or 27.");
    }

	//Get shape function quadrature data-------------------
     //SFquad function in Thompson's book Introduction to the Finite Element Method
     //The SF array is split into two seperate arrays, one for the volume quadrature (SFV) and one for the surface quadrature (SFS)(NOTE: SURFACE QUADRATURE NOT NEEDED BECAUSE OF PARTICULAR BCs!!!)
	for(int j=0;j<NUMQPTV;j++)
     {
          for(int i=0;i<mesh_->NNPE;i++)
          {
               SFV[0*mesh_->NNPE*NUMQPTV+i*NUMQPTV+j]=SFNV(QPTV[j*3+0],QPTV[j*3+1],QPTV[j*3+2],i);
               SFV[1*mesh_->NNPE*NUMQPTV+i*NUMQPTV+j]=SFNVu(QPTV[j*3+0],QPTV[j*3+1],QPTV[j*3+2],i);
               SFV[2*mesh_->NNPE*NUMQPTV+i*NUMQPTV+j]=SFNVv(QPTV[j*3+0],QPTV[j*3+1],QPTV[j*3+2],i);
               SFV[3*mesh_->NNPE*NUMQPTV+i*NUMQPTV+j]=SFNVw(QPTV[j*3+0],QPTV[j*3+1],QPTV[j*3+2],i);
          }
     }
}

//element::element(const element &e)	// Copy constructor
//{
//
//
//}

//element& element::operator =(const element &e)	// Assignment operator
//{
//	if(&e != this)
//	{
//		//allocate(e.rows_, e.cols_, e.layers_);
//
//		//for(int i=0; i<rows_*cols_*layers_; i++)
//		//	data_[i] = m(i);
//	}
//	return *this;
//
//}

void element::deallocate()
{
	delete[] SFV;
	SFV = NULL;

	delete[] QPTV;
	QPTV = NULL;

	delete[] QE;
	QE=NULL;

	delete[] S;
	S=NULL;

	delete[] DNDX;
	DNDX=NULL;

	delete[] DNDY;
	DNDY=NULL;

	delete[] DNDZ;
	DNDZ=NULL;

	delete[] RJACV;
	RJACV=NULL;

	delete[] RJACVI;
	RJACVI=NULL;
}

void element::computeJacobianEtc(int &elementNum, const double &u, const double &v, const double &w, double &x, double &y, double &z)
{
	//Given elementNum and (u,v,w), function computes the Jacobian, inverse Jacobian, determinant of the Jacobian, and (x,y,z)
    if(SFV == NULL)
		initializeQuadPtArrays();

	node0=mesh_->get_node0(elementNum);  //get the global nodal number of local node 0 of element i

	x=0.0;
    y=0.0;
    z=0.0;

	for(int l=0;l<3;l++)
	{
        for(int m=0;m<3;m++)
             RJACV[l*3+m]=0.0;    //Initialize Jacobian matrix
	}
	for(int k=0;k<mesh_->NNPE;k++)          //Start loop over nodes in the element
	{

        NPK=mesh_->get_global_node(k, elementNum);            //NPK is the global node number

        x=x+SFNV(u, v, w, k) * mesh_->XORD(NPK);          //don't need since none of the coefs are in terms of the x,y,z coordinates like in book
        y=y+SFNV(u, v, w, k) * mesh_->YORD(NPK);
        z=z+SFNV(u, v, w, k) * mesh_->ZORD(NPK);

	   RJACV[0*3+0]=RJACV[0*3+0] + SFNVu(u, v, w, k) * mesh_->XORD(NPK);
        RJACV[0*3+1]=RJACV[0*3+1] + SFNVv(u, v, w, k) * mesh_->XORD(NPK);
        RJACV[0*3+2]=RJACV[0*3+2] + SFNVw(u, v, w, k) * mesh_->XORD(NPK);
        RJACV[1*3+0]=RJACV[1*3+0] + SFNVu(u, v, w, k) * mesh_->YORD(NPK);
        RJACV[1*3+1]=RJACV[1*3+1] + SFNVv(u, v, w, k) * mesh_->YORD(NPK);
        RJACV[1*3+2]=RJACV[1*3+2] + SFNVw(u, v, w, k) * mesh_->YORD(NPK);
        RJACV[2*3+0]=RJACV[2*3+0] + SFNVu(u, v, w, k) * mesh_->ZORD(NPK);
        RJACV[2*3+1]=RJACV[2*3+1] + SFNVv(u, v, w, k) * mesh_->ZORD(NPK);
        RJACV[2*3+2]=RJACV[2*3+2] + SFNVw(u, v, w, k) * mesh_->ZORD(NPK);
	}                             //End loop over nodes in the element

	//Calculate the determinant of the 3x3 Jacobian.
	DETJ=RJACV[0*3+0]*(RJACV[1*3+1]*RJACV[2*3+2]-RJACV[1*3+2]*RJACV[2*3+1])-RJACV[0*3+1]*(RJACV[1*3+0]*RJACV[2*3+2]-RJACV[1*3+2]*RJACV[2*3+0])+RJACV[0*3+2]*(RJACV[1*3+0]*RJACV[2*3+1]-RJACV[1*3+1]*RJACV[2*3+0]);
	//cout<<"DETJ = " <<DETJ<<endl;
	if(DETJ<=0)
		throw std::runtime_error("Volume Jacobian 1 is zero or negative.");

	//Calculate the inverse of the Jacobian (3x3 matrix) using information from http://mathworld.wolfram.com/MatrixInverse.html
	RJACVI[0*3+0]=(RJACV[1*3+1]*RJACV[2*3+2]-RJACV[1*3+2]*RJACV[2*3+1])/DETJ;
	RJACVI[0*3+1]=(RJACV[0*3+2]*RJACV[2*3+1]-RJACV[0*3+1]*RJACV[2*3+2])/DETJ;
	RJACVI[0*3+2]=(RJACV[0*3+1]*RJACV[1*3+2]-RJACV[0*3+2]*RJACV[1*3+1])/DETJ;
	RJACVI[1*3+0]=(RJACV[1*3+2]*RJACV[2*3+0]-RJACV[1*3+0]*RJACV[2*3+2])/DETJ;
	RJACVI[1*3+1]=(RJACV[0*3+0]*RJACV[2*3+2]-RJACV[0*3+2]*RJACV[2*3+0])/DETJ;
	RJACVI[1*3+2]=(RJACV[0*3+2]*RJACV[1*3+0]-RJACV[0*3+0]*RJACV[1*3+2])/DETJ;
	RJACVI[2*3+0]=(RJACV[1*3+0]*RJACV[2*3+1]-RJACV[1*3+1]*RJACV[2*3+0])/DETJ;
	RJACVI[2*3+1]=(RJACV[0*3+1]*RJACV[2*3+0]-RJACV[0*3+0]*RJACV[2*3+1])/DETJ;
	RJACVI[2*3+2]=(RJACV[0*3+0]*RJACV[1*3+1]-RJACV[0*3+1]*RJACV[1*3+0])/DETJ;
}

void element::computeJacobianQuadraturePoint(const int &localQuadPointNum, const int &elementNum, double &x, double &y, double &z)
{
	//Given localQuadPointNum and elementNum, function computes the Jacobian, inverse Jacobian, determinant of the Jacobian, and (x,y,z)
	if(SFV == NULL)
		initializeQuadPtArrays();
	
	x=0.0;
    y=0.0;
    z=0.0;

	for(int l=0;l<3;l++)
	{
        for(int m=0;m<3;m++)
             RJACV[l*3+m]=0.0;    //Initialize Jacobian matrix
	}
	for(int k=0;k<mesh_->NNPE;k++)          //Start loop over nodes in the element
	{



        NPK=mesh_->get_global_node(k, elementNum);            //NPK is the global nodal number

        x=x+SFV[0*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->XORD(NPK);
        y=y+SFV[0*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->YORD(NPK);
        z=z+SFV[0*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->ZORD(NPK);

        RJACV[0*3+0]=RJACV[0*3+0]+SFV[1*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->XORD(NPK);
        RJACV[0*3+1]=RJACV[0*3+1]+SFV[2*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->XORD(NPK);
        RJACV[0*3+2]=RJACV[0*3+2]+SFV[3*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->XORD(NPK);
        RJACV[1*3+0]=RJACV[1*3+0]+SFV[1*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->YORD(NPK);
        RJACV[1*3+1]=RJACV[1*3+1]+SFV[2*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->YORD(NPK);
        RJACV[1*3+2]=RJACV[1*3+2]+SFV[3*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->YORD(NPK);
        RJACV[2*3+0]=RJACV[2*3+0]+SFV[1*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->ZORD(NPK);
        RJACV[2*3+1]=RJACV[2*3+1]+SFV[2*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->ZORD(NPK);
        RJACV[2*3+2]=RJACV[2*3+2]+SFV[3*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->ZORD(NPK);
	}                             //End loop over nodes in the element

	//Calculate the determinant of the 3x3 Jacobian.
	DETJ=RJACV[0*3+0]*(RJACV[1*3+1]*RJACV[2*3+2]-RJACV[1*3+2]*RJACV[2*3+1])-RJACV[0*3+1]*(RJACV[1*3+0]*RJACV[2*3+2]-RJACV[1*3+2]*RJACV[2*3+0])+RJACV[0*3+2]*(RJACV[1*3+0]*RJACV[2*3+1]-RJACV[1*3+1]*RJACV[2*3+0]);
	if(DETJ<=0)
		throw std::runtime_error("Volume Jacobian 1 is zero or negative.");

	//Calculate the inverse of the Jacobian (3x3 matrix) using information from http://mathworld.wolfram.com/MatrixInverse.html
	RJACVI[0*3+0]=(RJACV[1*3+1]*RJACV[2*3+2]-RJACV[1*3+2]*RJACV[2*3+1])/DETJ;
	RJACVI[0*3+1]=(RJACV[0*3+2]*RJACV[2*3+1]-RJACV[0*3+1]*RJACV[2*3+2])/DETJ;
	RJACVI[0*3+2]=(RJACV[0*3+1]*RJACV[1*3+2]-RJACV[0*3+2]*RJACV[1*3+1])/DETJ;
	RJACVI[1*3+0]=(RJACV[1*3+2]*RJACV[2*3+0]-RJACV[1*3+0]*RJACV[2*3+2])/DETJ;
	RJACVI[1*3+1]=(RJACV[0*3+0]*RJACV[2*3+2]-RJACV[0*3+2]*RJACV[2*3+0])/DETJ;
	RJACVI[1*3+2]=(RJACV[0*3+2]*RJACV[1*3+0]-RJACV[0*3+0]*RJACV[1*3+2])/DETJ;
	RJACVI[2*3+0]=(RJACV[1*3+0]*RJACV[2*3+1]-RJACV[1*3+1]*RJACV[2*3+0])/DETJ;
	RJACVI[2*3+1]=(RJACV[0*3+1]*RJACV[2*3+0]-RJACV[0*3+0]*RJACV[2*3+1])/DETJ;
	RJACVI[2*3+2]=(RJACV[0*3+0]*RJACV[1*3+1]-RJACV[0*3+1]*RJACV[1*3+0])/DETJ;

	//Calculate dN/dx, dN/dy, dN/dz (Remember we're using the transpose of the inverse!)
	for(int k=0;k<mesh_->NNPE;k++)
	{
		DNDX[k]=RJACVI[0*3+0]*SFV[1*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]+RJACVI[1*3+0]*SFV[2*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]+RJACVI[2*3+0]*SFV[3*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum];
		DNDY[k]=RJACVI[0*3+1]*SFV[1*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]+RJACVI[1*3+1]*SFV[2*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]+RJACVI[2*3+1]*SFV[3*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum];
		DNDZ[k]=RJACVI[0*3+2]*SFV[1*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]+RJACVI[1*3+2]*SFV[2*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]+RJACVI[2*3+2]*SFV[3*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum];
	}

}

void element::computeJacobianQuadraturePoint(const int &localQuadPointNum, const int &elementNum)
{
	//Given localQuadPointNum and elementNum, function computes the Jacobian, inverse Jacobian, determinant of the Jacobian, and (x,y,z)
	if(SFV == NULL)
		initializeQuadPtArrays();
	
	//x=0.0;
    //y=0.0;
    //z=0.0;

	for(int l=0;l<3;l++)
	{
        for(int m=0;m<3;m++)
             RJACV[l*3+m]=0.0;    //Initialize Jacobian matrix
	}
	for(int k=0;k<mesh_->NNPE;k++)          //Start loop over nodes in the element
	{



        NPK=mesh_->get_global_node(k, elementNum);            //NPK is the global nodal number

        //x=x+SFV[0*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->XORD(NPK);
        //y=y+SFV[0*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->YORD(NPK);
        //z=z+SFV[0*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->ZORD(NPK);

        RJACV[0*3+0]=RJACV[0*3+0]+SFV[1*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->XORD(NPK);
        RJACV[0*3+1]=RJACV[0*3+1]+SFV[2*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->XORD(NPK);
        RJACV[0*3+2]=RJACV[0*3+2]+SFV[3*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->XORD(NPK);
        RJACV[1*3+0]=RJACV[1*3+0]+SFV[1*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->YORD(NPK);
        RJACV[1*3+1]=RJACV[1*3+1]+SFV[2*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->YORD(NPK);
        RJACV[1*3+2]=RJACV[1*3+2]+SFV[3*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->YORD(NPK);
        RJACV[2*3+0]=RJACV[2*3+0]+SFV[1*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->ZORD(NPK);
        RJACV[2*3+1]=RJACV[2*3+1]+SFV[2*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->ZORD(NPK);
        RJACV[2*3+2]=RJACV[2*3+2]+SFV[3*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]*mesh_->ZORD(NPK);
	}                             //End loop over nodes in the element

	//Calculate the determinant of the 3x3 Jacobian.
	DETJ=RJACV[0*3+0]*(RJACV[1*3+1]*RJACV[2*3+2]-RJACV[1*3+2]*RJACV[2*3+1])-RJACV[0*3+1]*(RJACV[1*3+0]*RJACV[2*3+2]-RJACV[1*3+2]*RJACV[2*3+0])+RJACV[0*3+2]*(RJACV[1*3+0]*RJACV[2*3+1]-RJACV[1*3+1]*RJACV[2*3+0]);
	if(DETJ<=0)
		throw std::runtime_error("Volume Jacobian 1 is zero or negative.");

	//Calculate the inverse of the Jacobian (3x3 matrix) using information from http://mathworld.wolfram.com/MatrixInverse.html
	RJACVI[0*3+0]=(RJACV[1*3+1]*RJACV[2*3+2]-RJACV[1*3+2]*RJACV[2*3+1])/DETJ;
	RJACVI[0*3+1]=(RJACV[0*3+2]*RJACV[2*3+1]-RJACV[0*3+1]*RJACV[2*3+2])/DETJ;
	RJACVI[0*3+2]=(RJACV[0*3+1]*RJACV[1*3+2]-RJACV[0*3+2]*RJACV[1*3+1])/DETJ;
	RJACVI[1*3+0]=(RJACV[1*3+2]*RJACV[2*3+0]-RJACV[1*3+0]*RJACV[2*3+2])/DETJ;
	RJACVI[1*3+1]=(RJACV[0*3+0]*RJACV[2*3+2]-RJACV[0*3+2]*RJACV[2*3+0])/DETJ;
	RJACVI[1*3+2]=(RJACV[0*3+2]*RJACV[1*3+0]-RJACV[0*3+0]*RJACV[1*3+2])/DETJ;
	RJACVI[2*3+0]=(RJACV[1*3+0]*RJACV[2*3+1]-RJACV[1*3+1]*RJACV[2*3+0])/DETJ;
	RJACVI[2*3+1]=(RJACV[0*3+1]*RJACV[2*3+0]-RJACV[0*3+0]*RJACV[2*3+1])/DETJ;
	RJACVI[2*3+2]=(RJACV[0*3+0]*RJACV[1*3+1]-RJACV[0*3+1]*RJACV[1*3+0])/DETJ;

	//Calculate dN/dx, dN/dy, dN/dz (Remember we're using the transpose of the inverse!)
	for(int k=0;k<mesh_->NNPE;k++)
	{
		DNDX[k]=RJACVI[0*3+0]*SFV[1*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]+RJACVI[1*3+0]*SFV[2*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]+RJACVI[2*3+0]*SFV[3*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum];
		DNDY[k]=RJACVI[0*3+1]*SFV[1*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]+RJACVI[1*3+1]*SFV[2*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]+RJACVI[2*3+1]*SFV[3*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum];
		DNDZ[k]=RJACVI[0*3+2]*SFV[1*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]+RJACVI[1*3+2]*SFV[2*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum]+RJACVI[2*3+2]*SFV[3*mesh_->NNPE*NUMQPTV+k*NUMQPTV+localQuadPointNum];
	}
}

//void element::computeElementStiffnessMatrix(const int &elementNum, const wn_3dScalarField &u0, const wn_3dScalarField &v0, const wn_3dScalarField &w0, const double &alpha)
//{	
//	//Given the above parameters, function computes the element stiffness matrix
//
//	if(SFV == NULL)
//		initializeQuadPtArrays();
//
//	for(int j=0;j<mesh_->NNPE;j++)
//	{
//	   QE[j]=0.0;
//	   for(int k=0;k<mesh_->NNPE;k++)
//			S[j*mesh_->NNPE+k]=0.0;
//	}
//	//Begin quadrature for current element
//
//	node0=mesh_->get_node0(elementNum);  //get the global nodal number of local node 0 of element i
//
//	for(int j=0;j<NUMQPTV;j++)             //Start loop over quadrature points in the element
//	{
//
//        computeJacobianQuadraturePoint(j, elementNum);
//
//	   //Calculate the coefficient H here and the alpha-squared term in front of the second partial of z in governing equation (we are still on element i, quadrature point j)
//	   //
//	   //                d u0   d v0   d w0                           2
//	   //     H = 2 * ( ---- + ---- + ---- )      and      Rz = alpha
//	   //                d x    d y    d z
//	   //
//
//	   HVJ=0.0;
//	   for(int k=0;k<mesh_->NNPE;k++)          //Start loop over nodes in the element
//	   {
//			NPK=mesh_->get_global_node(k, elementNum);            //NPK is the global nodal double
//
//			HVJ=HVJ+((DNDX[k]*u0(NPK))+(DNDY[k]*v0(NPK))+(DNDZ[k]*w0(NPK)));
//
//	   }                             //End loop over nodes in the element
//	   HVJ=2*HVJ;                    //This is the H for quad point j (the 2* comes from governing equation)
//
//	   RZ=alpha*alpha;               //This is the RZ from the governing equation
//									 //Options here for later could be to incorporate an alpha that varies over x, y, and z
//									 // or if it is always uniform throughout, this can be taken out of the loops to speed computation time
//	   DV=DETJ;                      //DV is the DV for the volume integration (could be eliminated and just use DETJ everywhere)
//
//
//	   if(NUMQPTV==27)
//	   {
//			if(j<=7)
//			{
//				 WT=WT1;
//			}else if(j<=19)
//			{
//				 WT=WT2;
//			}else if(j<=25)
//			{
//				 WT=WT3;
//			}else
//			{
//				 WT=WT4;
//			}
//	   }
//
//	   //Create element stiffness matrix---------------------------------------------
//	   for(int k=0;k<mesh_->NNPE;k++)          //Start loop over nodes in the element
//	   {
//			QE[k]=QE[k]+WT*SFV[0*mesh_->NNPE*NUMQPTV+k*NUMQPTV+j]*HVJ*DV;
//			for(int l=0;l<mesh_->NNPE;l++)
//			{
//				 S[k*mesh_->NNPE+l]=S[k*mesh_->NNPE+l]+WT*(DNDX[k]*DNDX[l]+DNDY[k]*DNDY[l]+DNDZ[k]*RZ*DNDZ[l])*DV;
//			}
//
//	   }                            //End loop over nodes in the element
//
//	}                                  //End loop over quadrature points in the element
//}

double element::SFNV(const double &u, const double &v, const double &w, const int &n)
{
     //SFNV calculates the shape function values for the volume quadrature
     //n is the local nodal number
     double f;
     if(n==0)
          f=0.125*(1-u)*(1-v)*(1-w);
     else if(n==1)
          f=0.125*(1+u)*(1-v)*(1-w);
     else if(n==2)
          f=0.125*(1+u)*(1+v)*(1-w);
     else if(n==3)
          f=0.125*(1-u)*(1+v)*(1-w);
     else if(n==4)
          f=0.125*(1-u)*(1-v)*(1+w);
     else if(n==5)
          f=0.125*(1+u)*(1-v)*(1+w);
     else if(n==6)
          f=0.125*(1+u)*(1+v)*(1+w);
     else if(n==7)
          f=0.125*(1-u)*(1+v)*(1+w);
     else
          throw std::logic_error("Problem in function SFNV().");
 
     return f;
}

double element::SFNVu(const double &u, const double &v, const double &w, const int &n)
{
     //SFNVu evaluates dN/du
     double f;
     if(n==0)
          f=0.125*(-1)*(1-v)*(1-w);
     else if(n==1)
          f=0.125*(1)*(1-v)*(1-w);
     else if(n==2)
          f=0.125*(1)*(1+v)*(1-w);
     else if(n==3)
          f=0.125*(-1)*(1+v)*(1-w);
     else if(n==4)
          f=0.125*(-1)*(1-v)*(1+w);
     else if(n==5)
          f=0.125*(1)*(1-v)*(1+w);
     else if(n==6)
          f=0.125*(1)*(1+v)*(1+w);
     else if(n==7)
          f=0.125*(-1)*(1+v)*(1+w);
     else
          throw std::logic_error("Problem in function SFNVu().");
      
     return f;
}

double element::SFNVv(const double &u, const double &v, const double &w, const int &n)
{
     //SFNVv evaluates dN/dv
     double f;
     if(n==0)
          f=0.125*(1-u)*(-1)*(1-w);
     else if(n==1)
          f=0.125*(1+u)*(-1)*(1-w);
     else if(n==2)
          f=0.125*(1+u)*(1)*(1-w);
     else if(n==3)
          f=0.125*(1-u)*(1)*(1-w);
     else if(n==4)
          f=0.125*(1-u)*(-1)*(1+w);
     else if(n==5)
          f=0.125*(1+u)*(-1)*(1+w);
     else if(n==6)
          f=0.125*(1+u)*(1)*(1+w);
     else if(n==7)
          f=0.125*(1-u)*(1)*(1+w);
     else
		  throw std::logic_error("Problem in function SFNVv().");
          
     return f;
}

double element::SFNVw(const double &u, const double &v, const double &w, const int &n)
{
     //SFNVw evaluates dN/dw
     double f;
     if(n==0)
          f=0.125*(1-u)*(1-v)*(-1);
     else if(n==1)
          f=0.125*(1+u)*(1-v)*(-1);
     else if(n==2)
          f=0.125*(1+u)*(1+v)*(-1);
     else if(n==3)
          f=0.125*(1-u)*(1+v)*(-1);
     else if(n==4)
          f=0.125*(1-u)*(1-v)*(1);
     else if(n==5)
          f=0.125*(1+u)*(1-v)*(1);
     else if(n==6)
          f=0.125*(1+u)*(1+v)*(1);
     else if(n==7)
          f=0.125*(1-u)*(1+v)*(1);
     else
		  throw std::logic_error("Problem in function SFNVw().");
          
     return f;
}

double element::SFNS(const double &u, const double &v, const int &n)
{
     //SFNS calculates the shape function values for the surface quadrature
     //n is the local nodal point
     double f;
     if(n==0)
          f=0.25*(1-u)*(1-v);
     else if(n==1)
          f=0.25*(1+u)*(1-v);
     else if(n==2)
          f=0.25*(1+u)*(1+v);
     else if(n==3)
          f=0.25*(1-u)*(1+v);
     else
		  throw std::logic_error("Problem in function SFNS().");
         
     return f;
}

double element::SFNSu(const double &u, const double &v, const int &n)
{
     //SFNSu calculates dN/du for the surface quadrature
     //n is the local nodal point
     double f;
     if(n==0)
          f=0.25*(-1)*(1-v);
     else if(n==1)
          f=0.25*(1)*(1-v);
     else if(n==2)
          f=0.25*(1)*(1+v);
     else if(n==3)
          f=0.25*(-1)*(1+v);
     else
		  throw std::logic_error("Problem in function SFNSu().");
         
     return f;
}

double element::SFNSv(const double &u, const double &v, const int &n)
{
     //SFNSv calculates dN/dv for the surface quadrature
     //n is the local nodal point
     double f;
     if(n==0)
          f=0.25*(1-u)*(-1);
     else if(n==1)
          f=0.25*(1+u)*(-1);
     else if(n==2)
          f=0.25*(1+u)*(1);
     else if(n==3)
          f=0.25*(1-u)*(1);
     else
		  throw std::logic_error("Problem in function SFNSv().");
         
     return f;
}

/**
 * @brief Interpolate ZORD to a given (x,y) for a specified layer in the mesh.
 *
 * Uses an interpolation scheme in the x-y plane that is consistent with interpolation
 * methods used in other elemnent functions, such as element::get_uvw(). 
 *
 * @param x Requested x location in WN coordinates.
 * @param y Requested y location in WN coordinates.
 * @param layer Layer in WN mesh to perform the interpolation.
 * @return Interpolated value
 */

double element::interpolate_z(double const& x, double const& y, double const& layer)
{
    
    //needs to be double checked -- not sure if this interpolation is working correctly
    
    int node_i, node_j, node_k;
    int cell_i, cell_j;
    double answer; // the result of the interpolation

    node_k = layer; // the layer we want to interpolate on

    //initialize cell values
    cell_i = -1;
    cell_j = -1;

    //compute cell i value
    for(node_i=1; node_i<mesh_->nrows; node_i++)
    {
        if(y <= mesh_->YORD(node_i, 0, 0))
        {
            cell_i = node_i - 1;
            break;
        }
    }
    if(cell_i<0)
        throw std::range_error("Range error in element::interpolate_xy()");

    //compute cell j value
    for(node_j=1; node_j<mesh_->ncols; node_j++)
    {
        if(x <= mesh_->XORD(0, node_j, 0))
        {
            cell_j = node_j - 1;
            break;
        }
    }
    if(cell_j<0)
        throw std::range_error("Range error in element::interpolate_xy()");

    answer = (mesh_->ZORD(node_i-1, node_j-1, node_k) +
              mesh_->ZORD(node_i-1, node_j, node_k) +
              mesh_->ZORD(node_i, node_j-1, node_k) +
              mesh_->ZORD(node_i, node_j, node_k)) / 4.0;

    return answer;
}

/**
 * @brief Given (x,y), find cell (i,j) index.
 *
 * Similar to element::get_uvw() except does not compute (u,v,w) and does not 
 * find k since locating k requires iteration.
 * This saves computation time when only i,j are needed since it doesn't require iteration.
 * 
 * @param x Requested x location in WN coordinates.
 * @param y Requested y location in WN coordinates.
 * @param cell_i Variable to be populated wth i index of cell containing (x,y).
 * @param cell_j Variable to be populated wth j index of cell containing (x,y).
 */

void element::get_ij(double const& x,double const& y,
                      int& cell_i, int& cell_j)
{
    int node_i, node_j;

    //initialize cell values
    cell_i = -1;
    cell_j = -1;

    //compute cell i value
    for(node_i=1; node_i<mesh_->nrows; node_i++)
    {
        if(y <= mesh_->YORD(node_i, 0, 0))
        {
            cell_i = node_i - 1;
            break;
        }
    }
    if(cell_i<0)
        throw std::range_error("Range error in element::get_ij()");

    //compute cell j value
    for(node_j=1; node_j<mesh_->ncols; node_j++)
    {
        if(x <= mesh_->XORD(0, node_j, 0))
        {
            cell_j = node_j - 1;
            break;
        }
    }
    if(cell_j<0)
        throw std::range_error("Range error in element::get_ij()");

}                  

void element::get_uv(double const& x,double const& y,
		             int& cell_i, int& cell_j,
                     double& u, double &v)	//Given (x,y), this function locates the cell (i,j) that the point is in AND 
	                                                //    the internal "parent" local cell coordinates (u,v) 
{
	int node_i, node_j;

	//initialize cell values
	cell_i = -1;
	cell_j = -1;

	//compute cell i value
	for(node_i=1; node_i<mesh_->nrows; node_i++)
	{
		if(y <= mesh_->YORD(node_i, 0, 0))
		//if(y < mesh_->YORD(node_i, 0, 0))
		{
			cell_i = node_i - 1;
			break;
		}
	}
	if(cell_i<0)
		throw std::range_error("Range error in element::get_uv()");

	//compute cell j value
	for(node_j=1; node_j<mesh_->ncols; node_j++)
	{
		if(x <= mesh_->XORD(0, node_j, 0))
		//if(x < mesh_->XORD(0, node_j, 0))
		{
			cell_j = node_j - 1;
			break;
		}
	}
	if(cell_j<0)
		throw std::range_error("Range error in element::get_uv()");

	
	interpLocalCoords_xy(x, y, cell_i, cell_j, u, v);
	
    if(u > 1.0)
	{
        do{
		    cell_j++;
		    interpLocalCoords_xy(x, y, cell_i, cell_j, u, v);
        }while(u > 1.0);
	}
	if(v > 1.0)
	{
        do{
		    cell_i++;
		    interpLocalCoords_xy(x, y, cell_i, cell_j, u, v);
        }while(v > 1.0);
	}
	if(u < -1.0)
	{
        do{
		    cell_j--;
		    interpLocalCoords_xy(x, y, cell_i, cell_j, u, v);
        }while(u < -1.0);
	}
	if(v < -1.0)
	{
        do{
		    cell_i--;
		    interpLocalCoords_xy(x, y, cell_i, cell_j, u, v);
        }while(v < -1.0);
	}

}

void element::get_uvw(double const& x,double const& y, double const& z,
		         int& cell_i, int& cell_j, int& cell_k,
				 double& u, double &v, double& w)	//Given (x,y,z), this function locates the cell (i,j,k) that the point is in AND 
	                                                //    the internal "parent" local cell coordinates (u,v,w) for use in interpolation in
	                                                //    functions such as wn_3dScalarField::interpolate().
{
	int node_i, node_j, node_k;
	double zAverage, zAverage_ground;

	//initialize cell values
	cell_i = -1;
	cell_j = -1;
	cell_k = -1;

	//compute cell i value
	for(node_i=1; node_i<mesh_->nrows; node_i++)
	{
		if(y <= mesh_->YORD(node_i, 0, 0))
		//if(y < mesh_->YORD(node_i, 0, 0))
		{
			cell_i = node_i - 1;
			break;
		}
	}
	if(cell_i<0)
		throw std::range_error("Range error in element::get_uvw()");

	//compute cell j value
	for(node_j=1; node_j<mesh_->ncols; node_j++)
	{
		if(x <= mesh_->XORD(0, node_j, 0))
		//if(x < mesh_->XORD(0, node_j, 0))
		{
			cell_j = node_j - 1;
			break;
		}
	}
	if(cell_j<0)
		throw std::range_error("Range error in element::get_uvw()");

	//compute cell k value (estimate using average of 4 points surrounding)
	for(node_k=1; node_k<mesh_->nlayers; node_k++)
	{	    
		zAverage = (mesh_->ZORD(node_i-1, node_j-1, node_k) + mesh_->ZORD(node_i-1, node_j, node_k) + mesh_->ZORD(node_i, node_j-1, node_k) + mesh_->ZORD(node_i, node_j, node_k)) / 4.0; 
		//zAverage_ground = (mesh_->ZORD(node_i-1, node_j-1, 0) + mesh_->ZORD(node_i-1, node_j, 0) + mesh_->ZORD(node_i, node_j-1, 0) + mesh_->ZORD(node_i, node_j, 0)) / 4.0; 

		
		if(z <= zAverage)
		//if(z < zAverage)
		{
			cell_k = node_k - 1;
			break;
		}
	}
	
			
	if(cell_k<0)
		throw std::range_error("Range error in element::get_uvw()");
	
	interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
	
    if(u > 1.0)
	{
        do{
		    cell_j++;
		    interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
        }while(u > 1.0);
	}
	if(v > 1.0)
	{
        do{
		    cell_i++;
		    interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
        }while(v > 1.0);
	}
	if(w > 1.0)
	{
        do{
		    cell_k++;
		    interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
        }while(w > 1.0);
	}
	if(u < -1.0)
	{
        do{
		    cell_j--;
		    interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
        }while(u < -1.0);
	}
	if(v < -1.0)
	{
        do{
		    cell_i--;
		    interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
        }while(v < -1.0);
	}
	if(w < -1.0)
	{
        do{
            cell_k--;
		    interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
        }while(w < -1.0);
	}
}

void element::get_uvw(double const& x,double const& y, double const& z,
		         double& u, double &v, double& w)	//Given (x,y,z), this function locates the cell (i,j,k) that the point is in AND 
	                                                //    the internal "parent" local cell coordinates (u,v,w) for use in interpolation in
	                                                //    functions such as wn_3dScalarField::interpolate().
{
	int node_i, node_j, node_k;
	int cell_i, cell_j, cell_k;
	double zAverage;

	//compute cell i value
	for(node_i=1; node_i<mesh_->nrows; node_i++)
	{
		if(y <= mesh_->YORD(node_i, 0, 0))
		//if(y < mesh_->YORD(node_i, 0, 0))
		{
			cell_i = node_i - 1;
			break;
		}
	}
	//compute cell j value
	for(node_j=1; node_j<mesh_->ncols; node_j++)
	{
		if(x <= mesh_->XORD(0, node_j, 0))
		//if(x < mesh_->XORD(0, node_j, 0))
		{
			cell_j = node_j - 1;
			break;
		}
	}
	//compute cell k value (estimate using average of 4 points surrounding)
	for(node_k=1; node_k<mesh_->nlayers; node_k++)
	{
		zAverage = (mesh_->ZORD(node_i-1, node_j-1, node_k) + mesh_->ZORD(node_i-1, node_j, node_k) + mesh_->ZORD(node_i, node_j-1, node_k) + mesh_->ZORD(node_i, node_j, node_k)) / 4.0; 
		if(z <= zAverage)
		//if(z < zAverage)
		{
			cell_k = node_k - 1;
			break;
		}
	}
	
	interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);

	
    if(u > 1.0)
	{
        do{
		    cell_j++;
		    interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
        }while(u > 1.0);
	}
	if(v > 1.0)
	{
        do{
		    cell_i++;
		    interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
        }while(v > 1.0);
	}
	if(w > 1.0)
	{
        do{
		    cell_k++;
		    interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
        }while(w > 1.0);
	}
	if(u < -1.0)
	{
        do{
		    cell_j--;
		    interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
        }while(u < -1.0);
	}
	if(v < -1.0)
	{
        do{
		    cell_i--;
		    interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
        }while(v < -1.0);
	}
	if(w < -1.0)
	{
        do{
            cell_k--;
		    interpLocalCoords(x, y, z, cell_i, cell_j, cell_k, u, v, w);
        }while(w < -1.0);
	}
}

void element::interpLocalCoords_xy(const double &x,const double &y,
                                   const int& cell_i, const int& cell_j,
                                   double& u, double &v)	//Given (x,y) and cell (i,j), this function returns the internal
	                                                //    "parent" local cell coordinates (u,v).
													//    Iteration is used to accomplish this.
{
	double dist, xDist, yDist, total;

    int cell_k = 0;
	
	double cellsize = mesh_->ZORD(cell_i, cell_j, cell_k+1) - mesh_->ZORD(cell_i, cell_j, cell_k);
	double x_test, y_test, z_test;
    double u_new, v_new;
    double w, z;
	
	cell_k = 0; // vertical coord doesn't matter 
	int elemNum = mesh_->get_elemNum(cell_i, cell_j, cell_k);


    //start with a "good" guess for u
	dist = x - mesh_->XORD(cell_i, cell_j, cell_k);
	total = mesh_->XORD(cell_i, cell_j+1, cell_k) - mesh_->XORD(cell_i, cell_j, cell_k);
	u = 2.0*(dist/total) - 1.0;

    //start with a "good" guess for v
	dist = y - mesh_->YORD(cell_i, cell_j, cell_k);
	total = mesh_->YORD(cell_i+1, cell_j, cell_k) - mesh_->YORD(cell_i, cell_j, cell_k);
	v = 2.0*(dist/total) - 1.0;	
	
	w = 0.0;

    u_new = u;
    v_new = v;

	do{
        u = u_new;
        v = v_new;

        //compute test x,y based on current u,v
		computeJacobianEtc(elemNum, u, v, w, x_test, y_test, z_test);
		z = z_test; //vertical coord not important
        
        //compute new u,v,w which may be needed for the next iteration if we're not close enough yet
        u_new = u - (RJACVI[0*3+0]*(x_test-x) + RJACVI[0*3+1]*(y_test-y) + RJACVI[0*3+2]*(z_test-z));
        v_new = v - (RJACVI[1*3+0]*(x_test-x) + RJACVI[1*3+1]*(y_test-y) + RJACVI[1*3+2]*(z_test-z));
        
        //compute current error
        xDist = x-x_test;
        yDist = y-y_test;
		dist = std::sqrt((xDist)*(xDist) + (yDist)*(yDist));

	}while((dist/cellsize) > iterativeInterpTol);
}

void element::interpLocalCoords(const double &x,const double &y, const double &z,
		         const int& cell_i, const int& cell_j, const int& cell_k,
				 double& u, double &v, double& w)	//Given (x,y,z) and cell (i,j,k), this function returns the internal
	                                                //    "parent" local cell coordinates (u,v,w).
													//    Iteration is used to accomplish this.
{
	double dist, xDist, yDist, zDist, total;
	//double zAverageBottom, zAverageTop;
	//int node0;
	double cellsize = mesh_->ZORD(cell_i, cell_j, cell_k+1) - mesh_->ZORD(cell_i, cell_j, cell_k);
	double x_test, y_test, z_test;
    double u_new, v_new, w_new;
	
	int elemNum = mesh_->get_elemNum(cell_i, cell_j, cell_k);

	//node0 = get_node0(cell_i, cell_j, cell_k);

    //start with a "good" guess for u
	dist = x - mesh_->XORD(cell_i, cell_j, cell_k);
	total = mesh_->XORD(cell_i, cell_j+1, cell_k) - mesh_->XORD(cell_i, cell_j, cell_k);
	u = 2.0*(dist/total) - 1.0;

    //start with a "good" guess for v
	dist = y - mesh_->YORD(cell_i, cell_j, cell_k);
	total = mesh_->YORD(cell_i+1, cell_j, cell_k) - mesh_->YORD(cell_i, cell_j, cell_k);
	v = 2.0*(dist/total) - 1.0;	

    //start with a "good" guess for w, which is just in the middle here
	//zAverageBottom = (mesh_->ZORD(cell_i-1, cell_j-1, cell_k) + mesh_->ZORD(cell_i-1, cell_j, cell_k) + mesh_->ZORD(cell_i, cell_j-1, cell_k) + mesh_->ZORD(cell_i, cell_j, cell_k)) / 4.0;
	//zAverageTop = (mesh_->ZORD(cell_i-1, cell_j-1, cell_k+1) + mesh_->ZORD(cell_i-1, cell_j, cell_k+1) + mesh_->ZORD(cell_i, cell_j-1, cell_k+1) + mesh_->ZORD(cell_i, cell_j, cell_k+1)) / 4.0; 
	//dist = z - zAverageBottom;
	//if(dist < 0.0)
	//	dist = 0.01;
	//total = zAverageTop - zAverageBottom;
	//w = 2.0*(dist/total) - 1.0;
    w = 0.0;

    u_new = u;
    v_new = v;
    w_new = w;

	do{
        u = u_new;
        v = v_new;
        w = w_new;

        //compute test x,y,z based on current u,v,w
		computeJacobianEtc(elemNum, u, v, w, x_test, y_test, z_test);
        
        //compute new u,v,w which may be needed for the next iteration if we're not close enough yet
        u_new = u - (RJACVI[0*3+0]*(x_test-x) + RJACVI[0*3+1]*(y_test-y) + RJACVI[0*3+2]*(z_test-z));
        v_new = v - (RJACVI[1*3+0]*(x_test-x) + RJACVI[1*3+1]*(y_test-y) + RJACVI[1*3+2]*(z_test-z));
        w_new = w - (RJACVI[2*3+0]*(x_test-x) + RJACVI[2*3+1]*(y_test-y) + RJACVI[2*3+2]*(z_test-z));
        
        //compute current error
        xDist = x-x_test;
        yDist = y-y_test;
        zDist = z-z_test;
		dist = std::sqrt((xDist)*(xDist) + (yDist)*(yDist) + (zDist)*(zDist));

	}while((dist/cellsize) > iterativeInterpTol);
	
}

void element::get_xyz(const int &elementNum, const double &u, const double &v, const double &w, double &x, double &y, double &z)
{
	//Function computes (x,y,z)
	node0=mesh_->get_node0(elementNum);  //get the global nodal number of local node 0 of element i

	x=0.0;
    y=0.0;
    z=0.0;

	for(int k=0;k<mesh_->NNPE;k++)          //Start loop over nodes in the element
	{
        NPK=mesh_->get_global_node(k, elementNum);            //NPK is the global node number

        x=x+SFNV(u, v, w, k) * mesh_->XORD(NPK);

        y=y+SFNV(u, v, w, k) * mesh_->YORD(NPK);
                                                      //don't need since none of the coefs are in terms of the x,y,z coordinates like in book
        z=z+SFNV(u, v, w, k) * mesh_->ZORD(NPK);
        
	}                             //End loop over nodes in the element
}
