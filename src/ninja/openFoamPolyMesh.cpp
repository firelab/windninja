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

openFoamPolyMesh::openFoamPolyMesh(std::string outputPath, Mesh mesh, double xllCornerValue,
                                   double yllCornerValue, double inputDirectionValue, double UfreeStreamValue,
                                   double inputWindHeight_VegValue, double RdValue,
                                   wn_3dScalarField const& uwind,wn_3dScalarField const& vwind,
                                   wn_3dScalarField const& wwind)
{
    generateCaseDirectory(outputPath);

//values used for the constant polyMesh directory
    xpoints = mesh.ncols;
    ypoints = mesh.nrows;
    zpoints = mesh.nlayers;
    demCornerX = xllCornerValue;
    demCornerY = yllCornerValue;
    x = mesh.XORD;
    y = mesh.YORD;
    z = mesh.ZORD;

    foam_version = "2.0";
    fzout = NULL;
    xcells = xpoints-1;
    ycells = ypoints-1;
    zcells = zpoints-1;
    npoints = xpoints*ypoints*zpoints;
    ncells = xcells*ycells*zcells;

    Axpoints = xpoints*zpoints;
    Aypoints = ypoints*zpoints;
    Azpoints = xpoints*ypoints;
    Axcells = xcells*zcells;
    Aycells = ycells*zcells;
    Azcells = xcells*ycells;

    nfaces = Axcells*ypoints+Aycells*xpoints+Azcells*zpoints;
    ninternalfaces = nfaces-2*(Axcells+Aycells+Azcells);

    diffusivityConstant = 0.05;

//values used for time directory. Stuff above is for constant directory, but sometimes used for other stuff below as well
	inputDirection = inputDirectionValue;
    UfreeStream = UfreeStreamValue;
    uDirection = "";
    ComputeUdirection();
    inputWindHeight_Veg = inputWindHeight_VegValue;
    z0 = 0.01;  //note that this is the foam roughness as found in the NinjaFoam AddBcBlock function. This has been manually set to 0.01 in NinjaFoam, so watch out if this value ends up changing there.
    Rd = RdValue;
    //firstCellHeight = z(5*Azpoints)-z(Azpoints); //it takes a while to pull this out, but according to ninjaFoam,
    //this is essentially (meshvolume/cellcount)^(1/3) where cellcount is half the cells in the blockMesh and
    //half reserved for refineMesh. I'll just use the height of five cells from the minx and miny position
    // at least until I figure out how the native mesh decides to vary the heights for the first few layers of mesh
    double meshVolume = (x(zcells*Azpoints+ycells*xpoints+xcells)-x(0))
            *(y(zcells*Azpoints+ycells*xpoints+xcells)-y(0))
            *(z(zcells*Azpoints+ycells*xpoints+xcells)-z(0));
    firstCellHeight = pow(meshVolume/ncells,(1.0/3.0));
    std::cout << "meshVolume = " << meshVolume << "\n";
    std::cout << "cellcount = " << ncells << "\n";
    std::cout << "firstCellHeight = " << firstCellHeight << "\n";
    u = uwind;
    v = vwind;
    w = wwind;

    element elem(&mesh);

//values used for system directory

    //controlDict variables
    application = "myScalarTransportFoam";
    startFrom = "startTime";
    startTime = "0";    // I feel like this varies a lot depending on the different situations. So I guess this is the default value
    stopAt = "endTime";
    endTime = "3600";    // I feel like this varies a lot depending on the different situations. So I guess this is the default value
    deltaT = "1.0";
    writeControl = "timeStep";
    writeInterval = "300";    // I feel like this varies a lot depending on the different situations. So I guess this is the default value
    purgeWrite = "0";       // I like a value of 0 for smoke transport
    writeFormat = "ascii";
    writePrecision = "10";
    writeCompression = "uncompressed";
    timeFormat = "general";
    timePrecision = "10";
    runTimeModifiable = "true";

    //fvSchemes variables
    ddtSchemes_default = "Euler";
    gradSchemes_default = "Gauss linear";
    divSchemes_default = "none";
    divSchemes_divOfPhiAndT = "bounded Gauss upwind";
    laplacianSchemes_default = "Gauss linear limited 0.333";
    laplacianSchemes_laplacianOfDTandT = "Gauss linear corrected";
    interpolationSchemes_default = "linear";
    SnGradSchemes_default = "corrected";
    SnGradSchemes_SnGradOfT = "limited 0.5";
    fluxRequired_default = "no";
    fluxRequired_T = true;

    //fvSolution variables
    solvers_T_solver = "PBiCG";
    solvers_T_preconditioner = "DILU";
    solvers_T_tolerance = "1e-06";
    solvers_T_relTol = "0.2";
    SIMPLE_nNonOrthogonalCorrectors = "0";

    //setFieldsDict variables
    defaultTvalue = "0";
    defaultSourceValue = "0";
    distributionType = "boxToCell";
    boxMinCoordinates = "726019.742 5206748.293 1210";
    boxMaxCoordinates = "726283 5207018.3 1440";
    distributionTValue = "75";
    distributionSourceValue = "75";

    writePolyMeshFiles(elem);
    std::cout << "Finished meshConversion output\n";

}

openFoamPolyMesh::~openFoamPolyMesh()
{

}

void openFoamPolyMesh::generateCaseDirectory(std::string outputPath)
{
    //force temp dir to DEM location
    CPLSetConfigOption("CPL_TMPDIR", CPLGetDirname(outputPath.c_str()));
    CPLSetConfigOption("CPLTMPDIR", CPLGetDirname(outputPath.c_str()));
    CPLSetConfigOption("TEMP", CPLGetDirname(outputPath.c_str()));
    outputPath = CPLGetBasename(outputPath.c_str());
    outputPath.erase( std::remove_if( outputPath.begin(), outputPath.end(), ::isspace ), outputPath.end() );
    static const char *CaseDir = CPLStrdup(CPLGenerateTempFilename( CPLSPrintf("case-%s", outputPath.c_str())));
    VSIMkdir( CaseDir, 0777 );
    VSIMkdir( CPLSPrintf("%s/0",CaseDir), 0777 );
    VSIMkdir( CPLSPrintf("%s/constant",CaseDir), 0777 );
    VSIMkdir( CPLSPrintf("%s/constant/polyMesh",CaseDir), 0777 );
    VSIMkdir( CPLSPrintf("%s/system",CaseDir), 0777 );

//values used for the constant polyMesh directory
    pointsPath =CPLSPrintf("%s/constant/polyMesh/points",CaseDir);
    ownerPath = CPLSPrintf("%s/constant/polyMesh/owner",CaseDir);
    neighbourPath = CPLSPrintf("%s/constant/polyMesh/neighbour",CaseDir);
    facesPath = CPLSPrintf("%s/constant/polyMesh/faces",CaseDir);
    boundaryPath = CPLSPrintf("%s/constant/polyMesh/boundary",CaseDir);
    transportPropertiesPath = CPLSPrintf("%s/constant/transportProperties",CaseDir);
//values used for time directory. Stuff above is for constant directory, but sometimes used for other stuff below as well
    scalarPath = CPLSPrintf("%s/0/T",CaseDir);
    sourcePath = CPLSPrintf("%s/0/source",CaseDir);
    velocityPath = CPLSPrintf("%s/0/U",CaseDir);
//values used for system directory
    controlDictPath = CPLSPrintf("%s/system/controlDict",CaseDir);
    fvSchemesPath = CPLSPrintf("%s/system/fvSchemes",CaseDir);
    fvSolutionPath = CPLSPrintf("%s/system/fvSolution",CaseDir);
    setFieldsDictPath = CPLSPrintf("%s/system/setFieldsDict",CaseDir);
}

bool openFoamPolyMesh::writePolyMeshFiles(element elem)
{
    //this outputs the mesh files, though for now it also outputs all the case files

//start with the mesh files
    //now create points as an OpenFOAM file
    fzout = fopen(pointsPath.c_str(), "w");
    makeFoamHeader("vectorField","points","constant/polyMesh");
    printPoints();
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
    fprintf(fzout,"DT              DT [ 0 2 -1 0 0 0 0 ] %lf;\n",diffusivityConstant);
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

    //write the velocities
    fzout = fopen(velocityPath.c_str(), "w");
    makeFoamHeader("volVectorField","U","0");
    printVelocity(elem);
    makeFoamFooter();
    fclose(fzout);

//now create the system files
    //write the controlDict file
    fzout = fopen(controlDictPath.c_str(), "w");
    makeFoamHeader("dictionary","controlDict","system");
    writeControlDict();
    makeFoamFooter();
    fclose(fzout);

    //write the fvSchemes file
    fzout = fopen(fvSchemesPath.c_str(),"w");
    makeFoamHeader("dictionary","fvSchemes","system");
    writeFvSchemes();
    makeFoamFooter();
    fclose(fzout);

    //write the fvSolution file
    fzout = fopen(fvSolutionPath.c_str(),"w");
    makeFoamHeader("dictionary","fvSolution","system");
    writeFvSolution();
    makeFoamFooter();
    fclose(fzout);

    //write the setFieldsDict file
    fzout = fopen(setFieldsDictPath.c_str(),"w");
    makeFoamHeader("dictionary","setFieldsDict","system");
    writeSetFieldsDict();
    makeFoamFooter();
    fclose(fzout);

    return true;
}

void openFoamPolyMesh::makeFoamHeader(std::string theClassType, std::string theObjectType,
                                      std::string theFoamFileLocation)
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
        fprintf(fzout,"    note        \"nPoints: %0.0lf nCells: %0.0lf nFaces: %0.0lf nInternalFaces: %0.0lf\";\n",
                npoints,ncells,nfaces,ninternalfaces);
    }
    if (theFoamFileLocation != "" ) //need better error handling code here. Basically plan for whatever cases are normal, and have an else error for anything else
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
    //remember, i is for y or the number of rows. j is for x or the number of columns
    //Print columns before rows in C++ where it is often rows before columns in VBA
    fprintf(fzout, "\n%0.0lf\n(\n", npoints);
    for(double k=0; k<zpoints; k++)
    {
        for(double i=0; i<ypoints; i++)
        {
            for (double j = 0;j<xpoints;j++)
            {
                fprintf(fzout, "(%lf %lf %lf)\n", x(k*Azpoints + i*xpoints + j)+demCornerX,
                        y(k*Azpoints + i*xpoints + j)+demCornerY, z(k*Azpoints + i*xpoints + j));   //demCorner helps to go back to the original utm coordinates
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
        for (double i = 0; i < ycells; i++)
        {
            for (double j = 0; j < xcells; j++)
            {
                if (j != xcells-1)
                {
                    fprintf(fzout, "%0.0lf\n",k*Azcells+i*xcells+j);
                }
                if (i != ycells-1)
                {
                    fprintf(fzout, "%0.0lf\n",k*Azcells+i*xcells+j);
                }
                if (k != zcells-1)
                {
                    fprintf(fzout, "%0.0lf\n",k*Azcells+i*xcells+j);
                }
            }
        }
    }

    //now fill out the owners information for the north outside volume face
    for (double j=0;j<xcells;j++)
    {
        for (double k = 0;k<zcells;k++)
        {
            fprintf(fzout, "%0.0lf\n",k*Azcells+j+xcells*(ycells-1));
        }
    }

    //now fill out the owners information for the west outside volume face
    for (double k = 0;k<zcells;k++)
    {
        for (double i = 0;i<ycells;i++)
        {
            fprintf(fzout, "%0.0lf\n",k*Azcells+i*xcells);
        }
    }

    //now fill out the owners information for the east outside volume face
    for (double k=0;k<zcells;k++)
    {
        for (double i=0;i<ycells;i++)
        {
            fprintf(fzout, "%0.0lf\n",k*Azcells+i*xcells+(xcells-1));
        }
    }

    //now fill out the owners information for the south outside volume face
    for (double j=0;j<xcells;j++)
    {
        for (double k=0;k<zcells;k++)
        {
            fprintf(fzout, "%0.0lf\n",k*Azcells+j);
        }
    }

    //now fill out the owners information for the bottom outside volume face
    for (double j=0;j<xcells;j++)
    {
        for (double i=0;i<ycells;i++)
        {
            fprintf(fzout, "%0.0lf\n",i*xcells+j);
        }
    }

    //now fill out the owners information for the top outside volume face
    for (double j=0;j<xcells;j++)
    {
        for (double i=0;i<ycells;i++)
        {
            fprintf(fzout, "%0.0lf\n",i*xcells+j+Azcells*(zcells-1));
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
        for (double i = 0; i < ycells; i++)
        {
            for (double j = 0; j < xcells; j++)
            {
                if (j != xcells-1)
                {
                    fprintf(fzout,"%0.0lf\n",k*Azcells+i*xcells+j+1);
                }
                if (i != ycells-1)
                {
                    fprintf(fzout,"%0.0lf\n",k*Azcells+(i+1)*xcells+j);
                }
                if (k != zcells-1)
                {
                    fprintf(fzout,"%0.0lf\n",(k+1)*Azcells+i*xcells+j);
                }
            }
        }
    }

    fprintf(fzout, ")\n");
}

void openFoamPolyMesh::printFaces()
{

    fprintf(fzout, "\n%0.0lf\n(\n",nfaces);

    for (double k = 0; k < zcells; k++)
    {
        for (double i = 0; i < ycells; i++)
        {
            for (double j = 0; j < xcells; j++)
            {
                if (j != xcells-1)
                {
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",
                            k*Azpoints+i*xpoints+j+1,
                            k*Azpoints+(i+1)*xpoints+j+1,
                            (k+1)*Azpoints+(i+1)*xpoints+j+1,
                            (k+1)*Azpoints+i*xpoints+j+1);
                }
                if (i != ycells-1)
                {
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",
                            k*Azpoints+(i+1)*xpoints+j,
                            (k+1)*Azpoints+(i+1)*xpoints+j,
                            (k+1)*Azpoints+(i+1)*xpoints+j+1,
                            k*Azpoints+(i+1)*xpoints+j+1);
                }
                if (k != zcells-1)
                {
                    fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",
                            (k+1)*Azpoints+i*xpoints+j,
                            (k+1)*Azpoints+i*xpoints+j+1,
                            (k+1)*Azpoints+(i+1)*xpoints+j+1,
                            (k+1)*Azpoints+(i+1)*xpoints+j);
                }
            }
        }
    }

    //now fill out the faces for the north outside volume face
    double layerMinusRow = xpoints*(ypoints-1);    //x is columns, y is rows, so it is all of one layer but one row

    for (double j=0;j<xcells;j++)
    {
        for (double k = 0;k<zcells;k++)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",
                    k*Azpoints+j+layerMinusRow,
                    (k+1)*Azpoints+j+layerMinusRow,
                    (k+1)*Azpoints+j+layerMinusRow+1,
                    k*Azpoints+j+layerMinusRow+1);
        }
    }

    //now fill out the faces for the west outside volume face
    for (double k=0;k<zcells;k++)
    {
        for (double i = 0;i<ycells;i++)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",
                    k*Azpoints+i*xpoints,
                    (k+1)*Azpoints+i*xpoints,
                    (k+1)*Azpoints+(i+1)*xpoints,
                    k*Azpoints+(i+1)*xpoints);
        }
    }

    //now fill out the faces for the east outside volume face
    for (double k=0;k<zcells;k++)
    {
        for (double i = 0;i<ycells;i++)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",
                    k*Azpoints+(i+1)*xpoints-1,
                    k*Azpoints+(i+2)*xpoints-1,
                    (k+1)*Azpoints+(i+2)*xpoints-1,
                    (k+1)*Azpoints+(i+1)*xpoints-1);
        }
    }

    //now fill out the faces for the south outside volume face
    for (double j=0;j<xcells;j++)
    {
        for (double k = 0;k<zcells;k++)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",
                    k*Azpoints+j,
                    k*Azpoints+j+1,
                    (k+1)*Azpoints+j+1,
                    (k+1)*Azpoints+j);
        }
    }

    //now fill out the faces for the bottom outside volume face
    for (double j = 0;j<xcells;j++)
    {
        for (double i=0;i<ycells;i++)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",
                    i*xpoints+j,
                    (i+1)*xpoints+j,
                    (i+1)*xpoints+j+1,
                    i*xpoints+j+1);
        }
    }

    //now fill out the faces for the top outside volume face
    double allButOneLayer = Azpoints*(zpoints-1);   //represents everything but the last layer, or complete mesh - 1 layer
    for (double j = 0;j<xcells;j++)
    {
        for (double i=0;i<ycells;i++)
        {
            fprintf(fzout,"4(%0.0lf %0.0lf %0.0lf %0.0lf)\n",
                    i*xpoints+j+allButOneLayer,
                    i*xpoints+j+allButOneLayer+1,
                    (i+1)*xpoints+j+allButOneLayer+1,
                    (i+1)*xpoints+j+allButOneLayer);
        }
    }

    fprintf(fzout, ")\n");
}

void openFoamPolyMesh::printBoundaryPatch(std::string patchName,std::string patchType,double nFaces,double startFace,std::string physicalType)
{
    fprintf(fzout,"    %s\n    {\n",patchName.c_str());
    fprintf(fzout,"\ttype\t\t%s;\n",patchType.c_str());
    if(physicalType != "")  //need an error handle for this
    {
        fprintf(fzout,"\tphysicalType\t%s;\n",physicalType.c_str());
    }
    fprintf(fzout,"\tnFaces\t\t%0.0lf;\n",nFaces);
    fprintf(fzout,"\tstartFace\t%0.0lf;\n",startFace);
    fprintf(fzout,"    }\n");
}

void openFoamPolyMesh::printBoundaries()
{
    fprintf(fzout,"\n6\n(\n");    //the number of boundaries we have in windninja. Should someday make this a variable or something so it can vary? I guess for now we will always have 6

    //now print out the north patch
    printBoundaryPatch("north_face","patch",Axcells,ninternalfaces,"");

    //now print out the west patch
    printBoundaryPatch("west_face","patch",Aycells,ninternalfaces+Axcells,"");

    //now print out the east patch
    printBoundaryPatch("east_face","patch",Aycells,ninternalfaces+Axcells+Aycells,"");

    //now print out the south patch
    printBoundaryPatch("south_face","patch",Axcells,ninternalfaces+Axcells+2*Aycells,"");

    //now print out the bottom patch
    //notice that this one is a different type than the others
    printBoundaryPatch("minZ","wall",Azcells,ninternalfaces+2*Axcells+2*Aycells,"");

    //now print out the top patch
    printBoundaryPatch("maxZ","patch",Azcells,ninternalfaces+2*Axcells+2*Aycells+Azcells,"");

    fprintf(fzout, ")\n");
}

//maybe change this to printListValues and add in the way to do it with other stuff that is non-uniform.
//Man I just realized how the function pointers could be handy here. Still might not be the right choice
//since the loops change (though they be the same for each type of patch), but could do something like function(functionPointer patchType (for knowing which order to do the loops), functionPointer something to know which print statement to use). So I guess its a function with two function pointers?
//Anyhow I still think it varies just enough to not be worth it. Plus its not like there's going to be another way to output the points.
//Maybe the datatype that you are outputing the points for, but not the indices since the format is fixed.
void openFoamPolyMesh::printListHeader(std::string patchName,std::string ListType,
                                              std::string ListValue,bool extraReturn)
{
    fprintf(fzout,"    %s\n    {\n",patchName.c_str());
    fprintf(fzout,"        type            %s;\n",ListType.c_str());
    if(ListType == "fixedValue")
    {
        fprintf(fzout,"        value           uniform %s;\n}\n",ListValue.c_str());    //this allows arrays as well as points
    }else if(ListType == "pressureInletOutletVelocity")
    {
        fprintf(fzout,"        value           nonuniform List<vector>\n");
    }else if(ListType == "logProfileVelocityInlet")
    {
        fprintf(fzout,"        UfreeStream     %lf;\n",UfreeStream);
        fprintf(fzout,"        uDirection      %s;\n",uDirection.c_str());
        fprintf(fzout,"        inputWindHeight_Veg %lf;\n",inputWindHeight_Veg);
        fprintf(fzout,"        z0              %lf;\n",z0);
        fprintf(fzout,"        Rd              %lf;\n",Rd);
        fprintf(fzout,"        firstCellHeight %lf;\n",firstCellHeight);
        fprintf(fzout,"        value           nonuniform List<vector>\n");
    }else
    {
        fprintf(fzout,"    }\n");
    }
    if(extraReturn == true)
    {
        fprintf(fzout,"\n");
    }
}

void openFoamPolyMesh::printScalar()
{
    fprintf(fzout,"dimensions      [1 -3 0 0 0 0 0];\n\n");
    fprintf(fzout,"internalField   uniform 0;\n\n");
    fprintf(fzout,"boundaryField\n{\n");

    printListHeader("north_face","zeroGradient","",true);
    printListHeader("west_face","zeroGradient","",true);
    printListHeader("east_face","zeroGradient","",true);
    printListHeader("south_face","zeroGradient","",true);
    printListHeader("minZ","zeroGradient","",true);
    printListHeader("maxZ","zeroGradient","",false);   //notice this doesn't have the extra space, because it has a slightly different end (another bracket)

    fprintf(fzout,"}\n");

}

void openFoamPolyMesh::printSource()
{
    fprintf(fzout,"dimensions      [1 -3 -1 0 0 0 0];\n\n");
    fprintf(fzout,"internalField   uniform 0;\n\n");
    fprintf(fzout,"boundaryField\n{\n");

    printListHeader("north_face","zeroGradient","",true);
    printListHeader("west_face","zeroGradient","",true);
    printListHeader("east_face","zeroGradient","",true);
    printListHeader("south_face","zeroGradient","",true);
    printListHeader("minZ","zeroGradient","",true);
    printListHeader("maxZ","zeroGradient","",false);   //notice this doesn't have the extra space, because it has a slightly different end (another bracket)

    fprintf(fzout,"}\n");

}

void openFoamPolyMesh::ComputeUdirection()
{
    double d, d1, d2, dx, dy; //CW, d1 is first angle, d2 is second angle

    d = inputDirection - 180; //convert wind direction from --> wind direction to
    if(d < 0){
        d += 360;
    }

    if(d > 0 && d < 90){ //quadrant 1
        d1 = d;
        d2 = 90 - d;
        dx = sin(d1 * PI/180);
        dy = sin(d2 * PI/180);
    }
    else if(d > 90 && d < 180){ //quadrant 2
        d -= 90;
        d1 = d;
        d2 = 90 - d;
        dx = sin(d2 * PI/180);
        dy = -sin(d1 * PI/180);
    }
    else if(d > 180 && d < 270){ //quadrant 3
        d -= 180;
        d1 = d;
        d2 = 90 - d;
        dx = -sin(d1 * PI/180);
        dy = -sin(d2 * PI/180);
    }
    else if(d > 270 && d < 360){ //quadrant 4
        d -= 270;
        d1 = d;
        d2 = 90 - d;
        dx = -sin(d2 * PI/180);
        dy = sin(d1 * PI/180);
    }
    else if(d == 0 || d == 360){
        dx = 0;
        dy = 1;
    }
    else if(d == 90){
        dx = 1;
        dy = 0;
    }
    else if(d == 180){
        dx = 0;
        dy = -1;
    }
    else if(d == 270){
        dx = -1;
        dy = 0;
    }

    uDirection = CPLSPrintf("(%.4lf %.4lf 0)",dx,dy);
}

void openFoamPolyMesh::printVelocity(element elem)
{
    fprintf(fzout,"dimensions      [0 1 -1 0 0 0 0];\n\n");

    //first fill out the internal values
    fprintf(fzout,"internalField   nonuniform List<vector>\n%0.0lf\n(\n",ncells);
    for (double k = 0; k < zcells; k++)
    {
        for (double i = 0; i < ycells; i++)
        {
            for (double j = 0; j < xcells; j++)
            {
                fprintf(fzout, "(%lf %lf %lf)\n", u.interpolate(elem,i,j,k,0,0,0),v.interpolate(elem,i,j,k,0,0,0),w.interpolate(elem,i,j,k,0,0,0));
            }
        }
    }
    fprintf(fzout,")\n;\n\n");

 //this section defines the velocities on the boundaries, something the vtk format may not even do. These are slightly off from the vtk (though in the same position now) because they are cell centers, not point interpolated values
    fprintf(fzout,"boundaryField\n{\n");

    //now fill in the north face velocities
    if ((inputDirection >= 0 && inputDirection < 90) || (inputDirection <= 360 && inputDirection > 270))
    {
        printListHeader("north_face","logProfileVelocityInlet","",false);
    } else
    {
        printListHeader("north_face","pressureInletOutletVelocity","",false);
    }
    fprintf(fzout,"%0.0lf\n(\n",Axcells);
    for (double j = 0; j < xcells; j++)
    {
        for (double k = 0; k < zcells; k++)
        {
            fprintf(fzout, "(%lf %lf %lf)\n", u.interpolate(elem,ycells-1,j,k,0,1,0),v.interpolate(elem,ycells-1,j,k,0,1,0),w.interpolate(elem,ycells-1,j,k,0,1,0));
        }
    }
    fprintf(fzout,")\n;\n    }\n");

    //now fill in the west face velocities
    if (inputDirection > 180 && inputDirection < 360)
    {
        printListHeader("west_face","logProfileVelocityInlet","",false);
    } else
    {
        printListHeader("west_face","pressureInletOutletVelocity","",false);
    }
    fprintf(fzout,"%0.0lf\n(\n",Aycells);
    for (double k = 0; k < zcells; k++)
    {
        for (double i = 0; i < ycells; i++)
        {
            fprintf(fzout, "(%lf %lf %lf)\n", u.interpolate(elem,i,0,k,-1,0,0),v.interpolate(elem,i,0,k,-1,0,0),w.interpolate(elem,i,0,k,-1,0,0));
        }
    }
    fprintf(fzout,")\n;\n    }\n");

    //now fill in the east face velocities
    if (inputDirection > 0 && inputDirection < 180)
    {
        printListHeader("east_face","logProfileVelocityInlet","",false);
    } else
    {
        printListHeader("east_face","pressureInletOutletVelocity","",false);
    }
    fprintf(fzout,"%0.0lf\n(\n",Aycells);
    for (double k = 0; k < zcells; k++)
    {
        for (double i = 0; i < ycells; i++)
        {
            fprintf(fzout, "(%lf %lf %lf)\n", u.interpolate(elem,i,xcells-1,k,1,0,0),v.interpolate(elem,i,xcells-1,k,1,0,0),w.interpolate(elem,i,xcells-1,k,1,0,0));
        }
    }
    fprintf(fzout,")\n;\n    }\n");

    //now print the south face velocities
    if (inputDirection > 90 && inputDirection < 270)
    {
        printListHeader("south_face","logProfileVelocityInlet","",false);
    } else
    {
        printListHeader("south_face","pressureInletOutletVelocity","",false);
    }
    fprintf(fzout,"%0.0lf\n(\n",Axcells);
    for (double j = 0; j < xcells; j++)
    {
        for (double k = 0; k < zcells; k++)
        {
            fprintf(fzout, "(%lf %lf %lf)\n", u.interpolate(elem,0,j,k,0,-1,0),v.interpolate(elem,0,j,k,0,-1,0),w.interpolate(elem,0,j,k,0,-1,0));
        }
    }
    fprintf(fzout,")\n;\n    }\n");

    //now print the minZ face velocities
    printListHeader("minZ","fixedValue","(0 0 0)",true);

    //now print the maxZ face velocities
    printListHeader("maxZ","zeroGradient","",false);

    fprintf(fzout,"}\n");

    /*
    //now print the minZ face velocities
    printListHeader("minZ","pressureInletOutletVelocity","",false);
    fprintf(fzout,"%0.0lf\n(\n",Azcells);
    for (double j = 0; j < xcells; j++)
    {
        for (double i = 0; i < ycells; i++)
        {
            fprintf(fzout, "(%lf %lf %lf)\n", u.interpolate(elem,i,j,0,0,0,-1),v.interpolate(elem,i,j,0,0,0,-1),w.interpolate(elem,i,j,0,0,0,-1));
        }
    }
    fprintf(fzout,")\n;\n    }\n");

    //now print the maxZ face velocities
    printListHeader("maxZ","pressureInletOutletVelocity","",false);
    fprintf(fzout,"%0.0lf\n(\n",Azcells);
    for (double j = 0; j < xcells; j++)
    {
        for (double i = 0; i < ycells; i++)
        {
            fprintf(fzout, "(%lf %lf %lf)\n", u.interpolate(elem,i,j,zcells-1,0,0,1),v.interpolate(elem,i,j,zcells-1,0,0,1),w.interpolate(elem,i,j,zcells-1,0,0,1));
        }
    }
    fprintf(fzout,")\n;\n    }\n}\n");
    */
}

void openFoamPolyMesh::writeControlDict()
{
    fprintf(fzout,"application     %s;\n\n",application.c_str());
    fprintf(fzout,"startFrom       %s;\n\n",startFrom.c_str());
    fprintf(fzout,"startTime       %s;\n\n",startTime.c_str());
    fprintf(fzout,"stopAt          %s;\n\n",stopAt.c_str());
    fprintf(fzout,"endTime         %s;\n\n",endTime.c_str());
    fprintf(fzout,"deltaT          %s;\n\n",deltaT.c_str());
    fprintf(fzout,"writeControl    %s;\n\n",writeControl.c_str());
    fprintf(fzout,"writeInterval   %s;\n\n",writeInterval.c_str());
    fprintf(fzout,"purgeWrite      %s;\n\n",purgeWrite.c_str());
    fprintf(fzout,"writeFormat     %s;\n\n",writeFormat.c_str());
    fprintf(fzout,"writePrecision  %s;\n\n",writePrecision.c_str());
    fprintf(fzout,"writeCompression %s;\n\n",writeCompression.c_str());
    fprintf(fzout,"timeFormat      %s;\n\n",timeFormat.c_str());
    fprintf(fzout,"timePrecision   %s;\n\n",timePrecision.c_str());
    fprintf(fzout,"runTimeModifiable %s;\n",runTimeModifiable.c_str()); //one less new line since last line
}

void openFoamPolyMesh::writeFvSchemes()
{
    //could possibly write something that is called writeScheme that takes in
    //the scheme name and an array with what parts of the scheme will be written

    //write the ddtSchemes
    fprintf(fzout,"ddtSchemes\n{\n");
    fprintf(fzout,"    default         %s;\n",ddtSchemes_default.c_str());
    fprintf(fzout,"}\n\n");

    //write the gradSchemes
    fprintf(fzout,"gradSchemes\n{\n");
    fprintf(fzout,"    default         %s;\n",gradSchemes_default.c_str());
    fprintf(fzout,"}\n\n");

    //write the divSchemes
    fprintf(fzout,"divSchemes\n{\n");
    fprintf(fzout,"    default         %s;\n",divSchemes_default.c_str());
    fprintf(fzout,"    div(phi,T)      %s;\n",divSchemes_divOfPhiAndT.c_str());
    fprintf(fzout,"}\n\n");

    //write the laplacianSchemes
    fprintf(fzout,"laplacianSchemes\n{\n");
    fprintf(fzout,"    default         %s;\n",laplacianSchemes_default.c_str());
    fprintf(fzout,"    laplacian(DT,T) %s;\n",laplacianSchemes_laplacianOfDTandT.c_str());
    fprintf(fzout,"}\n\n");

    //write the interpolationSchemes
    fprintf(fzout,"interpolationSchemes\n{\n");
    fprintf(fzout,"    default         %s;\n",interpolationSchemes_default.c_str());
    fprintf(fzout,"}\n\n");

    //write the SnGradSchemes
    fprintf(fzout,"SnGradSchemes\n{\n");
    fprintf(fzout,"    default         %s;\n",SnGradSchemes_default.c_str());
    fprintf(fzout,"    SnGrad(T)       %s;\n",SnGradSchemes_SnGradOfT.c_str());
    fprintf(fzout,"}\n\n");

    //write the fluxRequired part
    fprintf(fzout,"fluxRequired\n{\n");
    fprintf(fzout,"    default         %s;\n",fluxRequired_default.c_str());
    if(fluxRequired_T == true)
    {
        fprintf(fzout,"    T;\n");
    }
    fprintf(fzout,"}\n");
}

void openFoamPolyMesh::writeFvSolution()
{
    fprintf(fzout,"solvers\n{\n");

    fprintf(fzout,"    T\n    {\n");
    fprintf(fzout,"        solver          %s;\n",solvers_T_solver.c_str());
    fprintf(fzout,"        preconditioner  %s;\n",solvers_T_preconditioner.c_str());
    fprintf(fzout,"        tolerance       %s;\n",solvers_T_tolerance.c_str());
    fprintf(fzout,"        relTol          %s;\n",solvers_T_relTol.c_str());
    fprintf(fzout,"    }\n");

    fprintf(fzout,"}\n\n");

    fprintf(fzout,"SIMPLE\n{\n");

    fprintf(fzout,"    nNonOrthogonalCorrectors %s;\n",SIMPLE_nNonOrthogonalCorrectors.c_str());

    fprintf(fzout,"}\n");
}

void openFoamPolyMesh::writeSetFieldsDict()
{
    fprintf(fzout,"defaultFieldValues\n(\n");

    fprintf(fzout,"    volScalarFieldValue T %s\n",defaultTvalue.c_str());
    fprintf(fzout,"    volScalarFieldValue source %s\n",defaultSourceValue.c_str());

    fprintf(fzout,");\n\n");

    fprintf(fzout,"regions\n(\n");
    if(distributionType == "boxToCell")
    {
        fprintf(fzout,"    boxToCell\n    {\n");
        fprintf(fzout,"        box (%s) (%s);\n",boxMinCoordinates.c_str(),boxMaxCoordinates.c_str());
        fprintf(fzout,"        fieldValues\n        (\n");
        fprintf(fzout,"            volScalarFieldValue T %s\n",distributionTValue.c_str());
        fprintf(fzout,"            volScalarFieldValue source %s\n",distributionSourceValue.c_str());
        fprintf(fzout,"        );\n    }\n");
    }
    fprintf(fzout,");\n");
}
