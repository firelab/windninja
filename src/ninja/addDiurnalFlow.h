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

#ifndef ADD_DIURNAL_H
#define ADD_DIURNAL_H

//#define ADD_DIURNAL_DEBUG
//test

#include <iostream>
#include <iomanip>
#include <fstream>
	
#include <string>
//#include <windows.h>
//#include <process.h>
#include "Elevation.h"
#include "Aspect.h"
#include "Slope.h"
#include "Shade.h"
#include "solar.h"
#include "cellDiurnal.h"
#include "SurfProperties.h"
#include "WindNinjaInputs.h"

#ifdef _OPENMP
#include <omp.h>
#endif

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Class that stores and computes the diurnal component of wind flow for
the whole grid.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
class addDiurnal
{	

public:
	addDiurnal(AsciiGrid<double> *u, AsciiGrid<double> *v, AsciiGrid<double> *w, 
                AsciiGrid<double> *height, AsciiGrid<double> *L, AsciiGrid<double> *U_star, 
                AsciiGrid<double> *BL_height, Elevation const* dem, Aspect const* asp, 
                Slope const* slp, Shade const* shd, Solar *inSolar, surfProperties const* surface, 
                AsciiGrid<double> const* cloudCover, AsciiGrid<double> const* airTemperature, 
                int const number_CPUs, double const downDragCoeff,
                double const downEntrainmentCoeff, double const upDragCoeff, 
                double const upEntrainmentCoeff);
	~addDiurnal();

private:

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif	//ADD_DIURNAL_H
