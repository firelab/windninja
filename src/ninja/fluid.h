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

	double get_t();
	double get_cSubP(double);
	double get_rho(double);
	double get_mu(double);
	double get_v(double);
	double get_k(double);
	double get_alpha(double);
	double get_pr(double);



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
};

#endif /* FLUID_H */
