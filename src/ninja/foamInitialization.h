/******************************************************************************
*
* $Id:$
*
* Project:  WindNinja
* Purpose:  Initializing with NinjaFOAM simulations for use with diurnal 
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

#ifndef FOAM_INITIALIZATION_H
#define FOAM_INITIALIZATION_H

#include "initialize.h"
#include "ascii_grid.h"

class foamInitialization : public initialize
{
    public:
        foamInitialization();
        virtual ~foamInitialization();
    
        virtual void initializeFields(WindNinjaInputs &input,
		        Mesh const& mesh,
		        wn_3dScalarField& u0,
		        wn_3dScalarField& v0,
		        wn_3dScalarField& w0,
		        AsciiGrid<double>& cloud,
		        AsciiGrid<double>& L,
		        AsciiGrid<double>& u_star,
		        AsciiGrid<double>& bl_height);
        
        AsciiGrid<double> inputVelocityGrid;
        AsciiGrid<double> inputAngleGrid;

    private:
    
};
#endif /* FOAM_INITIALIZATION_H */
