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

/*
currently this program is set up to populate the constant/polymesh directory in a given case folder with files that replicate the OpenFoam cavity tutorial, not the windninja mesh points. The idea is to get a better understanding of how the faces and neighbors need to be written. After this works for normal points, I'm going to adjust it to mesh specific stuff.
*/


class openFoamPolyMesh
{
public:

    openFoamPolyMesh();
    openFoamPolyMesh(std::string outputPath, double lastx, double lasty, double lastz);
    ~openFoamPolyMesh();

    //this is the equivalent main function
    bool writePolyMeshFiles();

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

    //prints the face point indexes for the faces file
    void printFaces();

private:

    std::string foam_version;	//the foam version number, probably will get rid of this later
    std::string polyMesh_path;	//path to directory where the files will be written
    FILE *fzout;		//changing file to which information will be written

    double xmax;		//number of points in the x direction
    double ymax;		//number of points in the y direction
    double zmax;		//number of points in the z direction
    double Ax;		//number of cells in the x-z plane, or the x side of the mesh
    double Ay;		//number of cells in the y-z plane, or the y side of the mesh
    double Az;		//number of cells in the x-y plane, or the z side of the mesh

};


#endif	//OPENFOAMPOLYMESH_H

