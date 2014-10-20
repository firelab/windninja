/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Function that runs multiple instances using openMP
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


#include "startRuns.h"

/*	//this is the startRuns function Jason tried to make, but OpenMP doesn't have the functionality needed yet...
bool startRuns(int numRuns, int numProcessors, ninja *windsim)
{
	//Function takes as input the desired number of runs, the number of processors(threads) to use,
	//and a pointer to an array of ninja classes
	//The function starts up the wind runs maximizing the number of processors desired.

	int Integer, Left, Done, i, j;

	if(numRuns<1 || numProcessors<1)
		return false;

	#ifdef _OPENMP
		omp_set_nested(true);
		//omp_set_dynamic(true); 
	#endif

	Integer = numProcessors/numRuns;
	if(Integer > 0)	//if number of processors is equal to or greater than the number of runs
	{
		#ifdef _OPENMP
		omp_set_num_threads(Integer);
		#endif

		//for(i=0; i<numRuns; i++)
		//{
			//set number of threads for the run
		//	windsim[i].set_numberCPUs(Integer);
		//}

		#pragma omp parallel for private(i)
		for(i=0; i<numRuns; i++)
		{
			//set number of threads for the run
			windsim[i].set_numberCPUs(Integer);
			//start the run
 			windsim[i].simulate_wind();

		}
	}else		//if number of processors is less than the number of runs
	{
		Integer = numRuns/numProcessors;

		#ifdef _OPENMP
		omp_set_num_threads(numProcessors);
		#endif

		//for(i=0; i<Integer; i++)	//spread runs on single threads here
		//{
		//	for(j=0; j<numProcessors; j++)
		//	{
				//set number of threads for the run
		//		windsim[i*numProcessors+j].set_numberCPUs(1);
		//	}
		//}

		for(i=0; i<Integer; i++)	//spread runs on single threads here
		{
			#pragma omp parallel for private(j) firstprivate(i)
			for(j=0; j<numProcessors; j++)
			{
				//set number of threads for the run
				windsim[i*numProcessors+j].set_numberCPUs(1);
				//start the run
				windsim[i*numProcessors+j].simulate_wind();
			}
		}
		
		//take care of remaining runs, using multiple processors for runs if possible
		Done = Integer*numProcessors;
		Left = numRuns - Done;
		if(Left == 0)
			return true;
		Integer = numProcessors/Left;

		#ifdef _OPENMP
		omp_set_num_threads(Left);
		#endif

		//for(i=0; i<Left; i++)
		//{
			//set number of threads for the run
		//	windsim[i+Done].set_numberCPUs(Integer);
		//}

		#pragma omp parallel for private(i)
		for(i=0; i<Left; i++)
		{
			//set number of threads for the run
			windsim[i+Done].set_numberCPUs(Integer);
			//start the run
			windsim[i+Done].simulate_wind();
		}
	}
	
	return true;
}
*/

/**Function to start WindNinja core runs using multiple threads.
 *
 * @param numRuns Number of runs to perform.
 * @param numProcessors Number of processors to use.
 * @param windsim Pointer to an array of ninja classes.
 * @return True if runs complete properly.
 */
bool startRuns(int numRuns, int numProcessors, ninja *windsim)
{
	//Function takes as input the desired number of runs, the number of processors(threads) to use,
	//and a pointer to an array of ninja classes
	//The function starts up the wind runs maximizing the number of processors desired.

	farsiteAtm atmosphere;
	for( int i=0; i<numRuns; i++ )
    {
		atmosphere.push(windsim[i].get_date_time(), 
                        windsim[i].get_VelFileName(),
                        windsim[i].get_AngFileName(),
                        windsim[i].get_CldFileName() );
    }
	atmosphere.writeAtmFile(std::string("farsite_atmosphere.atm"), 
                            windsim[0].get_outputSpeedUnits(),
                            windsim[0].get_outputWindHeight() );

	int j;

	if(numRuns<1 || numProcessors<1)
		return false;

	#ifdef _OPENMP
		omp_set_nested(false);
		//omp_set_dynamic(true); 
	#endif

	if(numRuns == 1)
	{
		//set number of threads for the run
		windsim[0].set_numberCPUs( numProcessors );
		//start the run
		if( !windsim[0].simulate_wind() )
			printf("Return of false from simulate_wind()");

	}
    else
	{
		for( j=0; j < numRuns; j++ )
			windsim[j].set_numberCPUs(1);
		#ifdef _OPENMP
		omp_set_num_threads(numProcessors);
		#endif

		#pragma omp parallel for //spread runs on single threads
		for(j=0; j<numRuns; j++)
		{
			//start the run
			windsim[j].simulate_wind();	//runs are done on 1 thread each since omp_set_nested(false)
		}
	}
	
	return true;
}
