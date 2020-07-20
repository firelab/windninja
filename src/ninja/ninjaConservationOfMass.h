/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Conservation of mass solver
 * Author:   Natalie Wagenbrenner
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

#ifndef NINJA_CONSERVATION_OF_MASS_INCLUDED_
#define NINJA_CONSERVATION_OF_MASS_INCLUDED_

#include "ninja.h"

#include "assert.h"

#include "stl_create.h"
#include "ninja_conv.h"
#include "ninja_errors.h"
#include "wn_3dVectorField.h"

#include "gdal_alg.h"
#include "cpl_spawn.h"

/**
 * \brief Main interface to conservation of mass solver simulations.
 *
 */
class NinjaConservationOfMass : public ninja
{
public:
    NinjaConservationOfMass();
    virtual ~NinjaConservationOfMass();

    NinjaConservationOfMass( NinjaConservationOfMass const& A );
    NinjaConservationOfMass& operator= ( NinjaConservationOfMass const& A );

    virtual bool simulate_wind();
    inline virtual std::string identify() {return std::string("ninjaConservationOfMass");}

private:
    virtual void deleteDynamicMemory();
    FiniteElementMethod conservationOfMassEquation;
};

#endif /* NINJA_CONSERVATION_OF_MASS_INCLUDED_ */

