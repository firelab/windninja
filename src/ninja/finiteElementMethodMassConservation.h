/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Finite Element Method operations for mass conservation
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

#ifndef FINITE_ELEMENT_METHOD_MASS_CONSERVATION_H
#define FINITE_ELEMENT_METHOD_MASS_CONSERVATION_H

#include "finiteElementMethod.h"

class FiniteElementMethodMassConservation : public FiniteElementMethod
{
    public:

        FiniteElementMethodMassConservation();
        virtual ~FiniteElementMethodMassConservation();

        virtual void SetBoundaryConditions(const Mesh &mesh, WindNinjaInputs &input);
        virtual void SetStability(const Mesh &mesh, WindNinjaInputs &input,
                        wn_3dVectorField &U0,
                        AsciiGrid<double> &CloudGrid,
                        boost::shared_ptr<initialize> &init);
        virtual void ComputeUVWField(const Mesh &mesh, WindNinjaInputs &input,
                            wn_3dVectorField &U0,
                            wn_3dVectorField &U);

        virtual void CalculateRcoefficients(const Mesh &mesh, element &elem, int j);
        virtual void CalculateHterm(const Mesh &mesh, element &elem, wn_3dVectorField &U0, int i) ;

        double alphaH; //alpha horizontal from governing equation, weighting for change in horizontal winds
        wn_3dScalarField alphaVfield; //store spatially varying alphaV variable
    private:

};

#endif	//FINITE_ELEMENT_METHOD_MASS_CONSERVATION_H
