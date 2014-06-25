/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  QThread that fires n threads for the solver
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

#include "solveThread.h"

/** 
 * QThread for running all of the simulations
 * 
 * @param nProcs number of threads
 * @param a army to run on
 * 
 * @return true on success
 */
bool solveThread::run(int nProcs, ninjaArmy a)
{
    return a.startRuns(nProcs);
}

/** 
 * Distribute ninja runs over the allotted number of threads
 * 
 * @param numRuns number of ninja runs
 * @param numProcessors number of processors to use
 * @param windsim pointer to an array of ninjas
 * 
 * @return false on failure, true otherwise
 */
bool solveThread::startRunsQt(int numRuns, int numProcessors, ninja *windsim)
{
    bool ninjaSuccess = true;
    QString exceptString;

    bool *retval = new bool[numRuns];
    for(int i = 0;i < numRuns;i++)
	retval[i] = true;
   
    if(numRuns < 1 || numProcessors < 1)
	return false;
  
#ifdef _OPENMP
    omp_set_nested(false);
    //omp_set_dynamic(true); 
#endif
  
    if(numRuns == 1) {
	//set number of threads for the run
	windsim[0].set_numberCPUs( numProcessors );
	//start the run
	try {
	    retval[0] = windsim[0].simulate_wind();
	}
	catch  (std::bad_alloc& e) {
	    exceptString = "Exception bad_alloc caught: " + QString( e.what() );
	    exceptString += ". WindNinja appears to have run out of memory.";
	    ninjaSuccess = false;
	}
	catch (cancelledByUser& e) {
	    exceptString = "Exception caught: " + QString( e.what() );
	    ninjaSuccess = false;
	}
	catch (std::exception& e) {
	    exceptString = "Exception caught: " + QString( e.what() );
	    ninjaSuccess = false;
	}
	catch (...) {
	    exceptString =  "Exception caught: Cannot determine exception type.";
	    ninjaSuccess = false;
	}
	//writeToConsole( exceptString, Qt::red );
    }
    else {
#ifdef _OPENMP
	omp_set_num_threads(numProcessors);
#endif
      
#pragma omp parallel for schedule(static, 1) //spread runs on single threads
	for(int j = 0; j < numRuns; j++) {
	    //start the run
	    try {
		retval[j] = windsim[j].simulate_wind();	//runs are done on 1 thread each since omp_set_nested(false)
	    }
	    catch  (std::bad_alloc& e) {
		exceptString = "Exception bad_alloc caught: " + QString( e.what() );
		exceptString += ". WindNinja appears to have run out of memory.";
		ninjaSuccess = false;
	    }
	    catch (cancelledByUser& e) {
		exceptString = "Exception caught: " + QString( e.what() );
		ninjaSuccess = false;
	    }
	    catch (std::exception& e) {
		exceptString = "Exception caught: " + QString( e.what() );
		ninjaSuccess = false;
	    }
	    catch (...) {
		exceptString =  "Exception caught: Cannot determine exception type.";
		ninjaSuccess = false;
	    }
	    //writeToConsole( exceptString, Qt::red );
	}
    }
    for(int i = 0;i < numRuns;i++) {
	if (retval[i] == false)
	    ninjaSuccess = false;
    }
    return ninjaSuccess;
}
