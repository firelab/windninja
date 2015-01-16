/******************************************************************************
 *
 * $Id:$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  main() function to initiate cli
 * Author:   Kyle Shannon <ksshannon@gmail.com>
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

#include "cli.h"
#include "ninjaArmy.h"
#include "ninjaException.h"
#include "ninja_init.h"

#include "gdal.h"
#include "ogr_api.h"

#ifdef _OPENMP
    omp_lock_t netCDF_lock;
#endif

/**
 * @brief Main function for WindNinja.
 * Allow for a few options:
 * If CORE_RUN is defined, force a core run
 * The 'core run' is a editable command line run.
 * @see core_run.h
 * @param argc argument count
 * @param argv argument value(s)
 */
int main(int argc, char *argv[])
{
    CPLSetConfigOption( "NINJA_DISABLE_CALL_HOME", "ON" );
    NinjaInitialize();
    int result;
#ifdef _OPENMP
    omp_init_lock (&netCDF_lock);
#endif

#ifdef CORE_RUN
    result = coreMain(argc, argv);
#else
    result = windNinjaCLI(argc, argv);
#endif
#ifdef _OPENMP
    omp_destroy_lock (&netCDF_lock);
#endif

    return result;
}
