/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Finite Element Method operations 
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

#ifndef FINITE_ELEMENT_METHOD_H
#define FINITE_ELEMENT_METHOD_H

#include "stability.h"
#include "initialize.h"
#include "preconditioner.h"
#include "volVTK.h"

class FiniteElementMethod
{
    public:
        FiniteElementMethod();
        ~FiniteElementMethod();

        FiniteElementMethod(FiniteElementMethod const& A);
        FiniteElementMethod& operator=(FiniteElementMethod const& A);

        void Initialize(const Mesh &mesh, const WindNinjaInputs &input);
        void DiscretizeTransientTerms();
        void DiscretizeDiffusionTerms(double* SK, double* RHS, int* col_ind, int* row_ptr,
                wn_3dVectorField& U0, double alphaH, wn_3dScalarField& alphaVfield);
        void ComputeGradientField(double *scalar, wn_3dVectorField &U);
        void Deallocate();

    private:
        void CalculateDiffusionRcoefficients(int i, int j, double alphaH, wn_3dScalarField& alphaVfield);
        void CalculateDiffusionHterm(int i, wn_3dVectorField& U0) ;

        std::vector<element> elementArray;
        const Mesh *mesh_; //reference to the mesh
        const WindNinjaInputs *input_; //NOTE: don't use for Com since input.Com is set to NULL in equals operator
        double *DIAG;
};

#endif	//FINITE_ELEMENT_METHOD_H
