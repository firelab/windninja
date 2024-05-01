/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Application for creating a slope and aspect grid
 * Author:   Loren Atwoodr <loren.atwood@usda.gov>
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

#include "Elevation.h"
#include "ascii_grid.h"
#include "Aspect.h"
#include "Slope.h"
#include "ninja_conv.h"
#include "ninja_init.h"


void Usage(const char *pszError)
{
    printf("slope_aspect_grid [--c/--output-cell-size size]\n"
           "           [--nd/--ndecimals-out-precision ndecimals]\n"
#ifdef _OPENMP
           "           [--n/--num-threads n]\n"
#endif
           "           input_dem_file output_path\n"
           "\n"
           "Defaults:\n"
           "    --nd/--ndecimals 2\n"
           "    --n/--num-threads 1\n");
    if(pszError)
    {
        fprintf(stderr, "%s\n", pszError);
    }
    exit(1);
}

int main(int argc, char *argv[])
{
    NinjaInitialize();
    
    double startTotal, endTotal;
    
#ifdef _OPENMP
    startTotal = omp_get_wtime();
#endif

    int nNumThreads = 1;
    int nDecimals = 2;
    double dfCellSize = -1;
    const char *pszInputDemFile = NULL;
    const char *pszOutputPath = NULL;
    
    int i = 1;
    while(i < argc)
    {
        if(EQUAL(argv[i], "--output-cell-size") || EQUAL(argv[i], "--c"))
        {
            dfCellSize = atof(argv[++i]);
        }
        else if(EQUAL(argv[i], "--ndecimals-out-precision") || EQUAL(argv[i], "--nd"))
        {
            nDecimals = atoi(argv[++i]);
        }
        else if(EQUAL(argv[i], "--num-threads") || EQUAL(argv[i], "--n"))
        {
            nNumThreads = atoi(argv[++i]);
        }
        else if(EQUAL(argv[i], "--help") || EQUAL(argv[i], "--h"))
        {
            Usage(NULL);
        }
        else if(pszInputDemFile == NULL)
        {
            pszInputDemFile = argv[i];
        }
        else if(pszOutputPath == NULL)
        {
            pszOutputPath = argv[i];
        }
        else
        {
            Usage(NULL);
        }
        i++;
    }
    if(pszInputDemFile == NULL || pszOutputPath == NULL)
    {
        Usage("Please Enter a valid input dem file and output path");
    }
    //if(dfCellSize <= 0)
    //    Usage("Invalid value for output-cell-size");
    //else if(nDecimals <= 0)
    if(nDecimals <= 0)
        Usage("Invalid value for ndecimals-out-precision");
    else if(nNumThreads <= 0)
        Usage("Invalid value for num-threads");
    
    // correct the path if it dropped the "/" on the end
    if(pszOutputPath[strlen(pszOutputPath)-1] != "/")
        pszOutputPath = CPLSPrintf("%s/", pszOutputPath);

    CPLDebug("SLOPE_ASPECT", "pszOutputPath = %s", pszOutputPath);
    
    // Read in elevation
    Elevation elev;
    elev.GDALReadGrid(pszInputDemFile, 1);
    if(dfCellSize > 0)
        elev.resample_Grid_in_place(dfCellSize, AsciiGrid<double>::order1);
    
#ifdef _OPENMP
    omp_set_num_threads(nNumThreads);
#endif
    
    // make aspect and slope grids
    Aspect asp(&elev,nNumThreads);
    Slope slp(&elev,nNumThreads);
    
    // set the prjString from the dem prjString
    asp.set_prjString(elev.prjString);
    slp.set_prjString(elev.prjString);
    
    // define the output file names from the path and the cell size
    const char *pszOutputSlopeFile = CPLSPrintf("%s%s", pszOutputPath, "slope");
    const char *pszOutputAspectFile = CPLSPrintf("%s%s", pszOutputPath, "aspect");
    if(dfCellSize > 0)
    {
        pszOutputSlopeFile = CPLSPrintf("%s_%dm", pszOutputSlopeFile, int(dfCellSize));
        pszOutputAspectFile = CPLSPrintf("%s_%dm", pszOutputAspectFile, int(dfCellSize));
        
        const char *pszOutputDemFile = CPLSPrintf("%sdem_%dm.asc", pszOutputPath, int(dfCellSize));
        elev.write_Grid(pszOutputDemFile, nDecimals);
    }
    pszOutputSlopeFile = CPLSPrintf("%s.asc", pszOutputSlopeFile);
    pszOutputAspectFile = CPLSPrintf("%s.asc", pszOutputAspectFile);
    
    asp.write_Grid(pszOutputAspectFile, nDecimals);
    slp.write_Grid(pszOutputSlopeFile, nDecimals);
    
#ifdef _OPENMP
    endTotal = omp_get_wtime();
#endif
    
    std::cout << "Total time = " << endTotal - startTotal << std::endl;
    
    return 0;
}

