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

#include "openFoamPolyMesh.h"

openFoamPolyMesh::openFoamPolyMesh()
{
    foam_version = "2.0";
    polyMesh_path = "/home/latwood/Downloads/case/constant/polyMesh/";
    fzout = NULL;

    xmax = 4;
    ymax = 5;
    zmax = 3;
    Ax = (xmax-1)*(zmax-1);
    Ay = (ymax-1)*(zmax-1);
    Az = (xmax-1)*(ymax-1);
    writePolyMeshFiles();
}

openFoamPolyMesh::openFoamPolyMesh(std::string outputPath, double lastx, double lasty, double lastz)
{

    polyMesh_path = outputPath;
    xmax = lastx;
    ymax = lasty;
    zmax = lastz;

    foam_version = "2.0";
    fzout = NULL;
    Ax = (xmax-1)*(zmax-1);
    Ay = (ymax-1)*(zmax-1);
    Az = (xmax-1)*(ymax-1);

    writePolyMeshFiles();

}

openFoamPolyMesh::~openFoamPolyMesh()
{

}

bool openFoamPolyMesh::writePolyMeshFiles()
{
    //this outputs the points as a vtk (helps to look at the mesh a second way.
    //Plus I know this works for the tutorial cases
    std::string current_path;

    current_path = polyMesh_path+"points.vtk";
    fzout = fopen(current_path.c_str(), "w");
    fprintf(fzout, "# vtk DataFile Version 3.0\n");
    fprintf(fzout, "This is a ground surface written by WindNinja.  It is on a structured grid.\n");
    fprintf(fzout, "ASCII\n\n");
    fprintf(fzout, "DATASET STRUCTURED_GRID\n");
    fprintf(fzout, "DIMENSIONS %0.0lf %0.0lf %0.0lf\n", xmax, ymax, zmax);
    fprintf(fzout, "POINTS %0.0lf double\n", xmax*ymax*zmax);
    for(double k=0; k<zmax; k++)
    {
            for(double j=0; j<ymax; j++)
        {
            for (double i = 0;i<xmax;i++)
            {
                fprintf(fzout, "%lf %lf %lf\n", i, j, k);
            }
        }
    }
    fclose(fzout);

    //now create points as an OpenFOAM file
    current_path = polyMesh_path+"points";
    fzout = fopen(current_path.c_str(), "w");
    makeFoamHeader("vectorField","points","constant/polyMesh");
    printPoints();
    makeFoamFooter();
    fclose(fzout);

    //now create the faces
    current_path = polyMesh_path+"faces";
    fzout = fopen(current_path.c_str(), "w");
    makeFoamHeader("faceList","faces","constant/polyMesh");
    printFaces();
    makeFoamFooter();
    fclose(fzout);

    return true;
}

void openFoamPolyMesh::makeFoamHeader(std::string theClassType, std::string theObjectType, std::string theFoamFileLocation)
{
    fprintf(fzout,"/*--------------------------------*- C++ -*----------------------------------*\\\n");
    fprintf(fzout,"| =========                 |                                                 |\n");
    fprintf(fzout,"| \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |\n");
    fprintf(fzout,"|  \\\\    /   O peration     | Version:  2.2.0                                 |\n");
    fprintf(fzout,"|   \\\\  /    A nd           | Web:      www.OpenFOAM.org                      |\n");
    fprintf(fzout,"|    \\\\/     M anipulation  |                                                 |\n");
    fprintf(fzout,"\\*---------------------------------------------------------------------------*/\n");
    fprintf(fzout,"FoamFile\n{\n");
    fprintf(fzout,"    version     %s;\n",foam_version.c_str());
    fprintf(fzout,"    format      ascii;\n");
    fprintf(fzout,"    class       %s;\n",theClassType.c_str());
    if (theFoamFileLocation != "" )
    {
        fprintf(fzout,"    location    \"%s\";\n",theFoamFileLocation.c_str());
    }
    fprintf(fzout,"    object      %s;\n",theObjectType.c_str());
    fprintf(fzout,"}\n// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //\n\n");
}

void openFoamPolyMesh::makeFoamFooter()
{
    fprintf(fzout,"\n\n// ************************************************************************* //");
}

void openFoamPolyMesh::printPoints()
{
    fprintf(fzout, "\n%0.0lf\n(\n", xmax*ymax*zmax);
    for(double k=0; k<zmax; k++)
    {
            for(double j=0; j<ymax; j++)
        {
            for (double i = 0;i<xmax;i++)
            {
                fprintf(fzout, "(%0.0lf %0.0lf %0.0lf)\n", i, j, k);
            }
        }
    }
    fprintf(fzout, ")\n");
}

void openFoamPolyMesh::printFaces()
{
    fprintf(fzout, "\n%0.0lf\n(\n",Ax*ymax+Ay*xmax+Az*zmax);
    for(double i=0; i<xmax; i++)
    {
        fprintf(fzout, "4(%0.0lf %0.0lf %0.0lf %0.0lf)\n", i, xmax*ymax+i, xmax*ymax+xmax+i,xmax+i);
    }
    for(double j=0; j<ymax; j++)
    {
        fprintf(fzout, "4(%0.0lf %0.0lf %0.0lf %0.0lf)\n", xmax*j, xmax*j+1, xmax*j+xmax*ymax+1,xmax*j+xmax*ymax);
    }
    for(double k=0; k<zmax; k++)
    {
        fprintf(fzout, "4(%0.0lf %0.0lf %0.0lf %0.0lf)\n", xmax*ymax*k, xmax*ymax*k+1, xmax*ymax*k+xmax, xmax*ymax*k+xmax+1);
    }
    fprintf(fzout, ")\n");
}



/* my notes:::

    //note that in the element.cpp, there are lots of times when it calls mesh stuff to get different coordinates or values, especially in the get_ij or get_uvw functions! It also has it's own functions called interpolate taht get what is actually wanted that is in the coordinates but not exactly the coordinates.

//foaminitialization.cpp has an interesting function call to convert from input grids to dem coincident grids by interpolation: speedInitializationGrid.interpolateFromGrid(inputVelocityGrid, AsciiGrid<double>::order0);    dirInitializationGrid.interpolateFromGrid(inputAngleGrid, AsciiGrid<double>::order0);

//okay these are useful looking functions from mesh.cpp, I'm just having trouble distinguishing the differences:
//get_elemNum   Calculates the global element number given the element's (i,j,k) index
//get_elemIndex   Calculates the element's (i,j,k) index given the global element number elemNum
//get_global_node    Calculates the global node number of local node "locNodeNum" in the element "elemNum"
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
//I think the idea is you have the cells pictured like this and it takes the cell index you would have and gets the actual one that it has? It gets the 0node in terms of global values, then adds one node to it to find the next node and so on till it gets what all the different parts are.

//get_node_type  //if test=0 => internal node
                    //   test=1 => surface node
                    //   test=2 => edge node
                    //   test=3 => corner node

//inMeshXY  checks to see if it is an xy in the mesh
//checkInBounds   checks to see if it is internal to the mesh

 from NinjaFoam it looks like a way to get the case directory set up
    if(CheckForValidCaseDir(pszFoamPath) != NINJA_SUCCESS){
        status = GenerateNewCase();
        if(status != 0){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error setting up new OpenFOAM case");
            return NINJA_E_OTHER;
        }
    }
    else{ //otherwise, we're just updating an existing case
        status = UpdateExistingCase();
        if(status != 0){
            input.Com->ninjaCom(ninjaComClass::ninjaNone, "Error setting up existing case.");
            return NINJA_E_OTHER;
        }
    }

//there are also nice update whatever foam control dict. I could make something nice like that for running the smoke transport. there's also functions for writing each separate folder and for writing each separate condition, so like write boundry patch or stuff like that.
//ninjafoam copy file looks like a very useful piece! Though I don't understand why a key is needed.
//okay writeFoamFiles() might be the way to generate a new case.
//here is the part in foamfiles that seems to work:

    if( osFullPath.find("0") == 0){
                WriteZeroFiles(fin, fout, pszFilename);
            }
            else if( osFullPath.find("system") == 0 ){
                WriteSystemFiles(fin, fout, pszFilename);
            }
            else if( osFullPath.find("constant") == 0 ){
                WriteConstantFiles(fin, fout, pszFilename);
            }
*/
//almost better off just to figure out how to read/write your own man! and make sure it doesn't already exist

