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
    #define WN_C_START       extern "C" {
    #define WN_C_END         }
#else
    #define WN_C_START
    #define WN_C_END
#endif //__cplusplus

/*-----------------------------------------------------------------------------
 *  Handle Types for Internal WindNinja Classes
 *-----------------------------------------------------------------------------*/
//Ninja WINDNINJADLL_EXPORT NinjaCreateArmy(int nRuns);

#ifndef WINDNINJADLL_EXPORT
    #if defined(WIN32) && defined(WindNinjadll_EXPORTS)
        #define WINDNINJADLL_EXPORT _declspec(dllexport)
    #else
        #define WINDNINJADLL_EXPORT
    #endif //WIN32 && WINDNINJA_EXPORTS
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
typedef int  NinjaErr;


    /*-----------------------------------------------------------------------------
     *  Contructor/Destructors
     *-----------------------------------------------------------------------------*/
#ifndef NINJAFOAM
    WINDNINJADLL_EXPORT NinjaH* NinjaCreateArmy
        ( unsigned int numNinjas, char ** papszOptions  );
#endif
#ifdef NINJAFOAM
    WINDNINJADLL_EXPORT NinjaH* NinjaCreateArmy
        ( unsigned int numNinjas, int momentumFlag, char ** papszOptions  );
#endif
    WINDNINJADLL_EXPORT NinjaErr NinjaDestroyArmy
        ( NinjaH * ninja );

    /*-----------------------------------------------------------------------------
     *  Ninja Simulation Executors
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaErr NinjaStartRuns
        ( NinjaH * ninja, const unsigned int nprocessors );

    WINDNINJADLL_EXPORT NinjaErr NinjaMakeArmy
        ( NinjaH * ninja, const char * forecastFilename,
          const char * timezone,
          int momentumFlag );

WINDNINJADLL_EXPORT NinjaErr NinjaSetEnvironment
        ( const char *pszGdalData, const char *pszWindNinjaData );

    WINDNINJADLL_EXPORT NinjaErr NinjaInit
        ( );

    /*-----------------------------------------------------------------------------
     *  Various Simulation Parameters
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaErr NinjaSetDem
        ( NinjaH * ninja, const int nIndex, const char * fileName );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetInMemoryDem
        ( NinjaH * ninja, const int nIndex, const double * demValues,
          const int nXSize, const int nYSize, const double * geoRef, const char * prj );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetPosition
        ( NinjaH * ninja, const int nIndex );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetInitializationMethod
        ( NinjaH * ninja, const int nIndex, const char * initializationMethod );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetNumberCPUs
        ( NinjaH * ninja, const int nIndex, const int nCPUs );

    /*  Communication  */
    WINDNINJADLL_EXPORT NinjaErr NinjaSetCommunication
        ( NinjaH * ninja, const int nIndex, const char * comType );

    /*  Input Parameters  */
    WINDNINJADLL_EXPORT NinjaErr NinjaSetInputSpeed
        ( NinjaH * ninja, const int nIndex, const double speed,
          const char * units );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetInputDirection
        ( NinjaH * ninja, const int nIndex, const double direction );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetInputWindHeight
        ( NinjaH * ninja, const int nIndex, const double height, const char * units );

    /*  Output Parameters  */
    WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputWindHeight
        ( NinjaH * ninja, const int nIndex, const double height,
          const char * units );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputSpeedUnits
        ( NinjaH * ninja, const int nIndex, const char * units );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetDiurnalWinds
        ( NinjaH * ninja, const int nIndex, const int flag );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetUniAirTemp
        ( NinjaH * ninja, const int nIndex, const double temp,
          const char * units );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetUniCloudCover
        ( NinjaH * ninja, const int nIndex, const double cloud_cover,
          const char * units );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetDateTime
        ( NinjaH * ninja, const int nIndex, const int yr, const int mo,
          const int day, const int hr, const int min, const int sec,
          const char * timeZoneString );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetWxStationFilename
        ( NinjaH * ninja, const int nIndex, const char * station_filename );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetUniVegetation
        ( NinjaH * ninja, const int nIndex, const char * vegetation );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetNumVertLayers
        ( NinjaH * ninja, const int nIndex, const int nLayers );

    WINDNINJADLL_EXPORT char ** NinjaGetWxStations
        ( NinjaH * ninja, const int nIndex );

    WINDNINJADLL_EXPORT int NinjaGetDiurnalWindFlag
        ( NinjaH * ninja, const int nIndex );

    WINDNINJADLL_EXPORT const char * NinjaGetInitializationMethod
        ( NinjaH * ninja, const int nIndex );

    /*-----------------------------------------------------------------------------
     *  Dust Methods
     *-----------------------------------------------------------------------------*/
#ifdef EMISSIONS
    WINDNINJADLL_EXPORT NinjaErr NinjaSetDustFilename
        ( NinjaH * ninja, const int nIndex, const char* filename );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetDustFileOut
        ( NinjaH * ninja, const int nIndex, const char* filename );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetDustFlag
        ( NinjaH * ninja, const int nIndex, const int flag );
#endif //EMISSIONS

    /*-----------------------------------------------------------------------------
     *  Stability Methods
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaErr NinjaSetStabilityFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetAlphaStability
        ( NinjaH * ninja, const int nIndex, const double stability_ );

//#ifdef NINJAFOAM
    /*-----------------------------------------------------------------------------
     *  NinjaFoam Methods
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaErr NinjaSetMeshCount
        ( NinjaH * ninja, const int nIndex, const int meshCount );
//#endif //NINJAFOAM

    /*-----------------------------------------------------------------------------
     *  Mesh Methods
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaErr NinjaSetMeshResolutionChoice
        ( NinjaH * ninja, const int nIndex, const char * choice );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetMeshResolution
        ( NinjaH * ninja, const int nIndex, const double resolution,
         const char * units );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetNumVertLayers
        ( NinjaH * ninja, const int nIndex, int vertLayers );


    /*-----------------------------------------------------------------------------
     *  Output Methods
     *-----------------------------------------------------------------------------*/
    WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputPath
        ( NinjaH * ninja, const int nIndex, const char * path);

    WINDNINJADLL_EXPORT const double* NinjaGetOutputSpeedGrid
        ( NinjaH * ninja, const int nIndex );

    WINDNINJADLL_EXPORT const double* NinjaGetOutputDirectionGrid
        ( NinjaH * ninja, const int nIndex );

    WINDNINJADLL_EXPORT const char* NinjaGetOutputGridProjection
        ( NinjaH * ninja, const int nIndex );

    WINDNINJADLL_EXPORT const double NinjaGetOutputGridCellSize
        ( NinjaH * ninja, const int nIndex );

    WINDNINJADLL_EXPORT const double NinjaGetOutputGridxllCorner
        ( NinjaH * ninja, const int nIndex );

    WINDNINJADLL_EXPORT const double NinjaGetOutputGridyllCorner
        ( NinjaH * ninja, const int nIndex );

    WINDNINJADLL_EXPORT const int NinjaGetOutputGridnCols
        ( NinjaH * ninja, const int nIndex );

    WINDNINJADLL_EXPORT const int NinjaGetOutputGridnRows
        ( NinjaH * ninja, const int nIndex );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetOutputBufferClipping
        ( NinjaH * ninja, const int nIndex, const double percent );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetWxModelGoogOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetWxModelShpOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetWxModelAsciiOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogResolution
        ( NinjaH * ninja, const int nIndex, const double resolution,
          const char * units );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogSpeedScaling
        ( NinjaH * ninja, const int nIndex, const char * scaling );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetGoogLineWidth
        ( NinjaH * ninja, const int nIndex, const double width );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetShpOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetShpResolution
        ( NinjaH * ninja, const int nIndex, const double resolution,
          const char * units );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetAsciiOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetAsciiResolution
        ( NinjaH * ninja, const int nIndex, const double resolution,
          const char * units );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetVtkOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    WINDNINJADLL_EXPORT NinjaErr NinjaSetTxtOutFlag
        ( NinjaH * ninja, const int nIndex, const int flag );

    WINDNINJADLL_EXPORT const char * NinjaGetOutputPath
        ( NinjaH * ninja, const int nIndex );


    /*-----------------------------------------------------------------------------
     *  Termination Methods
     *-----------------------------------------------------------------------------*/

    WINDNINJADLL_EXPORT NinjaErr NinjaReset( NinjaH * ninja );
    WINDNINJADLL_EXPORT NinjaErr NinjaCancel( NinjaH * ninja );
    WINDNINJADLL_EXPORT NinjaErr NinjaCancelAndReset( NinjaH * ninja );

WN_C_END
