/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Explicit lumped-capacitance diffusion 
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

#ifndef EXPLICIT_LUMPED_CAPACITANCE_DIFFUSION_H
#define EXPLICIT_LUMPED_CAPACITANCE_DIFFUSION_H

#include "diffusionEquation.h"

class ExplicitLumpedCapacitanceDiffusion: public virtual DiffusionEquation
{
    public:
        ExplicitLumpedCapacitanceDiffusion();
        ExplicitLumpedCapacitanceDiffusion(const Mesh *mesh, WindNinjaInputs *input);
        ~ExplicitLumpedCapacitanceDiffusion();

        ExplicitLumpedCapacitanceDiffusion(ExplicitLumpedCapacitanceDiffusion const& A);
        ExplicitLumpedCapacitanceDiffusion& operator=(ExplicitLumpedCapacitanceDiffusion const& A);
        virtual ExplicitLumpedCapacitanceDiffusion *Clone() {return new ExplicitLumpedCapacitanceDiffusion(*this);}

        void Initialize();
        void Discretize();
        void Solve(wn_3dVectorField &U1, wn_3dVectorField &U, boost::posix_time::time_duration dt);
        std::string identify() {return std::string("explicitLumpedCapcitanceDiffusion");}
        void Deallocate();

    private:
        double *CL; //lumped capcitence matrix for transient term in discretized diffusion equation
};

#endif //EXPLICIT_LUMPED_CAPACITANCE_DIFFUSION_H
