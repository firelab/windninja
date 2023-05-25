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

#include "fluid.h"

Fluid::Fluid()
{
}

Fluid::~Fluid()
{
     if(t)
          delete[] t;
     if(rho)
          delete[] rho;
     if(cSubP)
          delete[] cSubP;
     if(mu)
          delete[] mu;
     if(v)
          delete[] v;
     if(k)
          delete[] k;
     if(alpha)
          delete[] alpha;
	 if(pr)
          delete[] pr;
}

bool Fluid::read_fluidProperties(std::string inputFile)
{
	std::fstream fin;
	fin.open(inputFile.c_str(),std::ios::in);
	char testString[32] = "";
	char krap[32] = "";
	if(!fin)
	{
		std::cout << "Cannot open *.prp file.";
		fin.close();
		return false;
	}
	else
	{
		fin >> name >> krap >> nRows >> testString >> krap;
		t = new double[nRows];
		for(int i = 0;i < nRows;i++)
		{
			fin >> t[i];
		}
		fin >> krap >> krap;
		rho = new double[nRows];
		for(int i = 0;i < nRows;i++)
		{
			fin >> rho[i];
		}
		fin >> krap >> krap;
		cSubP = new double[nRows];
		for(int i = 0;i < nRows;i++)
		{
			fin >> cSubP[i];
		}
		fin >> krap >> krap;
		mu = new double[nRows];
		for(int i = 0;i < nRows;i++)
		{
			fin >> mu[i];
		}
		fin >> krap >> krap;
		v = new double[nRows];
		for(int i = 0;i < nRows;i++)
		{
			fin >> v[i];
		}
		fin >> krap >> krap;
		k = new double[nRows];
		for(int i = 0;i < nRows;i++)
		{
			fin >> k[i];
		}
		fin >> krap >> krap;
		alpha = new double[nRows];
		for(int i = 0;i < nRows;i++)
		{
			fin >> alpha[i];
		}
		fin >> krap >> krap;
		pr = new double[nRows];
		for(int i = 0;i < nRows;i++)
		{
			fin >> pr[i];
		}
		fin.close();

		return true;
	}
}


double Fluid::interpolate(double T, double* vs)
{
	int rmax = nRows-1;

    if (T < t[0] || T > t[rmax]) throw std::runtime_error("Invalid temperature.");

	if (T == t[0]) {
		return vs[0];

	} else {
		double ti1 = t[0];

		for (int i=1; i<nRows; i++) {
			double ti = t[i];

			if (T < ti) {
				double dt = T - ti1;
				double dti = ti - ti1;
				double vi1 = vs[i-1];
				double dv = vs[i] - vi1;
				return vi1 + (dt / dti) * dv;

			} else if (T == ti) {
				return vs[i];
			}
			ti1 = ti;
		}

		throw std::runtime_error("can't get here");
	}
}


/*
double Fluid::get_rho(double T)
{
	if(T > t[nRows - 1])
	{
		std::cout << "Invalid temperature.";
		return -1;
	}
	else
	{
		int i = 0;
		while(T > t[i])
			i++;

		return (( (T - t[i - 1]) / (t[i] - t[i - 1])) * (rho[i] - rho[i - 1])) + rho[i - 1];
	}
}
*/

bool Fluid::print_t()
{
	std::cout << std::endl << "t" << std::endl;
	for(int i = 0;i < nRows;i++)
	{
		std::cout << t[i] << std::endl;
	}
	return true;
}
bool Fluid::print_rho()
{
	std::cout << std::endl << "rho" << std::endl;
	for(int i = 0;i < nRows;i++)
	{
		std::cout << rho[i] << std::endl;
	}
	return true;
}
bool Fluid::print_cSubP()
{
	std::cout << std::endl << "cSubP" << std::endl;
	for(int i = 0;i < nRows;i++)
	{
		std::cout << cSubP[i] << std::endl;
	}
	return true;
}
bool Fluid::print_mu()
{
	std::cout << std::endl << "mu" << std::endl;
	for(int i = 0;i < nRows;i++)
	{
		std::cout << mu[i] << std::endl;
	}
	return true;
}
bool Fluid::print_v()
{
	std::cout << std::endl << "v" << std::endl;
	for(int i = 0;i < nRows;i++)
	{
		std::cout << v[i] << std::endl;
	}
	return true;
}
bool Fluid::print_k()
{
	std::cout << std::endl << "k" << std::endl;
	for(int i = 0;i < nRows;i++)
	{
		std::cout << k[i] << std::endl;
	}
	return true;
}
bool Fluid::print_alpha()
{
	std::cout << std::endl << "alpha" << std::endl;
	for(int i = 0;i < nRows;i++)
	{
		std::cout << alpha[i] << std::endl;
	}
	return true;
}
bool Fluid::print_pr()
{
	std::cout << std::endl << "pr" << std::endl;
	for(int i = 0;i < nRows;i++)
	{
		std::cout << pr[i] << std::endl;
	}
	return true;
}
bool Fluid::print_table()
{
	std::cout << name << std::endl <<"Temp\trho\tcSubP\tmu\tv\tk\talpha\tpr" << std::endl;
	for(int i = 0;i < nRows;i++)
	{
		std::cout << t[i] << "\t" << rho[i] << "\t" << cSubP[i] << "\t" << mu[i] << "\t" << v[i] << "\t" << k[i] << "\t" << alpha[i] << "\t" << pr[i] << std::endl;
	}
	return true;
}
