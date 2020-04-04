/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing a 3D field of vectors
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

#include "wn_3dVectorField.h"

wn_3dVectorField::wn_3dVectorField()
{

}


wn_3dVectorField::wn_3dVectorField(const wn_3dScalarField& x, const wn_3dScalarField& y, const wn_3dScalarField& z)
{
    vectorData_x = x;
    vectorData_y = y;
    vectorData_z = z;
}

wn_3dVectorField::wn_3dVectorField(wn_3dVectorField const& f)	// Copy constructor
{
	vectorData_x = f.vectorData_x;
	vectorData_y = f.vectorData_y;
	vectorData_z = f.vectorData_z;
}

wn_3dVectorField::~wn_3dVectorField()
{

}

wn_3dVectorField& wn_3dVectorField::operator= (const wn_3dVectorField& f)	// Assignment operator
{
    if(&f != this){
        vectorData_x = f.vectorData_x;
        vectorData_y = f.vectorData_y;
        vectorData_z = f.vectorData_z;
    }
    return *this;
}

void wn_3dVectorField::allocate(Mesh const* m)
{
    vectorData_x.allocate(m);
    vectorData_y.allocate(m);
    vectorData_z.allocate(m);
}

void wn_3dVectorField::deallocate()
{
    vectorData_x.deallocate();
    vectorData_y.deallocate();
    vectorData_z.deallocate();
}
