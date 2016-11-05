/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Abstract base class for initializing WindNinja wind fields
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

#include "initialize.h"

initialize::initialize()
{

}

initialize::~initialize()
{	
    height.deallocate();
    uDiurnal.deallocate();
    vDiurnal.deallocate();
    wDiurnal.deallocate();
}

void initialize::initializeWindToZero( Mesh const& mesh,
                                    wn_3dScalarField& u0,
                                    wn_3dScalarField& v0,
                                    wn_3dScalarField& w0)
{
    int i, j, k;

    //initialize u0, v0, w0 equal to zero
    #pragma omp parallel for default(shared) private(i,j,k)
    for(k=0;k<mesh.nlayers;k++)
    {
        for(i=0;i<mesh.nrows;i++)
        {
            for(j=0;j<mesh.ncols;j++)
            {
                u0(i, j, k) = 0.0;
                v0(i, j, k) = 0.0;
                w0(i, j, k) = 0.0;
            }
        }
    }

}

void initialize::initializeDiurnal(WindNinjaInputs& input,
                         AsciiGrid<double>& cloud,
                         AsciiGrid<double>& L,
                         AsciiGrid<double>& u_star,
                         AsciiGrid<double>& bl_height,
                         AsciiGrid<double>& airTempGrid,
                         AsciiGrid<double>& speedInitializationGrid)
{
    //Monin-Obukhov length, surface friction velocity, and atmospheric boundary layer height
    L.set_headerData(input.dem.get_nCols(),
                     input.dem.get_nRows(),
                     input.dem.get_xllCorner(),
                     input.dem.get_yllCorner(),
                     input.dem.get_cellSize(),
                     input.dem.get_noDataValue(),
                     0.0);

    u_star.set_headerData(input.dem.get_nCols(),
                         input.dem.get_nRows(),
                         input.dem.get_xllCorner(),
                         input.dem.get_yllCorner(),
                         input.dem.get_cellSize(),
                         input.dem.get_noDataValue(),
                         0.0);

    bl_height.set_headerData(input.dem.get_nCols(),
                        input.dem.get_nRows(),
                        input.dem.get_xllCorner(),
                        input.dem.get_yllCorner(),
                        input.dem.get_cellSize(),
                        input.dem.get_noDataValue(),
                        -1.0);

    //compute diurnal wind, Monin-Obukhov length, surface friction velocity, and ABL height
    if(input.diurnalWinds == true)
    {
        //height of diurnal flow above "z=0" in log profile
        height.set_headerData(input.dem.get_nCols(),
                        input.dem.get_nRows(),
                        input.dem.get_xllCorner(),
                        input.dem.get_yllCorner(),
                        input.dem.get_cellSize(),
                        input.dem.get_noDataValue(),
                        0);

        uDiurnal.set_headerData(input.dem.get_nCols(),
                        input.dem.get_nRows(),
                        input.dem.get_xllCorner(),
                        input.dem.get_yllCorner(),
                        input.dem.get_cellSize(),
                        input.dem.get_noDataValue(),
                        0);
 
        vDiurnal.set_headerData(input.dem.get_nCols(),
                        input.dem.get_nRows(),
                        input.dem.get_xllCorner(),
                        input.dem.get_yllCorner(),
                        input.dem.get_cellSize(),
                        input.dem.get_noDataValue(),
                        0);

        wDiurnal.set_headerData(input.dem.get_nCols(),
                        input.dem.get_nRows(),
                        input.dem.get_xllCorner(),
                        input.dem.get_yllCorner(),
                        input.dem.get_cellSize(), 
                        input.dem.get_noDataValue(),
                        0);

        double aspect_temp = 0;	//just placeholder, basically
        double slope_temp = 0;	//just placeholder, basically

        Solar solar(input.ninjaTime, input.latitude, input.longitude, aspect_temp, slope_temp);
        Aspect aspect(&input.dem, input.numberCPUs);
        Slope slope(&input.dem, input.numberCPUs);
        Shade shade(&input.dem, solar.get_theta(), solar.get_phi(), input.numberCPUs);

        addDiurnal diurnal(&uDiurnal, &vDiurnal, &wDiurnal, &height, &L,
                        &u_star, &bl_height, &input.dem, &aspect, &slope,
                        &shade, &solar, &input.surface, &cloud,
                        &airTempGrid, input.numberCPUs, input.downDragCoeff,
                        input.downEntrainmentCoeff, input.upDragCoeff,
                        input.upEntrainmentCoeff);

    }else{	//compute neutral ABL height

        int i, j, k;

        double f;

        //compute f -> Coriolis parameter
        if(input.latitude<=90.0 && input.latitude>=-90.0)
        {
            f = (1.4544e-4) * sin(pi/180 * input.latitude);	// f = 2 * omega * sin(theta)
            // f should be about 10^-4 for mid-latitudes
            // (1.4544e-4) here is 2 * omega = 2 * (2 * pi radians) / 24 hours = 1.4544e-4 seconds^-1
            // obtained from Stull 1988 book
            if(f<0)
                f = -f;
            }else{
            f = 1e-4;	//if latitude is not available, set f to mid-latitude value
        }

        if(f==0.0)	//zero will give division by zero below
            f = 1e-8;	//if latitude is zero, set f small

        //compute neutral ABL height
#pragma omp parallel for default(shared) private(i,j)
        for(i=0;i<input.dem.get_nRows();i++)
        {
            for(j=0;j<input.dem.get_nCols();j++)
            {
                u_star(i,j) = speedInitializationGrid(i,j)*0.4/
                            (log((input.inputWindHeight+input.surface.Rough_h(i,j)-
                                  input.surface.Rough_d(i,j))/input.surface.Roughness(i,j)));

                //compute neutral ABL height
                //from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
                bl_height(i,j) = 0.2 * u_star(i,j) / f;	
            }
        }
    }
}
