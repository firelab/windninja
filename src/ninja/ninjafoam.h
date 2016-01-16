/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  OpenFOAM interaction
 * Author:   Kyle Shannon <kyle at pobox dot com>
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

#ifndef NINJA_FOAM_INCLUDED_
#define NINJA_FOAM_INCLUDED_

#include "ninja.h"

#include "assert.h"

#include "stl_create.h"
#include "ninja_conv.h"
#include "ninja_errors.h"

#include "gdal_alg.h"
#include "cpl_spawn.h"

#define PIPE_BUFFER_SIZE 4096

#define NINJA_FOAM_OGR_VRT "<OGRVRTDataSource>" \
                           "  <OGRVRTLayer name=\"%s\">" \
                           "    <SrcDataSource>%s</SrcDataSource>" \
                           "    <SrcLayer>%s</SrcLayer>" \
                           "    <GeometryType>wkbPoint</GeometryType>" \
                           "    <GeometryField encoding=\"PointFromColumns\" x=\"x\" y=\"y\" z=\"z\"/>"  \
                           "    <Field name=\"X\" src=\"x\" type=\"Real\"/>" \
                           "    <Field name=\"Y\" src=\"y\" type=\"Real\"/>" \
                           "    <Field name=\"Z\" src=\"z\" type=\"Real\"/>" \
                           "    <Field name=\"U\" src=\"U_x\" type=\"Real\"/>" \
                           "    <Field name=\"V\" src=\"U_y\" type=\"Real\"/>" \
                           "    <Field name=\"W\" src=\"U_z\" type=\"Real\"/>" \
                           "  </OGRVRTLayer>" \
                           "</OGRVRTDataSource>"

/**
 * \brief Main interface to OpenFOAM solver runs.
 *
 */
class NinjaFoam : public ninja
{

public:
    NinjaFoam();
    virtual ~NinjaFoam();

    NinjaFoam( NinjaFoam const& A );
    NinjaFoam& operator= ( NinjaFoam const& A );

    virtual bool simulate_wind();
    inline virtual std::string identify() {return std::string("ninjafoam");}

private:

    std::vector<double> direction; //input.inputDirection converted to unit vector notation
    std::vector<std::string> inlets; // e.g., north_face
    std::vector<std::string> bcs; 

    const char *pszTempPath;

    int GenerateTempDirectory();
    int WriteJson();
    
    int WriteFoamFiles();
    int WriteZeroFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename);
    int WriteSystemFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename);
    int WriteConstantFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename);
    int AddBcBlock(std::string &dataString);
    int WritePBoundaryField(std::string &dataString);
    int WriteUBoundaryField(std::string &dataString);
    int WriteKBoundaryField(std::string &dataString);
    int WriteEpsilonBoundaryField(std::string &dataString);
    
    void ComputeDirection(); //converts direction from degrees to unit vector notation
    void SetInlets();
    void SetBcs();
    int writeMoveDynamicMesh();
    int writeBlockMesh();
    int readDem(double &ratio_); //sets blockMesh data from DEM 
    int readLogFile(double &ratio); //sets blockMesh data from STL log file when DEM not available
    
    std::string boundary_name;
    std::string terrainName;
    std::string type;
    std::string value;
    std::string gammavalue;
    std::string pvalue;
    std::string inletoutletvalue;
    std::string template_;
    
    std::vector<std::string> bboxField;
    std::vector<std::string> cellField;
    std::vector<double> bbox;
    std::vector<int> nCells; //number of cells in x,y,z directions of blockMesh
    double side; // length of side of regular hex cell
    double initialFirstCellHeight; //approx height of near-ground cell after moveDynamicMesh
    double oldFirstCellHeight; //approx height of near-ground cell at previous time-step
    double finalFirstCellHeight; //final approx height of near-ground cell after refinement
    int latestTime; //latest time directory
    int cellCount; //total cell count in the mesh 
    
    int ReplaceKey(std::string &s, std::string k, std::string v);
    int ReplaceKeys(std::string &s, std::string k, std::string v, int n = INT_MAX);
    int CopyFile(const char *pszInput, const char *pszOutput, std::string key="", std::string value="");
    
    int SurfaceTransformPoints();
    int SurfaceCheck();
    int RefineSurfaceLayer();
    int MoveDynamicMesh();
    int TopoSet();
    int RefineMesh();
    int BlockMesh();
    int DecomposePar();
    int ReconstructParMesh();
    int ReconstructPar();
    int RenumberMesh();
    int CheckMesh();
    int ApplyInit();
    int SimpleFoam();
    int Sample();
    int ReadStl();
    void UpdateDictFiles(); //updates U, p, epsilon, and k files for new timesteps (meshes)

    /* GDAL/OGR output */
    const char *pszVrtMem;
    const char *pszGridFilename;
    
    int SampleRawOutput();
    int WriteOutputFiles();
    void SetOutputFilenames();
    
    //Timers
    double startTotal, endTotal;
    double startMesh, endMesh;
    double startInit, endInit;
    double startSolve, endSolve;
    double startWriteOut, endWriteOut;
    double startFoamFileWriting, endFoamFileWriting;
    double startOutputSampling, endOutputSampling;
    double startStlConversion, endStlConversion;
    
#ifdef NINJA_BUILD_TESTING
public:
#endif
    int SanitizeOutput();
    int SampleCloudGrid();
    int SampleCloud();
#ifdef NINJA_BUILD_TESTING
private:
#endif
    const char * GetGridFilename();

};

#endif /* NINJA_FOAM_INCLUDED_ */

