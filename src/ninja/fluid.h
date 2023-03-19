/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing fluid properties
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
#ifndef FLUID_H
#define FLUID_H

#include <string>
#include <iostream>
	
#include <fstream>
//#include "debuggin.h"
class Fluid
{
public:
	Fluid();
	Fluid(std::string inputFile);
	virtual ~Fluid();
	bool read_fluidProperties(std::string inputFile);

	inline double get_t();
	inline double get_cSubP(double);
	inline double get_rho(double);
	inline double get_mu(double);
	inline double get_v(double);
	inline double get_k(double);
	inline double get_alpha(double);
	inline double get_pr(double);



	bool print_name();
	bool print_t();
	bool print_cSubP();
	bool print_rho();
	bool print_mu();
	bool print_v();
	bool print_k();
	bool print_alpha();
	bool print_pr();
	bool print_table();

	std::string name;
	int nRows;
	double *t;
	double *rho;
	double *cSubP;
	double *mu;
	double *v;
	double *k;
	double *alpha;
	double *pr;

private:
	double interpolate (double T, double*v);
};

inline double Fluid::get_rho(double T) { return interpolate(T, rho); }
inline double Fluid::get_cSubP(double T) { return interpolate(T, cSubP); }
inline double Fluid::get_v(double T) { return interpolate(T, v); }
inline double Fluid::get_k(double T) { return interpolate(T, k); }
inline double Fluid::get_alpha(double T) { return interpolate(T, alpha); }
inline double Fluid::get_pr(double T) { return interpolate(T, pr); }

#endif /* FLUID_H */
