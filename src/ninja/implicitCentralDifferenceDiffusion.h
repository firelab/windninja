/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Implicit central difference diffusion 
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

#ifndef IMPLICIT_CENTRAL_DIFFERENCE_DIFFUSION_H
#define IMPLICIT_CENTRAL_DIFFERENCE_DIFFUSION_H

#include "diffusionEquation.h"

class ImplicitCentralDifferenceDiffusion: public virtual DiffusionEquation
{
    public:
        ImplicitCentralDifferenceDiffusion();
        ~ImplicitCentralDifferenceDiffusion();

        ImplicitCentralDifferenceDiffusion(ImplicitCentralDifferenceDiffusion const& A);
        ImplicitCentralDifferenceDiffusion& operator=(ImplicitCentralDifferenceDiffusion const& A);
        virtual ImplicitCentralDifferenceDiffusion *Clone() {return new ImplicitCentralDifferenceDiffusion(*this);}

        void Initialize(const Mesh *mesh, WindNinjaInputs *input); //pure virtual
        void SetBoundaryConditions();
        void Discretize();
        void Solve(wn_3dVectorField &U1, wn_3dVectorField &U, boost::posix_time::time_duration dt);
        void SetupSKCompressedRowStorage();
        std::string identify() {return std::string("implicitCentralDifferenceDiffusion");}
        void Deallocate();

    private:
        double *SK; //The A in Ax=b
        int *row_ptr, *col_ind;
        bool *isBoundaryNode;
        LinearAlgebra matrixEquation;
};

#endif //IMPLICIT_CENTRAL_DIFFERENCE_DIFFUSION_H
