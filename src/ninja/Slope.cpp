/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for slope calculations across a spatial grid
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

#include "Slope.h"

Slope::Slope():AsciiGrid<double>()
{
	grid_made = false;
	units = degrees;
	number_CPUs = 1;
}

Slope::Slope(Elevation const* elev, int number_threads):AsciiGrid<double>(elev->get_nCols(), elev->get_nRows(), elev->get_xllCorner(), elev->get_yllCorner(), elev->get_cellSize(), elev->get_noDataValue())
{	
	elevation = elev;
	number_CPUs = number_threads;
	grid_made = false;
	if(!compute_gridSlope())
	{
		#ifdef SLOPE_DEBUG
			std::cout << "Could not make slope grid..." << std::endl;
		#endif		
	}else
	{
		grid_made = true;
		units = degrees;
	}
}

Slope::Slope(Elevation const* elev, int slope_units, int number_threads):AsciiGrid<double>(elev->get_nCols(), elev->get_nRows(), elev->get_xllCorner(), elev->get_yllCorner(), elev->get_cellSize(), elev->get_noDataValue())
{	
	elevation = elev;
	number_CPUs = number_threads;
	grid_made = false;
	if(!compute_gridSlope())
	{
		#ifdef SLOPE_DEBUG
			std::cout << "Could not make slope grid..." << std::endl;
		#endif		
	}else
	{
		grid_made = true;
		set_units(slope_units);
	}
}

Slope::~Slope()
{
	
}

bool Slope::read_slope(std::string filename)
{
	read_Grid(filename);
	grid_made = true;
	set_units(degrees);
	return grid_made;
}

bool Slope::read_slope(std::string filename, int slope_units)
{
	read_Grid(filename);
	grid_made = true;
	set_units(slope_units);
	return grid_made;
}

bool Slope::compute_gridSlope()
{
	double dzdx, dzdy, a, b, c, d, e, f, g, h, i, rise_run;
	int j, k;
	
	if(!grid_made)
	{
		#pragma omp parallel for default(none) private(j,k,a,b,c,d,e,f,g,h,i,dzdx,dzdy,rise_run)
		for(j = 0;j < get_nRows();j++)
		{
			for(k = 0;k < get_nCols();k++)
			{
				if((*elevation)(j,k) != get_noDataValue())
				{
						
					if(!check_inBounds(j + 1,k - 1))
						a = (*elevation)(j,k);
					else
						a = (*elevation)(j + 1, k - 1);
					if(!check_inBounds(j + 1, k))
						b = (*elevation)(j,k);
					else
						b = (*elevation)(j + 1, k);
					if(!check_inBounds(j + 1,k + 1))
						c = (*elevation)(j,k);
					else
						c = (*elevation)(j + 1, k + 1);
					if(!check_inBounds(j,k - 1))
						d = (*elevation)(j,k);
					else
						d = (*elevation)(j, k - 1);
						e = (*elevation)(j,k);
		
					if(!check_inBounds(j,k + 1))
						f = (*elevation)(j,k);
					else
						f = (*elevation)(j, k + 1);
					if(!check_inBounds(j - 1,k - 1))
						g = (*elevation)(j,k);
					else
						g = (*elevation)(j - 1, k - 1);
					if(!check_inBounds(j -1,k))
						h = (*elevation)(j,k);
					else
						h = (*elevation)(j - 1, k);
					if(!check_inBounds(j - 1,k + 1))
						i = (*elevation)(j,k);
					else
						i = (*elevation)(j - 1, k + 1);

					dzdx=compute_celldzdx(a, d, g, c, f, i, e);
					dzdy=compute_celldzdy(a, b, c, g, h, i, e);
					rise_run=std::pow(dzdx * dzdx + dzdy * dzdy, 0.5);
					data(j,k) = (atan(rise_run) * 57.29578);
				}
			}
		}
		
		grid_made = true;
		return true;
	}
	else
	{
		#ifdef SLOPE_DEBUG
			std::cout << "Slope grid has already been made..." << std::endl;
		#endif
        return false;
	}


}

bool Slope::compute_gridSlope(Elevation const* elev, int number_threads)
{
	double dzdx, dzdy, a, b, c, d, e, f, g, h, i, rise_run;
	int j, k;

	elevation = elev;
	number_CPUs = number_threads;
	grid_made = false;

	set_headerData(elevation->get_nCols(), elevation->get_nRows(), elevation->get_xllCorner(), elevation->get_yllCorner(), elevation->get_cellSize(), elevation->get_noDataValue(), 0.0);
	
	#pragma omp parallel for default(none) private(j,k,a,b,c,d,e,f,g,h,i,dzdx,dzdy,rise_run)
	for(j = 0;j < get_nRows();j++)
	{
		for(k = 0;k < get_nCols();k++)
		{
			if((*elevation)(j,k) != get_noDataValue())
			{
				if(!check_inBounds(j + 1,k - 1))
					a = (*elevation)(j,k);
				else
					a = (*elevation)(j + 1, k - 1);
				if(!check_inBounds(j + 1, k))
					b = (*elevation)(j,k);
				else
					b = (*elevation)(j + 1, k);
				if(!check_inBounds(j + 1,k + 1))
					c = (*elevation)(j,k);
				else
					c = (*elevation)(j + 1, k + 1);
				
				if(!check_inBounds(j,k - 1))
					d = (*elevation)(j,k);
				else
					d = (*elevation)(j, k - 1);

				e = (*elevation)(j,k);
	
				if(!check_inBounds(j,k + 1))
					f = (*elevation)(j,k);
				else
					f = (*elevation)(j, k + 1);
				if(!check_inBounds(j - 1,k - 1))
					g = (*elevation)(j,k);
				else
					g = (*elevation)(j - 1, k - 1);
				if(!check_inBounds(j -1,k))
					h = (*elevation)(j,k);
				else
					h = (*elevation)(j - 1, k);
				if(!check_inBounds(j - 1,k + 1))
					i = (*elevation)(j,k);
				else
					i = (*elevation)(j - 1, k + 1);

				dzdx=compute_celldzdx(a, d, g, c, f, i, e);
				dzdy=compute_celldzdy(a, b, c, g, h, i, e);
				rise_run=std::pow(dzdx * dzdx + dzdy * dzdy, 0.5);
				data(j,k) = (atan(rise_run) * 57.29578);
			}
		}
	}
	
	grid_made = true;
	return true;
}

double Slope::compute_celldzdx(double a, double d, double g, double c, double f, double i, double e)
{
     double answer;

     if(a==get_noDataValue()) a=e;
     if(d==get_noDataValue()) d=e;
     if(g==get_noDataValue()) g=e;
     if(c==get_noDataValue()) c=e;
     if(f==get_noDataValue()) f=e;
     if(i==get_noDataValue()) i=e;

     answer=((a + 2.0 * d + g) - (c + 2.0 * f + i)) / (8.0 * get_cellSize());

     return answer;
}

double Slope::compute_celldzdy(double a, double b, double c, double g, double h, double i, double e)
{
     double answer;

     if(a==get_noDataValue()) a = e;
     if(b==get_noDataValue()) b = e;
     if(c==get_noDataValue()) c = e;
     if(g==get_noDataValue()) g = e;
     if(h==get_noDataValue()) h = e;
     if(i==get_noDataValue()) i = e;

     answer=((a + 2.0 * b + c) - (g + 2.0 * h + i)) / (8.0 * get_cellSize());

     return answer;
}
