/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Weather Model initialization base class
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

#ifndef WX_MODEL_INITIALIZATION_H
#define WX_MODEL_INITIALIZATION_H

#include "initialize.h"
#include "ascii_grid.h"
#include "ShapeVector.h"
/* netcdf */
#include "netcdf.h"
/* gdal and gdal convenience */
#include "gdal_priv.h"
#include "gdal_util.h"
#include "cpl_conv.h"
#include "cpl_vsi.h"
#include "cpl_port.h"

#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,10,0)
#include "cpl_progress.h"
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,10,0) */
#endif /* GDAL_COMPUTE_VERSION */

#include "cpl_http.h"
#include "cpl_string.h"
/* omp */
#include <omp.h>
#include "omp_guard.h"
/* boost */
#include "boost/date_time/local_time/local_time.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp" //no i/o just types
#include "boost/date_time/gregorian/gregorian_types.hpp"    //no i/o just types
namespace blt = boost::local_time;
namespace bpt = boost::posix_time;

#include "ninja_conv.h"
#include "ninjaUnits.h"

#include "volVTK.h"

extern omp_lock_t netCDF_lock;

static char **papszThreddsCsv = NULL;

/**
 * Class used for coarse weather model initialization.
 * Inherits from initialize.  Mostly contains virtual functions defined by
 * subclasses for specific types of weather models.  Only member is a string
 * holding the filename (wxModelFileName).
 * */


class wn_3dScalarField;
class wxModelInitialization : public initialize
{
 public:

    wxModelInitialization();
    wxModelInitialization( const wxModelInitialization& A );
    virtual ~wxModelInitialization();

    wxModelInitialization& operator= ( wxModelInitialization const& A );



    virtual void initializeFields( WindNinjaInputs &input, Mesh const& mesh,
                           wn_3dScalarField& u0, wn_3dScalarField& v0,
                           wn_3dScalarField& w0, AsciiGrid<double>& cloud,
                           AsciiGrid<double>& L,
                           AsciiGrid<double>& u_star,
                           AsciiGrid<double>& bl_height );

    //virtual time list functions
    virtual std::vector<blt::local_date_time> getTimeList(std::string timeZoneString = "Africa/Timbuktu");    //Africa/Timbuktu is GMT with no daylight savings
    virtual std::vector<blt::local_date_time> getTimeList(const char *pszVariable, std::string timeZoneString = "Africa/Timbuktu");    //Africa/Timbuktu is GMT with no daylight savings
    virtual std::vector<blt::local_date_time> getTimeList(blt::time_zone_ptr timeZonePtr);
    virtual std::vector<blt::local_date_time> getTimeList(const char *pszVariable, blt::time_zone_ptr timeZonePtr);
    std::string getHost();

    //Pure virtual functions defined in subclasses
    virtual bool identify( std::string fileName ) = 0;
    virtual double getGridResolution() = 0;
    virtual std::vector<std::string> getVariableList() = 0;
    virtual std::string getForecastIdentifier() = 0;
    virtual std::string getForecastReadable( const char bySwapWithSpace=' ');
    virtual std::string getPath();
    virtual int getStartHour() = 0;
    virtual int getEndHour() = 0;
    virtual void checkForValidData() = 0;
    virtual double Get_Wind_Height() = 0;

    int ComputeWxModelBuffer(GDALDataset *poDS, double buffer[4]);
    virtual double GetWxModelBuffer(double delta);

    std::string generateUrl( double north, double west, double east,
                             double south, int hours);
    virtual std::string fetchForecast( std::string demFile, int nHours );
    std::string generateForecastName();
    
    void setModelFileName( std::string filename ) {wxModelFileName = filename;}
    
    wn_3dScalarField air3d; //perturbation potential temperature
    
    std::vector<double> u10List;
    std::vector<double> v10List;
    
    std::vector<double> u_wxList;
    std::vector<double> v_wxList;
    std::vector<double> w_wxList;

    void SetProgressFunc( GDALProgressFunc );
    void SetProgressArg( void *p );

 protected:
    int LoadFromCsv();
    double GetWindHeight(std::string height_string);
    std::string wxModelFileName; /**< file associated with this model */

    std::string heightVarName;
    std::string host, path;

    std::vector<blt::local_date_time> aoCachedTimes;

    virtual void setSurfaceGrids( WindNinjaInputs &input,
                                  AsciiGrid<double> &airGrid,
                                  AsciiGrid<double> &cloudGrid,
                                  AsciiGrid<double> &uGrid,
                                  AsciiGrid<double> &vGrid,
                                  AsciiGrid<double> &wGrid ) = 0;
                                  
    #ifdef NOMADS_ENABLE_3D
    virtual void set3dGrids( WindNinjaInputs &input, Mesh const& mesh );
    virtual void setGlobalAttributes(WindNinjaInputs &input);
    virtual void buildWxMeshes(WindNinjaInputs &input, Mesh const& mesh);
    virtual void buildWxScalarFields();
    virtual void allocate(Mesh const& mesh);
    virtual void deallocateTemp();
    #endif

    std::string GetTimeName(const char *pszVariable);
    
    int wxModel_nLayers;
    int wxModel_nCols; //wx model ncols/nrows in reprojected coords (DEM space) after ndvs are stripped
    int wxModel_nRows; //from outer edges of image; for non-xy-staggered variables (T, ph, phb, etc.)
    
    double dfNoData; // no data value from netcdf file
    
    Mesh cellCenterWxMesh;
    Mesh xStaggerWxMesh;
    Mesh yStaggerWxMesh;
    Mesh zStaggerWxMesh;
    
    wn_3dScalarField u3d;
    wn_3dScalarField v3d;
    wn_3dScalarField w3d;
    wn_3dScalarField cloud3d;
    
    wn_3dScalarField wxU3d;
    wn_3dScalarField wxV3d;
    wn_3dScalarField wxW3d;
    wn_3dScalarField wxAir3d;
    wn_3dScalarField wxCloud3d;

    GDALProgressFunc pfnProgress;
};

#endif /* WX_MODEL_INITIALIZATION_H */
