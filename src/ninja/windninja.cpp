/******************************************************************************
 *
 * $Id: ninja_conv.cpp 2258 2013-03-18 23:50:59Z kyle.shannon $
 *
 * Project:  WindNinja
 * Purpose:  C API
 * Author:   Levi Malott <lmnn3@mst.edu>
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

#include "windninja.h"
#include "ninjaArmy.h"
#include "ninjaException.h"
#include <string>

#ifdef _OPENMP
    omp_lock_t netCDF_lock;
#endif

/**
 * \file windninja.cpp
 *
 * Public C API for windninja.
 */

NinjaErr handleException()
{
    try
    {
        throw;
    }
    catch( std::bad_alloc& e )
    {
        return NINJA_E_BAD_ALLOC;
    }
    catch( cancelledByUser& e )
    {
        return NINJA_E_CANCELLED;
    }
    catch( std::exception& e )
    {
        return NINJA_E_OTHER;
    }
    catch( ... )
    {
        return NINJA_E_UNKNOWN;
    }
}


extern "C"
{
/**
 * \brief Create a new suite of windninja runs.
 *
 * Use this method to create a finite, known number of runs for windninja.
 * There are other creation methods that automatically allocate the correct
 * number of runs for the input type.
 *
 * \see NinjaMakeArmy
 *
 * Avaliable Creation Options:
 *                             None
 *
 * \param numNinjas The number of runs to create.
 * \param papszOptions Key, value option pairs from the options listed above.
 *
 * \return An opaque handle to a ninjaArmy on success, NULL otherwise.
 */

#ifndef NINJAFOAM
WINDNINJADLL_EXPORT NinjaH* NinjaCreateArmy
    ( unsigned int numNinjas, char ** papszOptions  )
{
    try
    {
        return reinterpret_cast<NinjaH*>( new ninjaArmy( numNinjas ) );
    }
    catch( bad_alloc& )
    {
        return NULL;
    }
}
#endif

#ifdef NINJAFOAM
WINDNINJADLL_EXPORT NinjaH* NinjaCreateArmy
    ( unsigned int numNinjas, int momentumFlag, char ** papszOptions  )
{
    try
    {
        return reinterpret_cast<NinjaH*>( new ninjaArmy( numNinjas, momentumFlag ) );
    }
    catch( bad_alloc& )
    {
        return NULL;
    }
}
#endif

/**
 * \brief Destroy a suite of windninja runs.
 *
 * Destory the ninjaArmy and free all associated memory.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 *
 * \return NINJA_SUCCESS on success, NINJA_E_NULL_PTR on failure, although this
 *                       can be ignored.  The error is only returned if the
 *                       handle was null.  In this case, the function is a
 *                       no-op.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaDestroyArmy
    ( NinjaH * ninja )
{
    if( NULL != ninja )
    {
       delete reinterpret_cast<ninjaArmy*>( ninja );
       ninja = NULL;
       return NINJA_SUCCESS;
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Automatically allocate and generate a ninjaArmy from a forecast file.
 *
 * This method will create a set of runs for windninja based on the contents of
 * the weather forecast file.  One run is done for each timestep in the *.nc
 * file.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param forecastFilename A valid thredds/UCAR based weather model file.
 * \param timezone a timezone string representing a valid timezone, e.g.
 *                 America/Boise.
 *                 See WINDNINJA_DATA/date_time_zonespec.csv
 *
 * \return NINJA_SUCCESS on success, NINJA_E_INVALID otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaMakeArmy
    ( NinjaH * ninja, const char * forecastFilename,
      const char * timezone,
      int momentumFlag )
{
    NinjaErr retval = NINJA_E_INVALID;
    if( NULL != ninja )
    {
       try
       {
           reinterpret_cast<ninjaArmy*>( ninja )->makeArmy
               ( std::string( forecastFilename ),
                 std::string( timezone ),
                 momentumFlag );

           retval = NINJA_SUCCESS;
       }
       catch( armyException & e )
       {
           retval = NINJA_E_INVALID;
       }
    }
    return retval;
}

/**
 * \brief Start the simulations.
 *
 * Run all of the members of the ninjaArmy using one or more processors.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nprocessors number of processors to use when compiled with OpenMP
 *                    support.
 *
 * \return NINJA_SUCCESS on succes, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaStartRuns
    ( NinjaH * ninja, const unsigned int nprocessors )
{
    if( NULL != ninja )
    {
        try
        {
            return reinterpret_cast<ninjaArmy*>( ninja )->startRuns( nprocessors );
        }
        catch( ... )
        {
            return handleException();
        }
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the initialization method.
 *
 * Set the initialization method for a single run.  There are currently three
 * initialization methods, descibed below.
 *
 * Domain-Averaged Initialization
 * ------------------------------
 * key -> "domain_average"
 *
 * Description: Initialize the entire field with a constant speed and
 *              direction.
 *
 * Point Initialization
 * ---------------------
 * key-> "point"
 *
 * Description: Initialize the field using point data described in a weather
 *              model file.  This is a file consisting of a location an weather
 *              observations for a point somewhere on the landscape.  The point
 *              *does not* have to be within the domain of the input DEM or
 *              LCP.  The file format is discussed in the documentation for
 *              NinjaSetWxStationFilename
 * \see NinjaSetWxStationFilename
 *
 * Weather Model Initialization
 * ----------------------------
 * key-> "wxmodel"
 *
 * Description: Initialize the field using coarse-scale weather model data.
 *              These data come from UCAR/thredds through version 2.2.0,
 *              although it may change in the future.  This initialization
 *              method should be used in conjuction with NinjaMakeArmy.
 * \see NinjaMakeArmy
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param initializationMethod a string representation of a valid
 *                             initialization method (see 'key' above).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetInitializationMethod
    (NinjaH * ninja, const int nIndex, const char * initializationMethod )
{
    if( NULL != ninja && NULL != initializationMethod )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setInitializationMethod
            ( nIndex, std::string( initializationMethod ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT NinjaErr NinjaSetEnvironment
    (const char *pszGdalData, const char *pszWindNinjaData )

{
    //set GDAL_DATA and WINDNINJA_DATA
    GDALAllRegister();
    OGRRegisterAll();    

    CPLSetConfigOption( "GDAL_HTTP_UNSAFESSL", "YES");

    CPLDebug( "WINDNINJA", "Setting GDAL_DATA:%s", pszGdalData );
    CPLSetConfigOption( "GDAL_DATA", pszGdalData );

    CPLDebug( "WINDNINJA", "Setting WINDNINJA_DATA:%s", pszWindNinjaData );
    CPLSetConfigOption( "WINDNINJA_DATA", pszWindNinjaData );

    globalTimeZoneDB.load_from_file(FindDataPath("date_time_zonespec.csv"));

    return NINJA_SUCCESS;
}
        
WINDNINJADLL_EXPORT NinjaErr NinjaInit
    ( )
{
    NinjaErr retval = NINJA_E_INVALID;

    retval = NinjaInitialize();

    return retval;
}

/**
 * \brief Set the number of CPUs to use for simulations.
 *
 * \note Only valid with OpenMP support
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param nCPUs Thread count.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetNumberCPUs
    ( NinjaH * ninja, const int nIndex, const int nCPUs )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setNumberCPUs( nIndex, nCPUs );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the communication handler for simulations.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param comType Type of communication. For now, comType is always "cli".
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetCommunication
    ( NinjaH * ninja, const int nIndex, const char * comType )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setNinjaCommunication
            ( nIndex, std::string( comType ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the DEM to use for the simulations.
 *
 * \see NinjaSetInMemoryDem
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param fileName Path to a valid DEM file on disk.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetDem
    ( NinjaH * ninja, const int nIndex, const char * fileName)
{
    if( NULL != ninja && NULL != fileName )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setDEM
            ( nIndex, std::string( fileName ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set an in-memory DEM to use for the simulations.
 *
 * \note NinjaSetOutputPath must be called if an in-memory DEM
 *       is used.
 *
 * \see NinjaSetPosition
 * \see NinjaSetDem
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param demValues An array of elevation values (must be in meters).
 * \param nXSize The number of pixels in the x-direction.
 * \param nYSize The number of pixels in the y-direction.
 * \param geoRef The georeferencing transform.
 *
 *               geoRef[0]  top left x
 *               geoRef[1]  w-e pixel resolution
 *               geoRef[2]  rotational coefficient, zero for north up images
 *               geoRef[3]  top left y
 *               geoRef[4]  rotational coefficient, zero for north up images
 *               geoRef[5]  n-s pixel resolution (negative value)
 *
 * \param prj The projection definition string.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetInMemoryDem
    ( NinjaH * ninja, const int nIndex, const double * demValues,
      const int nXSize, const int nYSize, const double * geoRef, const char * prj )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setDEM
            ( nIndex, demValues, nXSize, nYSize, geoRef, std::string( prj ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set additional information related to the DEM.
 *
 * \note Must be called after NinjaSetDem is called. This function should
 *       not be called if NinjaSetInMemoryDem is called.
 *
 * \see NinjaSetDem
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetPosition
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setPosition
            ( nIndex );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the input wind speed for a domain-average simulation.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param speed The input speed.
 * \param units The input speed units ("mph", "mps", "kph", "kts").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetInputSpeed
    ( NinjaH * ninja, const int nIndex, const double speed,
      const char * units )
{
    if( NULL != ninja && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setInputSpeed
            ( nIndex, speed, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the input wind direction for a domain-average simulation.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param speed The input direction.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetInputDirection
    ( NinjaH * ninja, const int nIndex, const double direction )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setInputDirection( nIndex, direction );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the input wind height for a domain-average simulation.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param speed The input wind height above the canopy.
 * \param units The input wind height units ("ft", "m").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetInputWindHeight
    ( NinjaH * ninja, const int nIndex, const double height, const char * units )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setInputWindHeight
                    ( nIndex, height, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the output wind height for a domain-average simulation.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param speed The output wind height above the canopy.
 * \param units The output wind height units ("ft", "m").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputWindHeight
    ( NinjaH * ninja, const int nIndex, const double height,
      const char * units )
{
    if( NULL != ninja && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setOutputWindHeight
            ( nIndex, height, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the output wind speed units.
 *
 * \note This function currently only applies to outputs
 *       written to disk. In-memory wind speed output units
 *       are mps.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param units The output speed units ("mph", "mps", "kph", "kts").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputSpeedUnits
    ( NinjaH * ninja, const int nIndex, const char * units )
{
    if( NULL != ninja && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setOutputSpeedUnits
            ( nIndex, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the diurnal flag for a simulation.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag on = 1, off = 2.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetDiurnalWinds
    ( NinjaH * ninja, const int nIndex, const int flag )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setDiurnalWinds( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set a uniform air temperture for a domain-average simulation.
 *
 * \note This function only needs to be called if diurnal winds are on.
 *
 * \see NinjaSetDiurnalWinds
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param temp Air temperature.
 * \param units Air temperature units ("K", "C", "R", "F").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetUniAirTemp
    ( NinjaH * ninja, const int nIndex, const double temp,
      const char * units )
{
    if( NULL != ninja && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setUniAirTemp
            ( nIndex, temp, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set a uniform cloud cover for a domain-average simulation.
 *
 * \note This function only needs to be called if diurnal winds are on.
 *
 * \see NinjaSetDiurnalWinds
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param cloud_cover Cloud cover.
 * \param units Cloud cover units ("percent", "fraction").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetUniCloudCover
    ( NinjaH * ninja, const int nIndex, const double cloud_cover,
      const char * units )
{
    if( NULL != ninja && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setUniCloudCover
            ( nIndex, cloud_cover, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the date and time for a domain-average simulation.
 *
 * \note This function only needs to be called if diurnal winds are on.
 *
 * \see NinjaSetDiurnalWinds
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param yr Year.
 * \param mo Month.
 * \param day Day.
 * \param hr Hour.
 * \param min Minute.
 * \param sec Second.
 * \param timeZoneString Time zone string. Can be set to "auto-detect".
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetDateTime
    ( NinjaH * ninja, const int nIndex, const int yr, const int mo,
      const int day, const int hr, const int min, const int sec,
      const char * timeZoneString )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setDateTime
            ( nIndex, yr, mo, day, hr, min, sec, std::string( timeZoneString ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set a weather station file name for a point simulation.
 *
 * \note Only valid if point initialization is used.
 *
 * \see NinjaSetInitializationMethod
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param station_filename Weather station file name.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetWxStationFilename
    ( NinjaH * ninja, const int nIndex, const char * station_filename )
{
    if( NULL != ninja )
    {
         return reinterpret_cast<ninjaArmy*>( ninja )->setWxStationFilename
            ( nIndex, std::string( station_filename ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set a uniform vegeation cover to use for a simulation.
 *
 * \note Not valid if a Landscape (*.lcp) file is used.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param vegetation Vegetation option to use ("grass", "brush", "trees").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetUniVegetation
    ( NinjaH * ninja, const int nIndex, const char * vegetation )
{
    if( NULL != ninja && NULL != vegetation )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setUniVegetation
            ( nIndex, std::string( vegetation ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT char ** NinjaGetWxStations
    ( NinjaH * ninja, const int nIndex );

/**
 * \brief Get the diurnal flag set for a simulation.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return flag indicating whether or not the diurnal parameterization is on (1 = on, 0 = off).
 */
WINDNINJADLL_EXPORT int NinjaGetDiurnalWindFlag
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->getDiurnalWindFlag( nIndex );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Get the initialization method for a simulation.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return string indicating the initialization method.
 */
WINDNINJADLL_EXPORT const char * NinjaGetInitializationMethod
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja )
    {
        return strdup(reinterpret_cast<ninjaArmy*>(ninja)->getInitializationMethodString(nIndex).c_str());
    }
    else
    {
        return NULL;
    }
}

/*-----------------------------------------------------------------------------
 *  Dust Methods
 *-----------------------------------------------------------------------------*/
#ifdef EMISSIONS
WINDNINJADLL_EXPORT NinjaErr NinjaSetDustFilename
    (NinjaH * ninja, const int nIndex, const char* filename )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setDustFilename
                ( nIndex, std::string(filename) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT NinjaErr NinjaSetDustFlag
    ( NinjaH * ninja, const int nIndex, const int flag )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setDustFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}
#endif //EMISSIONS

/*-----------------------------------------------------------------------------
 *  Stability Methods
 *-----------------------------------------------------------------------------*/
WINDNINJADLL_EXPORT NinjaErr NinjaSetStabilityFlag
    ( NinjaH * ninja, const int nIndex, const int flag )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setStabilityFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT NinjaErr NinjaSetAlphaStability
    ( NinjaH * ninja, const int nIndex, const double stability_ )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setAlphaStability( nIndex, stability_ );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/*-----------------------------------------------------------------------------
 *  NinjaFoam Methods
 *-----------------------------------------------------------------------------*/
#ifdef NINJAFOAM
/**
 * \brief Set the mesh count for a simulation.
 *
 * \note Only for use with the momentum solver.
 *
 * \see NinjaCreateArmy
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param meshCount The number of cells to use in the mesh.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetMeshCount
    ( NinjaH * ninja, const int nIndex, const int meshCount )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setMeshCount
            ( nIndex, meshCount );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}
#endif //NINJAFOAM

/*-----------------------------------------------------------------------------
 *  Mesh Methods
 *-----------------------------------------------------------------------------*/
/**
 * \brief Set the mesh resolution choice.
 *
 * \see NinjaSetMeshResolution
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param choice The mesh resolution choice ("fine", "medium", "coarse").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetMeshResolutionChoice
    ( NinjaH * ninja, const int nIndex, const char * choice )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setMeshResolutionChoice
            ( nIndex, std::string( choice ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the mesh resolution.
 *
 * \see NinjaSetMeshResolutionChoice
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param choice The mesh resolution.
 * \param units The mesh resolution units ("ft", "m").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetMeshResolution
    (NinjaH * ninja, const int nIndex, const double resolution, const char * units )
{
    if( NULL != ninja && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setMeshResolution
            ( nIndex, resolution, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the number of vertical layers in the mesh.
 *
 * \note Only for use with the conservation of mass solver.
 *
 * \see NinjaCreateArmy
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param nLayers The number of layers to use (20 is typcial).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr  NinjaSetNumVertLayers
    ( NinjaH * ninja, const int nIndex, const int nLayers )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setNumVertLayers( nIndex, nLayers );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/*-----------------------------------------------------------------------------
 *  Output Methods
 *-----------------------------------------------------------------------------*/
/**
 * \brief Set the output path for a simulation.
 *
 * \note This must be set if an in-memory DEM is used and outputs are written
 *       to disk. The path must exist, it will not be created at runtime.
 *
 * \see NinjaSetInMemoryDem
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param path The full path where outputs should be written.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputPath
    ( NinjaH * ninja, const int nIndex, const char * path)
{
    if( NULL != ninja ){
        return reinterpret_cast<ninjaArmy*>( ninja )->setOutputPath( nIndex, std::string( path ) );
    } else {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Get the output speed grid from a simulation.
 *
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridxllCorner
 * \see NinjaGetOutputGridyllCorner
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return An array of speed values in mps.
 */
WINDNINJADLL_EXPORT const double* NinjaGetOutputSpeedGrid
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja ) {
           return reinterpret_cast<ninjaArmy*>( ninja )->getOutputSpeedGrid( nIndex );
    } else {
        return NULL;
    }
}

/**
 * \brief Get the output direction grid from a simulation.
 *
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridxllCorner
 * \see NinjaGetOutputGridyllCorner
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return An array of direction values.
 */
WINDNINJADLL_EXPORT const double* NinjaGetOutputDirectionGrid
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja ) {
        return reinterpret_cast<ninjaArmy*>( ninja )->getOutputDirectionGrid( nIndex );
    } else {
        return NULL;
    }
}

/**
 * \brief Get the output grid projection string from a simulation.
 *
 * \see NinjaGetOutputSpeedGrid
 * \see NinjaGetOutputDirectionGrid
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridxllCorner
 * \see NinjaGetOutputGridyllCorner
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return The output grid projeciton string.
 */
WINDNINJADLL_EXPORT const char* NinjaGetOutputGridProjection
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja ) {
        return reinterpret_cast<ninjaArmy*>( ninja )->getOutputGridProjection( nIndex );
    } else {
        return NULL;
    }
}

/**
 * \brief Get the output grid cell size from a simulation.
 *
 * \see NinjaGetOutputSpeedGrid
 * \see NinjaGetOutputDirectionGrid
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridxllCorner
 * \see NinjaGetOutputGridyllCorner
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return The output grid cell size in m.
 */
WINDNINJADLL_EXPORT const double NinjaGetOutputGridCellSize
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja ) {
        return reinterpret_cast<ninjaArmy*>( ninja )->getOutputGridCellSize( nIndex );
    } else {
        throw std::runtime_error("no ninjaArmy");
    }
}

/**
 * \brief Get the x-coordinate of the lower left corner of the output grid from a simulation.
 *
 * \see NinjaGetOutputSpeedGrid
 * \see NinjaGetOutputDirectionGrid
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridyllCorner
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return The lower left x-coordinate (in m) of the output grid.
 */
WINDNINJADLL_EXPORT const double NinjaGetOutputGridxllCorner
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja ) {
        return reinterpret_cast<ninjaArmy*>( ninja )->getOutputGridxllCorner( nIndex );
    } else {
        throw std::runtime_error("no ninjaArmy");
    }
}

/**
 * \brief Get the y-coordinate of the lower left corner of the output grid from a simulation.
 *
 * \see NinjaGetOutputSpeedGrid
 * \see NinjaGetOutputDirectionGrid
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridxllCorner
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return The lower left y-coordinate (in m) of the output grid.
 */
WINDNINJADLL_EXPORT const double NinjaGetOutputGridyllCorner
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja ) {
        return reinterpret_cast<ninjaArmy*>( ninja )->getOutputGridyllCorner( nIndex );
    } else {
        throw std::runtime_error("no ninjaArmy");
    }
}

/**
 * \brief Get the number of columns in the output grid from a simulation.
 *
 * \see NinjaGetOutputSpeedGrid
 * \see NinjaGetOutputDirectionGrid
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridxllCorner
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return The number of columns in the output grid.
 */
WINDNINJADLL_EXPORT const int NinjaGetOutputGridnCols
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja ) {
        return reinterpret_cast<ninjaArmy*>( ninja )->getOutputGridnCols( nIndex );
    } else {
        throw std::runtime_error("no ninjaArmy");
    }
}

/**
 * \brief Get the number of rows in the output grid from a simulation.
 *
 * \see NinjaGetOutputSpeedGrid
 * \see NinjaGetOutputDirectionGrid
 * \see NinjaGetOutputGridProjection
 * \see NinjaGetOutputGridCellSize
 * \see NinjaGetOutputGridxllCorner
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 *
 * \return The number of rows in the output grid.
 */
WINDNINJADLL_EXPORT const int NinjaGetOutputGridnRows
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja ) {
        return reinterpret_cast<ninjaArmy*>( ninja )->getOutputGridnRows( nIndex );
    } else {
        throw std::runtime_error("no ninjaArmy");
    }
}

/**
 * \brief Set the output buffer clipping for a simulation.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param percent The percent by which to clip the output.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputBufferClipping
    ( NinjaH * ninja, const int nIndex, const double percent )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setOutputBufferClipping( nIndex, percent );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the flag to write the weather model winds used for initialzation as
 *        a Google Earth file.
 *
 * \note Only valid if wxModelInitialization is used.
 *
 * \see NinjaSetInitializationMethod
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not the weather model winds will be
 *             written as a Google Earth file (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetWxModelGoogOutFlag
    ( NinjaH * ninja, const int nIndex, const int flag )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setWxModelGoogOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the flag to write the weather model winds used for initialzation as
 *        a shapefile.
 *
 * \note Only valid if wxModelInitialization is used.
 *
 * \see NinjaSetInitializationMethod
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not the weather model winds will be
 *             written as a shapefile (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetWxModelShpOutFlag
    ( NinjaH * ninja, const int nIndex, const int flag )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setWxModelShpOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}

/**
 * \brief Set the flag to write the weather model winds used for initialzation as
 *        a raster file.
 *
 * \note Only valid if wxModelInitialization is used.
 *
 * \see NinjaSetInitializationMethod
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not the weather model winds will be
 *             written as a raster file (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetWxModelAsciiOutFlag
    ( NinjaH * ninja, const int nIndex, const int flag )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setWxModelAsciiOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}

/**
 * \brief Set the flag to write Google Earth output for a simulation.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not Google Earth output will be
 *             written (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogOutFlag
    ( NinjaH * ninja, const int nIndex, const int flag )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setGoogOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}

/**
 * \brief Set the resolution of the Google Earth output for a simulation.
 *
 * \note Only valid if NinjaSetGoogOutFlag is set to 1.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param resolution The resolution at which to write the Google Earth output.
 * \param units The units of the Google Earth output resolution ("ft", "m").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogResolution
    ( NinjaH * ninja, const int nIndex, const double resolution,
      const char * units )
{
    if( NULL != ninja && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setGoogResolution
            ( nIndex, resolution, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogSpeedScaling
    ( NinjaH * ninja, const int nIndex, const char * scaling )
{
    if( NULL != ninja && NULL != scaling )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setGoogSpeedScaling
            ( nIndex, std::string( scaling ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogLineWidth
    ( NinjaH * ninja, const int nIndex, const double width )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setGoogLineWidth( nIndex, width );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the flag to write shapefile output for a simulation.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not shapefile output will be
 *             written (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetShpOutFlag
    ( NinjaH * ninja, const int nIndex, const int flag )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setShpOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}

/**
 * \brief Set the resolution of the shapefile output for a simulation.
 *
 * \note Only valid if NinjaSetShpOutFlag is set to 1.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param resolution The resolution at which to write the shapefile output.
 * \param units The units of the shapefile output resolution ("ft", "m").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetShpResolution
    ( NinjaH * ninja, const int nIndex, const double resolution,
      const char * units )
{
    if( NULL != ninja && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setShpResolution
            ( nIndex, resolution, std::string( units ) );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the flag to write raster output for a simulation.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not raster output will be
 *             written (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetAsciiOutFlag
    ( NinjaH * ninja, const int nIndex, const int flag )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setAsciiOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}

/**
 * \brief Set the resolution of the raster output for a simulation.
 *
 * \note Only valid if NinjaSetAsciiOutFlag is set to 1.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param resolution The resolution at which to write the raster output.
 * \param units The units of the raster output resolution ("ft", "m").
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetAsciiResolution
    ( NinjaH * ninja, const int nIndex, const double resolution,
      const char * units )
{
    if( NULL != ninja && NULL != units )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setAsciiResolution
            ( nIndex, resolution, units );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

/**
 * \brief Set the flag to write VTK output for a simulation.
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nIndex The run to apply the setting to.
 * \param flag The flag which determines whether or not VTK output will be
 *             written (0 = no, 1 = yes).
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */
WINDNINJADLL_EXPORT NinjaErr NinjaSetVtkOutFlag
    ( NinjaH * ninja, const int nIndex, const int flag )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setVtkOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT NinjaErr NinjaSetTxtOutFlag
    ( NinjaH * ninja, const int nIndex, const int flag )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setTxtOutFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }

}

WINDNINJADLL_EXPORT const char * NinjaGetOutputPath
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja ) {
        return strdup(reinterpret_cast<ninjaArmy*>( ninja )->getOutputPath( nIndex ).c_str());
    } else {
        return NULL;
    }
}

/*-----------------------------------------------------------------------------
 *  Termination Methods
 *-----------------------------------------------------------------------------*/

WINDNINJADLL_EXPORT NinjaErr NinjaReset( NinjaH * ninja )
{
    if( NULL != ninja )
    {
        reinterpret_cast<ninjaArmy*>( ninja )->reset();
        return NINJA_SUCCESS;
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT NinjaErr NinjaCancel( NinjaH * ninja )
{
    if( NULL != ninja )
    {
        reinterpret_cast<ninjaArmy*>( ninja )->cancel();
        return NINJA_SUCCESS;
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT NinjaErr NinjaCancelAndReset( NinjaH * ninja )
{
    if( NULL != ninja )
    {
        reinterpret_cast<ninjaArmy*>( ninja )->cancelAndReset();
        return NINJA_SUCCESS;
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

}
