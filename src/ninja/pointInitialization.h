/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  A concrete class for initializing WindNinja wind fields using
 *			 the point initialization input method (weather stations)
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

#ifndef POINT_INITIALIZATION_H
#define POINT_INITIALIZATION_H

#include "initialize.h"
#include "cellDiurnal.h"

#include <limits>	//for large number

#include "ogr_api.h"
#include "sstream"
#include "algorithm"
#include "iterator"
#include "string"
#include "iostream"
#include "fstream"

#define dtoken "33e3c8ee12dc499c86de1f2076a9e9d4"
#define dstation "kmso" //Missoula International Airport
#define altstation "TR266" //FIRELAB Fire Raws
#define latest "latest"
#define start_stop "0"
#define mstation "kmso,TR266"
#define dvar "wind_speed,wind_direction,air_temp,solar_radiation"
#define drad "20"
#define dlim "15"

class pointInitialization : public initialize
{
	public:

		pointInitialization();								//Default constructor
		virtual ~pointInitialization();						// Destructor
		
		//pointInitialization(pointInitialization const& m);               // Copy constructor
		//pointInitialization& operator= (pointInitialization const& m);   // Assignment operator

		//Implementation of base class virtual function for initializing volume wind fields using the
		//domain averaged wind method.
		virtual void initializeFields(WindNinjaInputs &input,
		        Mesh const& mesh,
		        wn_3dScalarField& u0,
		        wn_3dScalarField& v0,
		        wn_3dScalarField& w0,
		        AsciiGrid<double>& cloud,
		        AsciiGrid<double>& L,
		        AsciiGrid<double>& u_star,
		        AsciiGrid<double>& bl_height);

        void stationcaller(std::string station_id,int nHours, bool btype,std::string fetcher, std::string yeara,std::string montha, std::string daya,std::string clocka,std::string yearb,std::string monthb,std::string dayb,std::string clockb);
        void singlestation_fetch(std::string token,bool type,int nHours, std::string station_id, std::string svar,std::string yeara,std::string montha, std::string daya,std::string clocka,std::string yearb,std::string monthb,std::string dayb,std::string clockb);
        void multistation_fetch(std::string token,bool type,int nHours, std::string station_ids, std::string svar,std::string yeara,std::string montha, std::string daya,std::string clocka,std::string yearb,std::string monthb,std::string dayb,std::string clockb);
        void radiusstation_fetch(std::string token,bool type,int nHours, std::string station_id,std::string radius, std::string limit, std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
        void radiuslatlon_fetch(std::string token, bool type,int nHours, std::string lat, std::string lon, std::string radius, std::string limit, std::string svar,std::string yeara,std::string montha, std::string daya,std::string clocka,std::string yearb,std::string monthb,std::string dayb,std::string clockb);
        void bbox_fetch(std::string token,bool type,int nHours,std::string lat1,std::string lon1,std::string lat2,std::string lon2,std::string svar,std::string yeara,std::string montha, std::string daya,std::string clocka,std::string yearb,std::string monthb,std::string dayb,std::string clockb);

	private:
                string sandbuild(std::string year_0,std::string month_0, std::string day_0,std::string clock_0,std::string year_1,std::string month_1,std::string day_1,std::string clock_1);
                string diversify(int a);
                vector<string> split(char* str,const char* delim);
                vector<string>  ameliorate(const double *puce,int counter);
                void stringprinter(char **redclay, int counter, std::string name);
                void floatprinter(const double *data, int counter,std::string name);
                void vectorprinter(std::vector<std::string> cata,std::string name);

                const char* singlebuilder(std::string token, std::string station_id, std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
                const char* latestsingle(std::string token, std::string station_id,std::string svar,int past);
                const char* multibuilder(std::string token,std::string station_ids,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
                const char* latestmulti(std::string token, std::string station_ids,std::string svar,int past);
                const char* latestradius(std::string token, std::string station_id,std::string radius,std::string limit,std::string svar,int past);
                const char* radiusbuilder(std::string token, std::string staion_id, std::string radius,std::string limit,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
                const char* latlonrad(std::string token,std::string lat, std::string lon, std::string radius, std::string limit,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
                const char* latestlatlon(std::string token,std::string lat, std::string lon, std::string radius, std::string limit,std::string svar,int past);
                const char* bboxbuilder(std::string token,std::string lat1,std::string lon1, std::string lat2, std::string lon2,std::string svar,std::string yearx,std::string monthx, std::string dayx,std::string clockx,std::string yeary,std::string monthy,std::string dayy,std::string clocky);
                const char* latestbbox(std::string token,std::string lat1,std::string lon1, std::string lat2, std::string lon2,std::string svar,int past);

                double dfInvDistWeight;

};

#endif /* POINT_INITIALIZATION_H */
