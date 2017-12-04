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

#include "mesh.h"
#include "element.h"
#include "wn_3dArray.h"
#include "wn_3dScalarField.h"
#include "ninjaException.h"

#include "openFoam_fvSchemes.h"
#include "openFoam_fvSolutions.h"

/*
currently this program is set up to populate the constant/polymesh directory in a given case folder with files that replicate the OpenFoam cavity tutorial, not the windninja mesh points. The idea is to get a better understanding of how the faces and neighbors need to be written. After this works for normal points, I'm going to adjust it to mesh specific stuff.
*/


class openFoamPolyMesh
{
public:

    openFoamPolyMesh(std::string outputPath, Mesh mesh, double xllCornerValue, double yllCornerValue,
                     double inputDirectionValue, double UfreeStreamValue, double inputWindHeight_VegValue,
                     double RdValue, wn_3dScalarField const& uwind, wn_3dScalarField const& vwind,
                     wn_3dScalarField const& wwind, std::string BCtypeValue);
    ~openFoamPolyMesh();

private:

    //generates a new case using the dem location and dem name
    void generateCaseDirectory(std::string outputPath);

    //this is the equivalent main function
    bool writePolyMeshFiles(element elem);

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

//above stuff is technically for the constant polymesh directory. This stuff is for the rest of the constant directory.
    void printTransportProperties();

//above stuff is technically for the constant directory. This stuff is for the time 0 directory.

    //just a way to group a lot of stuff I was doing in single lines over and over. Should make it more legible and shrink the number of lines of code.
    void printFieldHeader(std::string patchName,std::string ListType,bool isNonuniformValue = false,
                                            std::string ListValue = "",bool extraReturn = false);

    //prints the T file in the 0 directory
    void printScalar();

    //prints the initial source file in the 0 directory
    void printSource();

    //computes uDirection for use in printing velocities
    void ComputeUdirection();

    //prints the velocities in the 0 directory
    /*
     * the velocities appear to be stored as cell center values for the internal part of the mesh,
     * and face center values for the external patches, placed in the same order that the points
     * are generated in the mesh conversion.
    */
    void printVelocity(element elem);


    /*
     * this section is a bit tricky because there are 3 types of simulations that can be done
     * each with their different dict files. 1) can output files like a standard WindNinja momentum run
     * 2) can output files to do smoke transport 3) can output files for doing an energy eqn simulation
     * Technically the time directory and constant non-polymesh files can also be different depending
     * on for which of these types of simulations the files are created.
     */

//now create the stuff for the system directory
    //creates the controlDict file
    void writeLibWindNinja();
    void writeControlDict();

    //creates the fvSchemes file
    void writeFvSchemes();

    //creates the fvSolution file
    void writeFvSolution();

    //creates the setFieldsDict file
    void writeSetFieldsDict();


// now declare variables
    std::string simulationType; // this is for adjusting what types of file output you get, depending on the type of solver the files will be used for
    std::string foam_version;	//the foam version number, probably will get rid of this later
    FILE *fzout;		//changing file to which information will be written

    double xpoints;		//number of points in the x direction
    double ypoints;		//number of points in the y direction
    double zpoints;		//number of points in the z direction

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

//values used for constant non-polymesh directory. Stuff above is for constant/polymesh directory, but sometimes used for other stuff below as well
    std::string transportPropertiesPath;    //path to the transportProperties file, found in constant from the case directory

    std::string transportModel;         // the transport model used in the transport properties (probably viscosity model)
    std::string thermalDiffusivityConstant;    //the thermal diffusivity constant
    std::string dynamicViscosity;            // this is for a newtonian transport model
    std::string kinematicViscosity;          // this is for a newtonian transport model
    std::string density;                     // density of the fluid (I believe for a single fluid)
    std::string thermalExpansionCoefficient;    // thermal expansion coefficient
    std::string Tref;                        // reference temperature
    std::string laminarPrandtlNumber;        // laminarPrandtlNumber
    std::string turbulentPrandtlNumber;      // turbulent prandtl number
    std::string heatCapacity;              // constant pressure heat capacity
    std::string rho0;                      // density used by wallHeatFluxIncompressible utility
    // technically could add in the Coeffs for other models, but need to think of the best way to do that

//values used for time directory. Stuff above is for constant/polymesh directory, but sometimes used for other stuff below as well
    std::string scalarPath;     //path to the scalar T data file, found in 0 from the case directory
    std::string sourcePath;     //path to the source data file, found in 0 from the case directory
    std::string velocityPath;   //path to the velocity U data file, found in 0 from the case directory

    std::string BCtype;         // the boundary condition type, so WindNinja or OpenFOAM for now, where WindNinja is as close to WindNinja standard BCs where OpenFOAM is as close to standard OpenFOAM BCs as there are
	double inputDirection;		//the input wind direction set by the user, used to properly set the conditions for each boundary condition
    double UfreeStream;         //this is the input speed. Used for the inlet velocity boundary condition
    std::string uDirection;          //this is the velocity vector of the input direction. Used for the inlet velocity boundary condition
    double inputWindHeight_Veg;
    double z0;
    double Rd;
    double firstCellHeight;
    wn_3dScalarField u, v, w; //the velocity profiles in each direction for the mesh

//values used for system directory
    std::string controlDictPath;    //path to controlDict file
    std::string fvSchemesPath;      //path to fvSchemes file
    std::string fvSolutionPath;     //path to fvSolution file
    std::string setFieldsDictPath;  //path to setFieldsDict file

    //controlDict variables
    std::string application;        //the simulation type, the application variable in controlDict file
    std::string startFrom;          //controls the start time of the simulation. Options are: firstTime, startTime, latestTime
    std::string startTime;          //startTime for the simulation if startFrom is specified to be startTime
    std::string stopAt;             //controls the end time of the simulation. Options are: endTime, writeNow, noWriteNow, nextWrite
    std::string endTime;            //end time for the simulation when stopAt is specified to be endTime
    std::string deltaT;             //time step of the simulation
    std::string writeControl;       //controls the timing of write output to file. Options are: timeStep, runTime, adjustableRunTime, cpuTime, clockTime
    std::string writeInterval;      //frequency after startTime for when new time directories are written
    std::string purgeWrite;         //number of written out time directories to be kept for a given simulation
    std::string writeFormat;        //specifies the format of the data files. Options are: ascii or binary
    std::string writePrecision;     //number of decimal places for writing time directories
    std::string writeCompression;   //compress written out files with gzip yes or no
    std::string timeFormat;         //choice of name format for the time directories. Options are: fixed, scientific, general
    std::string timePrecision;      //number of decimal places for the time directory names
    std::string runTimeModifiable;  //allow controlDict to change, thus changing simulation, in the middle of simulation
    std::string adjustTimeStep;     //allow the use of a courant number. Not applicable in steady state solvers
    std::string maxCo;              //maximum courant number, for use with adjustTimeStep

    //fvSchemes variables
    openFoam_fvSchemes foam_fvSchemes;

    //fvSolution variables
    //note this is similar to fvSchemes, but trickier. You can see where writing out the file stops
    //being as effective as using a template file that only needs minor changes
    openFoam_fvSolutions foam_fvSolutions;

    //setFieldsDict variables
    //I think the easiest way to do this is to have a writer class that just writes information
    //and a separate class that obtains the information
    std::string defaultTvalue;      //default scalar value, so value to be placed everywhere where not specified
    std::string defaultSourceValue; //default source value, so value to be placed everywhere where not specified
    std::string distributionType;   //method to be used. If boxToCell, then will use a bounding box for specifying the source
    std::string boxMinCoordinates;  //set of xmin, ymin, and zmin coordinates for the bounding box if using boxToCell distribution method
    std::string boxMaxCoordinates;  //set of xmax, ymax, and zmax coordinates for the bounding box if using boxToCell distribution method
    std::string distributionTValue; //value of scalar in the location to be specified
    std::string distributionSourceValue;    //value of source in the location to be specified

};


#endif	//OPENFOAMPOLYMESH_H

