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

#define NINJA_FOAM_OGR_VRT "<OGRVRTDataSource>" \
                           "  <OGRVRTLayer name=\"%s\">" \
                           "    <SrcDataSource>%s</SrcDataSource>" \
                           "    <SrcLayer>%s</SrcLayer>" \
                           "    <GeometryType>wkbPoint</GeometryType>" \
                           "    <LayerSRS>%s</LayerSRS>" \
                           "    <GeometryField encoding=\"PointFromColumns\" x=\"x\" y=\"y\"/>"  \
                           "  </OGRVRTLayer>" \
                           "</OGRVRTDataSource>"

#define NINJA_FOAM_JSON    "{ " \
                           "   \"MeshInput\": { " \
                           "     \"terrain\": \"%s\", " \
                           "     \"Available Terrains\" : \"hellscanyon,crazyhorse,bigbutte,salmonriver\" " \
                           "   }," \
                           "   \"General\": { " \
                           "     \"DataPath\": \"%s\"," \
                           "     \"nProcs\":\"4\"" \
                           "   }, " \
                           "   \"SolverInputs\": { " \
                           "     \"Rd\": 15, " \
                           "     \"direction\": \"(0 1 0)\", " \
                           "     \"U_freestream\": \"9\", " \
                           "     \"InputWindHeight\": \"35\", " \
                           "     \"z0\": \"10\", " \
                           "     \"inlets\": " \
                           "         [ \"%s\"], " \
                           "     \"BCs\": [ " \
                           "       \"east_face\", " \
                           "       \"west_face\", " \
                           "       \"north_face\", " \
                           "       \"south_face\" " \
                           "     ] " \
                           "    }, " \
                           "    \"PostInputs\": {\"ter\": \"crazyhorse_up.stl\"} " \
                           "}"

#define NINJA_FOAM_NORTH    0x01
#define NINJA_FOAM_EAST     0x02
#define NINJA_FOAM_SOUTH    0x04
#define NINJA_FOAM_WEST     0x08

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

    GDALDatasetH GetRasterOutputHandle();

private:
    const char *pszTerrainFile;

    std::vector<double> direction; //input.inputDirection converted to unit vector notation
    std::vector<std::string> inlets; // e.g., north_face
    std::vector<std::string> bcs;
    
    OGRDataSourceH hOFOutput;
    GDALDatasetH hGriddedDS;

    const char *pszTempPath;
    const char *pszOgrBase;
    const char *pszOutFile;
    int GenerateTempDirectory();
    int WriteOgrVrt( const char *pszSrsWkt );
    int RunGridSampling();
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
    int writeBlockMesh();
    int readLogFile(int &ratio);
    int writeSnappyMesh();
    
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
    std::vector<int> nCells;
    double side1; // length of side of regular hex cell in zone1
    
    int ReplaceKey(std::string &s, std::string k, std::string v);
    int ReplaceKeys(std::string &s, std::string k, std::string v);
    
    int SurfaceTransformPoints();
    int SurfaceCheck();
    int BlockMesh();
    int DecomposePar(VSILFILE *fout);
    int SnappyHexMesh();
    int ReconstructPar(const char *const arg, VSILFILE *fout);
    int RenumberMesh();
    int CheckMesh();
    int ApplyInit();
    int SimpleFoam();
    int Sample();

};

#endif /* NINJA_FOAM_INCLUDED_ */

