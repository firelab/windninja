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

#include "surfaceVectorField.h"

surfaceVectorField::surfaceVectorField()
{}

surfaceVectorField::surfaceVectorField(const Elevation *A)
{
	
	poX = new AsciiGrid<double>(A->get_nCols(), A->get_nRows(), A->get_xllCorner(), A->get_yllCorner(), A->get_cellSize(), A->get_noDataValue(),0.0);
	poY = new AsciiGrid<double>(A->get_nCols(), A->get_nRows(), A->get_xllCorner(), A->get_yllCorner(), A->get_cellSize(), A->get_noDataValue(),0.0);
	poZ = new AsciiGrid<double>(A->get_nCols(), A->get_nRows(), A->get_xllCorner(), A->get_yllCorner(), A->get_cellSize(), A->get_noDataValue(),0.0);
}


surfaceVectorField::~surfaceVectorField()
{
	if(poX)
		delete poX;	
	if(poY)
		delete poY;
	if(poZ)
		delete poZ;
}


bool surfaceVectorField::write_shapefile(std::string rootName)
{
	double xCoord = 0;
	double yCoord = 0;
	double velocity = 0;
	double direction = 0;
	double AMdirection = 0;
	double phi = 0;

	
	if(!poX || !poY || !poZ)
	{
		#ifdef SURFACE_VECTOR_FIELD_DEBUG
			std::cout << "This data has not been created." << std::endl;
		#endif
		return false;
	}
	
	ShapeFileName = rootName;
	ShapeFileName = ShapeFileName + ".shp";
	DataBaseName = rootName;
	DataBaseName = DataBaseName + ".dbf";

	#ifdef SURFACE_VECTOR_FIELD_DEBUG
		std::cout << "Opening file for writing..." << std::endl;
	#endif
	
	CreateShape();
     OpenShape();

	#ifdef SURFACE_VECTOR_FIELD_DEBUG
		std::cout << "File opened." << std::endl;
	#endif
	
	for(int i = 0; i < poX->get_nRows(); i++)
	{
		for (int j = 0; j < poX->get_nCols(); j++)
		{
			poX->get_cellPosition(i, j, &xCoord, &yCoord);
			uvw_to_rThetaPhi((*poX)(i,j), (*poY)(i,j), (*poZ)(i,j), &velocity, &direction, &phi);	//at this point, direction is the direction the wind vector points TO (not from)
			
			AMdirection = direction - 90.0;	//pervert the angle for display in ArcMap
			if(AMdirection < 0.0)
				AMdirection += 360.0;

			direction += 180.0;				//flip direction so it's the direction the wind COMES FROM
			if(direction > 360.0)
				direction -= 360.0;
			

			WriteShapePoint(xCoord, yCoord, velocity, (long)(direction+0.5), (long)(AMdirection + 0.5));	//write shapefile point, add 0.5 because of double to long conversion
		}
	}

	#ifdef SURFACE_VECTOR_FIELD_DEBUG
		std::cout << "File written." << std::endl;
	#endif
	CloseShape();
	#ifdef SURFACE_VECTOR_FIELD_DEBUG
		std::cout << "File closed." << std::endl;
	#endif
	return true;
}


bool surfaceVectorField::CreateShape()
{
	char 	DataBaseID[64]="";

	hSHP=SHPCreate(ShapeFileName.c_str(), SHPT_POINT);
	if(hSHP==NULL)
		return false;
	SHPClose( hSHP );
	// Create the database.
	hDBF=DBFCreate(DataBaseName.c_str());
	if(hDBF==NULL)
		return false;

     //sprintf(DataBaseID, "%s", "Xcoord");
    	//DBFAddField(hDBF, DataBaseID, FTDouble, 16, 6);
     //sprintf(DataBaseID, "%s", "Ycoord");
    	//DBFAddField(hDBF, DataBaseID, FTDouble, 16, 6);
     sprintf(DataBaseID, "%s", "Windspd");
    	DBFAddField(hDBF, DataBaseID, FTDouble, 16, 6 );
     sprintf(DataBaseID, "%s", "Winddir");
    	DBFAddField(hDBF, DataBaseID, FTInteger, 8, 0);
     sprintf(DataBaseID, "%s", "AV_dir");
    	DBFAddField(hDBF, DataBaseID, FTInteger, 8, 0 );
     sprintf(DataBaseID, "%s", "AM_dir");
    	DBFAddField(hDBF, DataBaseID, FTInteger, 8, 0 );
	DBFClose(hDBF);

     return true;
}


void surfaceVectorField::OpenShape()
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


void surfaceVectorField::CloseShape()
{
     SHPClose(hSHP);
   	DBFClose(hDBF);
}


void surfaceVectorField::WriteShapePoint(double xpt, double ypt, double spd, long dir, long map_dir)
{
     long NumRecord;
	double zpt=0;
     SHPObject *pSHP;

     pSHP=SHPCreateObject(SHPT_POINT, -1, 0, NULL, NULL, 1, &xpt, &ypt, &zpt, NULL);
     SHPWriteObject(hSHP, -1, pSHP);
     SHPDestroyObject(pSHP);

	NumRecord = DBFGetRecordCount(hDBF);
//	DBFWriteDoubleAttribute(hDBF, NumRecord, 0, xpt);
//   DBFWriteDoubleAttribute(hDBF, NumRecord, 1, ypt);
//	DBFWriteIntegerAttribute(hDBF, NumRecord, 2, spd);
//   DBFWriteIntegerAttribute(hDBF, NumRecord, 3, dir);
	DBFWriteDoubleAttribute(hDBF, NumRecord, 0, spd);
     DBFWriteIntegerAttribute(hDBF, NumRecord, 1, dir);
     DBFWriteIntegerAttribute(hDBF, NumRecord, 2, map_dir);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
