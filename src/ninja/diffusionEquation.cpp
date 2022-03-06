/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Diffusion equation operations
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
#include "diffusionEquation.h"

DiffusionEquation::DiffusionEquation()
{
    //Pointers to dynamically allocated memory
    PHI=NULL;
    xRHS=NULL;
    yRHS=NULL;
    zRHS=NULL;
    writePHIandRHS=false;
    phiOutFilename="!set";
    rhsOutFilename="!set";
}

/**
 * Copy constructor.
 * @param A Copied value.
 */

DiffusionEquation::DiffusionEquation(DiffusionEquation const& A)
{
    PHI=A.PHI;
    xRHS=A.xRHS;
    yRHS=A.yRHS;
    zRHS=A.zRHS;
    writePHIandRHS=A.writePHIandRHS;
    phiOutFilename=A.phiOutFilename;
    rhsOutFilename=A.rhsOutFilename;
}

/**
 * Equals operator.
 * @param A Value to set equal to.
 * @return a copy of an object
 */

DiffusionEquation& DiffusionEquation::operator=(DiffusionEquation const& A)
{
    if(&A != this) {
        PHI=A.PHI;
        xRHS=A.xRHS;
        yRHS=A.yRHS;
        zRHS=A.zRHS;
        writePHIandRHS=A.writePHIandRHS;
        phiOutFilename=A.phiOutFilename;
        rhsOutFilename=A.rhsOutFilename;
    }
    return *this;
}

DiffusionEquation::~DiffusionEquation()      //destructor
{

}

