/******************************************************************************
*
* $Id$
*
* Project:  WindNinja
* Purpose:  A class for doing multiple ninja runs.
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

#ifndef NINJA_ARMY_H
#define NINJA_ARMY_H

#ifdef _OPENMP
#include "omp.h"
#endif

#ifdef NINJAFOAM
#include "ninjafoam.h"
#endif

#include "ninja_threaded_exception.h"
#include "farsiteAtm.h"
#include "wxModelInitializationFactory.h"
#include "ninja_errors.h"
#include <algorithm>
#ifndef Q_MOC_RUN
#include "boost/typeof/typeof.hpp"
#endif
#include "WindNinjaInputs.h"
#include "fetch_factory.h"

/*-----------------------------------------------------------------------------
 *  Helper Macros
 *-----------------------------------------------------------------------------*/
/**
 Defines a FOR_EVERY macro that simplifies writing vector iterator loops.
 Example usage is FOR_EVERY( it, ninjas ) { it->get_someAttribute(); }
*/
#define FOR_EVERY(iter, iterable) \
    for(BOOST_TYPEOF((iterable).begin()) iter = (iterable).begin();iter != (iterable).end(); ++iter)
/* *
 * Macro IF_VALID_INDEX completes simple range checking for iterables
 * */
#define IF_VALID_INDEX( i, iterable ) \
   if( i >= 0 && i < iterable.size() )
/* *
 * Macro IF_VALID_INDEX_DO is a boiler plate for most of the ninjaArmy functions.
 * First, a range check is done on iterable to determine if 'i' is a valid index.
 * If 'i' is a valid index, then the function call 'func' is executed.
 * 'func' is located inside a try-catch statement block so upon a thrown exception
 * it is handled and NINJA_E_INVALID is returned. Otherwise, NINJA_SUCCESS is returned.
 *  */
#define IF_VALID_INDEX_TRY( i, iterable, func ) \
    if( i >= 0 && i < iterable.size() )        \
    {                                          \
        try                                    \
        {                                      \
           func;                               \
        }                                      \
        catch( ... )                           \
        {                                      \
            return NINJA_E_INVALID;            \
        }                                      \
        return NINJA_SUCCESS;                  \
    }                                          \
    return NINJA_E_INVALID;

//#include "ninjaCom.h"
/**
* Class used for doing multiple WindNinja runs.
*/
class  ninjaArmy
{
public:

    ninjaArmy();
#ifdef NINJAFOAM
    ninjaArmy(int numNinjas, bool momentumFlag);
#else
    ninjaArmy(int numNinjas);
#endif
    ninjaArmy(const ninjaArmy& A);
    ~ninjaArmy();

    ninjaArmy& operator= (ninjaArmy const& A);

    //ninjaComClass *Com;

    enum eWxModelType{
        ncepNdfd,
        ncepNamSurf,
        ncepRapSurf,
        ncepDgexSurf,
        ncepNamAlaskaSurf,
        ncepGfsSurf
    };

    void makeArmy(std::string forecastFilename, std::string timeZone, bool momentumFlag);
    void set_writeFarsiteAtmFile(bool flag);
    bool startRuns(int numProcessors);
    bool startFirstRun();

    /**
    * \brief Return the number of ninjas in the army
    *
    * \return num_ninjas the number of ninjas in the army
    */
    //int getSize() const { return ninjas.size(); }

    int getSize();

    /**
    * \brief Set the number of ninja in the army
    *
    * \param nRuns number of ninjas to create
    * \return
    */
    void setSize( int nRuns, bool momentumFlag);
    /*-----------------------------------------------------------------------------
     *  Ninja Communication Methods
     *-----------------------------------------------------------------------------*/
    /**
    * \brief Initialize the ninja communication of a ninja
    *
    * \param nIndex index of a ninja
    * \param RunNumber number of runs
    * \param comType type of communication
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setNinjaCommunication( const int nIndex, const int RunNumber,
                               const ninjaComClass::eNinjaCom comType,
                               char ** papszOptions = NULL );

#ifdef NINJA_GUI
    /**
    * \brief Set the number of runs for a ninjaCom
    *
    * \param nIndex index of a ninja
    * \param RunNumber number of runs
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setNinjaComNumRuns( const int nIndex, const int RunNumber,
                            char ** papszOptions=NULL );
    /**
    * \brief Returns the ninjaCom for a ninja
    *
    * \param nIndex index of a ninja
    * \return com the ninjaComClass of a ninja
    */
    ninjaComClass * getNinjaCom( const int nIndex, char ** papszOptions=NULL );
#endif //NINJA_GUI

    /*-----------------------------------------------------------------------------
     *  Ninja speed testing Methods
     *-----------------------------------------------------------------------------*/
#ifdef NINJA_SPEED_TESTING
    /**
    * \brief Set speed dampening ratio for a ninja
    *
    *
    * \param nIndex index of a ninja
    * \param ratio speed dampening ratio (0.0- 1.0)
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setSpeedDampeningRatio( const int nIndex, const double r, char ** papszOptions=NULL );

    /**
    * \brief Set downslope drag coefficient for a ninja
    *
    *
    * \param nIndex index of a ninja
    * \param coeff downslope drag coefficient
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setDownDragCoeff( const int nIndex, const double coeff, char ** papszOptions=NULL );
    
    /**
    * \brief Set downslope entrainment coefficient for a ninja
    *
    *
    * \param nIndex index of a ninja
    * \param coeff downslope entrainment coefficient
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setDownEntrainmentCoeff( const int nIndex, const double coeff, char ** papszOptions=NULL );
    
    /**
    * \brief Set upslope drag coefficient for a ninja
    *
    *
    * \param nIndex index of a ninja
    * \param coeff upslope drag coefficient
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setUpDragCoeff( const int nIndex, const double coeff, char ** papszOptions=NULL );
    
    /**
    * \brief Set upslope entrainment coefficient for a ninja
    *
    *
    * \param nIndex index of a ninja
    * \param coeff upslope entrainment coefficient
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setUpEntrainmentCoeff( const int nIndex, const double coeff, char ** papszOptions=NULL );
#endif

    /*-----------------------------------------------------------------------------
     *  Friciton Velocity Methods
     *-----------------------------------------------------------------------------*/
#ifdef FRICTION_VELOCITY
    /**
    * \brief Enable/disable friction velocity calculations for a ninja
    *
    *
    * \param nIndex index of a ninja
    * \param flag Enables friciton velocity if true, disables if false
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setFrictionVelocityFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );

    /**
    * \brief Set method for friction velocity calculations for a ninja
    *
    *
    * \param nIndex index of a ninja
    * \param calcMethod Calculation method to use
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setFrictionVelocityCalculationMethod( const int nIndex, const std::string calcMethod, char ** papszOptions=NULL );
#endif //FRICTION_VELOCITY

    /*-----------------------------------------------------------------------------
     *  Dust Methods
     *-----------------------------------------------------------------------------*/
#ifdef EMISSIONS
    /**
    * \brief Set the dust file input name for a ninja
    *
    * \param nIndex index of a ninja
    * \param filename name of the dust input file
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setDustFilename( const int nIndex, const std::string filename, char ** papszOptions=NULL );

    /**
    * \brief Enable/disable dust for a ninja
    *
    *
    * \param nIndex index of a ninja
    * \param flag Enables dust if true, disables if false
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setDustFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );
    
        /**
    * \brief Set the dust geotiff file output name for a ninja
    *
    * \param nIndex index of a ninja
    * \param filename name of the dust geotiff output file
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setGeotiffOutFilename( const int nIndex, const std::string filename, char ** papszOptions=NULL );

    /**
    * \brief Enable/disable dust geotiff file output for a ninja
    *
    *
    * \param nIndex index of a ninja
    * \param flag Enables dust geotiff output if true, disables if false
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setGeotiffOutFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );
#endif //EMISSIONS

#ifdef NINJAFOAM
    /*-----------------------------------------------------------------------------
     *  NinjaFOAM Methods
     *-----------------------------------------------------------------------------*/
    /**
    * \brief Set the number of iterations for a NinjaFOAM run
    *
    * \param nIndex index of a ninja
    * \param nIterations Number of iterations
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setNumberOfIterations( const int nIndex, const int nIterations, char ** papszOptions=NULL );
    
    /**
    * \brief Set the mesh count for a NinjaFOAM run
    *
    * \param nIndex index of a ninja
    * \param meshCount Mesh count
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setMeshCount( const int nIndex, const int meshCount, char ** papszOptions=NULL );
    
    /**
    * \brief Set the mesh count for a NinjaFOAM run
    *
    * \param nIndex index of a ninja
    * \param meshChoice Mesh resolution choice
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setMeshCount( const int nIndex, 
                      const WindNinjaInputs::eNinjafoamMeshChoice meshChoice, 
                      char ** papszOptions=NULL );

    /**
    * \brief Enable/disable non-equilbrium boundary conditions for a NinjaFOAM run
    *
    * \param nIndex index of a ninja
    * \param nMeshCount Mesh count
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setNonEqBc( const int nIndex, const bool flag, char ** papszOptions=NULL );

    /**
    * \brief Set the path to an existing case for a NinjaFOAM run
    *
    * \param nIndex index of a ninja
    * \param  path to existing directory 
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setExistingCaseDirectory( const int nIndex, const std::string directory, char ** papszOptions=NULL );
        
#endif //NINJAFOAM

    /*-----------------------------------------------------------------------------
     *  Forecast Model Methods
     *-----------------------------------------------------------------------------*/
    /**
    * \brief Set the wx forecast filename for a ninja
    *
    * \param nIndex index of a ninja
    * \param wx_filename path of the wx file
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setWxModelFilename(const int nIndex, const std::string wx_filename, char ** papszOptions=NULL);
    
    /**
    * \brief Set the DEM file for a ninja
    *
    * \param nIndex index of a ninja
    * \param dem_filename path of the DEM file
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setDEM( const int nIndex, const std::string dem_filename, char ** papszOptions=NULL );
    /**
    * \brief Set the latitude/longitude position of a ninja
    *
    * \param nIndex index of a ninja
    * \param lat_degrees position latitude in degrees
    * \param lon_degrees position longitude in degrees
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setPosition( const int nIndex, const double lat_degrees,
                     const double lon_degrees,
                     char ** papszOptions=NULL );
    int setPosition( const int nIndex, char ** papszOptions=NULL );
    /**
    * \brief Set the input points filename for a ninja
    *
    * \param nIndex index of a ninja
    * \param filename path of the input points file
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setInputPointsFilename( const int nIndex, const std::string filename,
                                char ** papszOptions=NULL);
    /**
    * \brief Set the output points filename for a ninja
    *
    *
    * \param nIndex index of a ninja
    * \param filename location of the output points file
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setOutputPointsFilename( const int nIndex, const std::string filename,
                                char **papszOptions=NULL);

    int readInputFile( const int nIndex, std::string filename, char ** papszOptions=NULL );
    int readInputFile( const int nIndex, char ** papszOptions=NULL );
    /*-----------------------------------------------------------------------------
     *  Simulation Parameter Methods
     *-----------------------------------------------------------------------------*/

    /* Mutators */
    /**
    * \brief Set the number of CPUs/Threads to use for a ninja
    *
    * \param nIndex index of a ninja
    * \param nCPUs number of CPUs to use
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setNumberCPUs( const int nIndex, const int nCPUs, char ** papszOptions=NULL );
    /**
    * \brief Set the intialization method for a ninja
    *
    * \param nIndex index of a ninja
    * \param method intialization method type
    * \param matchPoints
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setInitializationMethod( const int nIndex,
                                 const WindNinjaInputs::eInitializationMethod method,
                                 const bool matchPoints=false,
                                 char ** papszOptions=NULL );
    /**
    * \brief Set the initialization method for a ninja
    * Set the initialization method for a ninja with
    * a string-formatted intialization method.
    *
    * _Valid initialization method strings:_
    * - "noInitializationFlag" = WindNinjaInputs::noInitializationFlag
    * - "domainAverageInitializationFlag" = WindNinjaInputs::domainAverageInitializationFlag
    * - "pointInitializationFlag" = WindNinjaInputs::pointInitializationFlag
    * - "wxModelInitializationFlag" = WindNinjaInputs::wxModelInitializationFlag
    *
    * \param nIndex index of a ninja
    * \param method string-formatted initialization method
    * \param matchPoints
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setInitializationMethod( const int nIndex,
                                 std::string method,
                                 const bool matchPoints=false,
                                 char ** papszOptions=NULL );
    /**
    * \brief Set the input speed grid filename from a NinjaFOAM run for use with diurnal
    *
    * \param nIndex index of a ninja
    * \param stlFile path/filename of gridded speed file
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setSpeedInitGrid( const int nIndex, const std::string speedFile, char ** papszOptions=NULL );
    
    /**
    * \brief Set the input direction grid filename from a NinjaFOAM run for use with diurnal
    *
    * \param nIndex index of a ninja
    * \param stlFile path/filename of gridded direction file
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setDirInitGrid( const int nIndex, const std::string dirFile, char ** papszOptions=NULL );
    
    /**
    * \brief Set the input speed with units of a ninja
    *
    * \param nIndex index of a ninja
    * \param speed input speed value
    * \param units units of the input speed
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setInputSpeed( const int nIndex, const double speed,
                       const velocityUnits::eVelocityUnits units,
                       char ** papszOptions=NULL );
    /**
    * \brief Set the input speed with units of a ninja
    * Set the input speed  and units of a ninja given
    * the ninja index and string-formatted units.
    *
    * _Valid speed units:_
    * - "mps" = metersPerSecond
    * - "mph" = milesPerHour
    * - "kph" = kilometersPerHour
    *
    * \param nIndex index of a ninja
    * \param speed input speed value
    * \param units string-formatted velocity units
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setInputSpeed( const int nIndex, const double speed,
                       std::string units, char ** papszOptions=NULL );
    /**
    * \brief Set the input direction for a ninja
    *
    *
    * \param nIndex index of a ninja
    * \param direction value of the input direction
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setInputDirection( const int nIndex, const double direction,
                           char ** papszOptions=NULL );
    /**
    * \brief Set the input wind height of a ninja
    *
    * \param nIndex index of a ninja
    * \param height wind height value
    * \param units wind height units
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setInputWindHeight( const int nIndex, const double height,
                            const lengthUnits::eLengthUnits units,
                            char ** papszOptions=NULL );
    /**
    * \brief Set the input wind height of a ninja
    * Set the wind height for the input of a ninja given the
    * ninja index, height value, and string-formatted units.
    *
    * _Valid wind height units:_
    * - "ft"    = feet
    * - "m"     = meters
    * - "mi"    = miles
    * - "km"    = kilometers
    * - "ftx10" = feetTimesTen
    * - "mx10"  = metersTimesTen
    *
    * \param nIndex index of a ninja
    * \param height wind height value
    * \param units string-formatted length units
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setInputWindHeight( const int nIndex, const double height,
                            std::string units, char ** papszOptions=NULL );

    int setInputWindHeight( const int nIndex, const double height,
                            char ** papszOptions=NULL );
    /**
    * \brief Set the output wind height for a ninja
    *
    * \param nIndex index of a ninja
    * \param height wind height value
    * \param units wind height units
    * \return
    */
    int setOutputWindHeight( const int nIndex, const double height,
                             const lengthUnits::eLengthUnits units,
                             char ** papszOptions=NULL );
    /**
    * \brief Set the wind height for the output of a ninja
    * Set the wind height for the output of a ninja given the
    * ninja index, height value, and string-formatted units.
    *
    * _Valid wind height units:_
    * - "ft"    = feet
    * - "m"     = meters
    * - "mi"    = miles
    * - "km"    = kilometers
    * - "ftx10" = feetTimesTen
    * - "mx10"  = metersTimesTen
    *
    * \param nIndex index of a ninja
    * \param height wind height value
    * \param units string-formatted length units
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setOutputWindHeight( const int nIndex, const double height,
                             std::string units, char ** papszOptions=NULL );
    /**
    * \brief Set the speed units for the output of a ninja
    *
    * \param nIndex index of a ninja
    * \param units the speed units
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setOutputSpeedUnits( const int nIndex,
                             const velocityUnits::eVelocityUnits units,
                             char ** papszOptions=NULL );
    /**
    * \brief Set the speed units for the output of a ninja
    * Set the speed units for the output of a ninja given
    * the ninja index and string-formatted units.
    *
    * _Valid speed units:_
    * - "mps" = metersPerSecond
    * - "mph" = milesPerHour
    * - "kph" = kilometersPerHour
    *
    * \param nIndex index of a ninja
    * \param units string-formatted velocity units
    * \return
    */
    int setOutputSpeedUnits( const int nIndex, std::string units,
                             char ** papszOptions=NULL );
    /**
    * \brief Enable/disable diurnal winds for a ninja
    *
    * \param nIndex index of a ninja
    * \param flag Enables diurnal winds if true, disables if false
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setDiurnalWinds( const int nIndex, const bool flag, char ** papszOptions=NULL );
    /**
    * \brief Set the air temperature and units for a ninja
    *
    * \param nIndex index of a ninja
    * \param temp air temperature value
    * \param units air temperature units
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setUniAirTemp( const int nIndex, const double temp,
                       const temperatureUnits::eTempUnits units,
                       char ** papszOptions=NULL );
    /**
    * \brief Set the air temperature and units for a ninja
    * Set the air temperature and temperature units for a ninja
    * given the ninja index, temperature value, and string-formatted units
    *
    * _Valid temperature units:_
    * - "K" = K (Kelvin)
    * - "C" = C (Celcius)
    * - "R" = R (Rankine)
    * - "F" = F (Fahrenheit)
    *
    * \param nIndex index of a ninja
    * \param temp air temperature value
    * \param units string-formatted temperature units
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setUniAirTemp( const int nIndex, const double temp,
                       std::string units, char ** papszOptions=NULL );
    /**
    * \brief Set the cloud coverage and units for a ninja
    *
    * \param nIndex index of a ninja
    * \param cloud_cover cloud coverage value
    * \param units units of the cloud coverage value
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setUniCloudCover( const int nIndex, const double cloud_cover,
                          const coverUnits::eCoverUnits units, char ** papszOptions=NULL );
    /**
    * \brief Set the cloud coverage and units for a ninja
    * Set the cloud coverage and corresponding units for a ninja given
    * a ninja index, the cloud coverage value, and string-formatted units.
    *
    * _Valid cloud coverage units:_
    * - "fraction" = fraction
    * - "percent"  = percent
    * - "canopy_category" = canopyCategories
    *
    *
    * \param nIndex index of a ninja
    * \param cloud_cover value of the cloud coverage
    * \param units units of the cloud coverage
    * \return
    */
    int setUniCloudCover( const int nIndex, const double cloud_cover,
                          std::string units, char ** papszOptions=NULL );
    /**
    * \brief Set the simulation date and time for a ninja
    *
    * \param nIndex index of a ninja
    * \param yr   simulation year
    * \param mo   simulation month
    * \param day  simulation day
    * \param hr   simulation hour
    * \param min  simulation minute
    * \param sec  simulation second
    * \param timeZoneString timezone of the date and time (see WINDNINJA_DATA/date_time_zonespec.csv)
    * \return
    */
    int setDateTime( const int nIndex, int const &yr, int const &mo, int const &day,
                     int const &hr, int const &min, int const &sec,
                     std::string const &timeZoneString, char ** papszOptions=NULL );        
                          
    
    /**
    * \brief Set the wxStation filename for a ninja
    *
    * \param nIndex index of a ninja
    * \param station_filename name of the wxStation file
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setWxStationFilename( const int nIndex, const std::string station_filename,
                              char ** papszOptions=NULL );
    /**
    * \brief Set the vegetation parameter for a ninja
    *
    * \param nIndex index of a ninja
    * \param vegetation_ the vegetation type
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setUniVegetation( const int nIndex,
                          const WindNinjaInputs::eVegetation vegetation_,
                          char ** papszOptions=NULL );
    /**
    * \brief Set the vegetation parameter for a ninja from a string
    * Set the vegetation parameter for a ninja from a properly formatted
    * string.
    *
    * _Valid vegetation strings:_
    * - "grass" = WindNinjaInputs::grass
    * - "g"     = WindNinjaInputs::grass
    * - "brush" = WindNinjaInputs::brush
    * - "b"     = WindNinjaInputs::brush
    * - "trees" = WindNinjaInputs::trees
    * - "t"     = WindNinjaInputs::trees
    *
    * \param
    * \return
    */
    int setUniVegetation( const int nIndex, std::string vegetation, char ** papszOptions=NULL );

    int setUniVegetation( const int nIndex, char ** papszOptions=NULL );
    /**
    * \brief Set the mesh resolution choice from a string for a ninja
    *  Set the mesh resolution choice given the index of a ninja and
    *  a proper choice string
    *
    *  _Valid choice strings:_
    *  - "coarse" = Mesh::coarse
    *  - "medium" = Mesh::medium
    *  - "fine"   = Mesh::fine
    *
    * \param nIndex index of a ninja
    * \param choice mesh resolution choice
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setMeshResolutionChoice( const int nIndex, const std::string choice,
                                 char ** papszOptions=NULL );
    /**
    * \brief Set the mesh resolution choice for a ninja
    *
    * \param nIndex index of a ninja
    * \param choice mesh resolution choice
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setMeshResolutionChoice( const int nIndex, const Mesh::eMeshChoice choice,
                                 char ** papszOptions=NULL );
    /**
    * \brief Set the mesh resolution for a ninja
    *
    * \param nIndex index of a ninja
    * \param resolution desired resolution value
    * \param units units of the resolution value
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setMeshResolution( const int nIndex, const double resolution,
                           const lengthUnits::eLengthUnits units,
                           char ** papszOptions=NULL );
    /**
    * \brief Set the mesh resolution for a ninja
    * Set the mesh resolution for a ninja given a resolution
    * and a string formatted unit.
    *
    * _Valid units include:_
    *  - "ft" = feet
    *  - "m"  = meters
    *  - "mi" = miles
    *  - "km" = kilometers
    *  - "ftx10" = feet times 10
    *  - "mx10"  = meters times 10
    *
    * \param nIndex index of a ninja
    * \param resolution desired resolution value
    * \param units string denoting which units resolution is in
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setMeshResolution( const int nIndex, const double resolution,
                           std::string units, char ** papszOptions=NULL );
    /**
    * \brief Set the number of vertical layers for a ninja
    *
    * \param nIndex index of a ninja
    * \param nLayers number of layers
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setNumVertLayers( const int nIndex, const int nLayers, char ** papszOptions=NULL );

    /*  Accessors  */
    std::vector<wxStation> getWxStations( const int nIndex, char ** papszOptions=NULL );
    /**
    * \brief Returns the flag indicating if diurnal winds are enabled for a ninja
    *
    * \param nIndex index of the ninja
    * \return diurnalWindFlag true if diurnal winds are enabled, false if disabled
    */
    bool getDiurnalWindFlag( const int nIndex, char ** papszOptions=NULL );
    /**
    * \brief Return the initialization method from a ninja as an eInitializationMethod type
    *
    * \param nIndex index of a ninja
    * \return init_method the initialization method of the ninja
    */
    WindNinjaInputs::eInitializationMethod getInitializationMethod( const int nIndex,
                                            char ** papszOptions=NULL );
    /**
    * \brief Return the initialization method type as a string from a ninja
    * Returns the initialization method type as a string from a ninja.
    *
    * _String formatted method types:_
    * - WindNinjaInputs::noInitializationFlag = "noInitializationFlag"
    * - WindNinjaInputs::domainAverageInitializationFlag = "domainAverageInitializationFlag"
    * - WindNinjaInputs::pointInitializationFlag = "pointInitializationFlag"
    * - WindNinjaInputs::wxModelInitializationFlag = "wxModelInitializationFlag"
    * - if no initialization method = ""
    *
    * \param nIndex index of a ninja
    * \return init_method string containing the initialization method type
    */
    std::string getInitializationMethodString( const int nIndex, char ** papszOptions=NULL );


    /*-----------------------------------------------------------------------------
     *  STABILITY section
     *-----------------------------------------------------------------------------*/
#ifdef STABILITY
    /**
    * \brief Enable/disable stability for a given ninja
    *
    * \param nIndex index of a ninja
    * \param flag Stability enabled if true, disabled if false
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setStabilityFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );
    /**
    * \brief Set the alpha stability for a given ninja
    *
    * \param nIndex index of a ninja
    * \param stability_ the alpha stability value
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setAlphaStability( const int nIndex, const double stability_, char ** papszOptions=NULL );
#endif //STABILITY

    /*-----------------------------------------------------------------------------
     *  Output Parameter Methods
     *-----------------------------------------------------------------------------*/
    /**
    * \brief Set the output path for a ninja
    *
    * \param nIndex index of a ninja
    * \param path path where output will be written
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setOutputPath( const int nIndex, std::string path,
                                 char ** papszOptions=NULL );
    
    /**
    * \brief Set the percent of output buffer clipping for a ninja
    *
    * \param nIndex index of a ninja
    * \param percent percent of the output buffer to clip
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setOutputBufferClipping( const int nIndex, const double percent,
                                 char ** papszOptions=NULL );
    /**
    * \brief Enable/disable the wxModel Google KML output for a ninja
    *
    * \param nIndex index of a ninja
    * \param flag Enabled if true, disabled if false
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setWxModelGoogOutFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );
    /**
    * \brief Enable/disable the wxModel SHP output for a ninja
    *
    * \param nIndex index of a ninja
    * \param flag Enabled if true, disabled if false
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setWxModelShpOutFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );
    /**
    * \brief Enable/disable the wxModel ASCII output for a ninja
    *
    * \param nIndex index of a ninja
    * \param flag Enabled if true, disabled if false
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setWxModelAsciiOutFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );
    /**
    * \brief Enable/disable Google KML output for a ninja
    *
    * \param nIndex index of a ninja
    * \param flag Enabled if true, disabled if false
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setGoogOutFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );
    /**
    * \brief Set the Google KML output resolution for a ninja
    *
    * \param nIndex index of a ninja
    * \param resolution desired resolution value
    * \param units units of the resolution value
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setGoogResolution( const int nIndex, const double resolution,
                           const lengthUnits::eLengthUnits units, char ** papszOptions=NULL );
    /**
    * \brief Set the Google KML output resolution for a ninja
    * Set the Google KML output resolution for a ninja given a resolution
    * and a string formatted unit.
    *
    * _Valid units include:_
    *  - "ft" = feet
    *  - "m"  = meters
    *  - "mi" = miles
    *  - "km" = kilometers
    *  - "ftx10" = feet times 10
    *  - "mx10"  = meters times 10
    *
    * \param nIndex index of a ninja
    * \param resolution desired resolution value
    * \param units string denoting which units resolution is in
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setGoogResolution( const int nIndex, const double resolution,
                           std::string units, char ** papszOptions=NULL );
    /**
    * \brief Set the Google KML output speed scaling parameter for a ninja
    *
    * \param nIndex index of a ninja
    * \param scaling scaling option
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setGoogSpeedScaling( const int nIndex, const KmlVector::egoogSpeedScaling scaling,
                             char ** papszOptions=NULL );
    /**
    * \brief Set the Google KML output speed scaling parameter for a ninja
    * Set the Google KML output speed scaling parameter for a ninja given
    * the ninja index and string formatted scaling option.
    *
    * _Valid scaling options_:
    * - "equal_color"    = equal_color
    * - "color"          = equal_color
    * - "equal_interval" = equal_interval
    * - "interval"       = equal_interval
    *
    * \param nIndex index of a ninja
    * \param scaling string formatted scaling units
    * \return
    */
    int setGoogSpeedScaling( const int nIndex, std::string scaling, char ** papszOptions=NULL);
    /**
    * \brief Set the Google KML output line width for a ninja
    *
    * \param nIndex index of a ninja
    * \param width value of desired line width
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setGoogLineWidth( const int nIndex, const double width, char ** papszOptions=NULL );
    /**
    * \brief Enable/disable SHP output for a ninja
    *
    * \param nIndex index of a ninja
    * \param flag   enable if true, disable if false
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setShpOutFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );
    /**
    * \brief Set the resoultion of SHP output for a ninja
    * Set the resolution of SHP output for a ninja given the resolution
    * and units.
    *
    * \param nIndex index of a ninja
    * \param resolution desired resolution value
    * \param units units of the resolution
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setShpResolution( const int nIndex, const double resolution,
                          const lengthUnits::eLengthUnits units, char ** papszOptions=NULL );
    /**
    * \brief Set the SHP output resolution for a ninja
    * Set the SHP output resolution fro a ninja given a resolution
    * and a string formatted unit.
    *
    * _Valid units include:_
    *  - "ft" = feet
    *  - "m"  = meters
    *  - "mi" = miles
    *  - "km" = kilometers
    *  - "ftx10" = feet times 10
    *  - "mx10"  = meters times 10
    *
    * \param nIndex index of a ninja
    * \param resolution desired resolution value
    * \param units string denoting which units resolution is in
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setShpResolution( const int nIndex, const double resolution,
                          std::string units, char ** papszOptions=NULL );
    /**
    * \brief Enable/disable ASCII output for a ninja
    *
    * \param nIndex index of a ninja
    * \param flag   enable if true, disable if false
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setAsciiOutFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );
    /**
    * \brief Set the resoultion of ASCII output for a ninja
    * Set the resolution of ASCII output for a ninja given the resolution
    * and units.
    *
    * \param nIndex index of a ninja
    * \param resolution desired resolution value
    * \param units units of the resolution
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setAsciiResolution( const int nIndex, const double resolution,
                            const lengthUnits::eLengthUnits units, char ** papszOptions=NULL );
    /**
    * \brief Set the resolution of ASCII output for a ninja
    * Set the resolution of ASCII output of a ninja given the resolution
    * and string formatted units.
    *
    * _Valid units include:_
    *  - "ft" = feet
    *  - "m"  = meters
    *  - "mi" = miles
    *  - "km" = kilometers
    *  - "ftx10" = feet times 10
    *  - "mx10"  = meters times 10
    *
    * \param nIndex index of a ninja
    * \param resolution desired resolution value
    * \param units string denoting which units resolution is in
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setAsciiResolution( const int nIndex, const double resolution,
                            std::string units, char ** papszOptions=NULL );
    /**
    * \brief Enable/disable VTK output for a ninja
    *
    * \param nIndex index of a ninja
    * \param flag   determines if VTK should be enabled or not
    * \return errval Returns NINJA_SUCCESS if successful
    */
    int setVtkOutFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );
    /**
    * \brief Enable/disable txt output for a ninja
    *
    * \param nIndex index of a ninja
    * \param flag   determines if txt output is enabled or not
    * \return errval Returns NINJA_SUCCESS if successful
    */
    int setTxtOutFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );
    /**
    * \brief Enable/disable PDF output for a ninja 
    *
    * \param nIndex index of a ninja
    * \param flag   determines if pdf output is enabled or not
    * \return errval Returns NINJA_SUCCESS if successful
    */
    int setPDFOutFlag( const int nIndex, const bool flag, char ** papszOptions=NULL );
    /**
    * \brief Set the resoultion of PDF output for a ninja
    * Set the resolution of PDF output for a ninja given the resolution
    * and units.
    *
    * \param nIndex index of a ninja
    * \param resolution desired resolution value
    * \param units units of the resolution
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setPDFResolution( const int nIndex, const double resolution,
                                     const lengthUnits::eLengthUnits units, 
                                     char ** papszOptions=NULL );
    /**
    * \brief Set the resolution of PDF output for a ninja
    * Set the resolution of PDF output of a ninja given the resolution
    * and string formatted units.
    *
    * _Valid units include:_
    *  - "ft" = feet
    *  - "m"  = meters
    *  - "mi" = miles
    *  - "km" = kilometers
    *  - "ftx10" = feet times 10
    *  - "mx10"  = meters times 10
    *
    * \param nIndex index of a ninja
    * \param resolution desired resolution value
    * \param units string denoting which units resolution is in
    * \return errval Returns NINJA_SUCCESS upon success
    */
    int setPDFResolution( const int nIndex, const double resolution,
                                     std::string units, char ** papszOptions=NULL );


    /* --------------------------------------------------------------------------*/
    /** 
     * @brief Configures the PDF output vector line width (defaults to 1.0)
     * 
     * @Param nIndex index of a ninja
     * @Param linewidth value of the desired line width ( > 0.0 )
     * 
     * @Returns errval NINJA_SUCCESS if linewidth correctly set
     */
    /* ----------------------------------------------------------------------------*/
    int setPDFLineWidth( const int nIndex, const float linewidth, char ** papszOptions=NULL );

    /**
     * \brief Set the background image of the PDF (default is attempt at topo
     *        map)
     *
     * \param nIndex index of a ninja
     * \param eType 0->hillshade, 1->topo map
     * \return NINJA_SUCCESS if valid type is provided.
     */
    int setPDFBaseMap( const int nIndex,
                       const int linewidth );

    int setPDFDEM( const int nIndex, const std::string dem_filename, char ** papszOptions=NULL );

    int setPDFSize( const int nIndex, const double height, const double width,
                    const unsigned short dpi );
    /**
    * \brief Returns the output path of a ninja
    *
    * \param nIndex index of a ninja
    * \return path String of the path, which is empty if no output is set
    */
    std::string getOutputPath( const int nIndex, char ** papszOptions=NULL );
    /*-----------------------------------------------------------------------------
     *  Termination Section
     *-----------------------------------------------------------------------------*/
    void reset();
    void cancel();
    void cancelAndReset();
    
    GDALDatasetH hSpdMemDS; //in-memory dataset for GTiff output writer
    GDALDatasetH hDirMemDS; //in-memory dataset for GTiff output writer
    GDALDatasetH hDustMemDS; //in-memory dataset for GTiff output writer

    std::vector<std::string> wxList;
protected:
    std::vector<ninja*> ninjas;
    std::string tz;

    bool writeFarsiteAtmFile;
    void writeFarsiteAtmosphereFile();
    void setAtmFlags();

    /*
    ** This function initializes various data for the lifetime of the
    ** ninjaArmy.  This should be used for various tasks, such as downloading
    ** the color relief for the background of the PDF file.  It is the same for
    ** all runs, is the same for all runs.
    */
    void initLocalData(void);
    void destoryLocalData(void);
    void copyLocalData( const ninjaArmy &A );

private:
    char *pszTmpColorRelief;
};

#endif /* NINJA_ARMY_H */
