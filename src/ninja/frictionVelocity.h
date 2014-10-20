/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Friction velocity calculations
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

#ifndef FRICTION_VELOCITY_H
#define FRICTION_VELOCITY_H

#include <math.h>

#include "ascii_grid.h"
#include "WindNinjaInputs.h"
#include "mesh.h"
#include "wn_3dScalarField.h"
#include "wn_3dVectorField.h"


class FrictionVelocity{
    public:
        FrictionVelocity();
        ~FrictionVelocity();
        void ComputeVertexNormals(const Mesh &mesh, WindNinjaInputs &input);
        void ComputeUstar(WindNinjaInputs &input,
                          AsciiGrid<double> &grid,
                          wn_3dScalarField &u,
                          wn_3dScalarField &v,
                          wn_3dScalarField &w,
                          const Mesh &mesh,
                          std::string calcMethod = "logProfile");

        AsciiGrid<double> VertexNormalX;  //grids to store vertex normals
        AsciiGrid<double> VertexNormalY;
        AsciiGrid<double> VertexNormalZ;
        AsciiGrid<double> dUdz; //final velocity gradient (dU/dz) in rotated coordinates

    private:
        double *L;
        double *TL;
        double *S;     //arrays for getting dU/dz in rotated coords
        double *SxTL;
        double *LxSxTL;
        double *S2;    //arrays for getting u,v,w in rotated coords
        double *S2xTL;
        double *LxS2xTL;

};


#endif /* FRICTION_VELOCITY_H */
