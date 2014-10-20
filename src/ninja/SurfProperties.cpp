/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing gridded surface parameters (roughness, etc)
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

#include "SurfProperties.h"

surfProperties::surfProperties()
{
	windGridExists = false;
}

//kss
surfProperties::surfProperties(const surfProperties &rhs)
{
  Roughness = rhs.Roughness;
  RoughnessUnits = rhs.RoughnessUnits;
  Rough_d = rhs.Rough_d;
  Rough_dUnits = rhs.Rough_dUnits;
  Rough_h = rhs.Rough_h;
  Rough_hUnits = rhs.Rough_hUnits;
  Albedo = rhs.Albedo;
  Bowen = rhs.Bowen;
  Cg = rhs.Cg;
  Anthropogenic = rhs.Anthropogenic;
  Windspeed = rhs.Windspeed;
  windSpeedGrid = rhs.windSpeedGrid;
  Z = rhs.Z;
  windGridExists = rhs.windGridExists;
}


surfProperties::~surfProperties()
{

}

/**
 * Function used to deallocate (clear) memory of grids.
 */
void surfProperties::deallocate()
{
    Roughness.deallocate();
    Rough_d.deallocate();
    Rough_h.deallocate();
    Albedo.deallocate();
    Bowen.deallocate();
    Cg.deallocate();
    Anthropogenic.deallocate();
    windSpeedGrid.deallocate();
}

bool surfProperties::resample_in_place(double resampleCellSize, AsciiGrid<double>::interpTypeEnum interpType)
{
	Roughness.resample_Grid_in_place(resampleCellSize, interpType);
	Rough_d.resample_Grid_in_place(resampleCellSize, interpType);
	Rough_h.resample_Grid_in_place(resampleCellSize, interpType);
	Albedo.resample_Grid_in_place(resampleCellSize, interpType);
	Bowen.resample_Grid_in_place(resampleCellSize, interpType);
	Cg.resample_Grid_in_place(resampleCellSize, interpType);
	Anthropogenic.resample_Grid_in_place(resampleCellSize, interpType);

	return true;
}

bool surfProperties::BufferGridInPlace( int nAddCols, int nAddRows )
{
	Roughness.BufferGridInPlace(nAddCols, nAddRows);
	Rough_d.BufferGridInPlace(nAddCols, nAddRows);
	Rough_h.BufferGridInPlace(nAddCols, nAddRows);
	Albedo.BufferGridInPlace(nAddCols, nAddRows);
	Bowen.BufferGridInPlace(nAddCols, nAddRows);
	Cg.BufferGridInPlace(nAddCols, nAddRows);
	Anthropogenic.BufferGridInPlace(nAddCols, nAddRows);

	return true;
}

bool surfProperties::set_windspeed(double windspeed)
{
	Windspeed = windspeed;
	windGridExists = false;

	return true;
}

bool surfProperties::set_windspeed(AsciiGrid<double> &speedGrid)
{
	windSpeedGrid = speedGrid;
	windGridExists = true;

	return true;
}

surfProperties &surfProperties::operator=(const surfProperties &rhs)
{
	if(&rhs != this)
	{
	  Roughness = rhs.Roughness;
	  RoughnessUnits = rhs.RoughnessUnits;
	  Rough_d = rhs.Rough_d;
	  Rough_dUnits = rhs.Rough_dUnits;
	  Rough_h = rhs.Rough_h;
	  Rough_hUnits = rhs.Rough_hUnits;
	  Albedo = rhs.Albedo;
	  Bowen = rhs.Bowen;
	  Cg = rhs.Cg;
	  Anthropogenic = rhs.Anthropogenic;
	  Windspeed = rhs.Windspeed;
	  Z = rhs.Z;
	}
	return *this;
}
