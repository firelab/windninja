/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  TransportSemiLagrangian
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
#include "transportSemiLagrangian.h"

/**
 * \brief Default constructor of a TransportSemiLagrangian class.
 *
 * Default constructor of a TransportSemiLagrangian class.
 *
 * \param
 *
 * \return
 */
TransportSemiLagrangian::TransportSemiLagrangian()        //default constructor
{
    transportType = firstOrderTransport;
    interpolationType = firstOrderInterpolation;
}

/**
 * \brief Destructor of a TransportSemiLagrangian class.
 *
 * Destructor of a TransportSemiLagrangian class.
 *
 * \param
 *
 * \return
 */
TransportSemiLagrangian::~TransportSemiLagrangian()      //destructor
{

}

/**
 * \brief Transport a vector field one time step, given a vector flow field.
 *
 * Transports the vector field U0 for one time step dt based on the vector flow field U0.
 *
 * \param U0 Both the vector field to transport, and the vector flow field the (vector) quantity is tranported by.
 *        U1 The resulting transported vector field.
 *        dt Time step the field is tranported for.  Should be positive value (this will back trace).
 *
 * \return
 */
void TransportSemiLagrangian::transportVector(const wn_3dVectorField &U0, wn_3dVectorField &U1, double dt)
{
    double xDeparture, yDeparture, zDeparture, reducedDt;
    dt = -dt;   //Set to negative to back trace
    element elem(U0.vectorData_x.mesh_);
    Mesh::eMeshBoundary boundary;
    for(int k=0;k<U0.vectorData_x.mesh_->nlayers;k++)
    {
        for(int i=0;i<U0.vectorData_x.mesh_->nrows;i++)
        {
            for(int j=0;j<U0.vectorData_x.mesh_->ncols;j++)
            {
                if(U0.isOnGround(i,j,k))  //If this node is on the ground, set to zero velocity for no-slip boundary
                {
                    U1.vectorData_x(i, j, k) = 0.0;
                    U1.vectorData_y(i, j, k) = 0.0;
                    U1.vectorData_z(i, j, k) = 0.0;
                }else if(U0.isInlet(i,j,k))   //If this is an inlet boundary node, don't traceParticle, just set to boundary value
                {
                    U1.vectorData_x(i, j, k) = U0.vectorData_x(i,j,k);
                    U1.vectorData_y(i, j, k) = U0.vectorData_y(i,j,k);
                    U1.vectorData_z(i, j, k) = U0.vectorData_z(i,j,k);
                }else   //Else we're on an interior node and should back-trace a particle
                {
                    //reducedDt = dt;
                    //do {
                    //    traceParticle(U0, reducedDt, i, j, k, xDeparture, yDeparture, zDeparture);
                    //    reducedDt /= 2.0;
                    //}
                    //while(!elem.isInMesh(xDeparture, yDeparture, zDeparture));
                    traceParticle(U0, reducedDt, i, j, k, xDeparture, yDeparture, zDeparture);
                    if(!elem.isInMesh(xDeparture, yDeparture, zDeparture)) //COULD SPEED THIS UP A BIT BY STORING THE ELEMENT NUMBER WE FIND HERE AND USE THAT BELOW DURING THE INTERPOLATION
                    {
                        boundary = U0.vectorData_x.mesh_->getNearestMeshBoundaryFromOutsidePoint(xDeparture, yDeparture, zDeparture);
                        //U0.vectorData_x.mesh_->getTraceIntersectionOnBoundary(xDeparture, yDeparture, zDeparture,
                        //                                                    U0.vectorData_x.mesh_->XORD(i,j,k), U0.vectorData_x.mesh_->YORD(i,j,k), U0.vectorData_x.mesh_->ZORD(i,j,k), boundary);


                        //departure = Vector3D::intersectPoint(Vector3D rayVector, Vector3D rayPoint, Vector3D planeNormal, Vector3D planePoint);
                        Vector3D pn;    //Plane normal vector
                        Vector3D pp;    //Point on the plane
                        if(boundary == Mesh::north)
                        {
                            pn.setValues(0.0, 1.0, 0.0);
                            pp.setValues(0.0, U0.vectorData_x.mesh_->get_maxY(), 0.0);
                        }else if(boundary == Mesh::east)
                        {
                            pn.setValues(1.0, 0.0, 0.0);
                            pp.setValues(U0.vectorData_x.mesh_->get_maxX(), 0.0, 0.0);
                        }else if(boundary == Mesh::south)
                        {
                            pn.setValues(0.0, -1.0, 0.0);
                            pp.setValues(0.0, U0.vectorData_x.mesh_->get_minY(), 0.0);
                        }else if(boundary == Mesh::west)
                        {
                            pn.setValues(-1.0, 0.0, 0.0);
                            pp.setValues(U0.vectorData_x.mesh_->get_minX(), 0.0, 0.0);
                        }else if(boundary == Mesh::top)
                        {
                            pn.setValues(0.0, 0.0, 1.0);
                            pp.setValues(0.0, 0.0, U0.vectorData_x.mesh_->domainHeight);
                        }else if(boundary == Mesh::ground)
                        {
                            throw std::runtime_error("Boundary cannot be ground in transportSemiLagrangian::transportVector().");
                        }else
                        {
                            throw std::runtime_error("Cannot determine boundary in transportSemiLagrangian::transportVector().");
                        }

                        Vector3D rv = Vector3D(U0.vectorData_x.mesh_->XORD(i,j,k)-xDeparture, U0.vectorData_x.mesh_->YORD(i,j,k)-yDeparture, U0.vectorData_x.mesh_->ZORD(i,j,k)-zDeparture);    //Ray (line) direction vector
                        Vector3D rp = Vector3D(xDeparture, yDeparture, zDeparture);     //Point along the ray (line)

                        Vector3D departure;
                        departure.intersectPoint(rv, rp, pn, pp);
                        xDeparture = departure.get_x();
                        yDeparture = departure.get_y();
                        zDeparture = departure.get_z();
                    }
                    U1.vectorData_x(i, j, k) = U0.vectorData_x.interpolate(xDeparture, yDeparture, zDeparture);
                    U1.vectorData_y(i, j, k) = U0.vectorData_y.interpolate(xDeparture, yDeparture, zDeparture);
                    U1.vectorData_z(i, j, k) = U0.vectorData_z.interpolate(xDeparture, yDeparture, zDeparture);
                }
            }
        }
    }
}

/**
 * \brief Transport a scalar field one time step, given a vector flow field.
 *
 * Transports the scalar field S0 for one time step dt based on the vector flow field U0.
 *
 * \param U0 The vector flow the (scalar) quantity is tranported by.
 *        S0 The scalar field to tranport.
 *        S1 The resulting transported scalar field.
 *        dt Time step the field is tranported for.  Normally negative for the standard semi-lagrangian method.
 *
 * \return
 */
void TransportSemiLagrangian::transportScalar(const wn_3dVectorField &U0, const wn_3dScalarField &S0, wn_3dScalarField &S1, double dt)
{
    double xEnd, yEnd, zEnd, reducedDt;
    dt = -dt;   //Set to negative to back trace
    for(int k=0;k<U0.vectorData_x.mesh_->nlayers;k++)
    {
        for(int i=0;i<U0.vectorData_x.mesh_->nrows;i++)
        {
            for(int j=0;j<U0.vectorData_x.mesh_->ncols;j++)
            {
                if(U0.isOnGround(i,j,k))  //If this node is on the ground, set to value of S0 (because this is a no-slip boundary, zero velocity)
                {
                    S1(i, j, k) = S0(i,j,k);
                }else if(U0.isInlet(i,j,k))   //If this is an inlet boundary node, don't traceParticle, just set to boundary value
                {
                    S1(i, j, k) = S0(i,j,k);
                }else   //Else we're on an interior node and should back-trace a particle
                {
                    element elem(U0.vectorData_x.mesh_);
                    reducedDt = dt;
                    do {
                        traceParticle(U0, reducedDt, i, j, k, xEnd, yEnd, zEnd);
                        reducedDt /= 2.0;
                    }
                    while(!elem.isInMesh(xEnd, yEnd, zEnd));

                    S1(i, j, k) = S0.interpolate(xEnd, yEnd, zEnd);
                }
            }
        }
    }
}

/**
 * \brief Traces one particle through a flow field for one time step.
 *
 * Traces a particle starting at (startX, startY, startZ) through flow field U0 for one time step dt.  The resulting particle location is (endX, endY, endZ).
 *
 * \param U0 The vector flow field.
 *        dt Time step the particle is tranported for.  Normally negative for the standard semi-lagrangian method.
 *        startX Starting x position of particle.
 *        startY Starting y position of particle.
 *        startZ Starting z position of particle.
 *        endX Ending x position of particle.  Computed by this function (an output).
 *        endY Ending y position of particle.  Computed by this function (an output).
 *        endZ Ending z position of particle.  Computed by this function (an output).
 *
 * \return
 */
void TransportSemiLagrangian::traceParticle(const wn_3dVectorField &U0, const double &dt, int &startI, int &startJ, int &startK, double &endX, double &endY, double &endZ)
{
    double u, v, w;
    u = U0.vectorData_x(startI,startJ,startK);
    v = U0.vectorData_y(startI,startJ,startK);
    w = U0.vectorData_z(startI,startJ,startK);
    if(transportType==firstOrderTransport)
    {
        double directionCosineX, directionCosineY, directionCosineZ, vectorLength, travelDistance;
        vectorLength = sqrt(u*u + v*v + w*w);
        travelDistance = dt*vectorLength;   //Distance travelled by particle in dt
        directionCosineX = u/vectorLength;
        directionCosineY = v/vectorLength;
        directionCosineZ = w/vectorLength;

        double startX = U0.vectorData_x.mesh_->XORD(startI,startJ,startK);
        double startY = U0.vectorData_x.mesh_->YORD(startI,startJ,startK);
        double startZ = U0.vectorData_x.mesh_->ZORD(startI,startJ,startK);

        endX = U0.vectorData_x.mesh_->XORD(startI,startJ,startK) + directionCosineX*travelDistance;
        endY = U0.vectorData_x.mesh_->YORD(startI,startJ,startK) + directionCosineY*travelDistance;
        endZ = U0.vectorData_x.mesh_->ZORD(startI,startJ,startK) + directionCosineZ*travelDistance;
    }else if(transportType==secondOrderRungeKutta)
    {

    }else if(transportType==adaptiveParticleTracer)
    {

    }
}
