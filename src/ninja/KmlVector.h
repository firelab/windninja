/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for creating a kmz file with vectors given 2 grids
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

#ifndef KMLVECTOR_H
#define KMLVECTOR_H

#include "ascii_grid.h"
#include "EasyBMP.h"
#include "EasyBMP_Font.h"
#include "EasyBMP_DataStructures.h"
#include "EasyBMP_Geometry.h"
#include "ninjaUnits.h"
#include "ninjaMathUtility.h"

#include "ogr_spatialref.h"
#include "ogr_core.h"
#include "gdal_version.h"
#include "cpl_port.h"
#include "cpl_error.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "cpl_vsi.h"
#include "gdalwarper.h"
#include "gdal_priv.h"

#include "gdal_util.h" //nsw

#ifndef Q_MOC_RUN
#include "boost/date_time/local_time/local_time.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#endif

#include "ninjaException.h"


#include "Style.h"

//#include <process.h>
#include <stdio.h>
#include <fstream>

static const double PI = std::acos(-1.0);


class KmlVector
{
public:

    KmlVector();

    /*KmlVector(std::string kmzFileName);

    KmlVector(AsciiGrid<double> *s, AsciiGrid<double> *d, std::string demFileName, std::string kmzFileName, double res);

    KmlVector(AsciiGrid<double> *s, AsciiGrid<double> *d, std::string kmzFileName);
    KmlVector(AsciiGrid<double> *s, AsciiGrid<double> *d, std::string kmzFileName, double res);
    KmlVector(std::string sFileName, std::string dFileName, std::string kmzFileName);
    KmlVector(std::string sFileName, std::string dFileName, std::string kmzFileName, double res);*/
    ~KmlVector();

    AsciiGrid<double> spd, dir;
    AsciiGrid<double> turbulence;
    AsciiGrid<double> colMax;
    #ifdef FRICTION_VELOCITY
    AsciiGrid<double> ustar;
    #endif
    #ifdef EMISSIONS
    AsciiGrid<double> dust;
    #endif

    Style **colors;

    enum egoogSpeedScaling{
        equal_color,
        equal_interval
    };

    inline void setLineWidth(double width){lineWidth = width;}
    bool makeDefaultStyles(std::string cScheme,bool vec_scaling);
    void calcSpeedSplitVals(egoogSpeedScaling scaling);
    void calcSplitValsFromSplitVals(double **speedSplitVals, const int nSets, const int numSplitVals, egoogSpeedScaling scaling);
    double* getSpeedSplitVals(int &size);
    void setSpeedSplitVals(const double *speedSplitVals, const int size);

    inline void setKmzFileName(std::string fileName){kmzFile = fileName;}

    bool writeKml(egoogSpeedScaling scaling, std::string cScheme, bool vector_scaling);
    bool makeKmz();
    bool removeKmlFile();

    void orangeLegend();

    bool writeHeader(VSILFILE *fileOut);
    bool writeRegion(VSILFILE *fileOut);
    bool writeStyles(VSILFILE *fileOut);
    bool writeHtmlLegend(VSILFILE *fileOut);
    bool writeScreenOverlayLegend(VSILFILE *fileOut,std::string cScheme);
    bool writeScreenOverlayDateTimeLegend(VSILFILE *fileOut);
    bool writeScreenOverlayDateTimeLegendWxModelRun(VSILFILE *fileOut);

    bool writeVectors(VSILFILE *fileOut);
    bool writeTurbulence(VSILFILE *fileOut);
    bool writeColMax(VSILFILE *fileOut);
    #ifdef FRICTION_VELOCITY
    bool writeUstar(FILE *fileOut);
    #endif
    #ifdef EMISSIONS
    bool writeDust(FILE *fileOut);
    #endif

    void setDemFile(std::string fileName){demFile = fileName;}
    void setKmzFile(std::string fileName){kmzFile = fileName;}
    void setKmlFile(std::string fileName){kmlFile = fileName;}
    void setLegendFile(std::string fileName){legendFile = fileName;}
    void setDateTimeLegendFile(std::string fileName, const boost::local_time::local_date_time& time_){timeDateLegendFile = fileName; kmlTime = time_;}
    void setInputSpeedFile(std::string fileName){inputSpeedFile = fileName;}
    void setInputDirFile(std::string fileName){inputDirFile = fileName;}

    void setSpeedGrid(AsciiGrid<double> &s, velocityUnits::eVelocityUnits units);
    void setDirGrid(AsciiGrid<double> &d);
    void setTurbulenceGrid(AsciiGrid<double> &turb, velocityUnits::eVelocityUnits units);
    double colMax_colHeightAGL;
    lengthUnits::eLengthUnits colMax_colHeightAGL_units;
    void setColMaxGrid(AsciiGrid<double> &columnMax, velocityUnits::eVelocityUnits units,  const double colHeightAGL, const lengthUnits::eLengthUnits colHeightAGL_units);
    #ifdef FRICTION_VELOCITY
    void setUstarGrid(AsciiGrid<double> &ust);
    #endif
    #ifdef EMISSIONS
    void setDustGrid(AsciiGrid<double> &dst);
    #endif
    void setTime(const boost::local_time::local_date_time& timeIn){kmlTime = timeIn;}
    void setWxModel(const std::string& modelName, const boost::local_time::local_date_time& startTime);

    std::string getShortName(std::string file);

    //bool readPrjFile();
    bool setProj4(std::string prj);

    //OGR STUFF
    bool setOGR();
    OGRSpatialReference oSourceSRS, oTargetSRS;
    OGRCoordinateTransformation *coordTransform;


private:
    double resolution;
    std::string demFile;
    std::string inputSpeedFile;
    velocityUnits::eVelocityUnits speedUnits;
    std::string inputDirFile;
    std::string kmlFile;
    std::string kmzFile;
    std::string legendFile;
    std::string timeDateLegendFile;
    std::string wxModelName;
    bool turbulenceFlag;
    std::string turbulence_tiff;
    std::string turbulence_png;
    std::string turbulence_legend;
    bool colMaxFlag;
    std::string colMax_tiff;
    std::string colMax_png;
    std::string colMax_legend;
    #ifdef FRICTION_VELOCITY
    bool ustarFlag;
    std::string ustar_tiff;
    std::string ustar_png;
    std::string ustar_legend;
    #endif
    #ifdef EMISSIONS
    bool dustFlag;
    std::string dust_tiff;
    std::string dust_png;
    std::string dust_legend;
    #endif

    boost::local_time::local_date_time kmlTime;
    boost::local_time::local_date_time wxModelStartTime;

    double geTheta;

    static const int numColors = 5;
    static const int numSplits = numColors+1;

    double *splitValue;

    double northExtent, eastExtent, southExtent, westExtent;
    double lineWidth;

};

#endif  //KMLVECTOR_H
