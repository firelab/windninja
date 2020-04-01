/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing a 3D field of scalars (linked with a mesh for
 *               spatial information)
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

#ifndef WN_3D_SCALAR_FIELD_H
#define WN_3D_SCALAR_FIELD_H

#include "ninjaException.h"
#include "wn_3dArray.h"
#include "mesh.h"
#include "element.h"
#include "WindNinjaInputs.h"

class wxModelInitialization;
class wn_3dScalarField
{
public:
    wn_3dScalarField();
    wn_3dScalarField(Mesh const* m);
    wn_3dScalarField(wn_3dScalarField const* x);
    ~wn_3dScalarField();

    wn_3dScalarField(wn_3dScalarField const& f);
    wn_3dScalarField& operator= (wn_3dScalarField const& f);

    void allocate(Mesh const* m);
    void allocate();
    void deallocate();

    void interpolateScalarData(wn_3dScalarField &newScalarData,
                               Mesh const& mesh,
                               WindNinjaInputs const& input);
                               
    double interpolate(double const& x,double const& y, double const& z);
    double interpolate(element &elem, const int &cell_i, const int &cell_j, const int &cell_k, const double &u, const double &v, const double &w);
    void ComputeGradient(WindNinjaInputs &input, wn_3dScalarField &gradientVectorXComponent, wn_3dScalarField &gradientVectorYComponent, wn_3dScalarField &gradientVectorZComponent);

    double& operator() (int row, int col, int layer);
    double  operator() (int row, int col, int layer) const;
    double& operator() (int num);
    double  operator() (int num) const;
    Mesh const* mesh_;

private:
    wn_3dArray scalarData_;
};

#endif /* WN_3D_SCALAR_FIELD_H */

