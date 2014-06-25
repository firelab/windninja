/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing a 3D field of scalars (linked with a mesh for
 *               spatial information)
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

#include "wn_3dScalarField.h"

wn_3dScalarField::wn_3dScalarField()
{
	mesh_ = NULL;
}

wn_3dScalarField::wn_3dScalarField(Mesh const* m)
{
    allocate(m);
}

wn_3dScalarField::~wn_3dScalarField()
{
}

wn_3dScalarField::wn_3dScalarField(wn_3dScalarField const& f)	// Copy constructor
{
	allocate(f.mesh_);
	scalarData_ = f.scalarData_;
}

wn_3dScalarField& wn_3dScalarField::operator= (wn_3dScalarField const& f)	// Assignment operator
{
	if(&f != this)
	{
		allocate(f.mesh_);
		scalarData_ = f.scalarData_;
	}
	return *this;
}

void wn_3dScalarField::allocate(Mesh const* m)
{
    if(m == NULL)
    {
        mesh_ = NULL;
        scalarData_.allocate(0,0,0);
    }else{


#ifdef NINJA_DEBUG
        if (m->nrows <= 0 || m->ncols <= 0 || m->nlayers <= 0)
            throw std::range_error("Rows, columns, or layers are less than or equal to 0 in wn_3dScalarField::wn_3dScalarField(mesh const* m).");
#endif

        mesh_ = m;
        scalarData_.allocate(mesh_->nrows, mesh_->ncols, mesh_->nlayers);
    }
}

void wn_3dScalarField::deallocate()
{
	scalarData_.deallocate();
}

/**
 * @brief Interpolate a wn_3dScalarField from one mesh to another.
 * @param newScalarData The new wn_3dScalarField to be populated.
 * @param mesh WindNinja mesh.
 * @param input WindNinja inputs.
 */

void wn_3dScalarField::interpolateScalarData(wn_3dScalarField &newScalarData,
                                             Mesh const& mesh,
                                             WindNinjaInputs const& input)
{
    element elem(&mesh);
    element elem_wx(this->mesh_);

	int wx_i; //element index for wx model
	int elem_wx_i, elem_wx_j, elem_wx_k; // wx model cells
    int elem_i, elem_j, elem_k; // wn cells

    double x, y, z, x_wx, y_wx, z_wx;
    double u, v, w;
    double u_wx, v_wx, w_wx;

    AsciiGrid<double> wnNormDistanceGrid; // stores normalizing distances for WN grid
    AsciiGrid<double> wxNormDistanceGrid; // stores normalizeing distances for WX grid
    
    wnNormDistanceGrid.set_noDataValue(-9999.0);
    wnNormDistanceGrid.set_headerData(mesh.ncols, mesh.nrows,
                            mesh.XORD(0,0,0), mesh.YORD(0,0,0),
                            mesh.meshResolution, -9999.0, 0.0,
                            input.dem.prjString.c_str());
    
    wxNormDistanceGrid.set_noDataValue(-9999.0);
    wxNormDistanceGrid.set_headerData(mesh.ncols, mesh.nrows,
                            mesh.XORD(0,0,0), mesh.YORD(0,0,0),
                            mesh.meshResolution, -9999.0, 0.0,
                            input.dem.prjString.c_str());

    wnNormDistanceGrid = -1;
    wxNormDistanceGrid = -1;

    int elemNum;
    double normDist; //normalized distance to WN top height (0-1)

    /*
     * populate wnNormDistanceGrid and wxNormDistanceGrid
     * requires looping through WN gorund nodes to find
     * WN node locations in the WX mesh so we can find elevations
     */

    for(int i=0; i<mesh.nrows; i++){
        for(int j=0; j<mesh.ncols; j++){
            wnNormDistanceGrid(i,j) = mesh.ZORD(i,j,mesh.nlayers-1) - mesh.ZORD(i,j,0); //distance from WN ground to Ztop

            if(i == mesh.nrows-1 || j == mesh.ncols-1){
                continue;
            }
            elemNum = mesh.get_elemNum(i,j,0); // get element number in WN mesh for i,j
            elem.get_xyz(elemNum, -1, -1, -1, x, y, z); // get x,y,z for local node 0
            elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get i,j and u,v coordinates at x,y
            elem_wx_k = 0;
            wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
            elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground

            wxNormDistanceGrid(i,j) = mesh.ZORD(i,j,mesh.nlayers-1) - z_wx; //distance from WX ground to Ztop

            if(i == mesh.nrows-2 && j == mesh.ncols-2){ //if at upper right corner
                elem.get_xyz(elemNum, 1, -1, -1, x, y, z); // get x,y,z for local node 1
                elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get i,j and u,v coordinates at x,y
                elem_wx_k = 0;
                wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
                elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground
                wxNormDistanceGrid(i,j+1) = mesh.ZORD(i,j+1,mesh.nlayers-1) - z_wx; //distance from WX ground to Ztop

                elem.get_xyz(elemNum, 1, 1, -1, x, y, z); // get x,y,z for local node 2
                elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get i,j and u,v coordinates at x,y
                elem_wx_k = 0;
                wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
                elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground
                wxNormDistanceGrid(i+1,j+1) = mesh.ZORD(i+1,j+1,mesh.nlayers-1) - z_wx; //distance from WX ground to Ztop

                elem.get_xyz(elemNum, -1, 1, -1, x, y, z); // get x,y,z for local node 3
                elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get i,j and u,v coordinates at x,y
                elem_wx_k = 0;
                wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
                elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground
                wxNormDistanceGrid(i+1,j) = mesh.ZORD(i+1,j,mesh.nlayers-1) - z_wx; //distance from WX ground to Ztop
            }
            else if(i == mesh.nrows-2){ //get top-most row elevations
                elem.get_xyz(elemNum, -1, 1, -1, x, y, z); // get x,y,z for local node 3
                elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get i,j and u,v coordinates at x,y
                elem_wx_k = 0;
                wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
                elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground
                wxNormDistanceGrid(i+1,j) = mesh.ZORD(i+1,j,mesh.nlayers-1) - z_wx; //distance from WX ground to Ztop
            }
            else if(j == mesh.ncols-2){ //get right-most column elevations
                elem.get_xyz(elemNum, 1, -1, -1, x, y, z); // get x,y,z for local node 1
                elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get i,j and u,v coordinates at x,y
                elem_wx_k = 0;
                wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
                elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground
                wxNormDistanceGrid(i,j+1) = mesh.ZORD(i,j+1,mesh.nlayers-1) - z_wx; //distance from WX ground to Ztop
            }
        }
    }//wnNormDistanceGrid.write_Grid("wnNormDistGrid", 2);
    //wxNormDistanceGrid.write_Grid("wxNormDistGrid", 2);

    wn_3dScalarField heightAboveGround;
    heightAboveGround.allocate(&mesh);

    for(int i = 0; i < mesh.nrows; i++){
        for(int j = 0; j < mesh.ncols; j++){
            for(int k = 0; k < mesh.nlayers; k++){
                heightAboveGround(i,j,k) = mesh.ZORD(i,j,k) - mesh.ZORD(i,j,0);
            }
        }
    }

    for(int i = 0; i < mesh.NUMEL; i++){       // Start loop over elements in WN mesh
        mesh.get_elemIndex(i, elem_i, elem_j, elem_k);  // get i,j,k index for element i in WN mesh

        if(elem_k != 0 && elem_k % 2 != 0){ //only loop over even layers of elements so we don't interpolate same nodes twice
            if(elem_k != mesh.nlayersElem -1){ // loop through last layer no matter what to get topmost nodes
                continue;
            }
        }
        if(elem_i != 0 && elem_i % 2 != 0){ //only loop over even rows of elements so we don't interpolate same nodes twice
            if(elem_i != mesh.nrowsElem - 1){ // loop through last row no matter what to get topmost nodes
                continue;
            }
        }

        //----- node 0 -------------------------------------------------------
        elem.get_xyz(i, -1.0, -1.0, -1.0, x, y, z); // get x,y,z for local node 0
        elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get u and v coordinates
        elem_wx_k = 0;
        wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
        elem_wx.get_xyz(wx_i, u_wx, v_wx, -1.0, x_wx, y_wx, z_wx); //get real z_wx at wx model ground

       /*
        * Normalized Distance Method
        */

        normDist = heightAboveGround(elem_i, elem_j, elem_k)/wnNormDistanceGrid(elem_i, elem_j); //compute normalized distance (0-1)
        z = normDist * wxNormDistanceGrid(elem_i, elem_j); //compute height above ground in wx mesh
        z += z_wx; // add height above ground to wx model ground height
        

        if(elem_k == 0){// don't interpolate over ground nodes in layer 0 of WN mesh
            newScalarData(elem_i, elem_j, elem_k) = -9999.0;
        }
        else{ 
            elem_wx.get_uvw(x, y, z, elem_wx_i, elem_wx_j, elem_wx_k, u_wx, v_wx, w_wx);
        }
        

        if(elem_wx_k < 1){ //use log profile to fill below here later
            newScalarData(elem_i, elem_j, elem_k) = -9999.0;
        }
        else{
            newScalarData(elem_i, elem_j, elem_k) = this->interpolate(x, y, z);
        }

        //----- node 1 -------------------------------------------------------
        elem.get_xyz(i, 1, -1, -1, x, y, z); // get x,y,z for local node 1
        elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get u and v coordinates
        elem_wx_k = 0;
        wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
        elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground


        /*
         * Normalized Distance Method
         */

        normDist = heightAboveGround(elem_i, elem_j+1, elem_k)/wnNormDistanceGrid(elem_i, elem_j+1); //compute normalized distance (0-1)
        z = normDist * wxNormDistanceGrid(elem_i, elem_j+1); //compute height above ground in wx mesh
        z += z_wx; // add height above ground to wx model ground height
        
        if(elem_k == 0){ // don't interpolate over ground nodes in layer 0 of WN mesh
            newScalarData(elem_i, elem_j+1, elem_k) = -9999.0;
        }
        else{ 
            elem_wx.get_uvw(x, y, z, elem_wx_i, elem_wx_j, elem_wx_k, u_wx, v_wx, w_wx);
        }
        
        
        if(elem_wx_k < 1){ //use log profile to fill below here later
            newScalarData(elem_i, elem_j+1, elem_k) = -9999.0;
        }
        else{
            newScalarData(elem_i, elem_j+1, elem_k) = this->interpolate(x, y, z);
        }

        //----- node 2 -------------------------------------------------------
        elem.get_xyz(i, 1, 1, -1, x, y, z); // get x,y,z for local node 2
        elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get u and v coordinates
        elem_wx_k = 0;
        wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
        elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground

 
        /*
         * Normalized Distance Method
         */

        normDist = heightAboveGround(elem_i+1, elem_j+1, elem_k)/wnNormDistanceGrid(elem_i+1, elem_j+1); //compute normalized distance (0-1)
        z = normDist * wxNormDistanceGrid(elem_i+1, elem_j+1); //compute height above ground in wx mesh
        z += z_wx; // add height above ground to wx model ground height
                
        if(elem_k == 0){ // don't interpolate over ground nodes in layer 0 of WN mesh
            newScalarData(elem_i+1, elem_j+1, elem_k) = -9999.0;
        }
        else{ 
            elem_wx.get_uvw(x, y, z, elem_wx_i, elem_wx_j, elem_wx_k, u_wx, v_wx, w_wx);
        }
        
        
        if(elem_wx_k < 1){ //use log profile to fill below here later
            newScalarData(elem_i+1, elem_j+1, elem_k) = -9999.0;
        }
        else{
            newScalarData(elem_i+1, elem_j+1, elem_k) = this->interpolate(x, y, z);
        }

        //----- node 3 -------------------------------------------------------
        elem.get_xyz(i, -1, 1, -1, x, y, z); // get x,y,z for local node 3
        elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get u and v coordinates       
        elem_wx_k = 0;
        wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
        elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground


        /*
         * Normalized Distance Method
         */

        normDist = heightAboveGround(elem_i+1, elem_j, elem_k)/wnNormDistanceGrid(elem_i+1, elem_j); //compute normalized distance (0-1)
        z = normDist * wxNormDistanceGrid(elem_i+1, elem_j); //compute height above ground in wx mesh
        z += z_wx; // add height above ground to wx model ground height
        
        if(elem_k == 0){ // don't interpolate over ground nodes in layer 0 of WN mesh
            newScalarData(elem_i+1, elem_j, elem_k) = -9999.0;
        }
        else{ 
            elem_wx.get_uvw(x, y, z, elem_wx_i, elem_wx_j, elem_wx_k, u_wx, v_wx, w_wx);
        }
        

        if(elem_wx_k < 1){ //use log profile to fill below here later
            newScalarData(elem_i+1, elem_j, elem_k) = -9999.0;
        }
        else{
            newScalarData(elem_i+1, elem_j, elem_k) = this->interpolate(x, y, z);
        }

        //----- node 4 -------------------------------------------------------
        elem.get_xyz(i, -1, -1, 1, x, y, z); // get x,y,z for local node 4
        elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get u and v coordinates       
        elem_wx_k = 0;
        wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
        elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground

        /*
         * Normalized Distance Method
         */

        normDist = heightAboveGround(elem_i, elem_j, elem_k+1)/wnNormDistanceGrid(elem_i, elem_j); //compute normalized distance (0-1)
        z = normDist * wxNormDistanceGrid(elem_i, elem_j); //compute height above ground in wx mesh
        z += z_wx; // add height above ground to wx model ground height

        elem_wx.get_uvw(x, y, z, elem_wx_i, elem_wx_j, elem_wx_k, u_wx, v_wx, w_wx);

        if(elem_wx_k < 1){ //use log profile to fill below here later
            newScalarData(elem_i, elem_j, elem_k+1) = -9999.0;
        }
        else{
            newScalarData(elem_i, elem_j, elem_k+1) = this->interpolate(x, y, z);
        }

        //----- node 5 -------------------------------------------------------
        elem.get_xyz(i, 1, -1, 1, x, y, z); // get x,y,z for local node 5
        elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get u and v coordinates
        elem_wx_k = 0;
        wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
        elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground


        /*
         * Normalized Distance Method
         */

        normDist = heightAboveGround(elem_i, elem_j+1, elem_k+1)/wnNormDistanceGrid(elem_i, elem_j+1); //compute normalized distance (0-1)
        z = normDist * wxNormDistanceGrid(elem_i, elem_j+1); //compute height above ground in wx mesh
        z += z_wx; // add height above ground to wx model ground height

        elem_wx.get_uvw(x, y, z, elem_wx_i, elem_wx_j, elem_wx_k, u_wx, v_wx, w_wx);

        if(elem_wx_k < 1){ //use log profile to fill below here later
            newScalarData(elem_i, elem_j+1, elem_k+1) = -9999.0;
        }
        else{
            newScalarData(elem_i, elem_j+1, elem_k+1) = this->interpolate(x, y, z);
        }

        //----- node 6 -------------------------------------------------------
        elem.get_xyz(i, 1, 1, 1, x, y, z); // get x,y,z for local node 6
        elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get u and v coordinates
        elem_wx_k = 0;
        wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
        elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground


        /*
         * Normalized Distance Method
         */

        normDist = heightAboveGround(elem_i+1, elem_j+1, elem_k+1)/wnNormDistanceGrid(elem_i+1, elem_j+1); //compute normalized distance (0-1)
        z = normDist * wxNormDistanceGrid(elem_i+1, elem_j+1); //compute height above ground in wx mesh
        z += z_wx; // add height above ground to wx model ground height

        elem_wx.get_uvw(x, y, z, elem_wx_i, elem_wx_j, elem_wx_k, u_wx, v_wx, w_wx);

        if(elem_wx_k < 1){ //use log profile to fill below here later
            newScalarData(elem_i+1, elem_j+1, elem_k+1) = -9999.0;
        }
        else{
            newScalarData(elem_i+1, elem_j+1, elem_k+1) = this->interpolate(x, y, z);
        }

        //----- node 7 -------------------------------------------------------
        elem.get_xyz(i, -1, 1, 1, x, y, z); // get x,y,z for local node 7
        elem_wx.get_uv(x, y, elem_wx_i, elem_wx_j, u_wx, v_wx); //get u and v coordinates
        elem_wx_k = 0;
        wx_i = this->mesh_->get_elemNum(elem_wx_i, elem_wx_j, elem_wx_k);
        elem_wx.get_xyz(wx_i, u_wx, v_wx, -1, x_wx, y_wx, z_wx); //get real z_wx at wx model ground


        /*
         * Normalized Distance Method
         */

        normDist = heightAboveGround(elem_i+1, elem_j, elem_k+1)/wnNormDistanceGrid(elem_i+1, elem_j); //compute normalized distance (0-1)
        z = normDist * wxNormDistanceGrid(elem_i+1, elem_j); //compute height above ground in wx mesh
        z += z_wx; // add height above ground to wx model ground height

        elem_wx.get_uvw(x, y, z, elem_wx_i, elem_wx_j, elem_wx_k, u_wx, v_wx, w_wx);

        if(elem_wx_k < 1){ //use log profile to fill below here later
            newScalarData(elem_i+1, elem_j, elem_k+1) = -9999.0;
        }
        else{
            newScalarData(elem_i+1, elem_j, elem_k+1) = this->interpolate(x, y, z);
        }
    }

    heightAboveGround.deallocate();
    wnNormDistanceGrid.deallocate();
    wxNormDistanceGrid.deallocate();
}

double wn_3dScalarField::interpolate(double const& x,double const& y, double const& z)
{
	//function interpolates scalar field at point (x,y,z)

	element elem(mesh_);

	double value = 0.0;
	double u, v, w;
	int cell_i, cell_j, cell_k, NPK;

	elem.get_uvw(x, y, z, cell_i, cell_j, cell_k, u, v, w);

	for(int k=0;k<mesh_->NNPE;k++)          //Start loop over nodes in the element
	{
        NPK=mesh_->get_global_node(k, cell_i, cell_j, cell_k);            //NPK is the global node number

        value=value + elem.SFNV(u, v, w, k) * scalarData_(NPK);
	}                             //End loop over nodes in the element
	return value;
}

double wn_3dScalarField::interpolate(element &elem, const int &cell_i, const int &cell_j, const int &cell_k, const double &u, const double &v, const double &w)
{
	//Function calculates the value of the scalar field (scalarData) at the cell_i, cell_j, cell_k and (u, v, w) point in "parent cell" coordinates.
	double value = 0.0;
	int NPK;

	for(int k=0;k<mesh_->NNPE;k++)          //Start loop over nodes in the element
	{
        NPK=mesh_->get_global_node(k, cell_i, cell_j, cell_k);            //NPK is the global node number

        value=value + elem.SFNV(u, v, w, k) * scalarData_(NPK);

	}                             //End loop over nodes in the element
	return value;
}

double& wn_3dScalarField::operator() (int row, int col, int layer)
{
	return scalarData_(row, col, layer);
}

double wn_3dScalarField::operator() (int row, int col, int layer) const
{
	return scalarData_(row, col, layer);
}

double& wn_3dScalarField::operator() (int num)
{
	return scalarData_(num);
}

double wn_3dScalarField::operator() (int num) const
{
	return scalarData_(num);
}

/**
 * @brief Calculate and store gradients at each node
 *
 * @param Reference to inputs
 * @param Reference to a wn_3dVectorField
 */
void wn_3dScalarField::ComputeGradient(WindNinjaInputs &input, wn_3dVectorField &derivatives)
{
    int i, j, k;
    double *DIAG;
    wn_3dScalarField x, y, z;
    DIAG = NULL;

    x.allocate(mesh_);    //x is positive toward East
    y.allocate(mesh_);    //y is positive toward North
    z.allocate(mesh_);    //z is positive up

    if(DIAG == NULL)

        DIAG=new double[mesh_->nlayers*input.dem.get_nRows()*input.dem.get_nCols()];  //DIAG is the sum of the weights at each nodal point; eventually, dPHI/dx, etc. are divided by this value to get the "smoothed" (or averaged) value of dPHI/dx at each node point

    for(i=0;i<mesh_->NUMNP;i++)    //Initialize x,y, and z
    {
        x(i)=0.;
        y(i)=0.;
        z(i)=0.;
        DIAG[i]=0.;
    }

	#pragma omp parallel default(shared) private(i,j,k)
    {

    element elem(mesh_);

    double DPHIDX, DPHIDY, DPHIDZ;
    double XJ, YJ, ZJ;
    double wght, XK, YK, ZK;
    double *xScratch, *yScratch, *zScratch, *DIAGScratch;
    xScratch=NULL;
    yScratch=NULL;
    zScratch=NULL;
    DIAGScratch=NULL;

    xScratch=new double[mesh_->nlayers*input.dem.get_nRows()*input.dem.get_nCols()];
    yScratch=new double[mesh_->nlayers*input.dem.get_nRows()*input.dem.get_nCols()];
    zScratch=new double[mesh_->nlayers*input.dem.get_nRows()*input.dem.get_nCols()];
    DIAGScratch=new double[mesh_->nlayers*input.dem.get_nRows()*input.dem.get_nCols()];

    for(i=0;i<mesh_->NUMNP;i++)     //Initialize scratch x,y, and z
    {
        xScratch[i]=0.;
        yScratch[i]=0.;
        zScratch[i]=0.;
        DIAGScratch[i]=0.;
    }

    #pragma omp for
    for(i=0;i<mesh_->NUMEL;i++)     //Start loop over elements
    {
        elem.node0 = mesh_->get_node0(i);  //get the global node number of local node 0 of element i
        for(j=0;j<elem.NUMQPTV;j++)      //Start loop over quadrature points in the element
        {
            DPHIDX=0.0;     //Set DPHI/DX, etc. to zero for the new quad point
            DPHIDY=0.0;
            DPHIDZ=0.0;

            elem.computeJacobianQuadraturePoint(j, i, XJ, YJ, ZJ);

            //Calculate dN/dx, dN/dy, dN/dz (Remember we're using the transpose of the inverse!)
            for(k=0;k<mesh_->NNPE;k++)
            {
                elem.NPK=mesh_->get_global_node(k, i);            //NPK is the global node number

                DPHIDX=DPHIDX+elem.DNDX[k]*(*this)(elem.NPK);       //Calculate the DPHI/DX, etc. for the quad point we are on
                DPHIDY=DPHIDY+elem.DNDY[k]*(*this)(elem.NPK);
                DPHIDZ=DPHIDZ+elem.DNDZ[k]*(*this)(elem.NPK);
            }

            //Now we know DPHI/DX, etc. for quad point j.  We will distribute this inverse distance weighted average to each nodal point for the cell we're on
            for(k=0;k<mesh_->NNPE;k++)     //Start loop over nodes in the element
            {                            //Calculate the Jacobian at the quad point
                elem.NPK=mesh_->get_global_node(k, i);            //NPK is the global nodal number

                XK=mesh_->XORD(elem.NPK);      //Coodinates of the nodal point
                YK=mesh_->YORD(elem.NPK);
                ZK=mesh_->ZORD(elem.NPK);

                wght=std::pow((XK-XJ),2)+std::pow((YK-YJ),2)+std::pow((ZK-ZJ),2);
                wght=1.0/(std::sqrt(wght));
//c				#pragma omp critical
                {
                    xScratch[elem.NPK]=xScratch[elem.NPK]+wght*DPHIDX;   //Here we store the summing values of DPHI/DX, etc. in the x,y,z arrays
                    yScratch[elem.NPK]=yScratch[elem.NPK]+wght*DPHIDY;
                    zScratch[elem.NPK]=zScratch[elem.NPK]+wght*DPHIDZ;
                    DIAGScratch[elem.NPK]=DIAGScratch[elem.NPK]+wght;    //Store the sum of the weights for the node
                }
            }                             //End loop over nodes in the element
        }                                 //End loop over quadrature points in the element
    }                                     //End loop over elements

    #pragma omp critical
    {
    for(i=0;i<mesh_->NUMNP;i++)
    {
        x(i) += xScratch[i];      //Dividing by the DIAG[NPK] gives the value of DPHI/DX, etc. (stored in the x,y,z arrays)
        y(i) += yScratch[i];
        z(i) += zScratch[i];
        DIAG[i] += DIAGScratch[i];
    }
    } //end critical

    if(xScratch)
    {
        delete[] xScratch;
		xScratch=NULL;
    }
    if(yScratch)
    {
        delete[] yScratch;
		yScratch=NULL;
    }
    if(zScratch)
    {
		delete[] zScratch;
		zScratch=NULL;
    }
    if(DIAGScratch)
    {
		delete[] DIAGScratch;
		DIAGScratch=NULL;
    }

    #pragma omp barrier

    #pragma omp for
    for(i=0;i<mesh_->NUMNP;i++)
    {
        x(i)=x(i)/DIAG[i];      //Dividing by the DIAG[NPK] gives the value of DPHI/DX, etc.
        y(i)=y(i)/DIAG[i];
        z(i)=z(i)/DIAG[i];
    }
    }   //end parallel section

    wn_3dVectorField gradient_vector(x, y, z);
    derivatives = gradient_vector;

    x.deallocate();  //should these be here??
    y.deallocate();
    z.deallocate();

    //cout << "gradient_vector = " << (*gradient_vector.vectorData_x)(0,0,0) << endl;
}
