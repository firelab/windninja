/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing 3D mesh.  Values are stored at the nodes, not
			 cell centers.
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

#include "mesh.h"

Mesh::Mesh()
{
    NUMNP = 0;
    NUMEL = 0;
    NNPE = 8;
    nrows = 0;
    ncols = 0;
    nlayers = 0;
    nrowsElem = 0;
    ncolsElem = 0;
    nlayersElem = 0;

    meshResolutionUnits = lengthUnits::meters;
    meshResolution = -1.0;
    domainHeightUnits = lengthUnits::meters;
    domainHeight = -1.0;
    numVertLayers = -1;
    vertGrowth = 1.3;

    meshResChoice = coarse;
    targetNumHorizCells = -1;
    maxAspectRatio = 400;

    coarseTargetCells=4000; //IF THESE ARE CHANGED, make sure to change the ones in the windninja GUI
    mediumTargetCells=10000;
    fineTargetCells=20000;
}

Mesh::~Mesh()
{

}

Mesh::Mesh(Mesh const& m) // Copy constructor
{
    NUMNP = m.NUMNP;
    NUMEL = m.NUMEL;
    NNPE = m.NNPE;
    XORD = m.XORD;
    YORD = m.YORD;
    ZORD = m.ZORD;
    nrows = m.nrows;
    ncols = m.ncols;
    nlayers = m.nlayers;
    nrowsElem = m.nrowsElem;
    ncolsElem = m.ncolsElem;
    nlayersElem = m.nlayersElem;

    meshResolutionUnits = m.meshResolutionUnits;
    meshResolution = m.meshResolution;
    domainHeightUnits = m.domainHeightUnits;
    domainHeight = m.domainHeight;
    numVertLayers = m.numVertLayers;
    vertGrowth = m.vertGrowth;

    meshResChoice = m.meshResChoice;
    targetNumHorizCells = m.targetNumHorizCells;
    maxAspectRatio = m.maxAspectRatio;

    coarseTargetCells = m.coarseTargetCells; //IF THESE ARE CHANGED, make sure to change the ones in the windninja GUI
    mediumTargetCells = m.mediumTargetCells;
    fineTargetCells = m.fineTargetCells;
}

Mesh& Mesh::operator= (Mesh const& m) // Assignment operator
{
    if(&m != this)
    {
        NUMNP = m.NUMNP;
        NUMEL = m.NUMEL;
        NNPE = m.NNPE;
        XORD = m.XORD;
        YORD = m.YORD;
        ZORD = m.ZORD;
        nrows = m.nrows;
        ncols = m.ncols;
        nlayers = m.nlayers;
        nrowsElem = m.nrowsElem;
        ncolsElem = m.ncolsElem;
        nlayersElem = m.nlayersElem;

        meshResolutionUnits = m.meshResolutionUnits;
        meshResolution = m.meshResolution;
        domainHeightUnits = m.domainHeightUnits;
        domainHeight = m.domainHeight;
        numVertLayers = m.numVertLayers;
        vertGrowth = m.vertGrowth;

        meshResChoice = m.meshResChoice;
        targetNumHorizCells = m.targetNumHorizCells;
        maxAspectRatio = m.maxAspectRatio;

        coarseTargetCells = m.coarseTargetCells; //IF THESE ARE CHANGED, make sure to change the ones in the windninja GUI
        mediumTargetCells = m.mediumTargetCells;
        fineTargetCells = m.fineTargetCells;
    }
    return *this;
}

void Mesh::buildFrom3dWeatherModel(const WindNinjaInputs &input,
                                   const wn_3dArray &elevationArray,
                                   double dx, int rows, int cols, int layers,
                                   double xOffset, double yOffset)
{
    int i;   //"i" is row number with 0 being the South row
    int j;   //"j" is column number with 0 being the West row
    int k;   //"k" is layer number with 0 being the ground layer
    
    meshResolution = dx;
    
    nrows = rows;
    ncols = cols;
    nlayers = layers;
    numVertLayers = layers;

	nrowsElem = nrows - 1;
	ncolsElem = ncols - 1;
	nlayersElem = nlayers - 1;

    NUMNP=nrows*ncols*nlayers;              //number of nodal points
    NUMEL=(nrows-1)*(ncols-1)*(nlayers-1);  //number of elements
    //hexahedral elements are being used
    NNPE=8;                                 //number of nodes per element

	
	/*
	 * reset unused generic mesh variables (?)
	 */
	
    /*vertGrowth = -1;
    meshResChoice = coarse;
    targetNumHorizCells = -1;
    maxAspectRatio = -1;
    coarseTargetCells=-1;
    mediumTargetCells=-1;
    fineTargetCells=-1;*/
	
	XORD.allocate(nrows, ncols, nlayers);
	YORD.allocate(nrows, ncols, nlayers);
	ZORD.allocate(nrows, ncols, nlayers);

	//Set xyz coordinates ---------------------------------------------------------------
	#pragma omp parallel for default(shared) private(i,j,k)
	for(k=0;k<nlayers;k++) // k = 0 is cell center of 1st wx model layer
	{
		for(i=0;i<nrows;i++)
		{
			for(j=0;j<ncols;j++)
			{
				//Note that the XORDs and YORDs are the WindNinja nodal locations.
                //These are in a coordinate system with xy-location (0,0) at the lower left corner of the DEM.
                //Since the DEM is cell centered and the XORD/YORD are nodes, the mesh is built with the nodes
                //on the DEM cell center locations.
                //So, for example, XORD(0,0,0) = 0.5*cellsize.
				//These must later be "shifted" to the xllcorner and yllcorner of DEM for output products.
				//This is done to try to reduce roundoff error in calculations.

				XORD(i, j, k) = (double)j*meshResolution + xOffset + 0.5*meshResolution;
                YORD(i, j, k) = (double)i*meshResolution + yOffset + 0.5*meshResolution;
                ZORD(i, j, k) = elevationArray(i,j,k);
            }
        }
    }
	
	// testing
        /*
	std::string filename;
    AsciiGrid<double> testGrid(input.dem);
	
	for(int k = 0; k<nlayers; k++){
        for(int i = 0; i<testGrid.get_nRows(); i++){
            for(int j = 0; j<testGrid.get_nCols(); j++ ){
                testGrid(i,j) = ZORD(i,j,k);
                filename = "elevation" + boost::lexical_cast<std::string>(k) + ".asc";
                
            }
        }
        testGrid.write_Grid(filename.c_str(), 2);
    }
    testGrid.deallocate();
    */

	domainHeight = 0;
    for(int i = 0; i < nrows; i++){
        for(int j = 0; j < ncols; j++ ){
            if(ZORD(i, j, nlayers - 1) > domainHeight){
                domainHeight = ZORD(i, j, nlayers - 1);
            }
        }
    }
}

void Mesh::buildStandardMesh(WindNinjaInputs& input)
{
	 int i;   //"i" is row number with 0 being the South row
     int j;   //"j" is column number with 0 being the West row
     int k;   //"k" is layer number with 0 being the ground layer
     double aspect_ratio=0.0, equiangle_skew=0.0;
     //double minnearwallz=0.0, largenearwallz=0.0;

     int check_aspect_ratio;    //Flag to check aspect ratio of cells:  0=>don't check ratio   1=>check ratio
     int check_equiangle_skew;  //Flag to check equiangle skew of cells:  0=>don't check skewness   1=>check skewness


#ifdef NINJA_DEBUG
      check_aspect_ratio=1;    //check
      check_equiangle_skew=1;  //check
#else
      check_aspect_ratio=0;    //don't check
      check_equiangle_skew=0;  //don't check
#endif

      //If horizontal cellsize hasn't been set yet.
      if(meshResolution < 0.0)
          compute_cellsize(input.dem);

      //If domainHeight wasn't set with set_domainHeight(), set using other parameters (numVertLayers, vertGrowth, etc.)
      if(domainHeight < 0.0)
          compute_domain_height(input);

      if(meshResolution < 0.0)
          throw std::out_of_range("The mesh resolution cannot be less than 0.");
      if(domainHeight < input.dem.get_maxValue())
          throw std::out_of_range("Domain height is below the elevation of the tallest mountain in Mesh::set_domainHeight().");
      if(numVertLayers <= 0)
          throw std::out_of_range("The number of vertical layers in the mesh cannot be less than or equal to 0.");
      if(vertGrowth < 0.0)
          throw std::out_of_range("The vertical mesh growth rate cannot be less than 0.");
      if(maxAspectRatio < 0.0)
              throw std::out_of_range("maxAspectRatio is less than 0.");

     if(vertGrowth==1)
		 throw std::range_error("Mesh::buildStandardMesh(WindNinjaInputs& input) detected a bad \"vertGrowth\" value.");

	//Resample DEM to desired computational resolution
     if(meshResolution < input.dem.get_cellSize())
     {
         input.dem.resample_Grid_in_place(meshResolution, Elevation::order1);	//make the grid finer
         input.surface.resample_in_place(meshResolution, AsciiGrid<double>::order1); //make the grid finer
								//NOTE: DEM IS THE ELEVATION ABOVE SEA LEVEL
     }else if(meshResolution > input.dem.get_cellSize())
     {
		 input.dem.resample_Grid_in_place(meshResolution, Elevation::order0);	//coarsen the grid
		 input.surface.resample_in_place(meshResolution, AsciiGrid<double>::order0);		//coarsen the grids
         
         input.dem.BufferGridInPlace();  //make sure grid at least covers the original domain
         input.surface.BufferGridInPlace();
     }

	 nrows = input.dem.get_nRows();
	 ncols = input.dem.get_nCols();
	 nlayers = numVertLayers;

	 nrowsElem = nrows - 1;
	 ncolsElem = ncols - 1;
	 nlayersElem = nlayers - 1;

     NUMNP=nrows*ncols*nlayers;              //number of nodal points
     NUMEL=(nrows-1)*(ncols-1)*(nlayers-1);  //number of elements
     //hexahedral elements are being used
     NNPE=8;                                 //number of nodes per element
     
	 XORD.allocate(nrows, ncols, nlayers);
	 YORD.allocate(nrows, ncols, nlayers);
	 ZORD.allocate(nrows, ncols, nlayers);


	//Set xyz coordinates ---------------------------------------------------------------
	#pragma omp parallel for default(shared) private(i,j,k)
	for(k=0;k<nlayers;k++)
	{
		for(i=0;i<nrows;i++)
		{
			for(j=0;j<ncols;j++)
			{
				//Note that the XORDs and YORDs are the WindNinja nodal locations.
                //These are in a coordinate system with xy-location (0,0) at the lower left corner of the DEM.
                //Since the DEM is cell centered and the XORD/YORD are nodes, the mesh is built with the nodes
                //on the DEM cell center locations.
                //So, for example, XORD(0,0,0) = 0.5*cellsize.
				//These must later be "shifted" to the xllcorner and yllcorner of DEM for output products.
				//This is done to try to reduce roundoff error in calculations.

				XORD(i, j, k) = (double)j*meshResolution + 0.5*meshResolution;
                YORD(i, j, k) = (double)i*meshResolution + 0.5*meshResolution;
                if(k==0)
                {
                     ZORD(i, j, k) = input.dem(i,j);
                }else
                {
                     ZORD(i, j, k) = get_z(i, j, k, input.dem(i,j));
                }
			}
		}
	}

     if(check_aspect_ratio==1)
          aspect_ratio=get_aspect_ratio(NUMEL, NUMNP, XORD, YORD, ZORD, nrows, ncols, nlayers);
     if(check_equiangle_skew==1)
          equiangle_skew=get_equiangle_skew(NUMEL, NUMNP, XORD, YORD, ZORD, nrows, ncols, nlayers);
#ifdef NINJA_DEBUG_VERBOSE
      input.Com->ninjaCom(ninjaComClass::ninjaDebug, "\n--------MESH INFORMATION--------");
//          input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Ground roughness = %lf",roughness);
      //input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Smallest ground cell = %lf",minnearwallz);
      //input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Largest ground cell = %lf",largenearwallz);
      if(check_aspect_ratio==1)
           input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Largest aspect ratio = %lf",aspect_ratio);
      if(check_equiangle_skew==1)
           input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Largest equiangle skew = %lf",equiangle_skew);
      input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Number of columns = %d",input.dem.get_nCols());
      input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Number of rows = %d",input.dem.get_nRows());
      input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Number of nodes = %ld",NUMNP);
      input.Com->ninjaCom(ninjaComClass::ninjaDebug, "Number of elements = %ld",NUMEL);
      input.Com->ninjaCom(ninjaComClass::ninjaDebug, "--------------------------------");
#endif //NINJA_DEBUG_VERBOSE

}

double Mesh::get_z(const int& i, const int& j, const int& k, const double& elev)
{
        double z;
        z=(domainHeight-elev)*((std::pow(vertGrowth,int(k-numVertLayers+1))-std::pow(vertGrowth,int(1-numVertLayers)))/(1-std::pow(vertGrowth,int(1-numVertLayers))))+elev;
        return z;
}


double Mesh::get_aspect_ratio(int NUMEL, int NUMNP, wn_3dArray& XORD, wn_3dArray& YORD, wn_3dArray& ZORD, int nrows, int ncols, int nlayers)
{
     double aspect_ratio=1.0;
     double e1, e2, e3, l1, l2, l3, l4, x, y, z, temp_asp_ratio, temp;
     int i, j, k;

     for(i=0;i<(nrows-1);i++)      //loop through all the elements in the first layer (first layer elements should be the most degenerate)
     {
          for(j=0;j<(ncols-1);j++)
          {

               k=0;

               //compute in i/x direction
               x=XORD(i, j+1, k)-XORD(i, j, k);
               y=YORD(i, j+1, k)-YORD(i, j, k);
               z=ZORD(i, j+1, k)-ZORD(i, j, k);
               l1=std::sqrt(x*x+y*y+z*z);

               x=XORD(i+1, j+1, k)-XORD(i+1, j, k);
               y=YORD(i+1, j+1, k)-YORD(i+1, j, k);
               z=ZORD(i+1, j+1, k)-ZORD(i+1, j, k);
               l2=std::sqrt(x*x+y*y+z*z);

               x=XORD(i, j+1, k+1)-XORD(i, j, k+1);
               y=YORD(i, j+1, k+1)-YORD(i, j, k+1);
               z=ZORD(i, j+1, k+1)-ZORD(i, j, k+1);
               l3=std::sqrt(x*x+y*y+z*z);

               x=XORD(i+1, j+1, k+1)-XORD(i+1, j, k+1);
               y=YORD(i+1, j+1, k+1)-YORD(i+1, j, k+1);
               z=ZORD(i+1, j+1, k+1)-ZORD(i+1, j, k+1);
               l4=std::sqrt(x*x+y*y+z*z);

               e1=(l1+l2+l3+l4)/4.0;

               //compute in j/y direction
               x=XORD(i+1, j, k)-XORD(i, j, k);
               y=YORD(i+1, j, k)-YORD(i, j, k);
               z=ZORD(i+1, j, k)-ZORD(i, j, k);
               l1=std::sqrt(x*x+y*y+z*z);

               x=XORD(i+1, j+1, k)-XORD(i, j+1, k);
               y=YORD(i+1, j+1, k)-YORD(i, j+1, k);
               z=ZORD(i+1, j+1, k)-ZORD(i, j+1, k);
               l2=std::sqrt(x*x+y*y+z*z);

               x=XORD(i+1, j, k+1)-XORD(i, j, k+1);
               y=YORD(i+1, j, k+1)-YORD(i, j, k+1);
               z=ZORD(i+1, j, k+1)-ZORD(i, j, k+1);
               l3=std::sqrt(x*x+y*y+z*z);

               x=XORD(i+1, j+1, k+1)-XORD(i, j+1, k+1);
               y=YORD(i+1, j+1, k+1)-YORD(i, j+1, k+1);
               z=ZORD(i+1, j+1, k+1)-ZORD(i, j+1, k+1);
               l4=std::sqrt(x*x+y*y+z*z);

               e2=(l1+l2+l3+l4)/4.0;

               //compute in k/z direction
               x=XORD(i, j, k+1)-XORD(i, j, k);
               y=YORD(i, j, k+1)-YORD(i, j, k);
               z=ZORD(i, j, k+1)-ZORD(i, j, k);
               l1=std::sqrt(x*x+y*y+z*z);

               x=XORD(i, j+1, k+1)-XORD(i, j+1, k);
               y=YORD(i, j+1, k+1)-YORD(i, j+1, k);
               z=ZORD(i, j+1, k+1)-ZORD(i, j+1, k);
               l2=std::sqrt(x*x+y*y+z*z);

               x=XORD(i+1, j, k+1)-XORD(i+1, j, k);
               y=YORD(i+1, j, k+1)-YORD(i+1, j, k);
               z=ZORD(i+1, j, k+1)-ZORD(i+1, j, k);
               l3=std::sqrt(x*x+y*y+z*z);

               x=XORD(i+1, j+1, k+1)-XORD(i+1, j+1, k);
               y=YORD(i+1, j+1, k+1)-YORD(i+1, j+1, k);
               z=ZORD(i+1, j+1, k+1)-ZORD(i+1, j+1, k);
               l4=std::sqrt(x*x+y*y+z*z);

               e3=(l1+l2+l3+l4)/4.0;

               //sort e's from largest (e1) to smallest (e3)
               if(e2>e1)
               {
                    temp=e1;
                    e1=e2;
                    e2=temp;
               }
               if(e3>e1)
               {
                    temp=e3;
                    e3=e2;
                    e2=e1;
                    e1=temp;
               }else if(e3>e2)
               {
                    temp=e3;
                    e3=e2;
                    e2=temp;
               }
               temp_asp_ratio=e1/e3;
               if(temp_asp_ratio>aspect_ratio)
                    aspect_ratio=temp_asp_ratio;
          }
     }


     return aspect_ratio;
}

double Mesh::get_equiangle_skew(int NUMEL, int NUMNP, wn_3dArray& XORD, wn_3dArray& YORD, wn_3dArray& ZORD, int nrows, int ncols, int nlayers)
{
     double equiangle_skew=0;
     double cell_max_angle, cell_min_angle, temp;
     double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, x6, y6, z6, x7, y7, z7, x8, y8, z8;
     int i, j, k;

     k=0;

     for(i=0;i<(nrows-1);i++)      //loop through all the elements in the first layer (first layer elements should be the most degenerate)
     {
          for(j=0;j<(ncols-1);j++)
          {

               cell_max_angle=0.0;
               cell_min_angle=180.0;

               //rename points to a cell local numbering starting at cell bottom, lower left and counting counter-clockwise
               x1=XORD(i, j, k);
               y1=YORD(i, j, k);
               z1=ZORD(i, j, k);

               x2=XORD(i, j+1, k);
               y2=YORD(i, j+1, k);
               z2=ZORD(i, j+1, k);

               x3=XORD(i+1, j+1, k);
               y3=YORD(i+1, j+1, k);
               z3=ZORD(i+1, j+1, k);

               x4=XORD(i+1, j, k);
               y4=YORD(i+1, j, k);
               z4=ZORD(i+1, j, k);

               x5=XORD(i, j, k+1);
               y5=YORD(i, j, k+1);
               z5=ZORD(i, j, k+1);

               x6=XORD(i, j+1, k+1);
               y6=YORD(i, j+1, k+1);
               z6=ZORD(i, j+1, k+1);

               x7=XORD(i+1, j+1, k+1);
               y7=YORD(i+1, j+1, k+1);
               z7=ZORD(i+1, j+1, k+1);

               x8=XORD(i+1, j, k+1);
               y8=YORD(i+1, j, k+1);
               z8=ZORD(i+1, j, k+1);


               //bottom face
               get_cell_angles(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, cell_max_angle, cell_min_angle);
               //top face
               get_cell_angles(x5, y5, z5, x6, y6, z6, x7, y7, z7, x8, y8, z8, cell_max_angle, cell_min_angle);
               //north face
               get_cell_angles(x4, y4, z4, x8, y8, z8, x7, y7, z7, x3, y3, z3, cell_max_angle, cell_min_angle);
               //south face
               get_cell_angles(x1, y1, z1, x5, y5, z5, x6, y6, z6, x2, y2, z2, cell_max_angle, cell_min_angle);
               //west face
               get_cell_angles(x4, y4, z4, x8, y8, z8, x5, y5, z5, x1, y1, z1, cell_max_angle, cell_min_angle);
               //east face
               get_cell_angles(x2, y2, z2, x6, y6, z6, x7, y7, z7, x3, y3, z3, cell_max_angle, cell_min_angle);

               //Compute cell equiangle skew
               temp=maxj((cell_max_angle-90.0)/(90.0),(90.0-cell_min_angle)/(90.0));
               if(temp>equiangle_skew)
                    equiangle_skew=temp;
          }
     }


     return equiangle_skew;
}

void Mesh::get_cell_angles(double xa, double ya, double za, double xb, double yb, double zb, double xc, double yc, double zc, double xd, double yd, double zd, double &cell_max_angle, double &cell_min_angle)
{
     double angle;

     //LL angle
     angle=get_angle(xd, yd, zd, xa, ya, za, xb, yb, zb);
     if(angle>cell_max_angle)
          cell_max_angle=angle;
     if(angle<cell_min_angle)
          cell_min_angle=angle;

     //LR angle
     angle=get_angle(xa, ya, za, xb, yb, zb, xc, yc, zc);
     if(angle>cell_max_angle)
          cell_max_angle=angle;
     if(angle<cell_min_angle)
          cell_min_angle=angle;

     //UR angle
     angle=get_angle(xb, yb, zb, xc, yc, zc, xd, yd, zd);
     if(angle>cell_max_angle)
          cell_max_angle=angle;
     if(angle<cell_min_angle)
          cell_min_angle=angle;

     //UL angle
     angle=get_angle(xc, yc, zc, xd, yd, zd, xa, ya, za);
     if(angle>cell_max_angle)
          cell_max_angle=angle;
     if(angle<cell_min_angle)
          cell_min_angle=angle;


}

double Mesh::get_angle(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3)
{
     //Function finds the angle between two vectors, assuming that point 2 is the common point shared by the two vectors and points 1 and 3 are the endpoints

     double angle, v_norm, w_norm;

     x1=x1-x2;      //shift coordinates so they are local to point 2
     x3=x3-x2;

     y1=y1-y2;
     y3=y3-y2;

     z1=z1-z2;
     z3=z3-z2;

     v_norm=std::sqrt(x1*x1+y1*y1+z1*z1);
     w_norm=std::sqrt(x3*x3+y3*y3+z3*z3);
     angle=180.0/pi*acos((x1*x3+y1*y3+z1*z3)/(v_norm*w_norm));

     return angle;
}

double Mesh::maxj(double value1, double value2)
{
     return ( (value1 > value2) ? value1 : value2);
}

int Mesh::get_node0(const int &elemNum) const
{
     //Calculates the global node number of element elemNum's local node number 0
     int node, row, col, layer, layer_elements;
     layer_elements=(nrows-1)*(ncols-1);
     layer=elemNum/(layer_elements);
     row=(elemNum-layer*layer_elements)/(ncols-1);
     col=elemNum-layer*layer_elements-row*(ncols-1);
     node=layer*(ncols*nrows)+row*ncols+col;

     return node;
}

int Mesh::get_node0(const int &elem_i, const int &elem_j, const int &elem_k) const
{
     //Calculates the global node number of element (i,j,k)'s local node number 0

     return elem_k*(ncols*nrows)+elem_i*ncols+elem_j;
}

int Mesh::get_elemNum(const int &elem_i, const int &elem_j, const int &elem_k) const
{
	//Calculates the global element number given the element's (i,j,k) index

	return elem_k*(ncolsElem*nrowsElem)+elem_i*ncolsElem+elem_j;
}

void Mesh::get_elemIndex(const int &elemNum, int &elem_i, int &elem_j, int &elem_k) const
{
	//Calculates the element's (i,j,k) index given the global element number elemNum
	int layer_elements;
	layer_elements=nrowsElem*ncolsElem;
	elem_k=elemNum/layer_elements;
	elem_i=(elemNum-elem_k*layer_elements)/ncolsElem;
	elem_j=elemNum-elem_k*layer_elements-elem_i*ncolsElem;
}

int Mesh::get_global_node(const int &locNodeNum, const int &elemNum) const
{
     //Function calculates the global node number of local node "locNodeNum" in the element "elemNum"
     //with local node 0 equal to global node "node0".

	//NOTE that local node numbering goes counter clockwise and from bottom to top starting at
	//the lower left cell corner as shown below
	//
	//		7----6
	//      |    |    at the upper level (higher z)
	//      |    |
	//      4----5
	//
	//
	//
	//		3----2
	//      |    |    at the lower level (lower z)
	//      |    |
	//      0----1


	 int node0 = get_node0(elemNum);

     int node;

     if(locNodeNum==0)
          node=node0;
	 else if(locNodeNum==1)
          node=node0+1;
     else if(locNodeNum==2)
          node=node0+ncols+1;
     else if(locNodeNum==3)
          node=node0+ncols;
     else if(locNodeNum==4)
          node=node0+(ncols*nrows);
     else if(locNodeNum==5)
          node=node0+(ncols*nrows)+1;
     else if(locNodeNum==6)
          node=node0+(ncols*nrows)+ncols+1;
     else if(locNodeNum==7)
          node=node0+(ncols*nrows)+ncols;
     else
		  throw std::logic_error("Error in function \"get_global_node()\"");

     return node;
}

int Mesh::get_global_node(const int &locNodeNum, const int &cell_i, const int &cell_j, const int &cell_k) const
{
     //Function calculates the global node number of local node "locNodeNum" in the element "elemNum"
     //with local node 0 equal to global node "node0".

	//NOTE that local node numbering goes counter clockwise and from bottom to top starting at
	//the lower left cell corner as shown below
	//
	//		7----6
	//      |    |    at the upper level (higher z)
	//      |    |
	//      4----5
	//
	//
	//
	//		3----2
	//      |    |    at the lower level (lower z)
	//      |    |
	//      0----1

	 int node0 = get_node0(cell_i, cell_j, cell_k);

     int node;

     if(locNodeNum==0)
          node=node0;
	 else if(locNodeNum==1)
          node=node0+1;
     else if(locNodeNum==2)
          node=node0+ncols+1;
     else if(locNodeNum==3)
          node=node0+ncols;
     else if(locNodeNum==4)
          node=node0+(ncols*nrows);
     else if(locNodeNum==5)
          node=node0+(ncols*nrows)+1;
     else if(locNodeNum==6)
          node=node0+(ncols*nrows)+ncols+1;
     else if(locNodeNum==7)
          node=node0+(ncols*nrows)+ncols;
     else
		  throw std::logic_error("Error in function \"get_global_node()\"");

     return node;
}


int Mesh::get_node_type(const int &i, const int &j, const int &k) const
{
     //This function returns the "type" of node.  The node is specified using the i,j,k notation.
     int test=0;
     if((i==0)||(i==nrows-1))
     {
          test=test+1;
     }
     if((j==0)||(j==ncols-1))
     {
          test=test+1;
     }
     if((k==0)||(k==nlayers-1))
     {
          test=test+1;
     }
     return test;   //if test=0 => internal node
                    //   test=1 => surface node
                    //   test=2 => edge node
                    //   test=3 => corner node
}

bool Mesh::inMeshXY(double x, double y) const
{
    if(x<get_minX() || x>get_maxX() || y<get_minY() || y>get_maxY())
        return false;
    else
        return true;
}

void Mesh::set_meshResolution(double resolution, lengthUnits::eLengthUnits units)
{
    //set mesh resolution, always stored in meters
    if(resolution<0.0)
        throw std::range_error("Mesh resolution out of range in Mesh::set_meshResolution().");

    meshResolutionUnits = units;
    lengthUnits::toBaseUnits(resolution, units);
    meshResolution = resolution;
}

void Mesh::set_targetNumHorizCells(long cells)
{
    //set the target number of horizontal cells
    if(cells<0)
        throw std::range_error("The target number of cells must be greater than zero.");
    targetNumHorizCells = cells;
}

void Mesh::set_meshResChoice(eMeshChoice choice)
{
    meshResChoice = choice;
    if(meshResChoice == coarse)
        set_targetNumHorizCells(coarseTargetCells);
    else if(meshResChoice == medium)
        set_targetNumHorizCells(mediumTargetCells);
    else if(meshResChoice == fine)
        set_targetNumHorizCells(fineTargetCells);
    else
        throw std::range_error("The mesh resolution choice has been set improperly.");
}

void Mesh::compute_cellsize(Elevation& dem)
{
     double nXcells, nYcells, Xlength, Ylength, Xcellsize, Ycellsize;

     Xlength=(dem.get_nCols()+1)*dem.get_cellSize();
     Ylength=(dem.get_nRows()+1)*dem.get_cellSize();

     nXcells=2*std::sqrt((double)targetNumHorizCells)*(Xlength/(Xlength+Ylength));
     nYcells=2*std::sqrt((double)targetNumHorizCells)*(Ylength/(Xlength+Ylength));

     Xcellsize=Xlength/nXcells;
     Ycellsize=Ylength/nYcells;
     meshResolution=(Xcellsize+Ycellsize)/2;

     meshResolutionUnits = lengthUnits::meters;
}

/*
** Compute the domain height for the mesh.
**
** The first cell height is determined by the horizontal cellsize and the limit
** on the aspect ratio.
**
** The total domain height (height above ground) using the equation Isaac put
** together.
**
** Check and make sure the domain height isn't below the maximum roughness plus
** the output wind height.
**
** Reference the domain height against the mean sea level datum (add the max
** DEM value).
*/

void Mesh::compute_domain_height(WindNinjaInputs& input)
{
    double first_cell_ht=meshResolution/maxAspectRatio;
    domainHeight=(first_cell_ht*(std::pow(vertGrowth,double(numVertLayers))-1)/(vertGrowth-1));
    if(domainHeight < 3*(input.outputWindHeight + input.surface.Rough_h.get_maxValue()))
    {
        domainHeight = 3*(input.outputWindHeight + input.surface.Rough_h.get_maxValue());
    }
    domainHeight=domainHeight + input.dem.get_maxValue();
}

void Mesh::set_domainHeight(double height, lengthUnits::eLengthUnits units)
{
    //set domain height, always stored in meters
    domainHeightUnits = units;
    lengthUnits::toBaseUnits(height, units);
    domainHeight = height;
}

void Mesh::set_numVertLayers(long layers)
{
    //set the number of vertical layers in the mesh
    if(layers < 0)
        throw std::range_error("Number of vertical layers less than zero in Mesh::set_numVertLayers().");

    numVertLayers = layers;
}

void Mesh::set_vertGrowth(double growth)
{
    //set the growth rate vertically of the layers (must be greater than 1)
    if(growth < 1.0)
        throw std::range_error("Vertical cell growth parameter is less than one in Mesh::set_vertGrowth().");

    vertGrowth = growth;
}

/**
* @brief Checks if point in a wx model mesh is within the wn mesh extents.
* Right now, just checks x-y extent.
* @param wnMesh WN computational mesh.
* @param i Row in wx model mesh.
* @param j Column in wx model mesh.
* @return true if in bounds, otherwise false.
*
*/
bool Mesh::checkInBounds(const Mesh &wnMesh, const int &i, const int &j)
{
    if(this->XORD(i,j,0) > wnMesh.XORD(0, 0, 0) && 
       this->YORD(i,j,0) > wnMesh.YORD(0, 0, 0) &&
       this->XORD(i,j,0) < wnMesh.XORD(wnMesh.nrows-1, wnMesh.ncols-1, 0) && 
       this->YORD(i,j,0) < wnMesh.YORD(wnMesh.nrows-1, wnMesh.ncols-1, 0)){ //just check ground layer x,y
        return true;
    }
    else{
        return false;
    }
}
