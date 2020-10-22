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

#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <iostream>
#include <sstream>

class Vector3D {
public:
        Vector3D();

        Vector3D(double x, double y, double z) {
                this->x = x;
                this->y = y;
                this->z = z;
        }

        double get_x() {return x;}
        double get_y() {return y;}
        double get_z() {return z;}

        void setValues(double x, double y, double z) { {
                this->x = x;
                this->y = y;
                this->z = z;
        }

        double dot(const Vector3D& rhs) const {
                return x * rhs.x + y * rhs.y + z * rhs.z;
        }

        Vector3D operator-(const Vector3D& rhs) const {
                return Vector3D(x - rhs.x, y - rhs.y, z - rhs.z);
        }

        Vector3D operator*(double rhs) const {
                return Vector3D(rhs*x, rhs*y, rhs*z);
        }

        intersectPoint(Vector3D rayVector, Vector3D rayPoint, Vector3D planeNormal, Vector3D planePoint);

private:
        double x, y, z;
};

#endif	//VECTOR3D_H
