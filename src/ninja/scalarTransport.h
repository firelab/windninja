/******************************************************************************
 *
 * $Id:$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing scalar transport info.
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

#ifndef SCALAR_TRANSPORT_H
#define SCALAR_TRANSPORT_H

#include "ascii_grid.h"
#include "WindNinjaInputs.h"
#include "mesh.h"
#include "wn_3dScalarField.h"
#include "wn_3dVectorField.h"

class scalarTransport
{
	public:
		scalarTransport();	  //Default constructor
		~scalarTransport();   // Destructor
		
        wn_3dScalarField Rx, Ry, Rz;    // eddy diffusivities
        double volumeSource; // volume source used as source term (H) in govering equation
        double sourceElemNum; // elemnent number of scalar source (for a point source)
        
        void allocate(Mesh const& mesh);
        void deallocate();
        
        void computeDiffusivity(WindNinjaInputs &input,
                                const Mesh &mesh, 
                                const wn_3dScalarField &u,
                                const wn_3dScalarField &v);


	private:
        wn_3dScalarField heightAboveGround;
        wn_3dScalarField windVelocity;
        wn_3dVectorField velocityGradients;

};



#endif /* SCALAR_TRANPSORT_H */
