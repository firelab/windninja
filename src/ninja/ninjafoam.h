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
                           "    <GeometryField encoding=\"PointFromColumns\" x=\"x\" y=\"y\" z=\"z\" k=\"k\"/>"  \
                           "    <Field name=\"X\" src=\"x\" type=\"Real\"/>" \
                           "    <Field name=\"Y\" src=\"y\" type=\"Real\"/>" \
                           "    <Field name=\"Z\" src=\"z\" type=\"Real\"/>" \
                           "    <Field name=\"U\" src=\"U_x\" type=\"Real\"/>" \
                           "    <Field name=\"V\" src=\"U_y\" type=\"Real\"/>" \
                           "    <Field name=\"W\" src=\"U_z\" type=\"Real\"/>" \
                           "    <Field name=\"K\" src=\"k\" type=\"Real\"/>" \
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

    virtual void set_meshResolution(double resolution, lengthUnits::eLengthUnits units);
    virtual double get_meshResolution();
    static int GenerateFoamDirectory(std::string demName);
    static void SetFoamPath(const char *pszPath);

private:
    std::string foamVersion;
    
    static const char *pszFoamPath;

    /* OpenFOAM case setup */
    void UpdateExistingCase();
    void GenerateNewCase();
    void WriteFoamFiles();
    void WriteZeroFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename);
    void WriteSystemFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename);
    void WriteConstantFiles(VSILFILE *fin, VSILFILE *fout, const char *pszFilename);
    void AddBcBlock(std::string &dataString);
    void WritePBoundaryField(std::string &dataString);
    void WriteUBoundaryField(std::string &dataString);
    void WriteKBoundaryField(std::string &dataString);
    void WriteEpsilonBoundaryField(std::string &dataString);
    
    void ComputeDirection(); //converts direction from degrees to unit vector notation
    void SetInlets();
    void SetBcs();
    
    std::vector<double> direction; //input.inputDirection converted to unit vector notation
    std::vector<std::string> inlets; // e.g., north_face
    std::vector<std::string> bcs;
    std::string boundary_name;
    std::string type;
    std::string value;
    std::string gammavalue;
    std::string pvalue;
    std::string inletoutletvalue;
    std::string template_;

    /* mesh */
    void writeMoveDynamicMesh();
    void writeBlockMesh();
    void SetBlockMeshParametersFromDem(); //sets blockMesh data from DEM

    std::vector<std::string> bboxField;
    std::vector<std::string> cellField;
    std::vector<double> bbox;
    std::vector<int> nCells; //number of cells in x,y,z directions of blockMesh
    int cellCount; //total cell count in the mesh
    int nRoundsRefinement; //number of times refineMesh is called
    double meshResolution; // mesh resolution
    lengthUnits::eLengthUnits meshResolutionUnits; //mesh resolution units (feet, meters, miles, kilometers)
    double initialFirstCellHeight; //approx height of near-ground cell after moveDynamicMesh
    double oldFirstCellHeight; //approx height of near-ground cell at previous time-step
    double finalFirstCellHeight; //final approx height of near-ground cell after refinement

    /* OpenFOAM case control */
    int ReplaceKey(std::string &s, std::string k, std::string v);
    int ReplaceKeys(std::string &s, std::string k, std::string v, int n = INT_MAX);
    void CopyFile(const char *pszInput, const char *pszOutput, std::string key="", std::string value="");
    void UpdateTimeDirFiles(); //updates U, p, epsilon, and k files for new timesteps (meshes)
    void UpdateSimpleFoamControlDict();

    int latestTime; //latest time directory
    int simpleFoamEndTime; //set to last time directory

    int GetLatestTimeOnDisk();
    std::vector<std::string> GetTimeDirsOnDisk();
    std::vector<std::string> GetProcessorDirsOnDisk();
    bool StringIsNumeric(const std::string &str);
    double GetFirstCellHeightFromDisk(int time);
    bool CheckForValidCaseDir(const char* dir);
    bool CheckForValidDem();
    void SetMeshResolutionAndResampleDem();

    void WriteNinjaLog();
    void ReadNinjaLog();
    
    /* OpenFOAM utilities */
    void RefineSurfaceLayer();
    void MoveDynamicMesh();
    void TopoSet();
    void RefineMesh();
    void BlockMesh();
    void DecomposePar();
    void ReconstructPar();
    void RenumberMesh();
    void ApplyInit();
    bool SimpleFoam();
    void Sample();
    void createMinZpatchStl();
    void createOutputSurfSampleStl();

    /* Output */
    bool SampleRawOutput();
    void WriteOutputFiles();
    void SetOutputResolution();
    void SetOutputFilenames();
    bool CheckIfOutputWindHeightIsResolved();

    bool writeMassMesh;
    Mesh massMesh;
    wn_3dScalarField massMesh_u, massMesh_v, massMesh_w;
    wn_3dScalarField massMesh_k;
    void GenerateAndSampleMassMesh();
    void generateMassMesh();
    void writeProbeSampleFile(const wn_3dArray& x, const wn_3dArray& y, const wn_3dArray& z, 
                              const double dem_xllCorner, const double dem_yllCorner, 
                              const int ncols, const int nrows, const int nlayers);
    void runProbeSample();
    void readInProbeData(const wn_3dArray& x, const wn_3dArray& y, const wn_3dArray& z, 
                         const double dem_xllCorner, const double dem_yllCorner, 
                         const int ncols, const int nrows, const int nlayers, 
                         wn_3dScalarField& u, wn_3dScalarField& v, wn_3dScalarField& w);
    void readInProbeData(const wn_3dArray& x, const wn_3dArray& y, const wn_3dArray& z, 
                         const double dem_xllCorner, const double dem_yllCorner, 
                         const int ncols, const int nrows, const int nlayers, 
                         wn_3dScalarField& k);
    void fillEmptyProbeVals(const wn_3dArray& z, 
                            const int ncols, const int nrows, const int nlayers, 
                            wn_3dScalarField& u, wn_3dScalarField& v, wn_3dScalarField& w);
    void fillEmptyProbeVals(const wn_3dArray& z, 
                            const int ncols, const int nrows, const int nlayers, 
                            wn_3dScalarField& k);

    void generateColMaxGrid(const wn_3dArray& z, 
                            const double dem_xllCorner, const double dem_yllCorner, 
                            const int ncols, const int nrows, const int nlayers, 
                            const double massMeshResolution, std::string prjString, 
                            wn_3dScalarField& k);

    void writeMassMeshVtkOutput();

    const char *pszVrtMem;
    const char *pszVrtMemTurbulence;
    const char *pszGridFilename;
    const char *pszTurbulenceGridFilename;
    
    /* Timers */
    double startTotal, endTotal;
    double startMesh, endMesh;
    double startInit, endInit;
    double startSolve, endSolve;
    double startWriteOut, endWriteOut;
    double startFoamFileWriting, endFoamFileWriting;
    double startOutputSampling, endOutputSampling;
    double startGenerateAndSampleMassMesh, endGenerateAndSampleMassMesh;
    double startStlConversion, endStlConversion;
    std::vector<double> startRestart, endRestart;

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

