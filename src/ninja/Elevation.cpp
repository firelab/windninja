/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing elevation data
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

#include "Elevation.h"

Elevation::Elevation():AsciiGrid<double>()
{
	grid_made = false;
	elevationUnits = meters;
}

Elevation::Elevation(std::string filename):AsciiGrid<double>()
{	
	fileName = filename;
	read_Grid(filename);
	grid_made = true;
	elevationUnits = meters;
}

Elevation::Elevation(std::string filename, eElevDistanceUnits elev_units):AsciiGrid<double>()
{
	fileName = filename;
	read_Grid(filename);
	grid_made = true;
	elevationUnits = elev_units;

	if(grid_made)
	{
		if(elevationUnits==feet)	//feet
			operator/=(3.28084);
        else if(elevationUnits!=meters)
			throw std::runtime_error("Problem with units in Elevation::Elevation().");
	}
}

Elevation::Elevation(int nC, int nR, double xL, double yL, double cS, 
		     double nDV, eElevDistanceUnits units) : AsciiGrid<double>(nC, nR, xL, yL, cS, nDV)
{
  elevationUnits = units;
}

Elevation::Elevation(const Elevation &rhs) : AsciiGrid<double>(rhs)
{
  fileName = rhs.fileName;
  grid_made = rhs.grid_made;
  elevationUnits = rhs.elevationUnits;
}

Elevation::~Elevation()
{
	
}

void Elevation::readFromMemory(const double* dem, const int nXSize, const int nYSize,
                              const double* geoRef, std::string prj)
{
    //GDALDriverH hDriver = GDALGetDriverByName( "MEM" );
    GDALDriverH hDriver = GDALGetDriverByName( "GTiff" );
    fileName = "/vsimem/dem.tif"; 

    double *padfScanline;
    padfScanline = new double[nXSize];
    double adfGeoTransform[6];
    CPLErr eErr = CE_None;
    
    hDS = GDALCreate(hDriver, fileName.c_str(), nXSize, nYSize, 1, GDT_Float64, NULL);

    adfGeoTransform[0] = geoRef[0];
    adfGeoTransform[1] = geoRef[1];
    adfGeoTransform[2] = geoRef[2];
    adfGeoTransform[3] = geoRef[3];
    adfGeoTransform[4] = geoRef[4];
    adfGeoTransform[5] = geoRef[5];
    
    char* pszDstWKT = (char*)prj.c_str();
    GDALSetProjection(hDS, pszDstWKT);
    GDALSetGeoTransform(hDS, adfGeoTransform);
    
    GDALRasterBandH hBand = GDALGetRasterBand( hDS, 1 );
    
    GDALSetRasterNoDataValue(hBand, -9999.0);

    for(int i = nYSize - 1; i >= 0; i--)
    {
        for(int j = 0; j < nXSize; j++)
        {
            padfScanline[j] = dem[j + nXSize*(nYSize-i-1)];
        }
        GDALRasterIO(hBand, GF_Write, 0, i, nXSize, 1, padfScanline, nXSize,
                            1, GDT_Float64, 0, 0); 
    }

    GDALReadGrid(fileName, 1);
    grid_made = true;
    elevationUnits = meters;
    this->write_Grid("test_dem.tif",2);
    GDALClose( hDS );
}

void Elevation::read_elevation(std::string filename)
{
	read_Grid(filename);
	grid_made = true;
	fileName = filename;

	elevationUnits = meters;
}


void Elevation::read_elevation(std::string filename, eElevDistanceUnits elev_units)
{
    read_Grid(filename);
    grid_made = true;
    fileName = filename;
    elevationUnits = elev_units;
    if(elevationUnits==feet)	//feet
        operator/=(3.28084);
    else if(elevationUnits!=meters)
        throw std::runtime_error("Problem with units in Elevation::read_elevation().");
}

Elevation &Elevation::operator=(const Elevation &rhs)
{
    if(&rhs != this)
    {
        AsciiGrid<double>::operator=(rhs);

        fileName = rhs.fileName;
        grid_made = rhs.grid_made;
        elevationUnits = rhs.elevationUnits;
    }
    return *this;
}
