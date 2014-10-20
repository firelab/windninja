/******************************************************************************
 *
 * $Id:
 *
 * Project:  WindNinja
 * Purpose:  Class for calculating stability parameters
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

#ifndef STABILITY_H
#define STABILITY_H

#include <math.h>

#include "ascii_grid.h"
#include "WindNinjaInputs.h"
#include "mesh.h"
#include "wn_3dScalarField.h"
#include "wn_3dVectorField.h"
#include "solar.h"
#include "cellDiurnal.h"

class Stability{
    public:
        Stability();
        ~Stability();
        Stability(WindNinjaInputs &input);
        Stability& operator= (const Stability &rhs);

        void SetDomainAverageAlpha(WindNinjaInputs &input,
                                   const Mesh &mesh);
        void SetPointInitializationAlpha(WindNinjaInputs &input,
                                         const Mesh &mesh);
        void Set3dVariableAlpha(WindNinjaInputs &input,
                                const Mesh &mesh,
                                wn_3dScalarField &theta,
                                const wn_3dScalarField &u0,
                                const wn_3dScalarField &v0);
        void Set2dWxInitializationAlpha(WindNinjaInputs &input,
                                        const Mesh &mesh,
                                        const AsciiGrid<double> &cloud);
        double strouhalNumber;
        wn_3dScalarField alphaField;
        
        
    private:
        void SetAlphaField(const Mesh &mesh);
    
        AsciiGrid<double> QswGrid;
        AsciiGrid<double> cloudCoverGrid;
        AsciiGrid<double> speedGrid;
        std::string stabilityClass; //pasquill stability class
        
        double _N; // Brunt-Vaisala frequency
        AsciiGrid<double> _H; // characteristic height difference
        double _U; // characteristic velocity
        double _t; // buoyancy time scale
        double thetaDerivatives; // potential temperature gradient
        
        double  _g;
        double _c; //scaling parameter for _H

};

#endif /* STABILITY_H */
