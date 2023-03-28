/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing a 3D field of vectors
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

#include "wn_3dVectorField.h"

wn_3dVectorField::wn_3dVectorField()
{

}


wn_3dVectorField::wn_3dVectorField(const wn_3dScalarField& x, const wn_3dScalarField& y, const wn_3dScalarField& z)
{
    vectorData_x = x;
    vectorData_y = y;
    vectorData_z = z;
}

wn_3dVectorField::wn_3dVectorField(wn_3dVectorField const& f)	// Copy constructor
{
	vectorData_x = f.vectorData_x;
	vectorData_y = f.vectorData_y;
	vectorData_z = f.vectorData_z;
}

wn_3dVectorField::~wn_3dVectorField()
{

}

wn_3dVectorField& wn_3dVectorField::operator= (const wn_3dVectorField& f)	// Assignment operator
{
    if(&f != this){
        vectorData_x = f.vectorData_x;
        vectorData_y = f.vectorData_y;
        vectorData_z = f.vectorData_z;
    }
    return *this;
}

wn_3dVectorField& wn_3dVectorField::operator= (double value)// Assignment operator
{
    for(int k=0;k<vectorData_x.mesh_->nlayers;k++)
    {
        for(int i=0;i<vectorData_x.mesh_->nrows;i++)
        {
            for(int j=0;j<vectorData_x.mesh_->ncols;j++)
            {
                vectorData_x(i,j,k) = value;
                vectorData_y(i,j,k) = value;
                vectorData_z(i,j,k) = value;
            }
        }
    }
}

void wn_3dVectorField::allocate(Mesh const* m)
{
    vectorData_x.allocate(m);
    vectorData_y.allocate(m);
    vectorData_z.allocate(m);
}

void wn_3dVectorField::deallocate()
{
    vectorData_x.deallocate();
    vectorData_y.deallocate();
    vectorData_z.deallocate();
}
/**
 * \brief Copies inlet nodes from another wn_3dVectorField.
 *
 * Copies inlet nodes from one wn_3dVectorField to another.
 *
 * \param f wn_3dVectorField to copy nodes from.
 *
 * \return Void.
 */
void wn_3dVectorField::copyInletNodes(wn_3dVectorField &f)
{
    for(int k=0;k<f.vectorData_x.mesh_->nlayers;k++)
    {
        for(int i=0;i<f.vectorData_x.mesh_->nrows;i++)
        {
            for(int j=0;j<f.vectorData_x.mesh_->ncols;j++)
            {
                if(f.isInlet(i,j,k))
                {
                    vectorData_x(i,j,k) = f.vectorData_x(i,j,k);
                    vectorData_y(i,j,k) = f.vectorData_y(i,j,k);
                    vectorData_z(i,j,k) = 0.0;
                }
            }
        }
    }
}

/**
 * \brief Copies inlet and outlet nodes from another wn_3dVectorField.
 *
 * Copies inlet and outlet nodes from one wn_3dVectorField to another.
 *
 * \param f wn_3dVectorField to copy nodes from.
 *
 * \return Void.
 */
void wn_3dVectorField::copyInletAndOutletNodes(wn_3dVectorField &f)
{
    for(int k=0;k<f.vectorData_x.mesh_->nlayers;k++)
    {
        for(int i=0;i<f.vectorData_x.mesh_->nrows;i++)
        {
            for(int j=0;j<f.vectorData_x.mesh_->ncols;j++)
            {
                if(f.isInlet(i,j,k))
                {
                    vectorData_x(i,j,k) = f.vectorData_x(i,j,k);
                    vectorData_y(i,j,k) = f.vectorData_y(i,j,k);
                    vectorData_z(i,j,k) = 0.0;
                }
                if(f.isOutlet(i,j,k))
                {
                    vectorData_x(i,j,k) = f.vectorData_x(i,j,k);
                    vectorData_y(i,j,k) = f.vectorData_y(i,j,k);
                    vectorData_z(i,j,k) = 0.0;
                }
            }
        }
    }
}

/**
 * \brief Returns maximum value in a wn_3dVectorField.
 *
 * Returns the maximum value in a wn_3dVectorField.
 *
 * \return Maximum value.
 */
double wn_3dVectorField::getMaxValue()
{
    double magnitude;
    double largestMagnitude;
    largestMagnitude = getMagnitude(0,0,0);

    for(int k=0;k<vectorData_x.mesh_->nlayers;k++)
    {
        for(int i=0;i<vectorData_x.mesh_->nrows;i++)
        {
            for(int j=0;j<vectorData_x.mesh_->ncols;j++)
            {
                magnitude = getMagnitude(i,j,k);
                if(magnitude > largestMagnitude)
                {
                    largestMagnitude = magnitude;
                }
            }
        }
    }

    return largestMagnitude;
}

/**
 * \brief Returns minimum value in a wn_3dVectorField.
 *
 * Returns the minimum value in a wn_3dVectorField.
 *
 * \return Minimum value.
 */
double wn_3dVectorField::getMinValue()
{
    double magnitude;
    double smallestMagnitude;
    smallestMagnitude = getMagnitude(0,0,0);

    for(int k=0;k<vectorData_x.mesh_->nlayers;k++)
    {
        for(int i=0;i<vectorData_x.mesh_->nrows;i++)
        {
            for(int j=0;j<vectorData_x.mesh_->ncols;j++)
            {
                magnitude = getMagnitude(i,j,k);
                if(magnitude < smallestMagnitude)
                {
                    smallestMagnitude = magnitude;
                }
            }
        }
    }

    return smallestMagnitude;
}

/**
 * \brief Returns the magnitude of a vector in a wn_3dVectorField.
 *
 * \param i Node index in row (y) direction
 *        j Node index in row (x) direction
 *        k Node index in row (z) direction
 *
 * Returns magnitude of the vector at cell i,j,k in a wn_3dVectorField.
 *
 * \return Vector magnitude.
 */
double wn_3dVectorField::getMagnitude(const int &i, const int &j, const int &k)
{
    return sqrt(vectorData_x(i,j,k)*vectorData_x(i,j,k) +
            vectorData_y(i,j,k)*vectorData_y(i,j,k) +
            vectorData_z(i,j,k)*vectorData_z(i,j,k));
}
   
/**
 * \brief Checks if a node is an outlet node.
 *
 * Checks if a node is an outlet node, which means it is on the outer surface of the mesh and the normal component of the flow goes out of the mesh.
 *
 * \param i Node index in row (y) direction
 *        j Node index in row (x) direction
 *        k Node index in row (z) direction
 *
 * \return True if node is an inlet, false if not.
 */
bool wn_3dVectorField::isOutlet(const int &i, const int &j, const int &k)
{
    if(i==0 && vectorData_y(i,j,k)<0.0)
        return true;
    else if(i==vectorData_y.mesh_->nrows-1 && vectorData_y(i,j,k)>0.0)
        return true;
    else if(j==0 && vectorData_x(i,j,k)<0.0)
        return true;
    else if(j==vectorData_x.mesh_->ncols-1 && vectorData_x(i,j,k)>0.0)
        return true;
    else if(k==vectorData_z.mesh_->nlayers-1 && vectorData_z(i,j,k)>0.0)
        return true;
    else
        return false;
}

/**
 * \brief Checks if a node is an inlet node.
 *
 * Checks if a node is an inlet node, which means it is on the outer surface of the mesh and the normal component of the flow goes into the mesh.
 * We do not check the ground boundary here, because it is more difficult (not flat surface, have to compute normals, etc.) and not necessary
 * where this function is used because we always set the ground surface velocity to zero (no slip boundary).
 *
 * \param i Node index in row (y) direction
 *        j Node index in row (x) direction
 *        k Node index in row (z) direction
 *
 * \return True if node is an inlet, false if not.
 */
bool wn_3dVectorField::isInlet(const int &i, const int &j, const int &k)
{
    if(i==0 && vectorData_y(i,j,k)>=0.0)
    {
        return true;
    }
    else if(i==vectorData_y.mesh_->nrows-1 && vectorData_y(i,j,k)<=0.0)
    {
        return true;
    }
    else if(j==0 && vectorData_x(i,j,k)>=0.0)
    {
        return true;
    }
    else if(j==vectorData_x.mesh_->ncols-1 && vectorData_x(i,j,k)<=0.0)
    {
        return true;
    }
    else if(k==vectorData_z.mesh_->nlayers-1 && vectorData_z(i,j,k)<=0.0)
    {
        return true;
    }
    else
        return false;
}

/**
 * \brief Checks if a node is on the ground.
 *
 * Checks if a node is on the ground.
 *
 * \param i Node index in row (y) direction
 *        j Node index in row (x) direction
 *        k Node index in row (z) direction
 *
 * \return True if node is on the ground, false if not.
 */
bool wn_3dVectorField::isOnGround(const int &i, const int &j, const int &k)
{
    if(k!=0)
        return false;
    else
        return true;
}
