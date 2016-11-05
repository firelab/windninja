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
                AsciiGrid<double>& cloud,
                AsciiGrid<double>& L,
                AsciiGrid<double>& u_star,
                AsciiGrid<double>& bl_height)
{
    //Set cloud grid
    int longEdge = input.dem.get_nRows();
    if(input.dem.get_nRows() < input.dem.get_nCols())
        longEdge = input.dem.get_nCols();

    double tempCloudCover;
    if(input.cloudCover < 0)
        tempCloudCover = 0.0;
    else
        tempCloudCover = input.cloudCover;

    cloud.set_headerData(1, 1, input.dem.get_xllCorner(),
                        input.dem.get_yllCorner(),
                        (longEdge * input.dem.cellSize),
                        -9999.0, tempCloudCover, input.dem.prjString);

    double inwindu=0.0;		//input u wind component
    double inwindv=0.0;		//input v wind component
    double inwindw=0.0;		//input w wind component
    int i, j, k;

    windProfile profile;
    //make sure rough_h is set to zero if profile switch is 0 or 2
    profile.profile_switch = windProfile::monin_obukov_similarity;	//switch that detemines what profile is used...

    //Set inwindu and inwindv
    wind_sd_to_uv(input.inputSpeed, input.inputDirection, &inwindu, &inwindv);

    //Set inwindw
    inwindw=0.0;

    AsciiGrid<double> airTempGrid(input.dem.get_nCols(),
                                 input.dem.get_nRows(),
                                 input.dem.xllCorner,
                                 input.dem.yllCorner,
                                 input.dem.cellSize,
                                 input.dem.get_noDataValue(),
                                 input.airTemp);

    AsciiGrid<double> cloudCoverGrid(input.dem.get_nCols(),
                                 input.dem.get_nRows(),
                                 input.dem.xllCorner,
                                 input.dem.yllCorner,
                                 input.dem.cellSize,
                                 input.dem.get_noDataValue(),
                                 input.cloudCover);

    initializeWindToZero(mesh, u0, v0, w0);

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
        height.set_headerData(input.dem.get_nCols(),
                        input.dem.get_nRows(),
                        input.dem.get_xllCorner(),
                        input.dem.get_yllCorner(),
                        input.dem.get_cellSize(),
                        input.dem.get_noDataValue(),
                        0);	//height of diurnal flow above "z=0" in log profile
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

        addDiurnal diurnal(&uDiurnal, &vDiurnal, &wDiurnal, &height, &L, &u_star, 
                &bl_height, &input.dem, &aspect, &slope, &shade, 
                &solar, &input.surface, &cloudCoverGrid, &airTempGrid, 
                input.numberCPUs, input.downDragCoeff, input.downEntrainmentCoeff,
                input.upDragCoeff, input.upEntrainmentCoeff);
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

    //Initialize u0,v0,w0----------------------------------
    #pragma omp parallel for default(shared) firstprivate(profile) private(i,j,k)
    for(i=0;i<input.dem.get_nRows();i++)
    {
        for(j=0;j<input.dem.get_nCols();j++)
        {
            profile.ObukovLength = L(i,j);
            profile.ABL_height = bl_height(i,j);
            profile.Roughness = input.surface.Roughness(i,j);
            profile.Rough_h = input.surface.Rough_h(i,j);
            profile.Rough_d = input.surface.Rough_d(i,j);
            profile.inputWindHeight = input.inputWindHeight;

            for(k=0;k<mesh.nlayers;k++)
            {
                //this is height above THE GROUND!! (not "z=0" for the log profile)
                profile.AGL=mesh.ZORD(i, j, k)-input.dem(i,j);	
                
                profile.inputWindSpeed = inwindu;
                u0(i, j, k) += profile.getWindSpeed();

                profile.inputWindSpeed = inwindv;
                v0(i, j, k) += profile.getWindSpeed();

                profile.inputWindSpeed = inwindw;
                w0(i, j, k) += profile.getWindSpeed();

            }
        }
    }

    //Now add diurnal component if desired
    //height above top of roughness elements
    double AGL=0;
    if((input.diurnalWinds==true) && (profile.profile_switch==windProfile::monin_obukov_similarity))
    {
        #pragma omp parallel for default(shared) private(i,j,k,AGL)
        //start at 1, not zero because ground nodes must be zero for boundary conditions to work properly
        for(k=1;k<mesh.nlayers;k++)	
        {
            for(i=0;i<mesh.nrows;i++)
            {
                for(j=0;j<mesh.ncols;j++)
                {
                    //this is height above THE GROUND!! (not "z=0" for the log profile)
                    AGL=mesh.ZORD(i, j, k)-input.dem(i,j);	
                    if((AGL - (input.surface.Rough_d(i,j))) < height(i,j))
                    {
                        u0(i, j, k) += uDiurnal(i,j);
                        v0(i, j, k) += vDiurnal(i,j);
                        w0(i, j, k) += wDiurnal(i,j);
                    }		
                }
            }
        }
    }
}
