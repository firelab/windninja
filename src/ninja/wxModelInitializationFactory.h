/******************************************************************************
 *
 * $Id: wxModelInitializationFactory.h 
 *
 * Project:  WindNinja
 * Purpose:  Factory class for wxModelInitialization derived classes 
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

#ifndef WX_MODEL_INITIALIZATION_FACTORY_H 
#define WX_MODEL_INITIALIZATION_FACTORY_H 

#include "wxModelInitialization.h"
#include "ncepGfsSurfInitialization.h"
#include "ncepHrrrSurfInitialization.h"
#include "ncepNamAlaskaSurfInitialization.h"
#include "ncepNamGrib2SurfInitialization.h"
#include "ncepNamSurfInitialization.h"
#include "ncepNdfdInitialization.h"
#include "ncepRapSurfInitialization.h"
#include "genericSurfInitialization.h"
#include "wrfSurfInitialization.h"
#include "wrf3dInitialization.h"
#include "ArchivedHRRRinitialization.h"

#ifdef WITH_NOMADS_SUPPORT
#include "nomads_wx_init.h"
#endif


/*
 * =====================================================================================
 *        Class:  WxModelInitializationFactory
 *  Description:  Factory class for identifying model types based on the file type, then
 *                returning an instantiated surface model of the given type. 
 * =====================================================================================
 */
class wxModelInitializationFactory
{
    public:

        static wxModelInitialization* makeWxInitialization( std::string fileName );
        static wxModelInitialization* makeWxInitializationFromId( std::string identifier );


}; 
/* -----  end of class WxModelInitializationFactory  ----- */


#endif //WX_MODEL_INITIALIZATION_FACTORY_H
