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
            handleException();
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

WINDNINJADLL_EXPORT const char * NinjaGetInitializationMethod
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->getInitializationMethodString
            ( nIndex ).c_str();
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
#ifdef STABILITY
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
#endif //Stability

/*-----------------------------------------------------------------------------
 *  Scalar Methods
 *-----------------------------------------------------------------------------*/
#ifdef SCALAR
WINDNINJADLL_EXPORT NinjaErr NinjaSetScalarTransportFlag
    ( NinjaH * ninja, const int nIndex, const int flag )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setScalarTransportFlag( nIndex, flag );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT NinjaErr NinjaSetScalarSourceStrength
    ( NinjaH * ninja, const int nIndex, const double source_ )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setScalarSourceStrength( nIndex, source_ );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT NinjaErr NinjaSetScalarXcoord
    ( NinjaH * ninja, const int nIndex, const double xcoord_ )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setScalarXcoord( nIndex, xcoord_ );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}

WINDNINJADLL_EXPORT NinjaErr NinjaSetScalarYcoord
    ( NinjaH * ninja, const int nIndex, const double ycoord_ )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setScalarYcoord( nIndex, ycoord_ );
    }
    else
    {
        return NINJA_E_NULL_PTR;
    }
}
#endif //SCALAR

#ifdef NINJAFOAM
/*-----------------------------------------------------------------------------
 *  NinjaFoam Methods
 *-----------------------------------------------------------------------------*/
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
WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputPath
    ( NinjaH * ninja, const int nIndex, const char * path)
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->setOutputPath( nIndex, std::string( path ) );
    }
}

WINDNINJADLL_EXPORT const double* NinjaGetOutputSpeedGrid
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja )
    {
           return reinterpret_cast<ninjaArmy*>( ninja )->getOutputSpeedGrid( nIndex );
    }
}

WINDNINJADLL_EXPORT const double* NinjaGetOutputDirectionGrid
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja )
    {
           return reinterpret_cast<ninjaArmy*>( ninja )->getOutputDirectionGrid( nIndex );
    }
}

WINDNINJADLL_EXPORT const char* NinjaGetOutputGridProjection
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja )
    {
           return reinterpret_cast<ninjaArmy*>( ninja )->getOutputGridProjection( nIndex );
    }
}

WINDNINJADLL_EXPORT const double NinjaGetOutputGridCellSize
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja )
    {
           return reinterpret_cast<ninjaArmy*>( ninja )->getOutputGridCellSize( nIndex );
    }
}

WINDNINJADLL_EXPORT const double NinjaGetOutputGridxllCorner
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja )
    {
           return reinterpret_cast<ninjaArmy*>( ninja )->getOutputGridxllCorner( nIndex );
    }
}

WINDNINJADLL_EXPORT const double NinjaGetOutputGridyllCorner
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja )
    {
           return reinterpret_cast<ninjaArmy*>( ninja )->getOutputGridyllCorner( nIndex );
    }
}

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
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->getOutputPath( nIndex ).c_str();
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
