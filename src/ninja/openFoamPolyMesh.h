/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for writing native solver polyMesh files
 * Author:   Loren Atwood <pianotocador@gmail.com>
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

#ifndef OPENFOAMPOLYMESH_H
#define OPENFOAMPOLYMESH_H

#include <fstream>
#include <vector>
#include <math.h>

//volVTK includes
//#include <stdio.h>
//#include <string>

#include "mesh.h"
#include "element.h"
#include "wn_3dArray.h"
#include "wn_3dScalarField.h"
#include "ninjaException.h"

/*
currently this program is set up to populate the constant/polymesh directory in a given case folder with files that replicate the OpenFoam cavity tutorial, not the windninja mesh points. The idea is to get a better understanding of how the faces and neighbors need to be written. After this works for normal points, I'm going to adjust it to mesh specific stuff.
*/


class openFoamPolyMesh
{
public:

    openFoamPolyMesh(std::string outputPath, double nxcells, double nycells, double nzcells, double x0, double xf, double y0, double yf, double z0, double zf);
    openFoamPolyMesh(std::string outputPath, Mesh mesh, double xllCornerValue, double yllCornerValue, wn_3dScalarField const& uwind, wn_3dScalarField const& vwind, wn_3dScalarField const& wwind);
    ~openFoamPolyMesh();

    //this is the equivalent main function
    bool writePolyMeshFiles(std::string pointWriteType, element elem);

    //technically not necessary, but it makes it nice for debugging since it makes it easier to replicate
    //and compare with OpenFoam tutorial files.
    void makeFoamHeader(std::string theClassType, std::string theObjectType, std::string theFoamFileLocation);

    //technically not necessary, but it makes it nice for debugging since it makes it easier to replicate
    //and compare with OpenFoam tutorial files.
    void makeFoamFooter();

    //prints the points in the points file
    //note that this exact format gives the exact replicate of the points file from the tutorial
    //note I want to allow decimals for the points when I do it for the regular mesh. Just the tutorial happens to have clean numbers with no decimals and this makes it more comparable
    void printPoints(std::string pointWriteType);

    //assigns the number of neighbor's per cell and cell owner's values
    //prints the cellOwners into the owners file
    void printOwners();

    //sets up the neighbors file
    void printNeighbors();

    //prints the face point indexes for the faces file
    void printFaces();

    //stuff for printing the boundary file
    void printBoundaryPatch(std::string patchName,std::string patchType,double nFaces,double startFace,std::string physicalType);  //for printing a patch in the boundaries. I noticed there was a lot of repetition with only one or two small parts changing for each patch and thought it would make a good function.
    void printBoundaries();     //for printing the boundary file, calls printBoundaryPatch();

//set up extra stuff to be moved elsewhere

    //just a way to group a lot of stuff I was doing in single lines over and over. Should make it more legible and shrink the number of lines of code.
    //was having a rough time thinking of a good name so maybe need to change this. Was trying to be consistent with names in openFoam format
    //also, this is designed for only one type of value per patch, which is all right with the basic OpenFoam stuff, but I noticed that in other types of files, there were multiple values given in a single patch, so multiple parts of a single variable each with their value.
    void printListHeader(std::string patchName,std::string ListType,std::string ListValue,bool extraReturn);

    //prints the T file in the 0 directory
    void printScalar();

    //prints the initial source file in the 0 directory
    void printSource();

    //prints the velocities in the 0 directory
    /*
     * the velocities appear to be stored as cell center values for the internal part of the mesh,
     * and face center values for the external patches, placed in the same order that the points
     * are generated in the mesh conversion.
    */
    void printVelocity(std::string pointWriteType, element elem);

    //prints the densities in the 0 directory
    /*
     * looks to be the volumetric flux across faces. Currently not needed, but could be in the future
     * but we need to figure out how to replicate phi from velocities and scalar for this to work correctly
    */
    void printPhi(element elem);

private:

    std::string foam_version;	//the foam version number, probably will get rid of this later
    FILE *fzout;		//changing file to which information will be written

    double xpoints;		//number of points in the x direction
    double ypoints;		//number of points in the y direction
    double zpoints;		//number of points in the z direction
    double hozRes;      //horizontal resolution of the mesh

    double demCornerX;  //x coordinate adjuster for taking the dem back to normal coordinates
    double demCornerY;  //y coordinate adjuster for taking the dem back to normal coordinates

    double xcells;      //number of cells in the x direction
    double ycells;      //number of cells in the y direction
    double zcells;      //number of cells in the z direction
    double npoints;     //total number of points in the mesh
    double ncells;      //total number of cells in the mesh
    double nfaces;      //total number of faces in the mesh. Also the number of owners in the mesh and the number of internal mesh faces
    double ninternalfaces;  //total number of neighbors in the mesh
    double Axpoints;    //number of points in the x-z plane, also the number of points in the north and south patches
    double Aypoints;    //number of points in the y-z plane, also the number of points in the west and east patches
    double Azpoints;	//number of points in the x-y plane, also the number of points in the minZ and maxZ patches
    double Axcells;		//number of faces/cells in the x-z plane, also the number of faces/cells in the north and south patches
    double Aycells;		//number of faces/cells in the y-z plane, also the number of faces/cells in the west and east patches
    double Azcells;		//number of faces/cells in the x-y plane, also the number of faces/cells in the minZ and maxZ patches

    double xmin;    //smallest x coordinate
    double xmax;    //largest x coordinate
    double ymin;    //smallest y coordinate
    double ymax;    //largest y coordinate
    double zmin;    //smallest z coordinate
    double zmax;    //largest z coordinate

    std::string pointsPath;    //path to the points file, found in constant/polymesh from the case directory
    std::string ownerPath;    //path to the owner file, found in constant/polymesh from the case directory
    std::string neighbourPath;    //path to the neighbour file, found in constant/polymesh from the case directory
    std::string facesPath;    //path to the faces file, found in constant/polymesh from the case directory
    std::string boundaryPath;    //path to the boundary file, found in constant/polymesh from the case directory

    wn_3dArray x, y, z;  //the x,y,z values for the mesh

    //values used in other files
    std::string transportPropertiesPath;    //path to the transportProperties file, found in constant from the case directory
    double diffusivityConstant;    //the diffusivity constant

    std::string scalarPath;     //path to the scalar T data file, found in 0 from the case directory
    std::string sourcePath;     //path to the source data file, found in 0 from the case directory
    std::string velocityPath;   //path to the velocity U data file, found in 0 from the case directory

    wn_3dScalarField u, v, w; //the velocity profiles in each direction for the mesh

    std::string phiPath;    //path to the density phi data file, found in 0 from the case directory


};


#endif	//OPENFOAMPOLYMESH_H

