/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  A concrete class for initializing WindNinja wind fields using
 *			 the domain average wind input method
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

#include "domainAverageInitialization.h"

domainAverageInitialization::domainAverageInitialization() : initialize()
{

}

domainAverageInitialization::~domainAverageInitialization()
{

}

void domainAverageInitialization::initializeFields(WindNinjaInputs &input,
                Mesh const& mesh,
                wn_3dScalarField& u0,
                wn_3dScalarField& v0,
                wn_3dScalarField& w0,
                AsciiGrid<double>& cloud)
{
    setGridHeaderData(input, cloud);

    setInitializationGrids(input);

    initializeWindToZero(mesh, u0, v0, w0);

    initializeBoundaryLayer(input);

    initializeWindFromProfile(input, mesh, u0, v0, w0);

    if((input.diurnalWinds==true) && (profile.profile_switch==windProfile::monin_obukov_similarity))
    {
        addDiurnalComponent(input, mesh, u0, v0, w0);
    }

    cloud = cloudCoverGrid;
}

void domainAverageInitialization::setInitializationGrids(WindNinjaInputs& input)
{
    //set initialization grids
    speedInitializationGrid = input.inputSpeed;
    dirInitializationGrid = input.inputDirection;
    airTempGrid = input.airTemp;
    setCloudCover(input);

    for(int i=0; i<speedInitializationGrid.get_nRows(); i++) {
        for(int j=0; j<speedInitializationGrid.get_nCols(); j++) {
            wind_sd_to_uv(speedInitializationGrid(i,j),
                    dirInitializationGrid(i,j),
                    &(uInitializationGrid)(i,j),
                    &(vInitializationGrid)(i,j));
        }
    }
}

void domainAverageInitialization::initializeBoundaryLayer(WindNinjaInputs& input)
{
    int i, j;

    double inwindu=0.0;		//input u wind component
    double inwindv=0.0;		//input v wind component
    double inwindw=0.0;		//input w wind component

    //Set inwindu and inwindv
    wind_sd_to_uv(input.inputSpeed, input.inputDirection, &inwindu, &inwindv);

    if(input.diurnalWinds == true)
    {
        double aspect_temp = 0;	//just placeholder, basically
        double slope_temp = 0;	//just placeholder, basically

        Solar solar(input.ninjaTime, input.latitude, input.longitude, aspect_temp, slope_temp);

        Aspect aspect(&input.dem, input.numberCPUs);
        Slope slope(&input.dem, input.numberCPUs);
        Shade shade(&input.dem, solar.get_theta(), solar.get_phi(), input.numberCPUs);

        addDiurnal(input, &aspect, &slope, &shade, &solar);  

    }else{	//compute neutral ABL height
        double f;
        double velocity;

        velocity=std::pow(inwindu*inwindu+inwindv*inwindv,0.5);     //Velocity is the velocity magnitude

        //This is the case of a "null" run, this is just done so things work to eventually write
        //out all zero valued output files.
        //if velocity is zero, we just need a decent value for bl_height so things don't blow up
        if(velocity == 0.0)
        {
            for(i=0;i<input.dem.get_nRows();i++)
            {
                for(j=0;j<input.dem.get_nCols();j++)
                {
                    bl_height(i,j) = 1000.0;
                }
            }

        }else{
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
                    u_star(i,j) = velocity*0.4/(log((input.inputWindHeight+
                                                    input.surface.Rough_h(i,j)-
                                                    input.surface.Rough_d(i,j))/
                                                    input.surface.Roughness(i,j)));

                    //compute neutral ABL height
                    //from Van Ulden and Holtslag 1985 (originally Blackadar and Tennekes 1968)
                    bl_height(i,j) = 0.2 * u_star(i,j) / f;	
                }
            }
        }
    }
}
