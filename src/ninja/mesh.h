/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing 3D meshes
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

#ifndef MESH_H
#define MESH_H

#include "gdal_priv.h"
#include "wn_3dArray.h"
#include "WindNinjaInputs.h"
#include "ninjaUnits.h"
#include "ninjaException.h"

#include "element.h"

class Mesh
{
public:
	Mesh();
	~Mesh();

	Mesh(Mesh const& m);               // Copy constructor
	Mesh& operator= (Mesh const& m);   // Assignment operator

	enum eMeshChoice{
	    coarse,
	    medium,
	    fine
	};

	int		NUMNP;	//number of nodal points
	int		NUMEL;  //number of elements
					//hexahedral elements are being used
    int		NNPE;	//number of nodes per element
	wn_3dArray	XORD;
	wn_3dArray	YORD;
	wn_3dArray	ZORD; 
	int		nrows;        //number of rows of NODES
	int		ncols;        //number of cols of NODES
	int		nlayers;      //number of layers of NODES
	int		nrowsElem;    //number of rows of ELEMENTS
	int		ncolsElem;    //number of cols of ELEMENTS
	int		nlayersElem;  //number of layers of ELEMENTS
	lengthUnits::eLengthUnits meshResolutionUnits;     //distance units of mesh resolution (feet, meters, miles, kilometers)
	double meshResolution;      //horizontal mesh resolution of model domain (usually 50 m to 400 m or so)
	lengthUnits::eLengthUnits domainHeightUnits;        //distance units of domain height
	double domainHeight;        //height of top of domain
	long numVertLayers;         //number of vertical layers in mesh (usually 20-30 layers)
	double vertGrowth;          //growth of cells (layers) vertically in mesh (must be greater than 1, typically 1.3)
	eMeshChoice meshResChoice;
	long targetNumHorizCells;
	double maxAspectRatio;
	long coarseTargetCells, mediumTargetCells, fineTargetCells;

	int get_node0(const int &elemNum) const;
	int get_node0(const int &elem_i, const int &elem_j, const int &elem_k) const;
	int get_elemNum(const int &elem_i, const int &elem_j, const int &elem_k) const;
	void get_elemIndex(const int &elemNum, int &elem_i, int &elem_j, int &elem_k) const;
	int get_global_node(const int &locNodeNum, const int &elemNum) const;
	int get_global_node(const int &locNodeNum, const int &cell_i, const int &cell_j, const int &cell_k) const;
	int get_node_type(const int &i, const int &j, const int &k) const;
    double get_minX() const {return XORD(0, 0, 0);}
    double get_minY() const {return YORD(0, 0, 0);}
    double get_maxX() const {return XORD(XORD.rows_ - 1, XORD.cols_ - 1, 0);}
    double get_maxY() const {return YORD(XORD.rows_ - 1, XORD.cols_ - 1, 0);}
    bool inMeshXY(double x, double y) const;    //checks if x,y point is in mesh (doesn't check z direction)

    void set_meshResolution(double resolution, lengthUnits::eLengthUnits units);
    void set_targetNumHorizCells(long cells);     //sets the target number of horizontal cells in the mesh and computes the cellsize
    void set_meshResChoice(eMeshChoice choice);               //sets the cellsize based on user selection of coarse, medium, or fine (and returns the cellsize, on error returns cellsize < 0)
    void compute_cellsize(Elevation& dem);                  //utility function to compute the horizontal cellsize given a target number of horizontal cells (and DEM)
    void compute_domain_height(WindNinjaInputs& input);
    void set_domainHeight(double height, lengthUnits::eLengthUnits units);
    void set_numVertLayers(long layers);
    void set_vertGrowth(double growth);

	void buildFrom3dWeatherModel(const WindNinjaInputs &input,
                                 const wn_3dArray &elevationArray,
                                 double dxWX, int nrowsWX, 
                                 int ncolsWX, int nlayersWX,
                                 double xOffset, double yOffset);		//build a mesh from a 3d weather model file
	void buildStandardMesh(WindNinjaInputs& input);				//build the "standard" WindNinja mesh using domain top, numbers of cells, grow, etc...

    bool checkInBounds(const Mesh &wnMesh, const int &i, const int &j);  // checks if WX mesh point is within WN x-y extent

private:

	double get_z(const int& i, const int& j, const int& k, const double& elev);
	double get_aspect_ratio(int NUMEL, int NUMNP, wn_3dArray& XORD, wn_3dArray& YORD, wn_3dArray& ZORD, int nrows, int ncols, int nlayers);
	double get_equiangle_skew(int NUMEL, int NUMNP, wn_3dArray& XORD, wn_3dArray& YORD, wn_3dArray& ZORD, int nrows, int ncols, int nlayers);
	void get_cell_angles(double xa, double ya, double za, double xb, double yb, double zb, double xc, double yc, double zc, double xd, double yd, double zd, double &cell_max_angle, double &cell_min_angle);
	double get_angle(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3);
	double maxj(double value1, double value2);
};

#endif /* MESH_H */
