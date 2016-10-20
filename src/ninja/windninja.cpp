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
NinjaH* WINDNINJADLL_EXPORT NinjaCreateArmy
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
NinjaH* WINDNINJADLL_EXPORT NinjaCreateArmy
    ( unsigned int numNinjas, bool momentumFlag, char ** papszOptions  )
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
NinjaErr WINDNINJADLL_EXPORT NinjaDestroyArmy
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
NinjaErr WINDNINJADLL_EXPORT NinjaMakeArmy
    ( NinjaH * ninja, const char * forecastFilename,
      const char * timezone,
      bool momentumFlag )
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

NinjaErr WINDNINJADLL_EXPORT NinjaStartRuns
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetInitializationMethod
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

/**
 * \brief Set the number of CPUs to use for simulations.
 *
 * \note Only valid with OpenMP support
 *
 * \param ninja An opaque handle to a valid ninjaArmy.
 * \param nCPUs Thread count.
 *
 * \return NINJA_SUCCESS on success, non-zero otherwise.
 */

NinjaErr WINDNINJADLL_EXPORT NinjaSetNumberCPUs
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetInputSpeed
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetInputDirection
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetInputWindHeight
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetOutputWindHeight
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetOutputSpeedUnits
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetDiurnalWinds
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetUniAirTemp
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetUniCloudCover
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetDateTime
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetWxStationFilename
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetUniVegetation
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetMeshResolutionChoice
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetNumVertLayers
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

char ** WINDNINJADLL_EXPORT NinjaGetWxStations
    ( NinjaH * ninja, const int nIndex );

int WINDNINJADLL_EXPORT NinjaGetDiurnalWindFlag
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
const char * WINDNINJADLL_EXPORT NinjaGetInitializationMethod
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetDustFilename
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetDustFlag
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetStabilityFlag
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetAlphaStability
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetScalarTransportFlag
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetScalarSourceStrength
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetScalarXcoord
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetScalarYcoord
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

/*-----------------------------------------------------------------------------
 *  Mesh Methods
 *-----------------------------------------------------------------------------*/
NinjaErr WINDNINJADLL_EXPORT NinjaSetMeshResolution
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
/*-----------------------------------------------------------------------------
 *  Output Methods
 *-----------------------------------------------------------------------------*/
NinjaErr WINDNINJADLL_EXPORT NinjaSetOutputBufferClipping
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetWxModelGoogOutFlag
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetWxModelShpOutFlag
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetWxModelAsciiOutFlag
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
NinjaErr WINDNINJADLL_EXPORT NinjaSetGoogOutFlag
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetGoogResolution
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetGoogSpeedScaling
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetGoogLineWidth
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetShpOutFlag
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetShpResolution
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetAsciiOutFlag
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetAsciiResolution
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetVtkOutFlag
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

NinjaErr WINDNINJADLL_EXPORT NinjaSetTxtOutFlag
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

const char * WINDNINJADLL_EXPORT NinjaGetOutputPath
    ( NinjaH * ninja, const int nIndex )
{
    if( NULL != ninja )
    {
        return reinterpret_cast<ninjaArmy*>( ninja )->getOutputPath( nIndex ).c_str();
    }
    else
    {
        return NULL;
    }
}

/*-----------------------------------------------------------------------------
 *  Termination Methods
 *-----------------------------------------------------------------------------*/

NinjaErr WINDNINJADLL_EXPORT NinjaReset( NinjaH * ninja )
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

NinjaErr WINDNINJADLL_EXPORT NinjaCancel( NinjaH * ninja )
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

NinjaErr WINDNINJADLL_EXPORT NinjaCancelAndReset( NinjaH * ninja )
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
