/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for writing esri shapefiles given two grids
 * Author:   Kyle Shannon <ksshannon@gmail.com>
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

#ifndef SHAPEVECTOR_H
#define SHAPEVECTOR_H

#include "ascii_grid.h"
#include "shapefil.h"
#include "ninjaException.h"

class ShapeVector
{

public:
	ShapeVector();
	~ShapeVector();

	AsciiGrid<double> spd, dir;
	//#ifdef EMISSIONS
	//AsciiGrid<double> emisisons;
	//#endif

	inline void setResolution(double r){resolution = r;}

	void setSpeedGrid(AsciiGrid<double> &s);
	void setDirGrid(AsciiGrid<double> &d);
	//#ifdef EMISSIONS
	//void setDustGrid(AsciiGrid<double> &dst);
	//#endif

	void setShapeFileName(std::string fileName);
	void setDataBaseName(std::string fileName);

	bool makeShapeFiles();

private:

	SHPHandle	hSHP;
	DBFHandle	hDBF;

	double resolution;

	std::string ShapeFileName;
	std::string DataBaseName;

	bool CreateShape();
	void OpenShape();

	void WriteShapePoint(double xpt, double ypt, double spd, long dir, long map_dir, long qgis_dir);

	void CloseShape();
};
#endif	//SHAPEVECTOR_H
