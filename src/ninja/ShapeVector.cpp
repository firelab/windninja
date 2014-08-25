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

#include "ShapeVector.h"

ShapeVector::ShapeVector()
{

}
ShapeVector::~ShapeVector()
{

}

void ShapeVector::setDirGrid(AsciiGrid<double> &d)
{
	dir = d;
}

void ShapeVector::setSpeedGrid(AsciiGrid<double> &s)
{
	spd = s;
}
//#ifdef EMISSIONS
//void ShapeVector::setSpeedGrid(AsciiGrid<double> &dst)
//{
//	emissions = dst;
//}
//#endif
void ShapeVector::setShapeFileName(std::string fileName)
{
	ShapeFileName = fileName;
}

void ShapeVector::setDataBaseName(std::string fileName)
{
	DataBaseName = fileName;
}

bool ShapeVector::makeShapeFiles()
{
	double xC, yC;
	xC = yC = 0;
	double mapDir, qgisDir;
	mapDir = qgisDir = 0;
	if(CreateShape())
		OpenShape();
	else
		return false;

	for(int i = 0;i < spd.get_nRows();i++)
	{
		for(int j = 0;j < spd.get_nCols();j++)
		{
			spd.get_cellPosition(i, j, &xC, &yC);
			mapDir = dir(i,j) + 180.0;
			qgisDir = dir(i,j) + 180.0;

			if(qgisDir > 360.0)
			  qgisDir -= 360.0;

			if(mapDir > 360.0)
				mapDir -= 360.0;

			mapDir -= 90.0;
			if(mapDir < 0.0)
				mapDir += 360.0;

			WriteShapePoint(xC, yC, spd(i,j), (long) (dir(i,j)+0.5), (long) (mapDir+0.5), (long) (qgisDir+0.5));
		}
	}
	CloseShape();

	if(!spd.prjString.empty())
	{
		std::string prjFilename(ShapeFileName);
		int stringPos = prjFilename.find_last_of('.');
		if(stringPos > 0)
			prjFilename.erase(stringPos);
		prjFilename.append(".prj");

		std::ofstream outputFile(prjFilename.c_str(), std::fstream::trunc);
		outputFile << spd.prjString;
		outputFile.close();
	}

	return true;
}


bool ShapeVector::CreateShape()
{
	char 	DataBaseID[64]="";

     hSHP=SHPCreate(ShapeFileName.c_str(), SHPT_POINT);
	if(hSHP==NULL)
		return false;
	SHPClose(hSHP);
	// Create the database.
	hDBF=DBFCreate(DataBaseName.c_str());
	if(hDBF==NULL)
		return false;

     //sprintf(DataBaseID, "%s", "Xcoord");
    	//DBFAddField(hDBF, DataBaseID, FTDouble, 16, 6);
     //sprintf(DataBaseID, "%s", "Ycoord");
    	//DBFAddField(hDBF, DataBaseID, FTDouble, 16, 6);
     sprintf(DataBaseID, "%s", "speed");
    	DBFAddField(hDBF, DataBaseID, FTDouble, 16, 6 );
     sprintf(DataBaseID, "%s", "dir");
    	DBFAddField(hDBF, DataBaseID, FTInteger, 8, 0);
     sprintf(DataBaseID, "%s", "AM_dir");
    	DBFAddField(hDBF, DataBaseID, FTInteger, 8, 0 );
     sprintf(DataBaseID, "%s", "QGIS_dir");
	DBFAddField(hDBF, DataBaseID,  FTInteger, 8, 0);
	DBFClose(hDBF);

     return true;
}

void ShapeVector::OpenShape()
{
/*
	FILE *fx=fopen(ShapeFileName, "w");
     if(fx)
	     fclose(fx);
	fx=fopen(DataBaseName, "w");
     if(fx)
	     fclose(fx);
*/
     hSHP=SHPOpen(ShapeFileName.c_str(), "rb+");
     hDBF=DBFOpen(DataBaseName.c_str(), "rb+");
}


void ShapeVector::CloseShape()
{
	SHPClose(hSHP);
	DBFClose(hDBF);
}


void ShapeVector::WriteShapePoint(double xpt, double ypt, double spd, long dir, long map_dir, long qgis_dir)
{
	long NumRecord;
	double zpt=0;
	SHPObject *pSHP;
	int ret;

	pSHP=SHPCreateObject(SHPT_POINT, -1, 0, NULL, NULL, 1, &xpt, &ypt, &zpt, NULL);
	ret = SHPWriteObject(hSHP, -1, pSHP);
	if(ret == -1)
		throw std::runtime_error("There was a problem writing the shape file");
	SHPDestroyObject(pSHP);

	NumRecord = DBFGetRecordCount(hDBF);
	//DBFWriteDoubleAttribute(hDBF, NumRecord, 0, xpt);
	//DBFWriteDoubleAttribute(hDBF, NumRecord, 1, ypt);
	//DBFWriteIntegerAttribute(hDBF, NumRecord, 2, spd);
	//DBFWriteIntegerAttribute(hDBF, NumRecord, 3, dir);
	DBFWriteDoubleAttribute(hDBF, NumRecord, 0, spd);
	DBFWriteIntegerAttribute(hDBF, NumRecord, 1, dir);
	DBFWriteIntegerAttribute(hDBF, NumRecord, 2, map_dir);
	DBFWriteIntegerAttribute(hDBF, NumRecord, 3, qgis_dir);
}
