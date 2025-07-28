/******************************************************************************
 *
 * $Id:
 *
 * Project:  WindNinja
 * Purpose:  Class for calculating stability parameters
 * Author:   Natalie Wagenbrenner <nwagenbrenner@gmail.com>
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

#include "stability.h"

Stability::Stability()        //default constructor
{
    _g = 9.81;
    _c = 0.5; //scaling parameter for _H
}

Stability::Stability(WindNinjaInputs &input)
{
    _g = 9.81;
    _c = 0.5; //scaling parameter for _H
    
    cloudCoverGrid = AsciiGrid<double>(input.dem.get_nCols(),
                                        input.dem.get_nRows(),
                                        input.dem.xllCorner,
                                        input.dem.yllCorner,
                                        input.dem.cellSize,
                                        -9999.0,
                                        input.dem.prjString);
                                            
    QswGrid = AsciiGrid<double>(input.dem.get_nCols(),
                                input.dem.get_nRows(),
                                input.dem.xllCorner,
                                input.dem.yllCorner,
                                input.dem.cellSize,
                                -9999.0,
                                input.dem.prjString);

    speedGrid = AsciiGrid<double>(input.dem.get_nCols(),
                                    input.dem.get_nRows(),
                                    input.dem.xllCorner,
                                    input.dem.yllCorner,
                                    input.dem.cellSize,
                                    -9999.0,
                                    input.dem.prjString);
}

Stability::~Stability()      //destructor
{

}

Stability& Stability::operator= (const Stability& rhs) // Assignment operator
{
    if(&rhs != this)
    {
        alphaField = rhs.alphaField;
        QswGrid = rhs.QswGrid;
        strouhalNumber = rhs.strouhalNumber;
        _N = rhs._N;
        _H = rhs._H;
        _U = rhs._U;
        _t = rhs._t;
        
        _g = rhs._g;
        _c = rhs._c;
        
        thetaDerivatives = rhs.thetaDerivatives;
        stabilityClass = rhs.stabilityClass;
        QswGrid = rhs.QswGrid;
        cloudCoverGrid = rhs.cloudCoverGrid;
        speedGrid = rhs.speedGrid;
    }
    return *this;
}

/**
 * @brief Set alpha for WX model run
 * Alpha is set based on local Strouhal number; this
 * function can only be used if the run is initiated
 * with a WX model with a full vertical profile.
 * @param input WindNinja inputs
 * @param mesh WindNinja computational mesh
 * @param theta wn_3dScalarField of thetas (potential temperature)
 * @param u0 initial u-component of the wind field
 * @param v0 initial v-compoennet of the wind field
 */

void Stability::Set3dVariableAlpha(WindNinjaInputs &input,
                                   const Mesh &mesh,
                                   wn_3dScalarField &theta,
                                   const wn_3dScalarField &u0,
                                   const wn_3dScalarField &v0)
{
    double hTest;
    double hMax, hMin; // max, min terrain heights
    double delta_h; //orographic height difference
    double _r; //horizontal distance 
    double sum1, sum2;
    
    alphaField.allocate(&mesh);
    _H.set_headerData(input.dem);
    
    for(int k = 0; k < mesh.nlayers; k++){
        for(int i = 0; i < mesh.nrows; i++){
            for(int j = 0; j < mesh.ncols; j++ ){
                theta(i,j,k) += 300.0; //convert from perturbation to total potential temperature (theta)
            }
        }
    }

    
    wn_3dVectorField thetaDerivatives;
    theta.ComputeGradient(input, thetaDerivatives); // calculate dtheta/dz, dx, dy at each node
    
    hTest = 0.0;
    hMax = mesh.ZORD(0,0,0);
    hMin = mesh.ZORD(0,0,0);
    for(int i=0; i<mesh.nrows; i++){
        for(int j=0; j<mesh.ncols; j++){
            hTest = mesh.ZORD(i,j,0);
            if(hTest < hMin){
                hMin = hTest;
            }
            if(hTest > hMax){
                hMax = hTest;
            }
        }
    }
    //cout<<"hMax, hMin = "<<hMax<<", "<<hMin<<endl;
    
    /*
     * Calculate the characteristic height for each ground node.
     * Characteristic height is calculated based on orographic height
     * differences and the horizontal distance between the current node
     * and each i,j ground node location. This calculation is based on 
     * Chan and Sugiyama 1997, p. 6, Eq. (2.10). Eq. (2.11) could be used
     * but appears to give bad values for H -- maybe there is a typo
     * in the equation (??)
     */
    sum1 = 0.0;
    sum2 = 0.0;
    for(int i=0; i<mesh.nrows; i++){
        for(int j=0; j<mesh.ncols; j++){
            for(int ii=0; ii<mesh.nrows; ii++){
                for(int jj=0; jj<mesh.ncols; jj++){
                    delta_h = std::abs( mesh.ZORD(i,j,0) - mesh.ZORD(ii,jj,0) );
                    _r = std::sqrt( std::pow( ( mesh.XORD(i,j,0) - mesh.XORD(ii,jj,0) ), 2 ) +
                                    std::pow( ( mesh.YORD(i,j,0) - mesh.YORD(ii,jj,0) ), 2 ) );
                                    
                    if(ii == i && jj == j){ //only summing distances between current node and other nodes 
                        continue;
                    }
                    else{
                        if(_r < 0){
                            throw std::runtime_error("Division by 0 in Set3dVariableAlpha().");
                        }
                        //sum1 += (delta_h / _r); //this calculation appears to be wrong (Eq 2.11)
                        //sum2 += (1 / _r); //this calculation appears to be wrong (Eq 2.11)
                        
                        sum1 += delta_h / ( std::pow(_r, 2) ); // Eq 2.10
                        sum2 += 1 / ( std::pow(_r, 2) ); //Eq. 2.10
                    }
                }
            }
            //_H(i,j) = _c * (hMax - hMin) + (1 - _c) * sum1 / sum2; //this calculation appears to be wrong (Eq 2.11)
            _H(i,j) = sum1 / sum2; //Eq. 2.10
            
        }
    }

    for(int k=0; k<mesh.nlayers; k++){
        for(int i=0; i<mesh.nrows; i++){
            for(int j=0; j<mesh.ncols; j++){

                _U = std::sqrt( ( std::pow(u0(i,j,k), 2) + std::pow(v0(i,j,k), 2) ) );
                    if (_U < 0.2){
                        _U = 0.2;
                    }
                
                if( (*thetaDerivatives.vectorData_z)(i,j,k) >= 0.0 ){ // Str = HN/U
                                        
                    _N = std::sqrt( _g/theta(i,j,k) * (*thetaDerivatives.vectorData_z)(i,j,k) );
                    strouhalNumber = _H(i,j) * _N / _U;
                }
                else if( (*thetaDerivatives.vectorData_z)(i,j,k) < 0.0 ){ // Str = -H/Ut
                    _t = std::sqrt( -_g/theta(i,j,k) * (*thetaDerivatives.vectorData_z)(i,j,k) );
                    strouhalNumber = -_H(i,j) / _U *_t;
                }
                else{
                    throw logic_error("Problem calculating Strouhal number for atmospheric stability.");
                }
                
                if(strouhalNumber >= 0.0){
//                    cout<<"variable check-----------"<<endl;
//                    cout<<"_H(i,j) = "<<_H(i,j)<<endl;
//                    cout<<"_U = "<<_U<<endl;
//                    cout<<"_g = "<<_g<<endl;
//                    cout<<"_c = "<<_c<<endl;
//                    cout<<"theta(i,j,k) = "<<theta(i,j,k)<<endl;
//                    cout<<"dtheta/dz = "<<(*thetaDerivatives.vectorData_z)(i,j,k)<<endl;
//                    cout<<"_N = "<<_N<<endl;
//                    cout<<"strouhalNumber = "<<strouhalNumber<<endl;
//                    cout<<"calculation = "<<std::sqrt( exp( -1.5 * std::pow(strouhalNumber, 1.5) ) )<<endl;
                    
                    alphaField(i,j,k) = std::sqrt( exp( -1.5 * std::pow(strouhalNumber, 1.5) ) );
                    
                    //cout<<"alphaField(i,j,k) = "<<alphaField(i,j,k)<<endl;
                }
                else if(strouhalNumber < 0.0){
                    alphaField(i,j,k) = std::sqrt( exp( 1.5 * std::pow(-strouhalNumber, 1.5) ) );
                }
                else{
                    throw logic_error("Problem calculating alpha from the Strouhal number for atmospheric stability.");
                }
                if(alphaField(i,j,k) > 5.0){ //max alpha allowed is 5
                    alphaField(i,j,k) = 5.0;
                }
                else if(alphaField(i,j,k) <= 0.0){ // alpha must be greater than 0
                    alphaField(i,j,k) = 0.1;
                }
//                if(alphaField(i,j,k) > 5 || alphaField(i,j,k) < 0){
//                    cout<<"alphaField = "<<alphaField(i,j,k)<<endl;
//                    throw std::runtime_error("Problem with atmospheric stability calculation: alpha < 0 or > 5 in Set3dVariableAlpha().");
//                }
                if(alphaField(i,j,k) > 5.0 || alphaField(i,j,k) <= 0.0){
                    throw std::runtime_error("Problem with atmospheric stability calculation: alpha < 0 or > 5 in Set3dVariableAlpha().");
                }
            }
        }
    }
}


/**
 * @brief Set alpha based on user-specified stability
 * Alphas are set as: very unstable = 5;
 * slightly unstable = 2; neutral = 1;
 * slightly stable = 0.5; very stable = 0.2.
 * alpha = alphaH/alphaV.
 * @param input WindNinja input
 * @param mesh WindNinja mesh
 */

void Stability::SetDomainAverageAlpha(WindNinjaInputs &input,
                                      const Mesh &mesh)
{
    cloudCoverGrid = input.cloudCover;
    speedGrid = input.inputSpeed;
                                    
    alphaField.allocate(&mesh);
                                                
    double aspect_temp = 0;	//just placeholder, basically
	double slope_temp = 0;	//just placeholder, basically
	    
    Solar solar(input.ninjaTime, input.latitude, input.longitude, aspect_temp, slope_temp);
	//solar.print_allSolarPosData();
	Aspect aspect(&input.dem, input.numberCPUs);
	Slope slope(&input.dem, input.numberCPUs);
	Shade shade(&input.dem, solar.get_theta(), solar.get_phi(), input.dem.getAngleFromNorth(), input.numberCPUs);
	cellDiurnal cDiurnal(&input.dem, &shade, &solar, 
                    input.downDragCoeff, input.downEntrainmentCoeff,
                    input.upDragCoeff, input.upEntrainmentCoeff);
	cDiurnal.CloudCover = input.cloudCover;  //set CloudCover in cellDiurnal bfore computing Qsw
		
	for(unsigned int i=0;i<input.dem.get_nRows();i++)
	{
        for(unsigned int j=0;j<input.dem.get_nCols();j++)
		{
		    cDiurnal.i=i;	//Set i,j of current cell
	        cDiurnal.j=j;
	        cDiurnal.aspect=aspect(i,j);
	        cDiurnal.slope=slope(i,j);
	        cDiurnal.compute_solarIntensity();
		    cDiurnal.compute_Qsw();
		    QswGrid(i,j) = cDiurnal.Qsw;  // get shortwave radiation for current cell
		}
    }

	//QswGrid.write_Grid("QswGrid", 2);
	//cloudCoverGrid.write_Grid("cloudgrid", 2);
	
	SetAlphaField(mesh);
}


/**
 * @brief Set alpha for a point-initialization run
 * Alphas are set based on cloud cover and wind speed
 * in the WX file and solar radiation. 
 * alpha = alphaH/alphaV.
 * @param Reference to inputs
 */

void Stability::SetPointInitializationAlpha(WindNinjaInputs &input,
                                            const Mesh &mesh)
{
    
    speedGrid = input.surface.windSpeedGrid;
                                    
    alphaField.allocate(&mesh);
        
    double *cc, *X, *Y, *influenceRadius;
	
	cc = new double [input.stations.size()];
	X = new double [input.stations.size()];
	Y = new double [input.stations.size()];
	influenceRadius = new double [input.stations.size()];
	
	double maxStationHeight = -1;	//height above ground of highest station, used as height of 2d layer to interpolate horizontally to

	for(unsigned int ii = 0; ii<input.stations.size(); ii++)
	{
        if(input.stations[ii].get_height() > maxStationHeight)
            maxStationHeight = input.stations[ii].get_height();
        cc[ii] = input.stations[ii].get_cloudCover();
        X[ii] = input.stations[ii].get_projXord();
        Y[ii] = input.stations[ii].get_projYord();
        influenceRadius[ii] = input.stations[ii].get_influenceRadius();
    }

    cloudCoverGrid.interpolateFromPoints(cc, X, Y, influenceRadius, input.stations.size(), 2.0);

	//Check one grid to be sure that the interpolation completely filled the grid
	if(cloudCoverGrid.checkForNoDataValues())
	{
        throw std::runtime_error("Fill interpolation from the wx stations didn't completely fill the grids.  " \
				    "To be sure everything is filled, let at least one wx station have an infinite influence radius.  " \
				    "This is specified by defining the influence radius to be a value less than zero in the wx " \
				    "station file.");
    }
	    
    double aspect_temp = 0;	//just placeholder, basically
    double slope_temp = 0;	//just placeholder, basically
	    
    Solar solar(input.ninjaTime, input.latitude, input.longitude, aspect_temp, slope_temp);
    //solar.print_allSolarPosData();
	Aspect aspect(&input.dem, input.numberCPUs);
    Slope slope(&input.dem, input.numberCPUs);
	Shade shade(&input.dem, solar.get_theta(), solar.get_phi(), input.dem.getAngleFromNorth(), input.numberCPUs);
	cellDiurnal cDiurnal(&input.dem, &shade, &solar,
                        input.downDragCoeff, input.downEntrainmentCoeff,
                        input.upDragCoeff, input.upEntrainmentCoeff);
		
	for(unsigned int i=0;i<input.dem.get_nRows();i++)
	{
	    for(unsigned int j=0;j<input.dem.get_nCols();j++)
	    {
	        cDiurnal.CloudCover = cloudCoverGrid(i,j);  //set CloudCover in cellDiurnal bfore computing Qsw
	        cDiurnal.i = i;
	        cDiurnal.j = j;
	        cDiurnal.aspect=aspect(i,j);
	        cDiurnal.slope=slope(i,j);
	        cDiurnal.compute_solarIntensity();
	        cDiurnal.compute_Qsw();
	        QswGrid(i,j) = cDiurnal.Qsw;  // get shortwave radiation for current cell
	    }
	}
		
    //cout<<"station.cloudCover"<<station.cloudCover<<endl;
	//cout<<"cDiurnal.cloudCover"<<cDiurnal.CloudCover<<endl;
	//cout<<"input.CloudCover = " <<input.cloudCover<<endl;
	
	//QswGrid.write_Grid("QswGrid_ptInit", 2);
    //cloudCoverGrid.write_Grid("cloudcover_pointInit", 2);
	
	SetAlphaField(mesh);
	
    delete[] cc;
        cc = NULL;
	delete[] X;
        X = NULL;
	delete[] Y;
        Y = NULL;
	delete[] influenceRadius;
        influenceRadius = NULL;
    
}

/**
 * @brief Set alpha for a 2D Wx model run
 * Alphas are set based on  WX model cloud cover,
 * initial wind speed, and solar radiation. 
 * alpha = alphaH/alphaV.
 * @param input WindNinja inputs
 * @param mesh WindNinja mesh
 * @param cloud Cloud cover grid
 */

void Stability::Set2dWxInitializationAlpha(WindNinjaInputs &input,
                                            const Mesh &mesh,
                                            const AsciiGrid<double> &cloud)
{
    
    cloudCoverGrid = cloud;
    speedGrid = input.surface.windSpeedGrid;
    
    alphaField.allocate(&mesh);
	    
    double aspect_temp = 0;	//just placeholder, basically
    double slope_temp = 0;	//just placeholder, basically
	    
    Solar solar(input.ninjaTime, input.latitude, input.longitude, aspect_temp, slope_temp);
    //solar.print_allSolarPosData();
	Aspect aspect(&input.dem, input.numberCPUs);
    Slope slope(&input.dem, input.numberCPUs);
	Shade shade(&input.dem, solar.get_theta(), solar.get_phi(), input.dem.getAngleFromNorth(), input.numberCPUs);
	cellDiurnal cDiurnal(&input.dem, &shade, &solar,
                        input.downDragCoeff, input.downEntrainmentCoeff,
                        input.upDragCoeff, input.upEntrainmentCoeff);
		
	for(unsigned int i=0;i<input.dem.get_nRows();i++)
	{
	    for(unsigned int j=0;j<input.dem.get_nCols();j++)
	    {
	        cDiurnal.CloudCover = cloud(i,j);  //set CloudCover in cellDiurnal bfore computing Qsw
	        cDiurnal.i = i;
	        cDiurnal.j = j;
	        cDiurnal.aspect=aspect(i,j);
	        cDiurnal.slope=slope(i,j);
	        cDiurnal.compute_solarIntensity();
	        cDiurnal.compute_Qsw();
	        QswGrid(i,j) = cDiurnal.Qsw;  // get shortwave radiation for current cell
	    }
	}
	
	//QswGrid.write_Grid("QswGrid_2dWxInit", 2);
    //cloudCoverGrid.write_Grid("cloudcover_2dWxInit", 2);
    //speedGrid.write_Grid("speed_2dWxInit", 2);
	
	SetAlphaField(mesh);
        
}

/**
 * @brief Sets alpha field for stability
 * alpha = alphaH/alphaV.
 * @param mesh WindNinja mesh.
 */

void Stability::SetAlphaField(const Mesh &mesh)
{
    for(unsigned int k=0; k<mesh.nlayers; k++)
    {
        for(unsigned int i=0; i<mesh.nrows; i++)
        {
            for(unsigned int j=0; j<mesh.ncols; j++)
            {	
                if(QswGrid(i,j) > 600.0)
                {
                    if(speedGrid(i,j) < 2.0)
                        stabilityClass = "A";
                    else if(speedGrid(i,j)  < 3.0)
                        stabilityClass = "AB";
                    else if(speedGrid(i,j) < 5.0)
                        stabilityClass = "B";
                    else stabilityClass = "C";
                }
                else if(QswGrid(i,j) > 350.0)
                {
                    if(speedGrid(i,j) < 2.0)
                        stabilityClass = "AB";
                    else if(speedGrid(i,j) < 3.0)
                        stabilityClass = "B";
                    else if(speedGrid(i,j) < 5.0)
                        stabilityClass = "BC";
                    else if(speedGrid(i,j) < 6.0)
                        stabilityClass = "CD";
                    else stabilityClass = "D";
                }
                else if(QswGrid(i,j) > 0.0)
                {
                    if(speedGrid(i,j) < 2.0)
                        stabilityClass = "B";
                    else if(speedGrid(i,j) < 5.0)
                        stabilityClass = "C";
                    else stabilityClass = "D";
                }
                else if(cloudCoverGrid(i,j) > 0.5)
                {  
                    if(speedGrid(i,j) < 3.0)
                        stabilityClass = "E";
                    else stabilityClass = "D";
                }
                else
                {
                    if(speedGrid(i,j) < 3.0)
                        stabilityClass = "F";
                    else if(speedGrid(i,j) < 5.0)
                        stabilityClass = "E";
                    else stabilityClass = "D";
                }
                if(stabilityClass=="A")
                {
                    alphaField(i,j,k) = 5.0;
                }
                else if(stabilityClass=="AB")
                {
                    alphaField(i,j,k) = 4.25;
                }
                else if(stabilityClass=="B")
                {
                    alphaField(i,j,k) = 3.5;
                }
                else if(stabilityClass=="BC")
                {
                    alphaField(i,j,k) = 2.75;
                }
                else if(stabilityClass=="C")
                {
                    alphaField(i,j,k) = 2.0;
                }   
                else if(stabilityClass=="CD")
                {
                    alphaField(i,j,k) = 1.5;
                }
                else if(stabilityClass=="D")
                {
                    alphaField(i,j,k) = 1.0;
                }
                else if(stabilityClass=="E")
                {
                    alphaField(i,j,k) = 0.5;
                }
                else if(stabilityClass=="F")
                {
                    alphaField(i,j,k) = 0.2;
                }
                else
                {
                    throw std::out_of_range("stabilityClass out of range in Stability::SetAlphaField()");
                }
            }
        }
    }
    QswGrid.deallocate();
    cloudCoverGrid.deallocate();
    speedGrid.deallocate();
}

