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

openFoamPolyMesh::openFoamPolyMesh(std::string outputPath, double nxcells, double nycells, double nzcells, double x0, double xf, double y0, double yf, double z0, double zf)
{

    pointsPath = outputPath+"constant/polyMesh/points";
    ownerPath = outputPath+"constant/polyMesh/owner";
    neighbourPath = outputPath+"constant/polyMesh/neighbour";
    facesPath = outputPath+"constant/polyMesh/faces";
    boundaryPath = outputPath+"constant/polyMesh/boundary";
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

    xmin = x0;
    xmax = xf;
    ymin = y0;
    ymax = yf;
    zmin = z0;
    zmax = zf;

    x,y,z;

    //stuff for other files
    transportPropertiesPath = outputPath+"constant/transportProperties";
    diffusivityConstant = 0.01;

    scalarPath = outputPath+"0/T";
    velocityPath = outputPath+"0/U";
    sourcePath = outputPath+"0/source";


    writePolyMeshFiles("points");

}

openFoamPolyMesh::openFoamPolyMesh(std::string outputPath, wn_3dArray& xcoord, wn_3dArray& ycoord, wn_3dArray& zcoord, double nxpoints, double nypoints, double nzpoints,wn_3dScalarField const& uwind, wn_3dScalarField const& vwind, wn_3dScalarField const& wwind)
{
    pointsPath = outputPath+"constant/polyMesh/points";
    ownerPath = outputPath+"constant/polyMesh/owner";
    neighbourPath = outputPath+"constant/polyMesh/neighbour";
    facesPath = outputPath+"constant/polyMesh/faces";
    boundaryPath = outputPath+"constant/polyMesh/boundary";
    xpoints = nxpoints;
    ypoints = nypoints;
    zpoints = nzpoints;
    x = xcoord;
    y = ycoord;
    z = zcoord;

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
    ninternalfaces = nfaces-2*(Ax+Ay+Az);

    //stuff for other files
    transportPropertiesPath = outputPath+"constant/transportProperties";
    diffusivityConstant = 0.01;

    scalarPath = outputPath+"0/T";
    sourcePath = outputPath+"0/source";
    velocityPath = outputPath+"0/U";

    u = uwind;
    v = vwind;
    w = wwind;

    writePolyMeshFiles("array");

}

openFoamPolyMesh::~openFoamPolyMesh()
{

}

bool openFoamPolyMesh::writePolyMeshFiles(std::string pointWriteType)
{
    //this outputs the mesh files, though for now it also outputs all the case files

//start with the mesh files
    //now create points as an OpenFOAM file
    fzout = fopen(pointsPath.c_str(), "w");
    makeFoamHeader("vectorField","points","constant/polyMesh");
    if (pointWriteType == "points")
    {
        printPoints();
    } else if (pointWriteType == "array")
    {
        print3dArrayPoints();
    }
    makeFoamFooter();
    fclose(fzout);

    //now create the owner OpenFOAM mesh file
    fzout = fopen(ownerPath.c_str(),"w");
    makeFoamHeader("labelList","owner","constant/polyMesh");
    printOwners();
    makeFoamFooter();
    fclose(fzout);

    //now create the neighbour OpenFOAM mesh file
    fzout = fopen(neighbourPath.c_str(),"w");
    makeFoamHeader("labelList","neighbour","constant/polyMesh");
    printNeighbors();
    makeFoamFooter();
    fclose(fzout);

    //now create the faces
    fzout = fopen(facesPath.c_str(), "w");
    makeFoamHeader("faceList","faces","constant/polyMesh");
    printFaces();
    makeFoamFooter();
    fclose(fzout);

    //now create the boundary file
    fzout = fopen(boundaryPath.c_str(), "w");
    makeFoamHeader("polyBoundaryMesh","boundary","constant/polyMesh");
    printBoundaries();
    makeFoamFooter();
    fclose(fzout);

//now start with the case files

//finish the constant directory files
    //now create the transportProperties file
    fzout = fopen(transportPropertiesPath.c_str(), "w");
    makeFoamHeader("dictionary","transportProperties","constant");
    fprintf(fzout,"nu              nu [ 0 2 -1 0 0 0 0 ] %lf;\n",diffusivityConstant);
    makeFoamFooter();
    fclose(fzout);

//now create the property value files, the 0 directory files
    //create the scalar field file
    fzout = fopen(scalarPath.c_str(), "w");
    makeFoamHeader("volScalarField","T","0");
    printScalar();
    makeFoamFooter();
    fclose(fzout);

    //create the source field file, note this will get changed by the setFields dict
    fzout = fopen(sourcePath.c_str(), "w");
    makeFoamHeader("volScalarField","source","");
    printSource();
    makeFoamFooter();
    fclose(fzout);

    fzout = fopen(velocityPath.c_str(), "w");
    makeFoamHeader("volVectorField","U","0");
    printVelocity(pointWriteType);
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
    double dx = (xmax-xmin)/xcells;
    double dy = (ymax-ymin)/ycells;
    double dz = (zmax-zmin)/zcells;

    fprintf(fzout, "\n%0.0lf\n(\n", npoints);
    for(double k=0; k<zpoints; k++)
    {
            for(double j=0; j<ypoints; j++)
        {
            for (double i = 0;i<xpoints;i++)
            {
                fprintf(fzout, "(%lf %lf %lf)\n", dx*i, dy*j, dz*k);
            }
        }
    }
    fprintf(fzout, ")\n");
}

void openFoamPolyMesh::print3dArrayPoints()
{
    fprintf(fzout, "\n%0.0lf\n(\n", npoints);
    for(double k=0; k<zpoints; k++)
    {
        for(double j=0; j<xpoints; j++)
        {
            for (double i = 0;i<ypoints;i++)
            {
                fprintf(fzout, "(%lf %lf %lf)\n", x(k*xpoints*ypoints + j*ypoints + i),
                        y(k*xpoints*ypoints + j*ypoints + i), z(k*xpoints*ypoints + j*ypoints + i));
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

void openFoamPolyMesh::printScalar()
{
    fprintf(fzout,"dimensions      [1 -3 0 0 0 0 0];\n\n");
    fprintf(fzout,"internalField   uniform 0;\n\n");
    fprintf(fzout,"boundaryField\n{\n");
    fprintf(fzout,"    north_face\n    {\n");
    fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
    fprintf(fzout,"    west_face\n    {\n");
    fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
    fprintf(fzout,"    east_face\n    {\n");
    fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
    fprintf(fzout,"    south_face\n    {\n");
    fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
    fprintf(fzout,"    minZ\n    {\n");
    fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
    fprintf(fzout,"    maxZ\n    {\n");
    fprintf(fzout,"        type            zeroGradient;\n    }\n}\n");
}

void openFoamPolyMesh::printSource()
{
    fprintf(fzout,"dimensions      [1 -3 -1 0 0 0 0];\n\n");
    fprintf(fzout,"internalField   uniform 0;\n\n");
    fprintf(fzout,"boundaryField\n{\n");
    fprintf(fzout,"    north_face\n    {\n");
    fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
    fprintf(fzout,"    west_face\n    {\n");
    fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
    fprintf(fzout,"    east_face\n    {\n");
    fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
    fprintf(fzout,"    south_face\n    {\n");
    fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
    fprintf(fzout,"    minZ\n    {\n");
    fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
    fprintf(fzout,"    maxZ\n    {\n");
    fprintf(fzout,"        type            zeroGradient;\n    }\n}\n");
}

void openFoamPolyMesh::printVelocity(std::string pointWriteType)
{
    if (pointWriteType == "points")
    {
        fprintf(fzout,"dimensions      [0 1 -1 0 0 0 0];\n\n");
        fprintf(fzout,"internalField   uniform (0,0,0);\n\n");
        fprintf(fzout,"boundaryField\n{\n");
        fprintf(fzout,"    north_face\n    {\n");
        fprintf(fzout,"        type            fixedValue;\n");
        fprintf(fzout,"        value           uniform (1 0 0);\n}\n\n");
        fprintf(fzout,"    west_face\n    {\n");
        fprintf(fzout,"        type            fixedValue;\n");
        fprintf(fzout,"        value           uniform (1 0 0);\n}\n\n");
        fprintf(fzout,"    east_face\n    {\n");
        fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
        fprintf(fzout,"    south_face\n    {\n");
        fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
        fprintf(fzout,"    minZ\n    {\n");
        fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
        fprintf(fzout,"    maxZ\n    {\n");
        fprintf(fzout,"        type            zeroGradient;\n    }\n}\n");

    } else if (pointWriteType == "array")
    {
        fprintf(fzout,"dimensions      [0 1 -1 0 0 0 0];\n\n");

        //first fill out the internal values
        fprintf(fzout,"internalField   nonuniform List<vector>\n%0.0lf\n(\n",ncells);      //needs to be internal points
        for (double k = 0; k < zcells; k++)
        {
            for (double j = 0; j < xcells; j++)
            {
                for (double i = 0; i < ycells; i++)
                {
                    fprintf(fzout, "(%lf %lf %lf)\n", u(k*xpoints*ypoints + j*ypoints + i),
                            v(k*xpoints*ypoints + j*ypoints + i), w(k*xpoints*ypoints + j*ypoints + i));
                }
            }
        }
        fprintf(fzout,")\n;\n\n");


        //this section works to get very similar to the vtk velocities. I think it is just barely off because the velocities are given as points when the OpenFoam needs them as cell centers
        fprintf(fzout,"boundaryField\n{\n");
        fprintf(fzout,"    north_face\n    {\n");
        fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
        fprintf(fzout,"    west_face\n    {\n");
        fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
        fprintf(fzout,"    east_face\n    {\n");
        fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
        fprintf(fzout,"    south_face\n    {\n");
        fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
        fprintf(fzout,"    minZ\n    {\n");
        fprintf(fzout,"        type            zeroGradient;\n    }\n\n");
        fprintf(fzout,"    maxZ\n    {\n");
        fprintf(fzout,"        type            zeroGradient;\n    }\n}\n");



    /*
      //this section defines the velocities on the boundaries, something the vtk format may not even do. These also appear to be off somehow
        //now fill in the north face velocities
        fprintf(fzout,"boundaryField\n{\n");
        fprintf(fzout,"    north_face\n    {\n");
        fprintf(fzout,"        type            pressureInletOutletVelocity;\n");
        fprintf(fzout,"        value           nonuniform List<vector>\n");
        fprintf(fzout,"%0.0lf\n(\n",Ax);
        for (double k = 0; k < zcells; k++)
        {
            for (double j = 0; j < xcells; j++)
            {
                for (double i = 0; i < ycells; i++)
                {
                    if (i == ycells-1)
                    {
                        fprintf(fzout, "(%lf %lf %lf)\n", u(k*xpoints*ypoints + j*ypoints + i),v(k*xpoints*ypoints + j*ypoints + i), w(k*xpoints*ypoints + j*ypoints + i));
                    }
                }
            }
        }
        fprintf(fzout,")\n;\n    }\n");

        //now fill in the west face velocities
        fprintf(fzout,"    west_face\n    {\n");
        fprintf(fzout,"        type            pressureInletOutletVelocity;\n");
        fprintf(fzout,"        value           nonuniform List<vector>\n");
        fprintf(fzout,"%0.0lf\n(\n",Ay);
        for (double k = 0; k < zcells; k++)
        {
            for (double j = 0; j < xcells; j++)
            {
                for (double i = 0; i < ycells; i++)
                {
                    if (j == 0)
                    {
                        fprintf(fzout, "(%lf %lf %lf)\n", u(k*xpoints*ypoints + j*ypoints + i),v(k*xpoints*ypoints + j*ypoints + i), w(k*xpoints*ypoints + j*ypoints + i));
                    }
                }
            }
        }
        fprintf(fzout,")\n;\n    }\n");

        //now fill in the east face velocities
        fprintf(fzout,"    east_face\n    {\n");
        fprintf(fzout,"        type            pressureInletOutletVelocity;\n");
        fprintf(fzout,"        value           nonuniform List<vector>\n");
        fprintf(fzout,"%0.0lf\n(\n",Ay);
        for (double k = 0; k < zcells; k++)
        {
            for (double j = 0; j < xcells; j++)
            {
                for (double i = 0; i < ycells; i++)
                {
                    if (j == xcells-1)
                    {
                        fprintf(fzout, "(%lf %lf %lf)\n", u(k*xpoints*ypoints + j*ypoints + i),v(k*xpoints*ypoints + j*ypoints + i), w(k*xpoints*ypoints + j*ypoints + i));
                    }
                }
            }
        }
        fprintf(fzout,")\n;\n    }\n");

        //now print the south face velocities
        fprintf(fzout,"    south_face\n    {\n");
        fprintf(fzout,"        type            pressureInletOutletVelocity;\n");
        fprintf(fzout,"        value           nonuniform List<vector>\n");
        fprintf(fzout,"%0.0lf\n(\n",Ax);
        for (double k = 0; k < zcells; k++)
        {
            for (double j = 0; j < xcells; j++)
            {
                for (double i = 0; i < ycells; i++)
                {
                    if (i == 0)
                    {
                        fprintf(fzout, "(%lf %lf %lf)\n", u(k*xpoints*ypoints + j*ypoints + i),v(k*xpoints*ypoints + j*ypoints + i), w(k*xpoints*ypoints + j*ypoints + i));
                    }
                }
            }
        }
        fprintf(fzout,")\n;\n    }\n");

        //now print the minZ face velocities
        fprintf(fzout,"    minZ\n    {\n");
        fprintf(fzout,"        type            pressureInletOutletVelocity;\n");
        fprintf(fzout,"        value           nonuniform List<vector>\n");
        fprintf(fzout,"%0.0lf\n(\n",Az);
        for (double k = 0; k < zcells; k++)
        {
            for (double j = 0; j < xcells; j++)
            {
                for (double i = 0; i < ycells; i++)
                {
                    if (k == 0)
                    {
                        fprintf(fzout, "(%lf %lf %lf)\n", u(k*xpoints*ypoints + j*ypoints + i),v(k*xpoints*ypoints + j*ypoints + i), w(k*xpoints*ypoints + j*ypoints + i));
                    }
                }
            }
        }
        fprintf(fzout,")\n;\n    }\n");

        //now print the maxZ face velocities
        fprintf(fzout,"    maxZ\n    {\n");
        fprintf(fzout,"        type            pressureInletOutletVelocity;\n");
        fprintf(fzout,"        value           nonuniform List<vector>\n");
        fprintf(fzout,"%0.0lf\n(\n",Az);
        for (double k = 0; k < zcells; k++)
        {
            for (double j = 0; j < xcells; j++)
            {
                for (double i = 0; i < ycells; i++)
                {
                    if (k == zcells-1)
                    {
                        fprintf(fzout, "(%lf %lf %lf)\n", u(k*xpoints*ypoints + j*ypoints + i),v(k*xpoints*ypoints + j*ypoints + i), w(k*xpoints*ypoints + j*ypoints + i));
                    }
                }
            }
        }
        fprintf(fzout,")\n;\n    }\n}\n");
    */


        //~/src/meshConversion/build-windninja-Desktop-Default/src/cli/WindNinja_cli ~/Downloads/ninjafoam.cfg

        //okay I discovered that the number of faces shown in the boundary file for each boundary equals the number of u values for each of those same boundaries.
        //I also discovered that the number of internal u values is equal to the number of total cells. So does this mean that it is actually a velocity across the cell center since there is one velocity per cell for the internal velocities and one velocity per boundary face for the boundary velocities?
    }
}
