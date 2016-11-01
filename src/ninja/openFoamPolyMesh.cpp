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

    xpoints = 4;
    ypoints = 5;
    zpoints = 3;
    xcells = xpoints-1;
    ycells = ypoints-1;
    zcells = zpoints-1;
    npoints = xpoints*ypoints*zpoints;
    ncells = xcells*ycells*zcells;
    Ax = xcells*zcells;
    Ay = ycells*zcells;
    Az = xcells*ycells; //where is the proper place to put something like this? They are constants that depend on the imput
    nfaces = Ax*ypoints+Ay*xpoints+Az*zpoints;
    ninternalfaces = 0;

    nNeighbors;
    cellOwners;

    writePolyMeshFiles();
}

openFoamPolyMesh::openFoamPolyMesh(std::string outputPath, double lastx, double lasty, double lastz)
{

    polyMesh_path = outputPath;
    xpoints = lastx;
    ypoints = lasty;
    zpoints = lastz;

    foam_version = "2.0";
    fzout = NULL;
    xcells = xpoints-1;
    ycells = ypoints-1;
    zcells = zpoints-1;
    npoints = xpoints*ypoints*zpoints;
    ncells = xcells*ycells*zcells;
    Ax = xcells*zcells;
    Ay = ycells*zcells;
    Az = xcells*ycells;
    nfaces = Ax*ypoints+Ay*xpoints+Az*zpoints;
    ninternalfaces = 0;

    nNeighbors;
    cellOwners;

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
    fprintf(fzout, "DIMENSIONS %0.0lf %0.0lf %0.0lf\n", xpoints, ypoints, zpoints);
    fprintf(fzout, "POINTS %0.0lf double\n", xpoints*ypoints*zpoints);
    for(double k=0; k<zpoints; k++)
    {
            for(double j=0; j<ypoints; j++)
        {
            for (double i = 0;i<xpoints;i++)
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

    //now create the owners OpenFOAM mesh file
    current_path = polyMesh_path+"owner";
    fzout = fopen(current_path.c_str(),"w");
    makeFoamHeader("labelList","owner","constant/polyMesh");
    printOwners();
    makeFoamFooter();
    fclose(fzout);

    //now create the neighbors OpenFOAM mesh file
    current_path = polyMesh_path+"neighbor";
    fzout = fopen(current_path.c_str(),"w");
    makeFoamHeader("labelList","neighbor","constant/polyMesh");
    printNeighbors();
    makeFoamFooter();
    fclose(fzout);

    //now create the faces
    current_path = polyMesh_path+"faces";
    fzout = fopen(current_path.c_str(), "w");
    makeFoamHeader("faceList","faces","constant/polyMesh");
    printFaces();
    makeFoamFooter();
    fclose(fzout);

    //now create the boundary file
    current_path = polyMesh_path+"boundary";
    fzout = fopen(current_path.c_str(), "w");
    makeFoamHeader("polyBoundaryMesh","boundary","constant/polyMesh");
    printBoundaries();
    makeFoamFooter();
    fclose(fzout);

    //debug viewValues
    current_path = polyMesh_path+"debugInfo";
    fzout = fopen(current_path.c_str(), "w");
    viewValues();
    fclose(fzout);

    return true;
}

void openFoamPolyMesh::viewValues()
{
    fprintf(fzout,"xpoints = %lf\n",xpoints);
    fprintf(fzout,"ypoints = %lf\n",ypoints);
    fprintf(fzout,"zpoints = %lf\n",zpoints);
    fprintf(fzout,"xcells = %lf\n",xcells);
    fprintf(fzout,"ycells = %lf\n",ycells);
    fprintf(fzout,"zcells = %lf\n",zcells);
    fprintf(fzout,"npoints = %lf\n",npoints);
    fprintf(fzout,"ncells = %lf\n",ncells);
    fprintf(fzout,"nfaces = %lf\n",nfaces);
    fprintf(fzout,"Ax = %lf\n",Ax);
    fprintf(fzout,"Ay = %lf\n",Ay);
    fprintf(fzout,"Az = %lf\n",Az);

    fprintf(fzout,"Neighbors Vector size: %zu\n",nNeighbors.size());
    for (double i = 0;i<nNeighbors.size();i++)
    {
        fprintf(fzout,"%lf\n",nNeighbors[i]);
    }

    fprintf(fzout,"Owners Vector size: %zu\n",cellOwners.size());
    for (double i=0;i<cellOwners.size();i++)
    {
        fprintf(fzout,"%lf\n",cellOwners[i]);
    }
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
    fprintf(fzout, "\n%0.0lf\n(\n", xpoints*ypoints*zpoints);
    for(double k=0; k<zpoints; k++)
    {
            for(double j=0; j<ypoints; j++)
        {
            for (double i = 0;i<xpoints;i++)
            {
                fprintf(fzout, "(%0.0lf %0.0lf %0.0lf)\n", i, j, k);
            }
        }
    }
    fprintf(fzout, ")\n");
}

void openFoamPolyMesh::printOwners()
{
    int lessValues = 0;
    bool ylimit = false;
    bool zlimit = false;

    //fill out the number of neighbors per cell and the interior cell owner information
    //note that xmax, ymax, and zmax are numbers of points, but this needed number of cells
    for (double i=0;i<ncells;i++)
    {
        if (fmod(i+1,3) == 0)
        {
            lessValues++;
        }
        if (ylimit == false && i >= (ycells-1)*xcells)
        {
            ylimit = true;
            lessValues++;
        }
        if (zlimit == false && i >= xcells*ycells*zcells-3)
        {
            zlimit = true;
            lessValues++;
        }

        nNeighbors.push_back(3-lessValues);
        //fill out the owners information for interior cells
        for (double j = 0;j<nNeighbors[i];j++)
        {
            cellOwners.push_back(i);
        }

        if (fmod(i+1,3) == 0)
        {
            lessValues--;
        }
    }

    //now update nneighbors
    ninternalfaces = cellOwners.size(); //should also be nfaces - 2*(Ax+Ay+Az)

    //now fill out the owners information for the north outside volume face
    for (double i=0;i<xcells;i++)
    {
        for (double k = 0;k<zcells;k++)
        {
            cellOwners.push_back(xcells*(ycells-1)+xcells*ycells*k+i);
        }
    }

    //now fill out the owners information for the west outside volume face
    for (double k = 0;k<zcells;k++)
    {
        for (double j = 0;j<ycells;j++)
        {
            cellOwners.push_back(xcells*j+xcells*ycells*k);
        }
    }

    //now fill out the owners information for the east outside volume face
    for (double k=0;k<zcells;k++)
    {
        for (double j=0;j<ycells;j++)
        {
            cellOwners.push_back((xcells-1)+xcells*j+xcells*ycells*k);
        }
    }

    //now fill out the owners information for the south outside volume face
    for (double i=0;i<xcells;i++)
    {
        for (double k=0;k<zcells;k++)
        {
            cellOwners.push_back(xcells*ycells*k+i);
        }
    }

    //now fill out the owners information for the bottom outside volume face
    for (double i=0;i<xcells;i++)
    {
        for (double j=0;j<ycells;j++)
        {
            cellOwners.push_back(xcells*j+i);
        }
    }

    //now fill out the owners information for the top outside volume face
    for (double i=0;i<xcells;i++)
    {
        for (double j=0;j<ycells;j++)
        {
            cellOwners.push_back(xcells*ycells*(zcells-1)+xcells*j+i);
        }
    }

    //now print out the information
    fprintf(fzout, "\n%0.0lf\n(\n",nfaces);
    for (double i = 0;i<cellOwners.size();i++)
    {
        fprintf(fzout, "%0.0lf\n",cellOwners[i]);
    }
    fprintf(fzout, ")\n");
}

void openFoamPolyMesh::printNeighbors()
{
    fprintf(fzout, "\n%0.0lf\n(\n",ninternalfaces);
    for (double j=0;j<ncells;j++)
    {
        if (nNeighbors[j] == 3)
        {
            fprintf(fzout,"%0.0lf\n",1+j);
            fprintf(fzout,"%0.0lf\n",xcells+j);
            fprintf(fzout,"%0.0lf\n",xcells*ycells+j);
        } else if (nNeighbors[j] == 2)
        {
            if (fmod(j+1,xcells) == 0)
            {
                fprintf(fzout,"%0.0lf\n",xcells+j);
                fprintf(fzout,"%0.0lf\n",xcells*ycells+j);
            } else if (j >= xcells*ycells*(zcells-1))
            {
                fprintf(fzout,"%0.0lf\n",1+j);
                fprintf(fzout,"%0.0lf\n",xcells+j);
            } else
            {
                fprintf(fzout,"%0.0lf\n",1+j);
                fprintf(fzout,"%0.0lf\n",xcells*ycells+j);
            }
        } else if (nNeighbors[j] == 1)
        {
            if (fmod(j+1,xcells*ycells) == 0)
            {
                fprintf(fzout,"%0.0lf\n",xcells*ycells+j);
            } else if (j >= xcells*ycells*(zcells-1) && j < xcells*ycells*(zcells-1)+xcells*(ycells-1))
            {
                fprintf(fzout,"%0.0lf\n",xcells+j);
            } else
            {
                fprintf(fzout,"%0.0lf\n",1+j);
            }
        }
    }
    fprintf(fzout, ")\n");
}

void openFoamPolyMesh::printFaces()
{
    fprintf(fzout, "\n%0.0lf\n(\n",nfaces);

    //first fill out the internal faces
    double lostPointIndices = 0;    //this is because all the stuff is based off of cells, not points, so 1 y row is not accounted for in j every time an x sweep is finished
    for (double j=0;j<ncells;j++)
    {
        if (nNeighbors[j] == 3)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",1+j+lostPointIndices,1+xpoints+j+lostPointIndices,1+xpoints*ypoints+xpoints+j+lostPointIndices,1+xpoints*ypoints+j+lostPointIndices);
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints+j+lostPointIndices,xpoints*ypoints+xpoints+j+lostPointIndices,1+xpoints*ypoints+xpoints+j+lostPointIndices,1+xpoints+j+lostPointIndices);
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*ypoints+j+lostPointIndices,1+xpoints*ypoints+j+lostPointIndices,1+xpoints*ypoints+xpoints+j+lostPointIndices,xpoints*ypoints+xpoints+j+lostPointIndices);
        } else if (nNeighbors[j] == 2)
        {
            if (fmod(j+1,xcells) == 0)
            {
                fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints+j+lostPointIndices,xpoints*ypoints+xpoints+j+lostPointIndices,1+xpoints*ypoints+xpoints+j+lostPointIndices,1+xpoints+j+lostPointIndices);
                fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*ypoints+j+lostPointIndices,1+xpoints*ypoints+j+lostPointIndices,1+xpoints*ypoints+xpoints+j+lostPointIndices,xpoints*ypoints+xpoints+j+lostPointIndices);
                lostPointIndices = lostPointIndices + 1;
            } else if (j >= xcells*ycells*(zcells-1))
            {
                fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",1+j+lostPointIndices,1+xpoints+j+lostPointIndices,1+xpoints*ypoints+xpoints+j+lostPointIndices,1+xpoints*ypoints+j+lostPointIndices);
                fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints+j+lostPointIndices,xpoints*ypoints+xpoints+j+lostPointIndices,1+xpoints*ypoints+xpoints+j+lostPointIndices,1+xpoints+j+lostPointIndices);
            } else
            {
                fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",1+j+lostPointIndices,1+xpoints+j+lostPointIndices,1+xpoints*ypoints+xpoints+j+lostPointIndices,1+xpoints*ypoints+j+lostPointIndices);
                fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*ypoints+j+lostPointIndices,1+xpoints*ypoints+j+lostPointIndices,1+xpoints*ypoints+xpoints+j+lostPointIndices,xpoints*ypoints+xpoints+j+lostPointIndices);
            }
        } else if (nNeighbors[j] == 1)
        {
            if (fmod(j+1,xcells*ycells) == 0)
            {
                fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*ypoints+j+lostPointIndices,1+xpoints*ypoints+j+lostPointIndices,1+xpoints*ypoints+xpoints+j+lostPointIndices,xpoints*ypoints+xpoints+j+lostPointIndices);
                lostPointIndices = lostPointIndices + ypoints;
            } else if (j >= xcells*ycells*(zcells-1) && j < xcells*ycells*(zcells-1)+xcells*(ycells-1))
            {
                fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints+j+lostPointIndices,xpoints*ypoints+xpoints+j+lostPointIndices,1+xpoints*ypoints+xpoints+j+lostPointIndices,1+xpoints+j+lostPointIndices);
                lostPointIndices = lostPointIndices + 1;
            } else
            {
                fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",1+j+lostPointIndices,1+xpoints+j+lostPointIndices,1+xpoints*ypoints+xpoints+j+lostPointIndices,1+xpoints*ypoints+j+lostPointIndices);
            }
        }
    }

    //now fill out the faces for the north outside volume face
    for (double i=0;i<xcells;i++)
    {
        for (double k = 0;k<zcells;k++)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*(ypoints-1)+xpoints*ypoints*k+i,xpoints*ypoints*(k+1)+xpoints*(ypoints-1)+i,1+xpoints*ypoints*(k+1)+xpoints*(ypoints-1)+i,1+xpoints*(ypoints-1)+xpoints*ypoints*k+i);
        }
    }

    //now fill out the faces for the west outside volume face
    for (double k=0;k<zcells;k++)
    {
        for (double j = 0;j<ycells;j++)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*ypoints*k+xpoints*j,xpoints*ypoints*(k+1)+xpoints*j,xpoints*ypoints*(k+1)+xpoints*(j+1),xpoints*ypoints*k+xpoints*(j+1));
        }
    }

    //now fill out the faces for the east outside volume face
    for (double k=0;k<zcells;k++)
    {
        for (double j = 0;j<ycells;j++)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*ypoints*k+xpoints*(j+1)-1,xpoints*ypoints*k+xpoints*(j+2)-1,xpoints*ypoints*(k+1)+xpoints*(j+2)-1,xpoints*ypoints*(k+1)+xpoints*(j+1)-1);
        }
    }

    //now fill out the faces for the south outside volume face
    for (double i=0;i<xcells;i++)
    {
        for (double k = 0;k<zcells;k++)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*ypoints*k+i,xpoints*ypoints*k+i+1,xpoints*ypoints*(k+1)+i+1,xpoints*ypoints*(k+1)+i);
        }
    }

    //now fill out the faces for the bottom outside volume face
    for (double i = 0;i<xcells;i++)
    {
        for (double j=0;j<ycells;j++)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*j+i,xpoints*(j+1)+i,xpoints*(j+1)+i+1,xpoints*j+i+1);
        }
    }

    //now fill out the faces for the top outside volume face
    for (double i = 0;i<xcells;i++)
    {
        for (double j=0;j<ycells;j++)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*ypoints*(zpoints-1)+xpoints*j+i,xpoints*ypoints*(zpoints-1)+xpoints*j+i+1,xpoints*ypoints*(zpoints-1)+xpoints*(j+1)+i+1,xpoints*ypoints*(zpoints-1)+xpoints*(j+1)+i);
        }
    }

    fprintf(fzout, ")\n");
}

void openFoamPolyMesh::printBoundaries()
{
    fprintf(fzout,"\n6\n(\n");    //the number of boundaries we have in windninja

    //now print out the north patch
    fprintf(fzout,"    north_face\n    {\n");
    fprintf(fzout,"\ttype\t\tpatch;\n");
    fprintf(fzout,"\tnFaces\t\t%0.0lf;\n",Ax);
    fprintf(fzout,"\tstartFace\t%0.0lf;\n",ninternalfaces);
    fprintf(fzout,"    }\n");

    //now print out the west patch
    fprintf(fzout,"    west_face\n    {\n");
    fprintf(fzout,"\ttype\t\tpatch;\n");
    fprintf(fzout,"\tnFaces\t\t%0.0lf;\n",Ay);
    fprintf(fzout,"\tstartFace\t%0.0lf;\n",ninternalfaces+Ax);
    fprintf(fzout,"    }\n");

    //now print out the east patch
    fprintf(fzout,"    east_face\n    {\n");
    fprintf(fzout,"\ttype\t\tpatch;\n");
    fprintf(fzout,"\tnFaces\t\t%0.0lf;\n",Ay);
    fprintf(fzout,"\tstartFace\t%0.0lf;\n",ninternalfaces+Ax+Ay);
    fprintf(fzout,"    }\n");

    //now print out the south patch
    fprintf(fzout,"    south_face\n    {\n");
    fprintf(fzout,"\ttype\t\tpatch;\n");
    fprintf(fzout,"\tnFaces\t\t%0.0lf;\n",Ax);
    fprintf(fzout,"\tstartFace\t%0.0lf;\n",ninternalfaces+Ax+2*Ay);
    fprintf(fzout,"    }\n");

    //now print out the top patch
    fprintf(fzout,"    minZ\n    {\n");
    fprintf(fzout,"\ttype\t\twall;\n");
    fprintf(fzout,"\tnFaces\t\t%0.0lf;\n",Az);
    fprintf(fzout,"\tstartFace\t%0.0lf;\n",ninternalfaces+2*Ax+2*Ay);
    fprintf(fzout,"    }\n");

    //now print out the bottom patch
    fprintf(fzout,"    maxZ\n    {\n");
    fprintf(fzout,"\ttype\t\tpatch;\n");
    fprintf(fzout,"\tnFaces\t\t%0.0lf;\n",Az);
    fprintf(fzout,"\tstartFace\t%0.0lf;\n",ninternalfaces+2*Ax+2*Ay+Az);
    fprintf(fzout,"    }\n");

    fprintf(fzout, ")\n");
}
