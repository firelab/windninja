/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Abstract base class for initializing WindNinja wind fields
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

#ifndef INITIALIZE_H
#define INITIALIZE_H

#include "ninjaMathUtility.h"
#include "ninjaException.h"
#include "WindNinjaInputs.h"
#include "mesh.h"
#include "wxStation.h"
#include "windProfile.h"
#include "wn_3dScalarField.h"
#include <vector>
#include "cellDiurnal.h"
#include "SurfProperties.h"

namespace blt = boost::local_time;

class initialize
{
    public:
        initialize();
        virtual ~initialize(); 
        
        //Pure virtual function for initializing volume wind fields.
        virtual void initializeFields(WindNinjaInputs &input,
                        Mesh const& mesh,
                        wn_3dScalarField& u0,
                        wn_3dScalarField& v0,
                        wn_3dScalarField& w0,
                        AsciiGrid<double>& cloud) = 0;
#ifdef NINJAFOAM
        virtual void ninjaFoamInitializeFields( WindNinjaInputs &input,
                                                AsciiGrid<double> &cloud ){};
#endif //NINJAFOAM

        /*TODO: refactor so these aren't accessed directly in ninja */
        virtual std::string  getForecastIdentifier(){};
        virtual std::vector<blt::local_date_time> getTimeList(blt::time_zone_ptr timeZonePtr){};
        wn_3dScalarField air3d; //perturbation potential temperature
        std::vector<double> u10List;
        std::vector<double> v10List;
        std::vector<double> u_wxList;
        std::vector<double> v_wxList;
        std::vector<double> w_wxList;

        AsciiGrid<double> L;		//Monin-Obukhov length
        AsciiGrid<double> bl_height;	//atmospheric boundary layer height

    protected:
	void addDiurnal(WindNinjaInputs& input, Aspect const* asp,
                    Slope const* slp, Shade const* shd, Solar *inSolar);

        void initializeWindToZero(Mesh const& mesh,
                                wn_3dScalarField& u0,
                                wn_3dScalarField& v0,
                                wn_3dScalarField& w0);

        void initializeWindFromProfile(WindNinjaInputs &input,
                                const Mesh& mesh,
                                wn_3dScalarField& u0,
                                wn_3dScalarField& v0,
                                wn_3dScalarField& w0);

        virtual void initializeBoundaryLayer(WindNinjaInputs& input);

        void addDiurnalComponent(WindNinjaInputs &input,
                                const Mesh& mesh,
                                wn_3dScalarField& u0,
                                wn_3dScalarField& v0,
                                wn_3dScalarField& w0);

        void setCloudCover(WindNinjaInputs &input);

        void setUniformCloudCover(WindNinjaInputs &input,
                                    AsciiGrid<double> cloud);

        void setGridHeaderData(WindNinjaInputs& input, AsciiGrid<double>& cloud);

        AsciiGrid<double> u_star;	//Friction velocity

        AsciiGrid<double> height;	//height of diurnal flow above "z=0" in log profile
        AsciiGrid<double> uDiurnal;
        AsciiGrid<double> vDiurnal;
        AsciiGrid<double> wDiurnal;
        AsciiGrid<double> uInitializationGrid;
        AsciiGrid<double> vInitializationGrid;
        AsciiGrid<double> airTempGrid;
        AsciiGrid<double> cloudCoverGrid;
        AsciiGrid<double> speedInitializationGrid;
        AsciiGrid<double> dirInitializationGrid;

        windProfile profile;

    private:
 

};

#endif /* INITIALIZE_H */
