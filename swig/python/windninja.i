%module windninja
%{
#include "windninja.h"
%}

%include <std_string.i>
%include <std_vector.i>
%include "windninja.h"

// Expose the NinjaH structure
struct NinjaH;
typedef struct NinjaH NinjaH;

// Expose the functions from the header file
%{
extern "C" {
    NinjaH** NinjaCreateHandle();
    NinjaH* NinjaCreateArmy(unsigned int numNinjas, char **papszOptions);
    NinjaErr NinjaFetchStation(std::string output_path, std::string elevation_file, std::vector<boost::posix_time::ptime> timeList, std::string osTimeZone, bool fetchLatest);
    NinjaErr NinjaFetchDEMBBox(double *boundsBox, const char *fileName, double resolution, char* fetchType);
    std::string NinjaFetchForecast(const char* wx_model_type, unsigned int forecastDuration, const char* elevation_file);
    NinjaErr NinjaDestroyArmy(NinjaH *ninja);
    NinjaErr NinjaStartRuns(NinjaH *ninja, const unsigned int nprocessors);
    NinjaH * NinjaMakeStationArmy( std::vector<boost::posix_time::ptime> timeList, std::string timeZone, std::string stationFileName, std::string elevationFile, bool matchPoints, int momentumFlag);
    NinjaH* NinjaMakeArmy(const char *forecastFilename, const char *timezone, int momentumFlag);
    NinjaErr NinjaSetEnvironment(const char *pszGdalData, const char *pszWindNinjaData);
    NinjaErr NinjaInit();
    NinjaErr NinjaSetDem(NinjaH *ninja, const int nIndex, const char *fileName);
    NinjaErr NinjaSetInMemoryDem(NinjaH *ninja, const int nIndex, const double *demValues, const int nXSize, const int nYSize, const double *geoRef, const char *prj);
    NinjaErr NinjaSetPosition(NinjaH *ninja, const int nIndex);
    NinjaErr NinjaSetInitializationMethod(NinjaH *ninja, const int nIndex, const char *initializationMethod);
    NinjaErr NinjaSetNumberCPUs(NinjaH *ninja, const int nIndex, const int nCPUs);
    NinjaErr NinjaSetCommunication(NinjaH *ninja, const int nIndex, const char *comType);
    NinjaErr NinjaSetInputSpeed(NinjaH *ninja, const int nIndex, const double speed, const char *units);
    NinjaErr NinjaSetInputDirection(NinjaH *ninja, const int nIndex, const double direction);
    NinjaErr NinjaSetInputWindHeight(NinjaH *ninja, const int nIndex, const double height, const char *units);
    NinjaErr NinjaSetOutputWindHeight(NinjaH *ninja, const int nIndex, const double height, const char *units);
    NinjaErr NinjaSetOutputSpeedUnits(NinjaH *ninja, const int nIndex, const char *units);
    NinjaErr NinjaSetDiurnalWinds(NinjaH *ninja, const int nIndex, const int flag);
    NinjaErr NinjaSetUniAirTemp(NinjaH *ninja, const int nIndex, const double temp, const char *units);
    NinjaErr NinjaSetUniCloudCover(NinjaH *ninja, const int nIndex, const double cloud_cover, const char *units);
    NinjaErr NinjaSetDateTime(NinjaH *ninja, const int nIndex, const int yr, const int mo, const int day, const int hr, const int min, const int sec, const char *timeZoneString);
    NinjaErr NinjaSetWxStationFilename(NinjaH *ninja, const int nIndex, const char *station_filename);
    NinjaErr NinjaSetUniVegetation(NinjaH *ninja, const int nIndex, const char *vegetation);
    NinjaErr NinjaSetNumVertLayers(NinjaH *ninja, const int nIndex, const int nLayers);
    int NinjaGetDiurnalWindFlag(NinjaH *ninja, const int nIndex);
    const char* NinjaGetInitializationMethod(NinjaH *ninja, const int nIndex);
    NinjaErr NinjaSetDustFilename(NinjaH *ninja, const int nIndex, const char* filename);
    NinjaErr NinjaSetDustFileOut(NinjaH *ninja, const int nIndex, const char* filename);
    NinjaErr NinjaSetDustFlag(NinjaH *ninja, const int nIndex, const int flag);
    NinjaErr NinjaSetStabilityFlag(NinjaH *ninja, const int nIndex, const int flag);
    NinjaErr NinjaSetAlphaStability(NinjaH *ninja, const int nIndex, const double stability_);
    NinjaErr NinjaSetMeshCount(NinjaH *ninja, const int nIndex, const int meshCount);
    NinjaErr NinjaSetMeshResolutionChoice(NinjaH *ninja, const int nIndex, const char *choice);
    NinjaErr NinjaSetMeshResolution(NinjaH *ninja, const int nIndex, const double resolution, const char *units);
    NinjaErr NinjaSetNumVertLayers(NinjaH *ninja, const int nIndex, int vertLayers);
    NinjaErr NinjaSetOutputPath(NinjaH *ninja, const int nIndex, const char *path);
    const double* NinjaGetOutputSpeedGrid(NinjaH *ninja, const int nIndex, double resolution, lengthUnits::eLengthUnits units);
    const double* NinjaGetOutputDirectionGrid(NinjaH *ninja, const int nIndex, double resolution, lengthUnits::eLengthUnits units);
    const char* NinjaGetOutputGridProjection(NinjaH *ninja, const int nIndex);
    const double NinjaGetOutputGridCellSize(NinjaH *ninja, const int nIndex);
    const double NinjaGetOutputGridxllCorner(NinjaH *ninja, const int nIndex);
    const double NinjaGetOutputGridyllCorner(NinjaH *ninja, const int nIndex);
    const int NinjaGetOutputGridnCols(NinjaH *ninja, const int nIndex);
    const int NinjaGetOutputGridnRows(NinjaH *ninja, const int nIndex);
    const double* NinjaGetu(NinjaH *ninja, const int nIndex);
    const double* NinjaGetv(NinjaH *ninja, const int nIndex);
    const double* NinjaGetw(NinjaH *ninja, const int nIndex);
    NinjaErr NinjaSetOutputBufferClipping(NinjaH *ninja, const int nIndex, const double percent);
    NinjaErr NinjaSetWxModelGoogOutFlag(NinjaH *ninja, const int nIndex, const int flag);
    NinjaErr NinjaSetWxModelShpOutFlag(NinjaH *ninja, const int nIndex, const int flag);
    NinjaErr NinjaSetWxModelAsciiOutFlag(NinjaH *ninja, const int nIndex, const int flag);
    NinjaErr NinjaSetGoogOutFlag(NinjaH *ninja, const int nIndex, const int flag);
    NinjaErr NinjaSetGoogResolution(NinjaH *ninja, const int nIndex, const double resolution, const char *units);
    NinjaErr NinjaSetGoogSpeedScaling(NinjaH *ninja, const int nIndex, const char *scaling);
    NinjaErr NinjaSetGoogLineWidth(NinjaH *ninja, const int nIndex, const double width);
    NinjaErr NinjaSetShpOutFlag(NinjaH *ninja, const int nIndex, const int flag);
    NinjaErr NinjaSetShpResolution(NinjaH *ninja, const int nIndex, const double resolution, const char *units);
    NinjaErr NinjaSetAsciiOutFlag(NinjaH *ninja, const int nIndex, const int flag);
    NinjaErr NinjaSetAsciiResolution(NinjaH *ninja, const int nIndex, const double resolution, const char *units);
    NinjaErr NinjaSetVtkOutFlag(NinjaH *ninja, const int nIndex, const int flag);
    NinjaErr NinjaSetTxtOutFlag(NinjaH *ninja, const int nIndex, const int flag);
    const char* NinjaGetOutputPath(NinjaH *ninja, const int nIndex);
    NinjaErr NinjaReset(NinjaH *ninja);
    NinjaErr NinjaCancel(NinjaH *ninja);
    NinjaErr NinjaCancelAndReset(NinjaH *ninja);
}
%}
