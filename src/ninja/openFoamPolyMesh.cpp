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
                                   wn_3dScalarField const& wwind,std::string BCtypeValue)
{
    std::cout << "starting meshConversion openFoam output case\n";
    generateCaseDirectory(outputPath);

    // determine what types and styles of files are written
    simulationType = "myScalarTransportFoam";   // myScalarTransportFoam, simpleFoam, buoyantBoussinesqPimpleFoam

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

//values used for the constant non-polyMesh directory
    // this works the same regardless of the simulation type
    // but do we want to add in some of the other files?
    transportModel = "Newtonian";   // changing this might mean a need to change structure for other values
    thermalDiffusivityConstant = "0.05"; // 0.05 m^2/s was our guess based off of diffusivity comparison videos
    dynamicViscosity = "1.846e-05";            // 1.846*10^05 kg/(m*s) for air at 300 K
    kinematicViscosity = "1.58946099535e-05";  // 1.589*10^-5 m^2/s for air at 300 K
    density = "1.1614";                     // 1.1614 kg/m^3 for air at 300 K
    thermalExpansionCoefficient = "3e-03";    // around 0.0034 1/K for air
    Tref = "300";                        // 300 K is reference temperature found in hot room examples
    laminarPrandtlNumber = "0.707";        // Early hot room had this as 0.9 dimensionless, but later changed to 0.707 dimensionless which is the value for air at 300 K
    turbulentPrandtlNumber = "0.85";       // Early hot room had this as 0.7 dimensionless, but later changed to 0.85 dimensionless

//values used for time directory. Stuff above is for constant directory, but sometimes used for other stuff below as well
    BCtype = BCtypeValue;
	inputDirection = inputDirectionValue;
    UfreeStream = UfreeStreamValue;
    uDirection = "";
    ComputeUdirection();
    inputWindHeight_Veg = inputWindHeight_VegValue;
    z0 = 0.01;  //note that this is the foam roughness as found in the NinjaFoam AddBcBlock function. This has been manually set to 0.01 in NinjaFoam, so watch out if this value ends up changing there.
    Rd = RdValue;
    /*
     * WindNinja does the following to compute firstCellHeight for the momentum solver
     * After breaking down the formula, it looks to be essentially (meshvolume/cellcount)^(1/3)
     * where cellcount is half the cells in the blockMesh and half reserved for refineMesh.
     *
     * So in the past I did the following calculation
    double meshVolume = (x(zcells*Azpoints+ycells*xpoints+xcells)-x(0))
            *(y(zcells*Azpoints+ycells*xpoints+xcells)-y(0))
            *(z(zcells*Azpoints+ycells*xpoints+xcells)-z(0));
    firstCellHeight = pow(meshVolume/ncells,(1.0/3.0));

     * But I have since learned that firstCellHeight is really the first cell height.
     * So it should be the following
    firstCellHeight = z(1*Azpoints+0*xpoints+0)-z(0*Azpoints + 0*xpoints + 0). So z(Azpoints)-z(0)
    Hm, this is way smaller than the other value! In fact, the minZ cell height at the middle of vshaped valley is 2 m tall, where on its edges is 0.333 m tall!
    For now use the z method
    */
    firstCellHeight = z(Azpoints)-z(0);
    std::cout << "z(Azpoints) = " << z(Azpoints) << ", z(0) = " << z(0) <<
                 ", firstCellHeight = " << firstCellHeight << "\n";
    u = uwind;
    v = vwind;
    w = wwind;

    element elem(&mesh);

//values used for system directory
    //controlDict variables
    if(simulationType == "simpleFoam")
    {
        application = "simpleFoam";
        startFrom = "latestTime";
        startTime = "latestTime";
        stopAt = "endTime";
        endTime = "352";
        deltaT = "1.0";
        writeControl = "timeStep";
        writeInterval = "300";
        purgeWrite = "1";
        writeFormat = "ascii";
        writePrecision = "10";
        writeCompression = "uncompressed";
        timeFormat = "general";
        timePrecision = "10";
        runTimeModifiable = "true";
        adjustTimeStep = "false";
        maxCo = "";
    } else if(simulationType == "buoyantBoussinesqPimpleFoam")
    {
        application = "buoyantBoussinesqPimpleFoam";
        startFrom = "startTime";
        startTime = "0";
        stopAt = "endTime";
        endTime = "3600";
        deltaT = "1.0";
        writeControl = "adjustableRunTime";
        writeInterval = "60";
        purgeWrite = "0";
        writeFormat = "ascii";
        writePrecision = "10";
        writeCompression = "uncompressed";
        timeFormat = "general";
        timePrecision = "10";
        runTimeModifiable = "true";
        adjustTimeStep = "true";
        maxCo = "0.5";
    } else
    {
        // assume it is a myScalarTransportFoam solution
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
        adjustTimeStep = "false";
        maxCo = "";
    }

    //fvSchemes variables
    foam_fvSchemes.setupDesiredValues(simulationType);

    //fvSolution variables
    foam_fvSolutions.setupDesiredValues(simulationType);

    //setFieldsDict variables
    foam_setFields.setupDesiredValues(simulationType);

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
    static const char *CaseDir = CPLStrdup(CPLGenerateTempFilename( CPLSPrintf("POLYMESH_%s", outputPath.c_str())));
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
    printTransportProperties();
    makeFoamFooter();
    fclose(fzout);

//now create the property value files, the 0 directory files
    //create the scalar field file
    fzout = fopen(scalarPath.c_str(), "w");
    makeFoamHeader("volScalarField","T","0");
    printScalar();
    makeFoamFooter();
    fclose(fzout);

    if(simulationType == "myScalarTransportFoam")
    {
        //create the source field file, note this will get changed by the setFields dict
        fzout = fopen(sourcePath.c_str(), "w");
        makeFoamHeader("volScalarField","source","");
        printSource();
        makeFoamFooter();
        fclose(fzout);
    }

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

void openFoamPolyMesh::printTransportProperties()
{
    // could put if statements around each one of these lines,
    // or just keep it all the same since it doesn't mess stuff up to have extra entries in this file
    // it could possibly be confusing to a new user, but you just comment stuff out to find what
    // is actually needed. A single universal file for this seems like a good thing
    fprintf(fzout,"transportModel  %s;\n\n",transportModel.c_str());
    fprintf(fzout,"//Thermal Diffusivity Constant. 0.05 m^2/s was our guess based off of diffusivity comparison videos\n");
    fprintf(fzout,"DT              DT [ 0 2 -1 0 0 0 0 ]   %s;\n\n",thermalDiffusivityConstant.c_str());
    fprintf(fzout,"//Dynamic Viscosity. 1.846*10^05 kg/(m*s) for air at 300 K\n");
    fprintf(fzout,"mu              mu [1 -1 -1 0 0 0 0]    %s;\n\n",dynamicViscosity.c_str());
    fprintf(fzout,"//Kinematic Viscosity. 1.589*10^-5 m^2/s for air at 300 K\n");
    fprintf(fzout,"nu              nu [0 2 -1 0 0 0 0]     %s;\n\n",kinematicViscosity.c_str());
    fprintf(fzout,"//Density. 1.1614 kg/m^3 for air at 300 K\n");
    fprintf(fzout,"rho             rho [1 -3 0 0 0 0 0]    %s;\n\n",density.c_str());
    fprintf(fzout,"//Thermal Expansion Coefficient. Value is around 0.0034 1/K for air\n");
    fprintf(fzout,"beta            beta [0 0 0 -1 0 0 0]   %s;\n\n",thermalExpansionCoefficient.c_str());
    fprintf(fzout,"//Reference Temperature. Value is 300 K in hot room examples\n");
    fprintf(fzout,"TRef            TRef [0 0 0 1 0 0 0]    %s;\n\n",Tref.c_str());
    fprintf(fzout,"//Laminar Prandtl Number. Early hot room had this as 0.9 dimensionless, but later changed to 0.707 dimensionless which is the value for air at 300 K\n");
    fprintf(fzout,"Pr              Pr [0 0 0 0 0 0 0]      %s;\n\n",laminarPrandtlNumber.c_str());
    fprintf(fzout,"//Turbulent Prandtl Number. Early hot room had this as 0.7 dimensionless, but later changed to 0.85 dimensionless\n");
    fprintf(fzout,"Prt             Prt [0 0 0 0 0 0 0]     %s;\n\n",turbulentPrandtlNumber.c_str());
    fprintf(fzout,"//Constant pressure heat capacity. 1.005 kJ/(kg*K) for air at 300 K, which is 1005 (kg*m^2/s^2)/(kg*K) = 1005 m^2/(s^2*K)\n");
    fprintf(fzout,"Cp0             Cp0 [0 2 -2 -1 0 0 0]   %s;\n\n",heatCapacity.c_str());
    fprintf(fzout,"//Density used by wallHeatFluxIncompressible utility. Technically it's the same thing as rho, but the utility is currently written to use this separate value\n");
    fprintf(fzout,"rho0            rho0 [1 -3 0 0 0 0 0]   %s;\n\n",rho0.c_str());

}

//Might be able to figure out a way to improve this method using function pointers.
void openFoamPolyMesh::printFieldHeader(std::string patchName,std::string ListType,bool isNonuniformValue /*=false*/,
                                        std::string ListValue /*=""*/,bool extraReturn /*=false*/)
{
    fprintf(fzout,"    %s\n    {\n",patchName.c_str());
    fprintf(fzout,"        type            %s;\n",ListType.c_str());
    if(ListType == "fixedValue")
    {
        if(isNonuniformValue == false)
        {
            fprintf(fzout,"        value           uniform %s;\n}\n",ListValue.c_str());
        } else
        {
            fprintf(fzout,"        value           nonuniform List<vector>\n");
        }
    } else if(ListType == "pressureInletOutletVelocity")
    {
        if(isNonuniformValue == false)
        {
            fprintf(fzout,"        value           uniform %s;\n}\n",ListValue.c_str());
        } else
        {
            fprintf(fzout,"        value           nonuniform List<vector>\n");
        }
    } else if(ListType == "logProfileVelocityInlet")
    {
        fprintf(fzout,"        UfreeStream     %lf;\n",UfreeStream);
        fprintf(fzout,"        uDirection      %s;\n",uDirection.c_str());
        fprintf(fzout,"        inputWindHeight_Veg %lf;\n",inputWindHeight_Veg);
        fprintf(fzout,"        z0              %lf;\n",z0);
        fprintf(fzout,"        Rd              %lf;\n",Rd);
        fprintf(fzout,"        firstCellHeight %lf;\n",firstCellHeight);
        if(isNonuniformValue == false)
        {
            fprintf(fzout,"        value           uniform %s;\n}\n",ListValue.c_str());
        } else
        {
            fprintf(fzout,"        value           nonuniform List<vector>\n");
        }
    } else
    {
        // this accounts for type zeroGradient
        fprintf(fzout,"    }\n");
    }
    if(extraReturn == true)
    {
        fprintf(fzout,"\n");
    }
}

void openFoamPolyMesh::printScalar()
{
    if(simulationType == "buoyantBoussinesqPimpleFoam")
    {
        fprintf(fzout,"dimensions      [0 0 0 1 0 0 0];\n\n");
        fprintf(fzout,"internalField   uniform 300;\n\n");
        fprintf(fzout,"boundaryField\n{\n");

        printFieldHeader("north_face","zeroGradient");
        printFieldHeader("west_face","zeroGradient");
        printFieldHeader("east_face","zeroGradient");
        printFieldHeader("south_face","zeroGradient");
        printFieldHeader("minZ","fixedValue",false,"310");
        printFieldHeader("maxZ","zeroGradient");

        fprintf(fzout,"}\n");
    } else
    {
        // assume that everything else is myScalarTransportFoam
        fprintf(fzout,"dimensions      [1 -3 0 0 0 0 0];\n\n");
        fprintf(fzout,"internalField   uniform 0;\n\n");
        fprintf(fzout,"boundaryField\n{\n");

        printFieldHeader("north_face","zeroGradient");
        printFieldHeader("west_face","zeroGradient");
        printFieldHeader("east_face","zeroGradient");
        printFieldHeader("south_face","zeroGradient");
        printFieldHeader("minZ","zeroGradient");
        printFieldHeader("maxZ","zeroGradient");

        fprintf(fzout,"}\n");
    }
}

void openFoamPolyMesh::printSource()
{
    fprintf(fzout,"dimensions      [1 -3 -1 0 0 0 0];\n\n");
    fprintf(fzout,"internalField   uniform 0;\n\n");
    fprintf(fzout,"boundaryField\n{\n");

    printFieldHeader("north_face","zeroGradient");
    printFieldHeader("west_face","zeroGradient");
    printFieldHeader("east_face","zeroGradient");
    printFieldHeader("south_face","zeroGradient");
    printFieldHeader("minZ","zeroGradient");
    printFieldHeader("maxZ","zeroGradient");

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
        if(BCtype == "WindNinja")
        {
            printFieldHeader("north_face","logProfileVelocityInlet",true);
        } else if(BCtype == "OpenFOAM")
        {
            printFieldHeader("north_face","fixedValue",true);
        } else
        {
            std::cout << "Error in openFoamPolyMesh! BCtype is invalid!\n";
        }
    } else
    {
        printFieldHeader("north_face","pressureInletOutletVelocity",true);
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
        if(BCtype == "WindNinja")
        {
            printFieldHeader("west_face","logProfileVelocityInlet",true);
        } else if(BCtype == "OpenFOAM")
        {
            printFieldHeader("west_face","fixedValue",true);
        } else
        {
            std::cout << "Error in openFoamPolyMesh! BCtype is invalid!\n";
        }
    } else
    {
        printFieldHeader("west_face","pressureInletOutletVelocity",true);
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
        if(BCtype == "WindNinja")
        {
            printFieldHeader("east_face","logProfileVelocityInlet",true);
        } else if(BCtype == "OpenFOAM")
        {
            printFieldHeader("east_face","fixedValue",true);
        } else
        {
            std::cout << "Error in openFoamPolyMesh! BCtype is invalid!\n";
        }
    } else
    {
        printFieldHeader("east_face","pressureInletOutletVelocity",true);
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
        if(BCtype == "WindNinja")
        {
            printFieldHeader("south_face","logProfileVelocityInlet",true);
        } else if(BCtype == "OpenFOAM")
        {
            printFieldHeader("south_face","fixedValue",true);
        } else
        {
            std::cout << "Error in openFoamPolyMesh! BCtype is invalid!\n";
        }
    } else
    {
        printFieldHeader("south_face","pressureInletOutletVelocity",true);
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
    printFieldHeader("minZ","fixedValue",false,"(0 0 0)");

    //now print the maxZ face velocities
    printFieldHeader("maxZ","zeroGradient");

    fprintf(fzout,"}\n");
}

void openFoamPolyMesh::writeLibWindNinja()
{
    fprintf(fzout,"libs (\"libWindNinja.so\")\n");
    fprintf(fzout,"functions\n(\n");
    fprintf(fzout,"/*\n    Surface-0\n");
    fprintf(fzout,"/*    {\n");
    fprintf(fzout,"    type faceSource;//patchAverage; //patchAverage, patchFieldFlow, patchIntegrate\n");
    fprintf(fzout,"    functionObjectLibs (\"libFieldFunctionObjects.so\");\n");
    fprintf(fzout,"    outputControl timeStep;\n");
    fprintf(fzout,"    outputInterval 1;\n");
    fprintf(fzout,"    verbose true;\n");
    fprintf(fzout,"    log yes;\n");
    fprintf(fzout,"    source faceZone;\n");
    fprintf(fzout,"    sourceName mixer_1;\n");
    fprintf(fzout,"    operation weightedAverage;\n");
    fprintf(fzout,"    fields\n    (\n        U\n    );\n");
    fprintf(fzout,"   // patches\n   // (\n   //     pressure_outlet.8\n");
    fprintf(fzout,"   // );\n   // factor 1.1614;\n");
    fprintf(fzout,"    }*/\n");
    fprintf(fzout,"{\n");
    fprintf(fzout,"        type            faceSource;\n");
    fprintf(fzout,"        functionObjectLibs (\"libfieldFunctionObjects.so\");\n");
    fprintf(fzout,"        enabled         yes;\n");
    fprintf(fzout,"        outputControl   timeStep;//outputTime;\n");
    fprintf(fzout,"        outputInterval  1;\n");
    fprintf(fzout,"        log             yes;\n");
    fprintf(fzout,"        valueOutput     no;\n");
    fprintf(fzout,"        source          sampledSurface;//faceZone;\n\n");
    fprintf(fzout,"        sampledSurfaceDict\n");
    fprintf(fzout,"        {\n");
    fprintf(fzout,"            // Sampling on triSurface\n");
    fprintf(fzout,"            type        sampledTriSurfaceMesh;\n");
    fprintf(fzout,"            surface     disc2.stl;\n");
    fprintf(fzout,"            source      cells;  // sample cells or boundaryFaces\n");
    fprintf(fzout,"            interpolate true;\n");
    fprintf(fzout,"        }\n\n");
    fprintf(fzout,"        // Operation: areaAverage/sum/weightedAverage ...\n");
    fprintf(fzout,"        //sourceName      maxX2minX0MasterZone;\n");
    fprintf(fzout,"        operation       areaAverage;//weightedAverage;\n");
    fprintf(fzout,"        //weightField     phi;\n");
    fprintf(fzout,"        autoInterpolate true;\n");
    fprintf(fzout,"        fields\n        (\n");
    fprintf(fzout,"            U\n");
    fprintf(fzout,"            p\n");
    fprintf(fzout,"        );\n");
    fprintf(fzout,"    }\n");
    fprintf(fzout,"*/\n);\n");
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
    fprintf(fzout,"runTimeModifiable %s;\n\n",runTimeModifiable.c_str());
    if(adjustTimeStep == "true")
    {
        fprintf(fzout,"adjustTimeStep  %s;\n\n",adjustTimeStep.c_str());
        fprintf(fzout,"maxCo           %s;\n\n",maxCo.c_str());
    }
    if(BCtype == "WindNinja")
    {
        writeLibWindNinja();
    }
}

void openFoamPolyMesh::writeFvSchemes()
{
    // need a separate variable with separate variables for each scheme type, if you don't use a class
    // becomes a simple function when using a class, and the sizes stay protected
    // just barely starts to be complex enough that it might be easier to use a template file
    // notice how much more complex fvSolution is, when it really isn't that different, just has another layer
    std::vector<std::string> fvScheme_types = foam_fvSchemes.get_types();
    std::vector<std::string> fvScheme_defaultwhitespace = foam_fvSchemes.get_defaultwhitespace();
    std::vector<std::string> fvScheme_defaultvalues = foam_fvSchemes.get_defaultvalues();
    std::vector< std::vector<std::string> > fvScheme_nondefaultnames = foam_fvSchemes.get_nondefaultnames();
    std::vector< std::vector<std::string> > fvScheme_nondefaultwhitespace = foam_fvSchemes.get_nondefaultwhitespace();
    std::vector< std::vector<std::string> > fvScheme_nondefaultvalues = foam_fvSchemes.get_nondefaultvalues();
    for(size_t j = 0; j < fvScheme_types.size(); j++)
    {
        fprintf(fzout,"%s\n",fvScheme_types[j].c_str());
        fprintf(fzout,"{\n");
        fprintf(fzout,"    default");
        fprintf(fzout,"%s",fvScheme_defaultwhitespace[j].c_str());
        fprintf(fzout,"%s;\n",fvScheme_defaultvalues[j].c_str());
        for(size_t i = 0; i < fvScheme_nondefaultnames[j].size(); i++)
        {
            fprintf(fzout,"    %s",fvScheme_nondefaultnames[j][i].c_str());
            fprintf(fzout,"%s",fvScheme_nondefaultwhitespace[j][i].c_str());
            fprintf(fzout,"%s;\n",fvScheme_nondefaultvalues[j][i].c_str());
        }
        fprintf(fzout,"}\n\n");
    }
}

void openFoamPolyMesh::writeFvSolution()
{
    // this is basically the same thing as fvSchemes, but I tried to make it into a more generic function
    // turns out that adding in just one vector layer more than fvSchemes made it super complex. It doesn't help
    // that there are small things different for each section that make it necessary to add small tweaks
    // I definitely think that this would be simpler if the sections vector were broken up into separate variables
    // this would be even simpler to just use a template file and replace keys, like in ninjaFoam.cpp
    // just look at all the exceptions I had to throw in to get the right format, throwing the different sections into one generic type
    std::vector<std::string> fvSolution_sections = foam_fvSolutions.get_sections();
    std::vector< std::vector<std::string> > fvSolution_headers = foam_fvSolutions.get_headers();
    std::vector< std::vector< std::vector<std::string> > > fvSolution_keyWords = foam_fvSolutions.get_keyWords();
    std::vector< std::vector< std::vector<std::string> > > fvSolution_whitespace = foam_fvSolutions.get_whitespace();
    std::vector< std::vector< std::vector<std::string> > > fvSolution_keyWordValues = foam_fvSolutions.get_keyWordValues();

    for(size_t j = 0; j < fvSolution_sections.size(); j++)
    {
        fprintf(fzout,"%s\n",fvSolution_sections[j].c_str());
        fprintf(fzout,"{\n");
        for(size_t i = 0; i < fvSolution_headers[j].size(); i++)
        {
            if(fvSolution_headers[j][i] == "noheader")
            {
                for(size_t k = 0; k < fvSolution_keyWords[j][i].size(); k++)
                {
                    fprintf(fzout,"    %s",fvSolution_keyWords[j][i][k].c_str());
                    fprintf(fzout,"%s",fvSolution_whitespace[j][i][k].c_str());
                    fprintf(fzout,"%s;\n",fvSolution_keyWordValues[j][i][k].c_str());
                }
                if(fvSolution_sections[j] == "PIMPLE" && i != fvSolution_headers[j].size())  // note this will only work if we assume there is always a "noheader" header for PIMPLE
                {
                    fprintf(fzout,"\n");
                    fprintf(fzout,"    residualControl\n");
                    fprintf(fzout,"    {\n");
                }
            } else
            {
                if(fvSolution_sections[j] == "PIMPLE")
                {
                    fprintf(fzout,"        %s\n",fvSolution_headers[j][i].c_str());
                    fprintf(fzout,"        {\n");
                    for(size_t k = 0; k < fvSolution_keyWords[j][i].size(); k++)
                    {
                        fprintf(fzout,"            %s",fvSolution_keyWords[j][i][k].c_str());
                        fprintf(fzout,"%s",fvSolution_whitespace[j][i][k].c_str());
                        fprintf(fzout,"%s;\n",fvSolution_keyWordValues[j][i][k].c_str());
                    }
                    fprintf(fzout,"        }\n");
                    if(i == fvSolution_headers[j].size())
                    {
                        // got to the end of the loop, need end part
                        fprintf(fzout,"    }\n");
                    }
                } else
                {
                    if(fvSolution_headers[j][i] == "residualControl")
                    {
                        fprintf(fzout,"\n");
                    }
                    fprintf(fzout,"    %s\n",fvSolution_headers[j][i].c_str());
                    fprintf(fzout,"    {\n");
                    for(size_t k = 0; k < fvSolution_keyWords[j][i].size(); k++)
                    {
                        fprintf(fzout,"        %s",fvSolution_keyWords[j][i][k].c_str());
                        fprintf(fzout,"%s",fvSolution_whitespace[j][i][k].c_str());
                        fprintf(fzout,"%s;\n",fvSolution_keyWordValues[j][i][k].c_str());
                    }
                    fprintf(fzout,"    }\n");
                }
            }
        }
        fprintf(fzout,"}\n\n");
    }
}

void openFoamPolyMesh::writeSetFieldsDict()
{
    std::vector<std::string> fieldNames = foam_setFields.get_fieldNames();
    std::vector<std::string> fieldTypes = foam_setFields.get_fieldTypes();
    std::vector<std::string> fieldDefaultValues = foam_setFields.get_fieldDefaultValues();
    std::vector< std::vector<std::string> > fieldRegions = foam_setFields.get_fieldRegions();
    std::vector< std::vector< std::vector<std::string> > > fieldRegionInfo = foam_setFields.get_fieldRegionInfo();
    std::vector< std::vector< std::vector<std::string> > > fieldRegionValues = foam_setFields.get_fieldRegionValues();

    fprintf(fzout,"defaultFieldValues\n");
    fprintf(fzout,"(\n");
    for(size_t j = 0; j < fieldNames.size(); j++)
    {
        fprintf(fzout,"    %s %s %s\n",fieldTypes[j].c_str(),fieldNames[j].c_str(),fieldDefaultValues[j].c_str());
    }
    fprintf(fzout,");\n");
    fprintf(fzout,"\n");
    fprintf(fzout,"regions\n");
    fprintf(fzout,"(\n");
    // hmm, should this actually be where regions is independent of names? So restructure by regions, but make sure there are only so many names?
    for(size_t j = 0; j < fieldNames.size(); j++)
    {
        for(size_t i = 0; i < fieldRegions[j].size(); i++)
        {
            fprintf(fzout,"    %s\n",fieldRegions[j][i].c_str());
            fprintf(fzout,"    {\n");
            for(size_t k = 0; k < fieldRegionInfo[j][i].size(); k++)
            {
                fprintf(fzout,"        %s\n",fieldRegionInfo[j][i][k].c_str());
            }
            fprintf(fzout,"        fieldValues\n");
            fprintf(fzout,"        (\n");
            for(size_t k = 0; k < fieldRegionValues[j][i].size(); k++)
            {
                fprintf(fzout,"            %s %s %s\n",fieldTypes[j].c_str(),fieldNames[j].c_str(),fieldRegionValues[j][i][k].c_str());
            }
            fprintf(fzout,"        );\n");
            fprintf(fzout,"    }\n");
        }
    }
    fprintf(fzout,");\n");
}
