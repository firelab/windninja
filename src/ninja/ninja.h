/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Main class for running WindNinja
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

#ifndef NINJA_HEADER
#define NINJA_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <ctime>
#include <iostream>

#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <sstream>
#include <cctype>
#include <cfloat>
#include <regex>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "gdal_priv.h"
#include "cpl_string.h"

#include "ninja_conv.h"

#include "constants.h"
#include "ascii_grid.h"
#include "SurfProperties.h"
#include "surfaceVectorField.h"
#include "WindNinjaInputs.h"
#include "KmlVector.h"
#include "ShapeVector.h"
#include "preconditioner.h"
#include "volVTK.h"
#include "ninjaCom.h"
#include "ninjaException.h"
#include "mesh.h"
#include "wn_3dArray.h"
#include "wn_3dScalarField.h"
#include "wn_3dVectorField.h"
#include "initialize.h"
#include "domainAverageInitialization.h"
#include "wxModelInitializationFactory.h"
#include "initializationFactory.h"
#include "pointInitialization.h"
#include "griddedInitialization.h"



#ifdef NINJAFOAM
#include "foamDomainAverageInitialization.h"
#include "foamWxModelInitialization.h"
#endif

#include "wxStation.h"
#include "ninjaUnits.h"
#include "element.h"
#include "farsiteAtm.h"
#include "OutputWriter.h"

#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>
#include "boost/date_time/local_time/local_time.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp" //no i/o just types
#endif

#include "stability.h"

#ifdef FRICTION_VELOCITY
#include "frictionVelocity.h"
#endif

#ifdef EMISSIONS
#include "dust.h"
#endif

#define OFFSET(N, incX) ((incX) > 0 ?  0 : ((N) - 1) * (-(incX))) //for cblas_dscal

#define LENGTH 256

//#define NINJA_DEBUG
//#define NINJA_DEBUG_VERBOSE

class ninja
{
public:
    ninja();
    virtual ~ninja();

    ninja(const ninja &rhs);
    ninja &operator=(const ninja &rhs);

    virtual bool simulate_wind();
    inline virtual std::string identify() {return std::string("ninja");}
    bool cancel;	//if set to "false" during a simulation (ie when "simulate_wind()" is running), the simulation will attempt to end
    Mesh mesh;

    //output grids to access the final wind grids (typically used by other programs running the windninja API such as WFDSS, FlamMap, etc.
    AsciiGrid<double>AngleGrid;
    AsciiGrid<double>VelocityGrid;
    AsciiGrid<double>CloudGrid;
#ifdef NINJAFOAM
    AsciiGrid<double>TurbulenceGrid; //this needs to be a member of ninja since we need to write this for ninjafoam runs with diurnal 
#endif

    wn_3dScalarField alphaVfield; //store spatially varying alphaV variable

    #ifdef FRICTION_VELOCITY
    AsciiGrid<double>UstarGrid;
    #endif

    #ifdef EMISSIONS
    AsciiGrid<double>DustGrid;
    #endif

    /*-----------------------------------------------------------------------------
     *
     *  Public Interface Methods
     *
     *
     *-----------------------------------------------------------------------------*/


    /*-----------------------------------------------------------------------------
     *  ninjaCom section
     *-----------------------------------------------------------------------------*/
    int get_inputsRunNumber() const;
    ninjaComClass::eNinjaCom get_inputsComType() const;
    char * get_lastComString() const;
    FILE * get_ComLogFp() const; //returns the Com Log file pointer
    ninjaComClass * get_Com() const; //returns the Com object
#ifdef NINJA_GUI
    int get_ComNumRuns() const;
    void set_ComNumRuns( int nRuns );
#endif //NINJA-GUI
    void set_progressWeight(double progressWeight); //For foam+diurnal simulations
    double get_progressWeight();
    /*************************************************************
      kyle's fx's for importing several file types through GDAL
      function lives in readInputFile.cpp for now.

    ***************************************************************/
    void readInputFile(std::string fileName);
    void readInputFile();
    void importSingleBand(GDALDataset*);
    void importLCP(GDALDataset*);
    void importGeoTIFF(GDALDataset*);
    void setSurfaceGrids();

    void set_memDs(GDALDatasetH hSpdMemDs, GDALDatasetH hDirMemDs, GDALDatasetH hDustMemDs); 
    void setArmySize(int n);
    void set_DEM(std::string dem_file_name);		//Sets elevation filename (Should be in units of meters!)
    void set_DEM(const double* dem, const int nXSize, const int nYSize, const double* geoRef,
                 std::string prj);
    void set_initializationMethod(WindNinjaInputs::eInitializationMethod method, bool matchPoints = false);	//input wind initialization method
    WindNinjaInputs::eInitializationMethod get_initializationMethod(); //returns the initializationMethod

    void set_uniVegetation(WindNinjaInputs::eVegetation vegetation_);			//set all uniform surface parameters based on a vegetation type (grass, brush, trees)
    void set_uniVegetation();
    static WindNinjaInputs::eVegetation get_eVegetationType(std::string veg);       //gets an eVegetation type from a string

    /*-----------------------------------------------------------------------------
     *  Stability Specific Functions
     *-----------------------------------------------------------------------------*/
    void set_stabilityFlag(bool flag);
    void set_alphaStability(double stability_);

    /*-----------------------------------------------------------------------------
     *  END Stability specific functions
     *-----------------------------------------------------------------------------*/
    void computeSurfPropForCell(long i, long j, double canopyHeight,
                                lengthUnits::eLengthUnits canopyHeightUnits,
                                double canopyCover, coverUnits::eCoverUnits canopyCoverUnits,
                                int fuelModel, double fuelBedDepth,
                                lengthUnits::eLengthUnits fuelBedDepthUnits);	//computes surface properties for cell i,j from inputs available from an .lcp file

    void set_uniRoughness(double roughness, lengthUnits::eLengthUnits units);	//set uniform values of Roughness
    void set_uniRoughH(double rough_h, lengthUnits::eLengthUnits units);		//set uniform values of RoughH
    void set_uniRoughD(double rough_d, lengthUnits::eLengthUnits units);		//set uniform values of RoughD
    void set_uniAlbedo(double albedo);					//set uniform values of Albedo
    void set_uniBowen(double bowen);				//set uniform values of Bowen
    void set_uniCg(double cg);						//set uniform values of Cg
    void set_uniAnthropogenic(double anthropogenic);		//set uniform values of Anthropogenic
    void set_inputSpeed(double speed, velocityUnits::eVelocityUnits units);	//for setting input speed for domain-averaged initialization
    void set_inputDirection(double direction);
    void set_inputWindHeight(double height, lengthUnits::eLengthUnits units);
    void set_inputWindHeight(double height);
    void set_outputSpeedUnits(velocityUnits::eVelocityUnits units);
    velocityUnits::eVelocityUnits get_outputSpeedUnits() const;
    void set_outputWindHeight(double height, lengthUnits::eLengthUnits units);
    double get_outputWindHeight() const;
    void set_diurnalWinds(bool flag);				//used to include or not include diurnal winds in the simulation (true or false)
    bool get_diurnalWindFlag(); //returns whether diurnal winds are set or not
    //void set_stabilityFlag(bool flag);              //used to include or not include stability
    void set_date_time(int const &yr, int const &mo, int const &day,
                       int const &hr, int const &min, int const &sec,
                       std::string const &timeZoneString);

    void set_date_time(boost::local_time::local_date_time time);
    boost::local_time::local_date_time get_date_time() const;

    void set_uniAirTemp(double temp, temperatureUnits::eTempUnits units);
    void set_uniCloudCover(double cloud_cover, coverUnits::eCoverUnits units);	//set cloud cover (fraction or percent of sky cover)
    void set_wxModelFilename(const std::string& forecast_initialization_filename);	//sets the surface wind field initialization file (such as NDFD, etc.)
    void set_wxStationFilename(std::string station_filename);	//sets the weather station(s) filename (for use in point initialization)
    void set_wxStations(std::vector<wxStation> &wxStations);
//stationFetch
    void set_stationFetchFlag( bool flag );
//    std::vector<std::vector<wxStationList> >get_wxStatList();


//stationFetch
    std::vector<wxStation> get_wxStations();
    void set_meshResChoice( std::string choice );
    void set_meshResChoice( const Mesh::eMeshChoice );
    virtual void set_meshResolution( double resolution, lengthUnits::eLengthUnits units );
    virtual double get_meshResolution();
    void set_numVertLayers( const int nLayers );
#ifdef NINJA_SPEED_TESTING
    void set_speedDampeningRatio(double r);
    void set_downDragCoeff(double coeff);
    void set_downEntrainmentCoeff(double coeff);
    void set_upDragCoeff(double coeff);
    void set_upEntrainmentCoeff(double coeff);
#endif
#ifdef FRICTION_VELOCITY
    void set_frictionVelocityFlag(bool flag);
    void set_frictionVelocityCalculationMethod(std::string calcMethod);
    void computeFrictionVelocity();
    const std::string get_UstarFileName() const; //returns the name of the ustar file name
#endif
#ifdef EMISSIONS
    void set_dustFilename(std::string filename);    //set the dust emissions input fire perimeter filename
    void set_dustFileOut(std::string filename); //set the dust file out filename
    void set_dustFlag(bool flag);
    void computeDustEmissions();
    const std::string get_DustFileName() const; //returns the name of the dust file name
    const std::string get_GeotiffFileName() const; //returns the name of the geotiff file name
    void set_geotiffOutFilename(std::string filename); //set the multiband geotiff output filename
    void set_geotiffOutFlag(bool flag);
#endif
#ifdef NINJAFOAM
    void set_NumberOfIterations(int nIterations); //number of iterations for a ninjafoam run
    void set_MeshCount(int meshCount); //mesh count for a ninjafoam run
    void set_MeshCount(WindNinjaInputs::eNinjafoamMeshChoice meshChoice); //mesh count for a ninjafoam run
    static WindNinjaInputs::eNinjafoamMeshChoice get_eNinjafoamMeshChoice(std::string meshChoice);
    void set_ExistingCaseDirectory(std::string directory); //use existing case for ninjafoam run
    void set_foamVelocityGrid(AsciiGrid<double> velocityGrid);
    void set_foamAngleGrid(AsciiGrid<double> angleGrid);
    void set_writeTurbulenceFlag(bool flag);
#endif

    void set_speedFile(std::string speedFile, velocityUnits::eVelocityUnits units);
    void set_dirFile(std::string dirFile);

    void set_position(double lat_degrees, double long_degrees);//input as decimal degrees

    void set_inputPointsFilename(std::string filename); //set name for input file of requested output locations
    void set_outputPointsFilename(std::string filename); //set name for output file with winds at requested locations

    const std::string get_VelFileName() const; //returns the name of the velocity file name
    const std::string get_AngFileName() const; //returns the name of the ang output file
    const std::string get_CldFileName() const; //returns the name of the cld output file

    //kyle set postion
    bool set_position();

    void add_WxStation(wxStation const& Station);

    void set_position(double lat_degrees, double lat_minutes, double long_degrees, double long_minutes);	//input as degrees, decimal minutes
    void set_position(double lat_degrees, double lat_minutes, double lat_seconds, double long_degrees, double long_minutes, double long_seconds);	//input as degrees, minutes, seconds
    void set_numberCPUs(int CPUs);
    double *get_outputSpeedGrid();
    double *get_outputDirectionGrid();
    const char* get_outputGridProjection();
    double get_outputGridCellSize();
    double get_outputGridxllCorner();
    double get_outputGridyllCorner();
    int get_outputGridnCols();
    int get_outputGridnRows();
    void set_outputBufferClipping(double percent);
    void set_writeAtmFile(bool flag);  //Flag that determines if an atm file should be written.  Usually set by ninjaArmy, NOT directly by the user!
    void set_googOutFlag(bool flag);

    void set_googColor(std::string scheme,bool scaling);

    void set_wxModelGoogOutFlag(bool flag);
    void set_googSpeedScaling(KmlVector::egoogSpeedScaling scaling);	//sets the desired method of speed scaling in the Google Earth legend (equal_color=>equal numbers of arrows for each color,  equal_interval=>equal speed intervals over the speed range)
    void set_googLineWidth(double width);								//sets the line width for the vectors in the Google Earth kmz file
    void set_googResolution(double Resolution, lengthUnits::eLengthUnits units);	//sets the output resolution of the Google Earth .kmz file, if negative value the computational mesh resolution is used
    void set_wxModelGoogSpeedScaling(KmlVector::egoogSpeedScaling scaling);
    void set_wxModelGoogLineWidth(double width);
    void set_shpOutFlag(bool flag);
    void set_wxModelShpOutFlag(bool flag);
    void set_shpResolution(double Resolution, lengthUnits::eLengthUnits units);	//sets the output resolution of the shapefile, if negative value the computational mesh resolution is used

    void set_asciiOutFlag(bool flag);
    inline void set_asciiAaigridOutFlag(bool flag) { input.asciiAaigridOutFlag = flag; }
    inline void set_asciiJsonOutFlag(bool flag) { input.asciiJsonOutFlag = flag; }
    inline void set_asciiUtmOutFlag(bool flag) { input.asciiUtmOutFlag = flag; }
    inline void set_ascii4326OutFlag(bool flag) { input.ascii4326OutFlag = flag; }
    inline void set_asciiUvOutFlag(bool flag) { input.asciiUvOutFlag = flag; }

    void set_wxModelAsciiOutFlag(bool flag);
    void set_asciiResolution(double Resolution, lengthUnits::eLengthUnits units);	//sets the output resolution of the velocity and angle ASCII grid output files, if negative value the computational mesh resolution is used
    void set_txtOutFlag(bool flag);
    void set_vtkOutFlag(bool flag);		//determines if VTK volume output files will be written
    void set_pdfOutFlag(bool flag);
    void set_pdfResolution(double Resolution, lengthUnits::eLengthUnits units);
    void set_pdfDEM(std::string dem_file_name);
    void set_pdfLineWidth(const float w);
    void set_pdfBaseMap(const int b);
    void set_pdfSize( const double height, const double width, const unsigned short dpi );
    void set_outputFilenames(double& meshResolution, lengthUnits::eLengthUnits meshResolutionUnits);
    const std::string get_outputPath() const;
    void keepOutputGridsInMemory(bool flag);
    void set_outputPath(std::string path);

    void set_PrjString(std::string prj);

    double getFuelBedDepth(int fuelModel);

    void set_ninjaCommunication(int RunNumber, ninjaComClass::eNinjaCom comType);
    void checkInputs();
    void dumpMemory();

    WindNinjaInputs input;	//The place were all inputs (except mesh) are stored.

protected:
    void checkCancel();
    void write_compare_output();
    boost::shared_ptr<initialize> init;

private:


    double matchTol;		//tolerance (in m/s) used in outer "matching" looping to check if solved velocity field matches at each wx station.
                            //Each u, v, w velocity component is checked.

    int nMaxMatchingIters;
    std::vector<int> num_outer_iter_tries_u;   //used in outer iterations calcs
    std::vector<int> num_outer_iter_tries_v;   //used in outer iterations calcs
    std::vector<int> num_outer_iter_tries_w;   //used in outer iterations calcs

    wn_3dScalarField u, v, w;
    wn_3dScalarField u0;		//u is positive toward East
    wn_3dScalarField v0;		//v is positive toward North
    wn_3dScalarField w0;		//w is positive up

    double *DIAG;
    double *PHI, *RHS, *SK;
    int *row_ptr, *col_ind;
    double alphaH; //alpha horizontal from governing equation, weighting for change in horizontal winds
    double alpha;                //alpha = alphaH/alphaV, determined by stability
    AsciiGrid<double> *uDiurnal, *vDiurnal, *wDiurnal, *height;
    Aspect *aspect;
    Slope *slope;
    Shade *shade;
    Solar *solar;

    double* outputSpeedArray; //output speed array returned in the API
    double* outputDirectionArray; //output direction array returned in the API

    bool isNullRun;			//flag identifying if this run is a "null" run, ie. run with all zero speed for intitialization
    double maxStartingOuterDiff;   //stores the maximum difference for "matching" runs from the first iteration (used to determine convergence)

    //Timers
    double startTotal, endTotal;
    double startMesh, endMesh;
    double startInit, endInit;
    double startBuildEq, endBuildEq;
    double startSolve, endSolve;
    double startWriteOut, endWriteOut;
    #ifdef FRICTION_VELOCITY
    double startComputeFrictionVelocity, endComputeFrictionVelocity;
    #endif
    #ifdef EMISSIONS
    double startDustEmissions, endDustEmissions;
    #endif

    double getSmallestRadiusOfInfluence();
    void get_rootname(const char *NAME,char *shortname);
    bool solve(double *SK, double *RHS, double *PHI, int *row_ptr,
               int *col_ind, int NUMNP, int MAXITS, int print_iters, double stop_tol);

    /*-----------------------------------------------------------------------------
     * alternative solvers                                                           
     *-----------------------------------------------------------------------------*/
    bool solveMinres(double *A, double *b, double *x, int *row_ptr, int *col_ind, int NUMNP, int max_iter, int print_iters, double tol);

    /*-----------------------------------------------------------------------------
     *  MKL Specific Functions
     *-----------------------------------------------------------------------------*/
    void cblas_dcopy(const int N, const double *X, const int incX,
                                        double *Y, const int incY);

    double cblas_ddot(const int N, const double *X, const int incX,
                                   const double *Y, const int incY);

    void cblas_daxpy(const int N, const double alpha, const double *X, const int incX,
                                                            double *Y, const int incY);

    double cblas_dnrm2(const int N, const double *X, const int incX);

    void mkl_dcsrmv(char *transa, int *m, int *k, double *alpha, char *matdescra,
                    double *val, int *indx, int *pntrb, int *pntre, double *x,
                    double *beta, double *y);

    void cblas_dscal(const int N, const double alpha, double *X, const int incX);
    void mkl_trans_dcsrmv(char *transa, int *m, int *k, double *alpha, char *matdescra, double *val, int *indx, int *pntrb, int *pntre, double *x, double *beta, double *y);

    /*-----------------------------------------------------------------------------
     *  End MKL Section
     *-----------------------------------------------------------------------------*/
    void interp_uvw();

    void write_A_and_b(int NUMNP, double *A, int *col_ind, int *row_ptr, double *b);
    double get_aspect_ratio(int NUMEL, int NUMNP, double *XORD, double *YORD, double *ZORD,
                            int nrows, int ncols, int nlayers);

    double get_equiangle_skew(int NUMEL, int NUMNP, double *XORD, double *YORD, double *ZORD,
                              int nrows, int ncols, int nlayers);

    void get_cell_angles(double xa, double ya, double za,
                         double xb, double yb, double zb,
                         double xc, double yc, double zc,
                         double xd, double yd, double zd,
                         double &cell_max_angle, double &cell_min_angle);

    double get_angle(double x1, double y1, double z1,
                     double x2, double y2, double z2,
                     double x3, double y3, double z3);

    double maxj(double value1, double value2);
    //double compute_cellsize(Elevation *dem, double target_num_horiz_cells);
    //double compute_domain_height(Elevation *dem, double max_desired_aspect_ratio, int num_vert_layers, double vert_growth, double CellSize, AsciiGrid<double> *Roughness);
    //double monin_obukov(double z, double U1, double z1, double z0, double L, double u_star);
    //double stability_function(double z_over_L, double L_switch);
    bool writePrjFile(std::string inPrjString, std::string outFileName);
    bool checkForNullRun();
    void discretize(); 
    void setBoundaryConditions();
    void computeUVWField();
    void prepareOutput();
    bool matched(int iter);
    void writeOutputFiles(); 
    void deleteDynamicMemory();

private:
    void setUvGrids (AsciiGrid<double>& angGrid, AsciiGrid<double>& velGrid, AsciiGrid<double>& uGrid, AsciiGrid<double>& vGrid);
    void writeAsciiOutputFiles (AsciiGrid<double>& cldGrid, AsciiGrid<double>& angGrid, AsciiGrid<double>& velGrid);
    void writeAsciiUvOutputFiles (AsciiGrid<double>& angGrid, AsciiGrid<double>& velGrid);
};

std::string derived_pathname (const char* pathname, const char* newpath, const char* pattern, const char* replacement);

#endif	//NINJA_HEADER


