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

#include "ninja_errors.h"

/*-----------------------------------------------------------------------------
 *  Macros for Compilation Compatibility with gcc and g++
 *-----------------------------------------------------------------------------*/
#ifdef __cplusplus
#   define WN_C_START       extern "C" {
#   define WN_C_END         }
#else
#   define WN_C_START
#   define WN_C_END
#endif //__cplusplus

/*-----------------------------------------------------------------------------
 *  Handle Types for Internal Wind Ninja Classes
 *-----------------------------------------------------------------------------*/
//Ninja WINDNINJADLL_EXPORT NinjaCreateArmy(int nRuns);

#ifndef WINDNINJADLL_EXPORT
#if defined(WIN32) && defined(WindNinja_EXPORTS)
#  define WINDNINJADLL_EXPORT _declspec(dllexport)
#else
#  define WINDNINJADLL_EXPORT
#endif //WIN32 && WindNinja_EXPORTS
#endif //WINDNINJADLL_EXPORT

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

WN_C_START

#include <stdlib.h>
//#include <stdint.h>

//Use structs instead of void * for type checking by C compilier
struct NinjaH;
typedef struct NinjaH NinjaH;
typedef int   NinjaErr;


    /*-----------------------------------------------------------------------------
     *  Contructor/Destructors
     *-----------------------------------------------------------------------------*/
    #ifndef NINJAFOAM
    NinjaH* WINDNINJADLL_EXPORT NinjaCreateArmy
        ( unsigned int numNinjas, char ** papszOptions  );
    #endif
    #ifdef NINJAFOAM
    NinjaH* WINDNINJADLL_EXPORT NinjaCreateArmy
        ( unsigned int numNinjas, bool momentumFlag, char ** papszOptions  );
    #endif
    NinjaErr WINDNINJADLL_EXPORT NinjaDestroyArmy
        ( NinjaH * ninja );

    /*-----------------------------------------------------------------------------
     *  Ninja Simulation Executors
     *-----------------------------------------------------------------------------*/
    NinjaErr WINDNINJADLL_EXPORT NinjaStartRuns
        ( NinjaH * ninja, const unsigned int nprocessors );

    NinjaErr WINDNINJADLL_EXPORT NinjaMakeArmy
        ( NinjaH * ninja, const char * forecastFilename,
          const char * timezone,
          bool momentumFlag );


    /*-----------------------------------------------------------------------------
     *  Various Simulation Parameters
     *-----------------------------------------------------------------------------*/
    NinjaErr WINDNINJADLL_EXPORT NinjaSetInitializationMethod
        (NinjaH * ninja, const int nIndex, const char * initializationMethod );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetNumberCPUs
        ( NinjaH * ninja, const int nIndex, const int nCPUs );

    /*  Input Parameters  */
    NinjaErr WINDNINJADLL_EXPORT NinjaSetInputSpeed
        ( NinjaH * ninja, const int nIndex, const double speed,
          const char * units );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetInputDirection
        ( NinjaH * ninja, const int nIndex, const double direction );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetInputWindHeight
        ( NinjaH * ninja, const int nIndex, const double height, const char * units );

    /*  Output Parameters  */
    NinjaErr WINDNINJADLL_EXPORT NinjaSetOutputWindHeight
        ( NinjaH * ninja, const int nIndex, const double height,
          const char * units );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetOutputSpeedUnits
        ( NinjaH * ninja, const int nIndex, const char * units );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetDiurnalWinds
        ( NinjaH * ninja, const int nIndex, const int flag );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetUniAirTemp
        ( NinjaH * ninja, const int nIndex, const double temp,
          const char * units );
    NinjaErr WINDNINJADLL_EXPORT NinjaSetUniCloudCover
        ( NinjaH * ninja, const int nIndex, const double cloud_cover,
          const char * units );
    NinjaErr WINDNINJADLL_EXPORT NinjaSetDateTime
        ( NinjaH * ninja, const int nIndex, const int yr, const int mo,
          const int day, const int hr, const int min, const int sec,
          const char * timeZoneString );
    NinjaErr WINDNINJADLL_EXPORT NinjaSetWxStationFilename
        ( NinjaH * ninja, const int nIndex, const char * station_filename );
    NinjaErr WINDNINJADLL_EXPORT NinjaSetUniVegetation
        ( NinjaH * ninja, const int nIndex, const char * vegetation );
    NinjaErr WINDNINJADLL_EXPORT NinjaSetNumVertLayers
        ( NinjaH * ninja, const int nIndex, const int nLayers );
    char ** WINDNINJADLL_EXPORT NinjaGetWxStations
        ( NinjaH * ninja, const int nIndex );
    int WINDNINJADLL_EXPORT NinjaGetDiurnalWindFlag
        ( NinjaH * ninja, const int nIndex );
    const char * WINDNINJADLL_EXPORT NinjaGetInitializationMethod
        ( NinjaH * ninja, const int nIndex );

    /*-----------------------------------------------------------------------------
     *  Dust Methods
     *-----------------------------------------------------------------------------*/
#ifdef EMISSIONS
    NinjaErr WINDNINJADLL_EXPORT NinjaSetDustFilename
        (NinjaH * ninja, const int nIndex, const char* filename );
    NinjaErr WINDNINJADLL_EXPORT NinjaSetDustFileOut
        ( NinjaH * ninja, const int nIndex, const char* filename );
    NinjaErr WINDNINJADLL_EXPORT NinjaSetDustFlag
        ( NinjaH * ninja, const int nIndex, const int flag );
#endif //EMISSIONS

    /*-----------------------------------------------------------------------------
     *  Stability Methods
     *-----------------------------------------------------------------------------*/
#ifdef STABILITY
    NinjaErr WINDNINJADLL_EXPORT NinjaSetStabilityFlag
        ( NinjaH * ninja, const int nIndex, const int flag );
    NinjaErr WINDNINJADLL_EXPORT NinjaSetAlphaStability
        ( NinjaH * ninja, const int nIndex, const double stability_ );
#endif //Stability

    /*-----------------------------------------------------------------------------
     *  Scalar Methods
     *-----------------------------------------------------------------------------*/
#ifdef SCALAR
    NinjaErr WINDNINJADLL_EXPORT NinjaSetScalarTransportFlag
        ( NinjaH * ninja, const int nIndex, const int flag );
    NinjaErr WINDNINJADLL_EXPORT NinjaSetScalarSourceStrength
        ( NinjaH * ninja, const int nIndex, const double source_ );
    NinjaErr WINDNINJADLL_EXPORT NinjaSetScalarSourceXcoord
        ( NinjaH * ninja, const int nIndex, const double xcoord_ );
    NinjaErr WINDNINJADLL_EXPORT NinjaSetScalarSourceYcoord
        ( NinjaH * ninja, const int nIndex, const double ycoord_ );
#endif //SCALAR

    /*-----------------------------------------------------------------------------
     *  Mesh Methods
     *-----------------------------------------------------------------------------*/
    NinjaErr WINDNINJADLL_EXPORT NinjaSetMeshResolutionChoice
        ( NinjaH * ninja, const int nIndex, const char * choice );

	NinjaErr WINDNINJADLL_EXPORT NinjaSetMeshResolution
        (NinjaH * ninja, const int nIndex, const double resolution,
         const char * units );

	NinjaErr WINDNINJADLL_EXPORT NinjaSetNumVertLayers
        (NinjaH * ninja, const int nIndex, int vertLayers );


    /*-----------------------------------------------------------------------------
     *  Output Methods
     *-----------------------------------------------------------------------------*/
    NinjaErr WINDNINJADLL_EXPORT NinjaSetOutputBufferClipping
        ( NinjaH * ninja, const int nIndex, const double percent );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetWxModelGoogOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetWxModelShpOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetWxModelAsciiOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetGoogOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetGoogResolution
        ( NinjaH * ninja, const int nIndex, const double resolution,
          const char * units );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetGoogSpeedScaling
        ( NinjaH * ninja, const int nIndex, const char * scaling );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetGoogLineWidth
        ( NinjaH * ninja, const int nIndex, const double width );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetShpOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetShpResolution
        ( NinjaH * ninja, const int nIndex, const double resolution,
          const char * units );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetAsciiOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetAsciiResolution
        ( NinjaH * ninja, const int nIndex, const double resolution,
          const char * units );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetVtkOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    NinjaErr WINDNINJADLL_EXPORT NinjaSetTxtOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    const char * WINDNINJADLL_EXPORT NinjaGetOutputPath
        ( NinjaH * ninja, const int nIndex );


    /*-----------------------------------------------------------------------------
     *  Termination Methods
     *-----------------------------------------------------------------------------*/

    NinjaErr WINDNINJADLL_EXPORT NinjaReset( NinjaH * ninja );
    NinjaErr WINDNINJADLL_EXPORT NinjaCancel( NinjaH * ninja );
    NinjaErr WINDNINJADLL_EXPORT NinjaCancelAndReset( NinjaH * ninja );

WN_C_END












