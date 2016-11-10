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
    ninternalfaces = nfaces-2*(Ax+Ay+Az);

    writePolyMeshFiles();
}

openFoamPolyMesh::openFoamPolyMesh(std::string outputPath, double nxcells, double nycells, double nzcells)
{

    polyMesh_path = outputPath;
    xcells = nxcells;
    ycells = nycells;
    zcells = nzcells;

    foam_version = "2.0";
    fzout = NULL;
    xpoints = xcells+1;
    ypoints = ycells+1;
    zpoints = zcells+1;
    npoints = xpoints*ypoints*zpoints;
    ncells = xcells*ycells*zcells;
    Ax = xcells*zcells;
    Ay = ycells*zcells;
    Az = xcells*ycells;
    nfaces = Ax*ypoints+Ay*xpoints+Az*zpoints;
    ninternalfaces = nfaces-2*(Ax+Ay+Az);

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
    current_path = polyMesh_path+"neighbour";   //I almost missed this. I spelled it the correct way and not their way at first
    fzout = fopen(current_path.c_str(),"w");
    makeFoamHeader("labelList","neighbour","constant/polyMesh");
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
    if (theObjectType == "neighbour" || theObjectType == "owner")
    {
        fprintf(fzout,"    note        \"nPoints: %0.0lf nCells: %0.0lf nFaces: %0.0lf nInternalFaces: %0.0lf\";\n",npoints,ncells,nfaces,ninternalfaces);
    }
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

    fprintf(fzout, "\n%0.0lf\n(\n",nfaces);

    //fill out the owners information for interior cells
    for (double k = 0; k < zcells; k++)
    {
        for (double j = 0; j < ycells; j++)
        {
            for (double i = 0; i < xcells; i++)
            {
                // might need to change the order of the parts in each statement to make them be checked quicker (ie if ycells has to be equal but xcells has to not be equal, switch the order so it skips the statement sooner?)
                if (i != xcells-1 && j != ycells-1 && k != zcells-1)
                {
                    //print 3 owners
                    fprintf(fzout, "%0.0lf\n",k*ycells*xcells+j*xcells+i);
                    fprintf(fzout, "%0.0lf\n",k*ycells*xcells+j*xcells+i);
                    fprintf(fzout, "%0.0lf\n",k*ycells*xcells+j*xcells+i);
                } else if (i == xcells-1 && j != ycells-1 && k != zcells-1)
                {
                    //print 2 owners
                    fprintf(fzout, "%0.0lf\n",k*ycells*xcells+j*xcells+i);
                    fprintf(fzout, "%0.0lf\n",k*ycells*xcells+j*xcells+i);
                } else if (i != xcells-1 && j == ycells-1 && k != zcells-1)
                {
                    //print 2 owners
                    fprintf(fzout, "%0.0lf\n",k*ycells*xcells+j*xcells+i);
                    fprintf(fzout, "%0.0lf\n",k*ycells*xcells+j*xcells+i);
                } else if (i == xcells-1 && j == ycells-1 && k != zcells-1)
                {
                    //print 1 owner
                    fprintf(fzout, "%0.0lf\n",k*ycells*xcells+j*xcells+i);
                } else if (i != xcells-1 && j != ycells-1 && k == zcells-1)
                {
                    //print 2 owners
                    fprintf(fzout, "%0.0lf\n",k*ycells*xcells+j*xcells+i);
                    fprintf(fzout, "%0.0lf\n",k*ycells*xcells+j*xcells+i);
                } else if (i == xcells-1 && j != ycells-1 && k == zcells-1)
                {
                    //print 1 owner
                    fprintf(fzout, "%0.0lf\n",k*ycells*xcells+j*xcells+i);
                } else if (i != xcells-1 && j == ycells-1 && k == zcells-1)
                {
                    //print 1 owner
                    fprintf(fzout, "%0.0lf\n",k*ycells*xcells+j*xcells+i);
                }
            }
        }
    }

    //now fill out the owners information for the north outside volume face
    for (double i=0;i<xcells;i++)
    {
        for (double k = 0;k<zcells;k++)
        {
            fprintf(fzout, "%0.0lf\n",xcells*(ycells-1)+xcells*ycells*k+i);
        }
    }

    //now fill out the owners information for the west outside volume face
    for (double k = 0;k<zcells;k++)
    {
        for (double j = 0;j<ycells;j++)
        {
            fprintf(fzout, "%0.0lf\n",xcells*j+xcells*ycells*k);
        }
    }

    //now fill out the owners information for the east outside volume face
    for (double k=0;k<zcells;k++)
    {
        for (double j=0;j<ycells;j++)
        {
            fprintf(fzout, "%0.0lf\n",(xcells-1)+xcells*j+xcells*ycells*k);
        }
    }

    //now fill out the owners information for the south outside volume face
    for (double i=0;i<xcells;i++)
    {
        for (double k=0;k<zcells;k++)
        {
            fprintf(fzout, "%0.0lf\n",xcells*ycells*k+i);
        }
    }

    //now fill out the owners information for the bottom outside volume face
    for (double i=0;i<xcells;i++)
    {
        for (double j=0;j<ycells;j++)
        {
            fprintf(fzout, "%0.0lf\n",xcells*j+i);
        }
    }

    //now fill out the owners information for the top outside volume face
    for (double i=0;i<xcells;i++)
    {
        for (double j=0;j<ycells;j++)
        {
            fprintf(fzout, "%0.0lf\n",xcells*ycells*(zcells-1)+xcells*j+i);
        }
    }

    fprintf(fzout, ")\n");

}

void openFoamPolyMesh::printNeighbors()
{
    fprintf(fzout, "\n%0.0lf\n(\n",ninternalfaces);

    //fill out the neighbors information
    for (double k = 0; k < zcells; k++)
    {
        for (double j = 0; j < ycells; j++)
        {
            for (double i = 0; i < xcells; i++)
            {
                // might need to change the order of the parts in each statement to make them be checked quicker (ie if ycells has to be equal but xcells has to not be equal, switch the order so it skips the statement sooner?)
                if (i != xcells-1 && j != ycells-1 && k != zcells-1)
                {
                    //print 3 neighbors
                    fprintf(fzout,"%0.0lf\n",1+k*ycells*xcells+j*xcells+i);
                    fprintf(fzout,"%0.0lf\n",xcells+k*ycells*xcells+j*xcells+i);
                    fprintf(fzout,"%0.0lf\n",xcells*ycells+k*ycells*xcells+j*xcells+i);
                } else if (i == xcells-1 && j != ycells-1 && k != zcells-1)
                {
                    //print 2 neighbors
                    fprintf(fzout,"%0.0lf\n",xcells+k*ycells*xcells+j*xcells+i);
                    fprintf(fzout,"%0.0lf\n",xcells*ycells+k*ycells*xcells+j*xcells+i);
                } else if (i != xcells-1 && j == ycells-1 && k != zcells-1)
                {
                    //print 2 neighbors
                                        fprintf(fzout,"%0.0lf\n",1+k*ycells*xcells+j*xcells+i);
                                        fprintf(fzout,"%0.0lf\n",xcells*ycells+k*ycells*xcells+j*xcells+i);
                } else if (i == xcells-1 && j == ycells-1 && k != zcells-1)
                {
                    //print 1 neighbor
                    fprintf(fzout,"%0.0lf\n",xcells*ycells+k*ycells*xcells+j*xcells+i);
                } else if (i != xcells-1 && j != ycells-1 && k == zcells-1)
                {
                    //print 2 neighbors
                    fprintf(fzout,"%0.0lf\n",1+k*ycells*xcells+j*xcells+i);
                    fprintf(fzout,"%0.0lf\n",xcells+k*ycells*xcells+j*xcells+i);
                } else if (i == xcells-1 && j != ycells-1 && k == zcells-1)
                {
                    //print 1 neighbor
                    fprintf(fzout,"%0.0lf\n",xcells+k*ycells*xcells+j*xcells+i);
                } else if (i != xcells-1 && j == ycells-1 && k == zcells-1)
                {
                    //print 1 neighbor
                    fprintf(fzout,"%0.0lf\n",1+k*ycells*xcells+j*xcells+i);
                }
            }
        }
    }

    fprintf(fzout, ")\n");
}

void openFoamPolyMesh::printFaces()
{

    fprintf(fzout, "\n%0.0lf\n(\n",nfaces);

    //first fill out the internal faces
    for (double k = 0; k < zcells; k++)
    {
        for (double j = 0; j < ycells; j++)
        {
            for (double i = 0; i < xcells; i++)
            {
                // might need to change the order of the parts in each statement to make them be checked quicker (ie if ycells has to be equal but xcells has to not be equal, switch the order so it skips the statement sooner?)
                if (i != xcells-1 && j != ycells-1 && k != zcells-1)
                {
                    //print 3 faces
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",1+k*ypoints*xpoints+j*xpoints+i,1+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+k*ypoints*xpoints+j*xpoints+i);
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints+k*ypoints*xpoints+j*xpoints+i,xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints+k*ypoints*xpoints+j*xpoints+i);
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*ypoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i);
                } else if (i == xcells-1 && j != ycells-1 && k != zcells-1)
                {
                    //print 2 faces
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints+k*ypoints*xpoints+j*xpoints+i,xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints+k*ypoints*xpoints+j*xpoints+i);
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*ypoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i);
                } else if (i != xcells-1 && j == ycells-1 && k != zcells-1)
                {
                    //print 2 faces
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",1+k*ypoints*xpoints+j*xpoints+i,1+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+k*ypoints*xpoints+j*xpoints+i);
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*ypoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i);
                } else if (i == xcells-1 && j == ycells-1 && k != zcells-1)
                {
                    //print 1 face
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints*ypoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i);
                } else if (i != xcells-1 && j != ycells-1 && k == zcells-1)
                {
                    //print 2 faces
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",1+k*ypoints*xpoints+j*xpoints+i,1+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+k*ypoints*xpoints+j*xpoints+i);
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints+k*ypoints*xpoints+j*xpoints+i,xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints+k*ypoints*xpoints+j*xpoints+i);
                } else if (i == xcells-1 && j != ycells-1 && k == zcells-1)
                {
                    //print 1 face
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",xpoints+k*ypoints*xpoints+j*xpoints+i,xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints+k*ypoints*xpoints+j*xpoints+i);
                } else if (i != xcells-1 && j == ycells-1 && k == zcells-1)
                {
                    //print 1 face
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",1+k*ypoints*xpoints+j*xpoints+i,1+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+xpoints+k*ypoints*xpoints+j*xpoints+i,1+xpoints*ypoints+k*ypoints*xpoints+j*xpoints+i);
                }
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
