#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>	//in order to use system calls

using namespace std;


void makeFoamHeader(FILE *outFile, string foamVersionNumber, string theClassType, string theObjectType, string theFoamFileLocation="")
{
	fprintf(outFile,"/*--------------------------------*- C++ -*----------------------------------*\\\n");
	fprintf(outFile,"| =========                 |                                                 |\n");
	fprintf(outFile,"| \\\\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |\n");
	fprintf(outFile,"|  \\\\    /   O peration     | Version:  2.2.0                                 |\n");
	fprintf(outFile,"|   \\\\  /    A nd           | Web:      www.OpenFOAM.org                      |\n");
	fprintf(outFile,"|    \\\\/     M anipulation  |                                                 |\n");
	fprintf(outFile,"\\*---------------------------------------------------------------------------*/\n");
	fprintf(outFile,"FoamFile\n{\n");
	fprintf(outFile,"    version     %s;\n",foamVersionNumber.c_str());
	fprintf(outFile,"    format      ascii;\n");
	fprintf(outFile,"    class       %s;\n",theClassType.c_str());
	if (theFoamFileLocation != "" )	
	{
		fprintf(outFile,"    location    \"%s\";\n",theFoamFileLocation.c_str());
	}
	fprintf(outFile,"    object      %s;\n",theObjectType.c_str());
	fprintf(outFile,"}\n// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //\n\n");
}

void makeFoamFooter(FILE *outFile)
{
	fprintf(outFile,"\n\n// ************************************************************************* //");
}

void printPoints(FILE *outFile, double xmax, double ymax, double zmax)
{	
	fprintf(outFile, "\n%0.0lf\n(\n", xmax*ymax*zmax);
	for(double k=0; k<zmax; k++)
  	{
      		for(double j=0; j<ymax; j++)
	  	{
			for (double i = 0;i<xmax;i++)
			{
				fprintf(outFile, "(%lf %lf %lf)\n", i, j, k);
			}
	  	}
  	}
	fprintf(outFile, ")\n");
}

void printFaces(FILE *outFile, double xmax, double ymax, double zmax)
{

}

void createMyPath(string given_path)
{
	string mkdir_command_path = "rm -r -f " + given_path;

	system(mkdir_command_path.c_str());
	mkdir_command_path = "mkdir -p " + given_path;
	system(mkdir_command_path.c_str());	//note that this will not remove files inside the case if it already exists. However, I decided to use the rm command to remove stuff first just in case. Just did it to the case directory before making more directories
	cout << "made " << mkdir_command_path << " path" << endl;
}

//int main()
int funRun()
{
	const string foam_version = "2.0";
	double xmax = 30;
	double ymax = 25;
	double zmax = 10;
	FILE *fzout;

	cout << "the program is running\n" << endl;

	string case_path = "/home/latwood/Downloads/case/";
	string zero_path = case_path + "0/";
	string system_path = case_path + "system/";
	string constant_path = case_path + "constant/";
	
	createMyPath(case_path);	//technically not needed, except this run the rm system call on the case folder, emptying the contents that are alongside the zero,system, and constant folders
	createMyPath(zero_path); 
	createMyPath(system_path);
	createMyPath(constant_path);


	//start making the transport properties file
	string current_path = constant_path+"transportProperties";
	string dT = "0.1";
	fzout = fopen(current_path.c_str(),"w");
	makeFoamHeader(fzout,foam_version, "dictionary","transportProperties","constant");
	fprintf(fzout,"DT\t\tDT [ 0 2 -1 0 0 0 0 ] %s;\n",dT.c_str());
	makeFoamFooter(fzout);
	fclose(fzout);
	//reset the constant_path to do the mesh
	constant_path = constant_path + "polyMesh/";
	createMyPath(constant_path);
	//finished transport properties file
	
	//now start making the mesh

	//this is setting up the points as a vtk
	current_path = case_path+"points.vtk";
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
	current_path = constant_path+"points";
	fzout = fopen(current_path.c_str(), "w");
	makeFoamHeader(fzout,foam_version, "vectorField","points","constant/polyMesh");
	printPoints(fzout,xmax,ymax,zmax);
	makeFoamFooter(fzout);
  	fclose(fzout);

	//now create the faces
	current_path = constant_path+"faces";
	fzout = fopen(current_path.c_str(), "w");
	makeFoamHeader(fzout,foam_version, "faceList","faces","constant/polyMesh");
	printFaces(fzout,xmax,ymax,zmax);
	makeFoamFooter(fzout);
  	fclose(fzout);


	//program is finished
	cout << "output finished\n";

	return 0;
}



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

/* from NinjaFoam it looks like a way to get the case directory set up
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
*/
//there are also nice update whatever foam control dict. I could make something nice like that for running the smoke transport. there's also functions for writing each separate folder and for writing each separate condition, so like write boundry patch or stuff like that.
//ninjafoam copy file looks like a very useful piece! Though I don't understand why a key is needed.
//okay writeFoamFiles() might be the way to generate a new case.
//here is the part in foamfiles that seems to work:
/*
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
