/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class that stores 2D arrays for u and v or spd and dir and writes
 *           outputs for those arrays (vectors or grids)
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

#ifndef SURFACE_VECTOR_FIELD_H
#define SURFACE_VECTOR_FIELD_H

//#define SURFACE_VECTOR_FIELD_DEBUG

#include <iostream>
#include <iomanip>
#include <fstream>
	
#include <string>
#include "ascii_grid.h"
#include "shapefil.h"
#include "ninjaMathUtility.h"
#include "Elevation.h"

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Class that stores a 3D vector field of data (x,y,z) over a 2D surface
on a regular Raster Grid (ie. to be used for surface wind velocity
(u,v,w) and such)
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
class surfaceVectorField
{	

public:
	surfaceVectorField(const Elevation *A);
	surfaceVectorField();
	~surfaceVectorField();
	bool write_shapefile(std::string rootName);

	AsciiGrid<double> *poX;	//Grid of X-component data
	AsciiGrid<double> *poY;	//Grid of Y-component data
	AsciiGrid<double> *poZ;	//Grid of Z-component data


private:
	
	std::string ShapeFileName;
	std::string DataBaseName;
	SHPHandle	hSHP;
	DBFHandle	hDBF;


	bool CreateShape();
	void OpenShape();
	void CloseShape();
	void WriteShapePoint(double xpt, double ypt, double spd, long dir, long view_dir, long map_dir);
};



#endif	//SURFACE_WIND_FIELD_H
