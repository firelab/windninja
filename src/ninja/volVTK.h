/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for writing volume and surface vtk files
 * Author:   Jason Forthofer <jforthofer@gmail.com>
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

#ifndef VOLVTK_H
#define VOLVTK_H

#include <stdio.h>
#include <string>
	

#include "wn_3dArray.h"
#include "wn_3dScalarField.h"
#include "ninjaException.h"

class volVTK
{
public:

	volVTK();
    volVTK(wn_3dScalarField const& u, wn_3dScalarField const& v, wn_3dScalarField const& w, 
           wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
           int i, int j, int k, std::string filename, std::string vtkWriteFormat);
	~volVTK();

    bool writeVolVTK(wn_3dScalarField const& u, wn_3dScalarField const& v, wn_3dScalarField const& w, 
                     wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
                     int i, int j, int k, std::string filename);
    bool writeMeshVolVTK(wn_3dArray& x, wn_3dArray& y, wn_3dArray& z,
                         int i, int j, int k, std::string filename);
    
    
    bool isBigEndian;
    void determineEndianness();
    
    template <typename T>
    void swapEnd(T& var);
    
    bool writeVolVTK_binary(wn_3dScalarField const& u, wn_3dScalarField const& v, wn_3dScalarField const& w, 
                            wn_3dArray& x, wn_3dArray& y, wn_3dArray& z, 
                            int i, int j, int k, std::string filename);
    bool writeMeshVolVTK_binary(wn_3dArray& x, wn_3dArray& y, wn_3dArray& z,
                                int i, int j, int k, std::string filename);

private:
	
};


#endif	//VOLVTK_H
