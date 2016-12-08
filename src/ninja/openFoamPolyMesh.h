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
    openFoamPolyMesh(std::string outputPath, Mesh mesh, wn_3dScalarField const& uwind, wn_3dScalarField const& vwind, wn_3dScalarField const& wwind);
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
    void printPoints();
    void print3dArrayPoints();

    //assigns the number of neighbor's per cell and cell owner's values
    //prints the cellOwners into the owners file
    void printOwners();

    //sets up the neighbors file
    void printNeighbors();

    //prints the face point indexes for the faces file
    void printFaces();

    //prints the boundary file
    void printBoundaries();

//set up extra stuff to be moved elsewhere

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

private:

    std::string foam_version;	//the foam version number, probably will get rid of this later
    FILE *fzout;		//changing file to which information will be written

    double xpoints;		//number of points in the x direction
    double ypoints;		//number of points in the y direction
    double zpoints;		//number of points in the z direction
    double xcells;      //number of cells in the x direction
    double ycells;      //number of cells in the y direction
    double zcells;      //number of cells in the z direction
    double npoints;     //total number of points in the mesh
    double ncells;      //total number of cells in the mesh
    double nfaces;      //total number of faces in the mesh. Also the number of owners in the mesh and the number of internal mesh faces
    double ninternalfaces;  //total number of neighbors in the mesh
    double Ax;		//number of faces in the x-z plane, also the number of faces in the north and south patches
    double Ay;		//number of faces in the y-z plane, also the number of faces in the west and east patches
    double Az;		//number of faces in the x-y plane, also the number of faces in the minZ and maxZ patches

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

};


#endif	//OPENFOAMPOLYMESH_H

