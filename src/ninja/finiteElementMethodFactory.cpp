/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Implementation file for Finite Element Method Factory Class
 * Author:   Natalie Wagenbrenner <nwagnebrenner@gmail.com>
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

#include "finiteElementMethodFactory.h"

/**
 * Create a FiniteElementMethod object
 *
 * @param equationType
 *
 * @return a pointer to a FiniteElementMethod object
 */
FiniteElementMethod* FiniteElementMethodFactory::makeFiniteElementMethod(FiniteElementMethod::eEquationType equationType)
{
    if(equationType==FiniteElementMethod::conservationOfMassEquation)
        return new FiniteElementMethodMassConservation;
    //else if(equationType==FiniteElementMethod::diffusionEquation)
    //    return new FiniteElementDiffusionEquation;
    //else if(equationType==FiniteElementMethod::projectionEquation)
    //    return new FiniteElementProjectionEquation;
    else{
        std::ostringstream outString;
        outString << "The finite element method object was set improperly.";
        throw std::runtime_error(outString.str());
    }
}


