/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Calculate diurnal wind for a spatial grid
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

#include "addDiurnalFlow.h"

addDiurnal::addDiurnal(AsciiGrid<double> *u, AsciiGrid<double> *v, AsciiGrid<double> *w, 
                    AsciiGrid<double> *height, AsciiGrid<double> *L, AsciiGrid<double> *U_star, 
                    AsciiGrid<double> *BL_height, Elevation const* dem, Aspect const* asp, Slope
                    const* slp, Shade const* shd, Solar *inSolar, surfProperties const* 
                    surface, AsciiGrid<double> const* cloudCover, AsciiGrid<double> const* 
                    airTemperature, int const number_CPUs, double const downDragCoeff,
                    double const downEntrainmentCoeff, double const upDragCoeff, 
                    double const upEntrainmentCoeff)
{
	
	cellDiurnal cDiurnal(dem, shd, inSolar, downDragCoeff, downEntrainmentCoeff, upDragCoeff, upEntrainmentCoeff);

	double u_, v_, w_, height_, L_, U_star_, BL_height_, Xord, Yord, WindSpeed, Z;
	int i,j;

	// DO THE WORK
	#pragma omp parallel for default(none) private(i,j,u_,v_,w_,height_,L_,U_star_,BL_height_, Xord, Yord, WindSpeed, Z) firstprivate(cDiurnal) shared(dem,u,v,w,height,L,U_star,BL_height, airTemperature, asp, cloudCover, slp, surface)
    for(i = 0; i < dem->get_nRows(); i++)
	{
		for(j = 0; j < dem->get_nCols(); j++)
		{
			//if simulation has been initialized with a surface wind field (NDFD etc.)
			if(surface->windGridExists == true)
				WindSpeed = surface->windSpeedGrid(i,j);
			else
				WindSpeed = surface->Windspeed;
			Z = surface->Z;

			dem->get_cellPosition(i, j, &Xord, &Yord);

			double airTemp = (*airTemperature)(i,j);
			cDiurnal.initialize(Xord, Yord, (*asp)(i,j),(*slp)(i,j), (*cloudCover)(i,j), airTemp, WindSpeed, Z, (*surface).Albedo(i,j), (*surface).Bowen(i,j), (*surface).Cg(i,j), (*surface).Anthropogenic(i,j), (*surface).Roughness(i,j), (*surface).Rough_h(i,j), (*surface).Rough_d(i,j));
			cDiurnal.compute_cell_diurnal_wind(i, j, &u_, &v_, &w_, &height_, &L_, &U_star_, &BL_height_);
			
			u->set_cellValue(i, j, u_);
		    v->set_cellValue(i, j, v_);
		    w->set_cellValue(i, j, w_);
			height->set_cellValue(i, j, height_);
			L->set_cellValue(i, j, L_);
			U_star->set_cellValue(i, j, U_star_);
			BL_height->set_cellValue(i, j, BL_height_);
		}
	}
}

addDiurnal::~addDiurnal()
{
	
}
