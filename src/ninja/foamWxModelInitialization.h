/******************************************************************************
*
* $Id:$
*
* Project:  WindNinja
* Purpose:  Initializing with wx model NinjaFOAM simulations for use with diurnal 
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

#ifndef FOAM_WX_MODEL_INITIALIZATION_H
#define FOAM_WX_MODEL_INITIALIZATION_H

#include "initialize.h"
#include "ascii_grid.h"

/* 
 * Right now, there is no difference between this class
 * and foamDomainAverageInitialization, but this could change.
 * We could initialize cloud and air from grids, instead of
 * just setting the corresponding inputs to the grid 
 * averages. Also, distiguishing between the various initialization
 * methods allows us to handle things more easily later in the 
 * simulation (e.g, during output file writing)
 *
 * This is a work in progress, not sure if this will stay
 * or go.
 */

class foamWxModelInitialization : public initialize
{
    public:
        foamWxModelInitialization();
        virtual ~foamWxModelInitialization();
    
        virtual void initializeFields(WindNinjaInputs &input,
		        Mesh const& mesh,
		        wn_3dScalarField& u0,
		        wn_3dScalarField& v0,
		        wn_3dScalarField& w0,
		        AsciiGrid<double>& cloud);
        
        AsciiGrid<double> inputVelocityGrid;
        AsciiGrid<double> inputAngleGrid;

    private:
    
        void setWn2dGrids(WindNinjaInputs &input);
        void setInitializationGrids(WindNinjaInputs &input);
};
#endif /* FOAM_WX_MODEL_INITIALIZATION_H */
