/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Implementation file for Initialization Factory Class
 * Author:   Natalie Wagenbrenner <nwagnebrenner@gmail.com>
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


#include "initializationFactory.h"

/**
 * Create an initialization based on the initialization method
 *
 * @param initMethod
 *
 * @return an initialize object
 */
initialize* initializationFactory::makeInitialization(WindNinjaInputs& input)
{
    if(input.initializationMethod==WindNinjaInputs::noInitializationFlag)
        throw std::runtime_error("The initialization method has not been set yet.");
    else if(input.initializationMethod==WindNinjaInputs::wxModelInitializationFlag){
        return wxModelInitializationFactory::makeWxInitialization(input.forecastFilename);
    }
    else if(input.initializationMethod==WindNinjaInputs::domainAverageInitializationFlag) {
        return new domainAverageInitialization;
    }
    else if(input.initializationMethod==WindNinjaInputs::pointInitializationFlag) {
        return new pointInitialization;
    }
    else if(input.initializationMethod==WindNinjaInputs::griddedInitializationFlag) {
        return new griddedInitialization;
    }
    else if(input.initializationMethod==WindNinjaInputs::foamDomainAverageInitializationFlag) {
        return new foamDomainAverageInitialization;
    }
    else if(input.initializationMethod==WindNinjaInputs::foamWxModelInitializationFlag) {
        return new foamWxModelInitialization;
    }else{
        std::ostringstream outString;
        outString << "The initialization method was set improperly.";
        throw std::runtime_error(outString.str());
    }
}


