/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Steady state semi-lagrangian solver
 * Author:   Jason Forthofer
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

#ifndef NINJA_SEMI_LAGRANGIAN_STEADY_STATE_INCLUDED_
#define NINJA_SEMI_LAGRANGIAN_STEADY_STATE_INCLUDED_

#include "ninja.h"

#include "assert.h"

#include "stl_create.h"
#include "ninja_conv.h"
#include "ninja_errors.h"
#include "transportSemiLagrangian.h"
#include "wn_3dVectorField.h"

#include "gdal_alg.h"
#include "cpl_spawn.h"

/**
 * \brief Main interface to semi lagrangian solver simulations.
 *
 */
class NinjaSemiLagrangianSteadyState: public ninja
{
public:
    NinjaSemiLagrangianSteadyState();
    virtual ~NinjaSemiLagrangianSteadyState();

    NinjaSemiLagrangianSteadyState( NinjaSemiLagrangianSteadyState const& A );
    NinjaSemiLagrangianSteadyState& operator= ( NinjaSemiLagrangianSteadyState const& A );

    virtual bool simulate_wind();
    inline virtual std::string identify() {return std::string("ninjaSemiLagrangianSteadyState");}

    TransportSemiLagrangian transport;

    int iteration;

    boost::local_time::local_date_time currentTime;//tracks current time as simulation progresses
    boost::posix_time::time_duration currentDt;//current time step size in seconds (can change during simulation)
    boost::posix_time::time_duration currentDt0;//current old time step size in seconds (from last time step)

private:
    /* Output */
    virtual void deleteDynamicMemory();
    void stepForwardOneTimestep();

    FiniteElementMethod conservationOfMassEquation;
    FiniteElementMethod diffusionEquation;
    wn_3dVectorField U00;   //Velocity field from two time steps ago, used sometimes in transient simulations
};

#endif /* NINJA_SEMI_LAGRANGIAN_STEADY_STATE_INCLUDED_ */

