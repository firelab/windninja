/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Projection equation operations 
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

#ifndef PROJECTION_EQUATION_H
#define PROJECTION_EQUATION_H

#include "stability.h"
#include "initialize.h"
#include "volVTK.h"
#include "linearAlgebra.h"
#include "finiteElementMethod.h"

class ProjectionEquation
{
    public:
        ProjectionEquation();
        ~ProjectionEquation();

        ProjectionEquation(ProjectionEquation const& A);
        ProjectionEquation& operator=(ProjectionEquation const& A);

        void Initialize(const Mesh &mesh, const WindNinjaInputs &input, wn_3dVectorField &U0);
        void SetupSKCompressedRowStorage();
        void SetBoundaryConditions();
        void SetStability(WindNinjaInputs &input,
                        AsciiGrid<double> &CloudGrid,
                        boost::shared_ptr<initialize> &init);
        void Solve(WindNinjaInputs &input);
        void ComputeUVWField();
        void Discretize();
        void Deallocate();

        double alphaH; //alpha horizontal from governing equation, weighting for change in horizontal winds
        bool stabilityUsingAlphasFlag;
        wn_3dScalarField alphaVfield; //stores spatially varying alphaV variable
        bool writePHIandRHS;
        std::string phiOutFilename;
        std::string rhsOutFilename;
        wn_3dVectorField U;

    private:
        void CalculateRcoefficients(element &elem, int j);
        void CalculateHterm(element &elem, int i) ;
        const Mesh mesh_;
        const WindNinjaInputs input_; //NOTE: don't use for Com since input.Com is set to NULL in equals operator
        wn_3dVectorField U0_;
        FiniteElementMethod fem; //finite element method operations
        LinearAlgebra matrixEquation; //linear algebra operations
        double *PHI;
        double *RHS, *SK;
        int *row_ptr, *col_ind;
        bool *isBoundaryNode;
};

#endif	//PROJECTION_EQUATION_H
