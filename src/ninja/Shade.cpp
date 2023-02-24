/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class that calculates shade for a spatial grid
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

#include "Shade.h"

Shade::Shade():AsciiGrid<short>()
{
	grid_made = false;
	number_CPUs = 1;
	smalll = 1.0e-013;
}

Shade::Shade(Elevation const* elev, double Theta, double Phi, int number_threads):AsciiGrid<short>(elev->get_nCols(), elev->get_nRows(), elev->get_xllCorner(), elev->get_yllCorner(), elev->get_cellSize(), elev->get_noDataValue())
{	
	elevation = elev;
	theta = Theta;
	phi = Phi;
	number_CPUs = number_threads;
	grid_made = false;
	smalll = 1.0e-013;
	if(!compute_gridShade())
	{
		#ifdef ASPECT_DEBUG
			std::cout << "Could not make shade grid..." << std::endl;
		#endif		
	}else
	{
		grid_made = true;
	}
}

Shade::~Shade()
{
	
}

bool Shade::read_shade(std::string filename)
{
	read_Grid(filename);
	grid_made = true;
	return grid_made;
}

bool Shade::compute_gridShade()
{	
	if(!grid_made)
	{
		if(phi <= 0.0)	//if sun is below horizon (night time)
		{
			for(int i = 0; i < get_nRows(); i++)
			{
				for(int j = 0; j < get_nCols(); j++)
				{
					data(i,j) = true;		//mark as shaded
				}
			}
			return true;
		}

		// create flag buffer to indicate where we've been
		flagMap = new AsciiGrid<double>(elevation->get_nCols(), elevation->get_nRows(),
		elevation->get_xllCorner(), elevation->get_yllCorner(),
		elevation->get_cellSize(), elevation->get_noDataValue(), 1.0);
		flagMap->set_cellSize(1);
		flagMap->set_xllCorner(0);
		flagMap->set_yllCorner(0);
		// make new elevation grid that is "normalized" to cellsize so that horizontal and vertical units are in "cellsize", this helps indexing computations later
		elevation_norm = new AsciiGrid<double> (*elevation);
		for(int i = 0; i < (elevation_norm->get_nRows()); i++)
		{
			for(int j = 0; j < (elevation_norm->get_nCols()); j++)
			{
				(*elevation_norm)(i,j) = (*elevation_norm)(i,j) / elevation_norm->get_cellSize();
				//elevation_norm->get_cellSize();
			}
		}
		elevation_norm->set_cellSize(1);
		elevation_norm->set_xllCorner(0);
		elevation_norm->set_yllCorner(0);

		double thetaXY;
		//convert theta to an "xy" math type angle to do trig math on
		thetaXY = n_to_xy<double>(theta);

		//compute sun ray "direction cosines" (negatives because this algorithm wants vector from sun to cell, not the other way how we	store theta and phi)
		x_light = -cos(thetaXY*pi/180.0);
		y_light = -sin(thetaXY*pi/180.0);
		z_light = -sin(phi*pi/180.0);

		int *X;			//a pointer, holds the current X position as we travel along the heightmap
		int *Y;			//a pointer, holds the current Y position as we travel along the heightmap
		int iX;			//represents the outer loop variable of our loops
		int iY;			//represents the inner loop variable of our loops
		//diriX;		//if less than 0 then we travel to the left after we	process the current point, else travel right
		//diriY;		//if less than 0 then we travel up after we process the	current point, else travel down

		// calculate absolute values for light direction lightDir
		lightDirXMagnitude = x_light;
		lightDirYMagnitude = y_light;
		if(lightDirXMagnitude < 0) lightDirXMagnitude *= -1;
		if(lightDirYMagnitude < 0) lightDirYMagnitude *= -1;

		// decide which loop will come first, the y loop or x loop
		// based on direction of light, makes calculations faster
		if(lightDirXMagnitude < lightDirYMagnitude)        //move in x direction first, x direction is inner loop
		{
			inner_loop_num = elevation->get_nCols();
			outer_loop_num = elevation->get_nRows();
			Y = &iX;        //outer loop
			X = &iY;      //inner loop
			sizeiY = elevation->get_nCols();      //set inner loop indexing size, ie ncols or nrows
			sizeiX = elevation->get_nRows();      //set outer loop indexing size, ie ncols or nrows

			if(x_light < 0)
			{
				iY = sizeiY-1;
				diriY = -1;
			}
			else
			{
				iY = 0;
				diriY = 1;
			}

			if(y_light < 0)
			{
				iX = sizeiX-1;
				diriX = -1;
			}
			else
			{
				iX = 0;
				diriX = 1;
			}
		}
		else               //move in y direction first, y direction is inner loop
		{
			inner_loop_num = elevation->get_nRows();
			outer_loop_num = elevation->get_nCols();
			Y = &iY;        //inner loop
			X = &iX;      //outer loop
			sizeiY = elevation->get_nRows();      //set inner loop indexing size
			sizeiX = elevation->get_nCols();      //set outer loop indexing size

			if(x_light < 0)
			{
				iX = sizeiX-1;
				diriX = -1;
			}
			else
			{
				iX = 0;
				diriX = 1;
			}

			if(y_light < 0)
			{
				iY = sizeiY-1;
				diriY = -1;
			}
			else
			{
				iY = 0;
				diriY = 1;
			}
		}


		///////////////////////////////////////////start multithreading/////////////////////////////////////
		
		double px;	//position as we track along the ray (px is real x-direction, py is real y-direction)
		double py;	//position as we track along the ray (px is real x-direction, py is real y-direction)
		
		//set number of threads
		//omp_set_num_threads(number_CPUs);

		//  outer loop of computations (inner loop is threaded)
		while(1)  
		{			
				
			////  inner loop (threaded)
			#pragma omp parallel for private(iY,X,Y,px,py)
			for(iY = 0; iY < inner_loop_num; iY++)
			{
				// travel along the terrain until we:
				// (1) intersect another point
				// (2) find another point with previous collision data
				// (3) or reach the edge of the map
				if(lightDirXMagnitude < lightDirYMagnitude)        //move in x direction first, x direction is inner loop
				{				
					Y = &iX;		//outer loop
					X = &iY;		//inner loop
				}else{
					Y = &iY;		//inner loop
					X = &iX;		//outer loop
				}
				px = *X;
				py = *Y;

				// travel along ray to determine if cell is shaded or not
				track_along_ray(px, py, X, Y);

			}

			// update outer loop variable
			if(diriX < 0)
			{
				iX--;
				if(iX < 0)
					break;
			}
			else
			{
				iX++;
				if(iX >= sizeiX)
					break;
			}
		}

		///////////////////////////////////////////end multithreading///////////////////////////////////////
			
		if(elevation_norm)
			delete elevation_norm;
		if(flagMap)
			delete flagMap;
		grid_made = true;
		return true;

	}
	else
	{
		#ifdef SHADE_DEBUG
			std::cout << "Shade grid has already been made..." << std::endl;
		#endif
        return false;
	}
}

bool Shade::compute_gridShade(Elevation const* elev, double Theta, double Phi, int number_threads)
{	
	elevation = elev;
	theta = Theta;
	phi = Phi;
	number_CPUs = number_threads;
	grid_made = false;

	set_headerData(elevation->get_nCols(), elevation->get_nRows(), elevation->get_xllCorner(), elevation->get_yllCorner(), elevation->get_cellSize(), elevation->get_noDataValue(), 0.0);
	
	if(phi <= 0.0)	//if sun is below horizon (night time)
	{
		for(int i = 0; i < get_nRows(); i++)
		{
			for(int j = 0; j < get_nCols(); j++)
			{
				data(i,j) = true;		//mark as shaded
			}
		}
		return true;
	}

	// create flag buffer to indicate where we've been
	flagMap = new AsciiGrid<double>(elevation->get_nCols(), elevation->get_nRows(),
	elevation->get_xllCorner(), elevation->get_yllCorner(),
	elevation->get_cellSize(), elevation->get_noDataValue(), 1.0);
	flagMap->set_cellSize(1);
	flagMap->set_xllCorner(0);
	flagMap->set_yllCorner(0);
	// make new elevation grid that is "normalized" to cellsize so that horizontal and vertical units are in "cellsize", this helps indexing computations later
	elevation_norm = new AsciiGrid<double> (*elevation);
	for(int i = 0; i < (elevation_norm->get_nRows()); i++)
	{
		for(int j = 0; j < (elevation_norm->get_nCols()); j++)
		{
			(*elevation_norm)(i,j) = (*elevation_norm)(i,j) / elevation_norm->get_cellSize();
			//elevation_norm->get_cellSize();
		}
	}
	elevation_norm->set_cellSize(1);
	elevation_norm->set_xllCorner(0);
	elevation_norm->set_yllCorner(0);

	double thetaXY;
	//convert theta to an "xy" math type angle to do trig math on
	thetaXY = n_to_xy(theta);

	//compute sun ray "direction cosines" (negatives because this algorithm wants vector from sun to cell, not the other way how we	store theta and phi)
	x_light = -cos(thetaXY*pi/180.0);
	y_light = -sin(thetaXY*pi/180.0);
	z_light = -sin(phi*pi/180.0);

	int *X;			//a pointer, holds the current X position as we travel along the heightmap
	int *Y;			//a pointer, holds the current Y position as we travel along the heightmap
	int iX;			//represents the outer loop variable of our loops
	int iY;			//represents the inner loop variable of our loops
	//diriX;		//if less than 0 then we travel to the left after we	process the current point, else travel right
	//diriY;		//if less than 0 then we travel up after we process the	current point, else travel down

	// calculate absolute values for light direction lightDir
	lightDirXMagnitude = x_light;
	lightDirYMagnitude = y_light;
	if(lightDirXMagnitude < 0) lightDirXMagnitude *= -1;
	if(lightDirYMagnitude < 0) lightDirYMagnitude *= -1;

	// decide which loop will come first, the y loop or x loop
	// based on direction of light, makes calculations faster
	if(lightDirXMagnitude < lightDirYMagnitude)        //move in x direction first, x direction is inner loop
	{
		inner_loop_num = elevation->get_nCols();
		outer_loop_num = elevation->get_nRows();
		Y = &iX;        //outer loop
		X = &iY;      //inner loop
		sizeiY = elevation->get_nCols();      //set inner loop indexing size, ie ncols or nrows
		sizeiX = elevation->get_nRows();      //set outer loop indexing size, ie ncols or nrows

		if(x_light < 0)
		{
			iY = sizeiY-1;
			diriY = -1;
		}
		else
		{
			iY = 0;
			diriY = 1;
		}

		if(y_light < 0)
		{
			iX = sizeiX-1;
			diriX = -1;
		}
		else
		{
			iX = 0;
			diriX = 1;
		}
	}
	else               //move in y direction first, y direction is inner loop
	{
		inner_loop_num = elevation->get_nRows();
		outer_loop_num = elevation->get_nCols();
		Y = &iY;        //inner loop
		X = &iX;      //outer loop
		sizeiY = elevation->get_nRows();      //set inner loop indexing size
		sizeiX = elevation->get_nCols();      //set outer loop indexing size

		if(x_light < 0)
		{
			iX = sizeiX-1;
			diriX = -1;
		}
		else
		{
			iX = 0;
			diriX = 1;
		}

		if(y_light < 0)
		{
			iY = sizeiY-1;
			diriY = -1;
		}
		else
		{
			iY = 0;
			diriY = 1;
		}
	}


	///////////////////////////////////////////start multithreading/////////////////////////////////////
	
	double px;	//position as we track along the ray (px is real x-direction, py is real y-direction)
	double py;	//position as we track along the ray (px is real x-direction, py is real y-direction)
	
	//set number of threads
	//omp_set_num_threads(number_CPUs);

	//  outer loop of computations (inner loop is threaded)
	while(1)  
	{			
			
		////  inner loop (threaded)
		#pragma omp parallel for private(iY,X,Y,px,py)
		for(iY = 0; iY < inner_loop_num; iY++)
		{
			// travel along the terrain until we:
			// (1) intersect another point
			// (2) find another point with previous collision data
			// (3) or reach the edge of the map
			if(lightDirXMagnitude < lightDirYMagnitude)        //move in x direction first, x direction is inner loop
			{				
				Y = &iX;		//outer loop
				X = &iY;		//inner loop
			}else{
				Y = &iY;		//inner loop
				X = &iX;		//outer loop
			}
			px = *X;
			py = *Y;

			// travel along ray to determine if cell is shaded or not
			track_along_ray(px, py, X, Y);

		}

		// update outer loop variable
		if(diriX < 0)
		{
			iX--;
			if(iX < 0)
				break;
		}
		else
		{
			iX++;
			if(iX >= sizeiX)
				break;
		}
	}

			
	///////////////////////////////////////////end multithreading///////////////////////////////////////
		
	if(elevation_norm)
		delete elevation_norm;
	if(flagMap)
		delete flagMap;
	grid_made = true;
	return true;
}

bool Shade::track_along_ray(double px, double py, int *X, int *Y) //function moves along a path toward the sun to determine if the cell in question is shaded
{
	double interpolatedHeight,interpolatedFlagMap,distance,val;
	double height;		//height of the ray at the current point
        int i = 0;
        int j = 0;
        double t = 0.;
        double u = 0.;
        double val1, val2, val3, val4;

	while(1)
	{
		px -= x_light;  //negative because "x_light" is vector FROM the sun, but we track TOWARD the sun
		py -= y_light;  //negative because "y_light" is vector FROM the sun, but we track TOWARD the sun

		// check if we've reached the boundary, if so, mark as unshaded
		if(px < (0 - smalll) || px >= (elevation_norm->get_nCols() - 1 + smalll) || py < (0 - smalll) || py >= (elevation_norm->get_nRows() -1 + smalll))
		{
			(*flagMap)(*Y,*X) = -1;
			data(*Y,*X) = false; //mark as unshaded
			break;
		}

		// calculate interpolated values

		// get interpolated height and flagMap value (using nearest neighbor interpolation)
		interpolatedHeight = elevation_norm->interpolateGrid(px + 0.5, py + 0.5, AsciiGrid<double>::order1);	//add 0.5 here because interpolateGrid() works on original "cell" based grid and px,py is in node based grid
		//interpolatedFlagMap = flagMap->interpolateGrid(px + 0.5, py + 0.5, AsciiGrid<double>::order0);

                //use a bilinear interpolation for flagMap; 0 order can induce artifacts along the edge of shadows
                //if within outermost two rows or columns use 0 order interpolation 
                if(px + 0.5 >= (flagMap->get_xllCorner() + (flagMap->get_xDimension() - 3*(flagMap->get_cellSize() / 2)))
                    || px + 0.5 <= flagMap->get_xllCorner() + 3*(flagMap->get_cellSize() / 2)
                    || py + 0.5 >= (flagMap->get_yllCorner() + (flagMap->get_yDimension() - 3*(flagMap->get_cellSize() / 2)))
                    || py + 0.5 <= flagMap->get_yllCorner() + 3*(flagMap->get_cellSize() / 2))
                {
                    //just do a 0 order interpolation
                    interpolatedFlagMap = flagMap->interpolateGrid(px + 0.5, py + 0.5, AsciiGrid<double>::order0);
                }
                else
                {
                    flagMap->get_cellIndex((px + 0.5 - flagMap->get_cellSize() / 2), (py + 0.5 - flagMap->get_cellSize() / 2), &i, &j);
                    t = (py + 0.5 - ((i * flagMap->get_cellSize() + (flagMap->get_cellSize() / 2)) + flagMap->get_yllCorner())) /

                        ((((i + 1) * flagMap->get_cellSize() + (flagMap->get_cellSize() / 2)) + flagMap->get_yllCorner()) -

                        (((i * flagMap->get_cellSize() + (flagMap->get_cellSize() / 2))) + flagMap->get_yllCorner()));

                    u = (px + 0.5 - ((j * flagMap->get_cellSize() + (flagMap->get_cellSize() / 2)) + flagMap->get_xllCorner())) /

                        ((((j + 1) * flagMap->get_cellSize() + (flagMap->get_cellSize() / 2)) + flagMap->get_xllCorner()) -

                        (((j * flagMap->get_cellSize() + (flagMap->get_cellSize() / 2))) + flagMap->get_xllCorner()));

                    //check x_light and y_light to see which direction to move for the interpolation
                    if(x_light < 0. && y_light < 0.)
                    {
                        val1 = flagMap->get_cellValue(i, j);
                        val2 = flagMap->get_cellValue(i + 1, j);
                        val3 = flagMap->get_cellValue(i + 1, j + 1);
                        val4 = flagMap->get_cellValue(i, j + 1);
                    }
                    else if(x_light > 0. && y_light < 0.)
                    {
                        val1 = flagMap->get_cellValue(i, j);
                        val2 = flagMap->get_cellValue(i + 1, j);
                        val3 = flagMap->get_cellValue(i + 1, j - 1);
                        val4 = flagMap->get_cellValue(i, j - 1);
                    }
                    else if(x_light > 0. && y_light > 0.)
                    {
                        val1 = flagMap->get_cellValue(i, j);
                        val2 = flagMap->get_cellValue(i - 1, j);
                        val3 = flagMap->get_cellValue(i - 1, j - 1);
                        val4 = flagMap->get_cellValue(i, j - 1);
                    }
                    else if(x_light < 0. && y_light > 0.)
                    {
                        val1 = flagMap->get_cellValue(i, j);
                        val2 = flagMap->get_cellValue(i - 1, j);
                        val3 = flagMap->get_cellValue(i - 1, j + 1);
                        val4 = flagMap->get_cellValue(i, j + 1);
                    }
                    if(val1==flagMap->get_NoDataValue() || val2==flagMap->get_NoDataValue() || val3==flagMap->get_NoDataValue() || val4==flagMap->get_NoDataValue())
                    {
                        //just do a 0 order interpolation
                        interpolatedFlagMap = flagMap->interpolateGrid(px + 0.5, py + 0.5, AsciiGrid<double>::order0);
                    }
                    if(val1 <= 0. || val2 <= 0. || val3 <= 0. || val4 <= 0.)
                    {
                        //just do a 0 order interpolation
                        interpolatedFlagMap = flagMap->interpolateGrid(px + 0.5, py + 0.5, AsciiGrid<double>::order0);
                    }
                    else
                    {
                        //replace -1 flags with 0 for interpolation of the shadow height
                        if(val1 == -1.0f)
                            val1 = 0.0;
                        if(val2 == -1.0f)
                            val2 = 0.0;
                        if(val3 == -1.0f)
                            val3 = 0.0;
                        if(val4 == -1.0f)
                            val4 = 0.0;
                        interpolatedFlagMap = (1 - t) * (1 - u) * val1
                                                 + t * (1 - u) * val2
                                                 + t * u * val3
                                                 + (1 - t) * u * val4;
                    }
                }

                //if a 0 order interpolation gave us -1, set to 0
                if(interpolatedFlagMap < 0.)
                    interpolatedFlagMap = 0.;

		// get distance from original point to current point
		distance = std::sqrt((px - *X)*(px - *X) + (py - *Y)*(py - *Y));

		// get height of light ray at current point while traveling along light ray
		height = (*elevation_norm)(*Y,*X) + tan(phi*pi/180.0)*distance;

		// check intersection with either terrain or flagMap
		val = interpolatedHeight + interpolatedFlagMap;

		if(height < val)        //then it is shaded
		{
			(*flagMap)(*Y,*X) = val - height;
			data(*Y,*X) = true;  //mark as shaded

			break;
		}	
		// check if pixel we've moved to is unshadowed
		if((interpolatedFlagMap-0.) < smalll)
		{
			(*flagMap)(*Y,*X) = -1.0f;
			data(*Y, *X) = false; //mark as unshaded
			break;
		}
	}

	return true;
}

