/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class that stores all inputs to run WindNinja
 * Author:   Jason Forthofer <jforthofer@gmail.com>
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

#include "WindNinjaInputs.h"

WindNinjaInputs::WindNinjaInputs()
: ninjaTime(boost::local_time::not_a_date_time)
{
    //Initialize variables
    hSpdMemDs = NULL;
    hDirMemDs = NULL;
    hDustMemDs = NULL;
    armySize = 1;
    vegetation = WindNinjaInputs::trees;
    initializationMethod = WindNinjaInputs::noInitializationFlag;
    inputSpeedUnits = velocityUnits::milesPerHour;
    outputSpeedUnits = velocityUnits::milesPerHour;
    inputSpeed = -1.0;
    inputDirection = -1.0;
    inputWindHeightUnits = lengthUnits::meters;
    inputWindHeight = -1.0;
    outputWindHeightUnits = lengthUnits::meters;
    outputWindHeight = -1.0;
    matchWxStations = false;
    outer_relax = atof(CPLGetConfigOption("NINJA_POINT_MATCH_OUT_RELAX", "1.0"));
    CPLDebug("NINJA", "Setting NINJA_POINT_MATCH_OUT_RELAX to %lf", outer_relax);
    //outer_relax = 0.01;
    diurnalWinds = false;
    airTempUnits = temperatureUnits::F;
    airTemp = -10000;
    cloudCoverUnits = coverUnits::percent;
    cloudCover = -1.0;
    latitude = -10000.0;
    longitude = -10000.0;
    numberCPUs = 1;
    outputBufferClipping = 0.0;
    googOutFlag = false;
    writeAtmFile = false;
    googSpeedScaling = KmlVector::equal_interval;
    googLineWidth = 1.0;
    wxModelGoogOutFlag = false;
    wxModelGoogSpeedScaling = KmlVector::equal_interval;
    wxModelGoogLineWidth = 3.0;
    shpOutFlag = false;
    asciiOutFlag = false;
    wxModelShpOutFlag = false;
    wxModelAsciiOutFlag = false;
    txtOutFlag = false;
    volVTKOutFlag = false;
    kmlFile = "!set";
    kmzFile = "!set";
    wxModelKmlFile = "!set";
    wxModelKmzFile = "!set";
    kmzResolution = -1.0;
    kmzUnits = lengthUnits::meters;
    shpFile = "!set";
    dbfFile = "!set";
    wxModelShpFile = "!set";
    wxModelDbfFile = "!set";
    shpResolution = -1.0;
    shpUnits = lengthUnits::meters;
    cldFile = "!set";
    velFile = "!set";
    wxModelCldFile = "!set";
    wxModelVelFile = "!set";
    velResolution = -1.0;
    velOutputFileDistanceUnits = lengthUnits::meters;
    angFile = "!set";
    atmFile = "!set";
    wxModelAngFile = "!set";
    angResolution = -1.0;
    angOutputFileDistanceUnits = lengthUnits::meters;
    legFile = "!set";
    dateTimeLegFile = "!set";
    volVTKFile = "!set";
    pdfOutFlag = false;
    pdfDEMFileName = "!set";
    pdfResolution = -1.0;
    pdfLineWidth  = 1.0;
    pdfUnits = lengthUnits::meters;
    pdfFile = "!set";
    pdfBaseType = TOPOFIRE;
    pdfWidth = 8.5;
    pdfHeight = 11.0;
    pdfDPI = 150;
    keepOutGridsInMemory = false;
    customOutputPath = "!set";
#ifdef NINJA_SPEED_TESTING
    speedDampeningRatio = 1;
#endif
    downDragCoeff = 0.0001;
    downEntrainmentCoeff = 0.01;
    upDragCoeff = 0.2;
    upEntrainmentCoeff = 0.2;
#ifdef STABILITY
    stabilityFlag = false;
    alphaStability = -1;
#endif
#ifdef FRICTION_VELOCITY
    frictionVelocityFlag = false; 
    frictionVelocityCalculationMethod = "!set";
    ustarFile = "!set";
#endif
#ifdef EMISSIONS
    dustFilename = "!set";
    dustFileOut = "!set";
    dustFlag = false;
    dustFile = "!set";
    ustarFile = "!set";
    geotiffOutFlag = false;
#endif
#ifdef NINJAFOAM
    nIterations = 1000;
    meshCount = -1;
    ninjafoamMeshChoice = WindNinjaInputs::fine;
    nonEqBc = true;
    existingCaseDirectory = "!set";
    stlFile = "!set";
    speedInitGridFilename = "!set";
    dirInitGridFilename= "!set";
#endif
    
    outputPointsFilename = "!set";
    inputPointsFilename = "!set";

    outputPath = "!set";

#ifdef _OPENMP
    omp_set_nested(false);
    omp_set_dynamic(false);
#endif //_OPENMP

}

WindNinjaInputs::~WindNinjaInputs()
{
}

/**
 * Copy constructor.
 * @param rhs WindNinjaInputs object to be copied.
 */
WindNinjaInputs::WindNinjaInputs(const WindNinjaInputs &rhs)
: ninjaTime(boost::local_time::not_a_date_time)
{
  armySize = rhs.armySize;
  hSpdMemDs = rhs.hSpdMemDs;
  hDirMemDs = rhs.hDirMemDs;
  hDustMemDs = rhs.hDustMemDs;
  
  vegetation = rhs.vegetation;

  initializationMethod = rhs.initializationMethod;
  forecastFilename = rhs.forecastFilename;
  inputSpeedUnits = rhs.inputSpeedUnits;
  outputSpeedUnits = rhs.outputSpeedUnits;
  inputSpeed = rhs.inputSpeed;
  inputDirection = rhs.inputDirection;
  inputWindHeightUnits = rhs.inputWindHeightUnits;
  inputWindHeight = rhs.inputWindHeight;
  outputWindHeightUnits = rhs.outputWindHeightUnits;
  outputWindHeight = rhs.outputWindHeight;
  stations = rhs.stations;
  wxStationFilename = rhs.wxStationFilename;
  stationsScratch = rhs.stationsScratch;
  stationsOldInput = rhs.stationsOldInput;
  stationsOldOutput = rhs.stationsOldOutput;
  matchWxStations = rhs.matchWxStations;
  outer_relax = rhs.outer_relax;
  CPLDebug("NINJA", "Setting NINJA_POINT_MATCH_OUT_RELAX to %lf", outer_relax);
  diurnalWinds = rhs.diurnalWinds;
#ifdef NINJAFOAM
    nIterations = rhs.nIterations;
    meshCount = rhs.meshCount;
    ninjafoamMeshChoice = rhs.ninjafoamMeshChoice;
    nonEqBc = rhs.nonEqBc;
    stlFile = rhs.stlFile;
#endif
  outputPointsFilename = rhs.outputPointsFilename;
  inputPointsFilename = rhs.inputPointsFilename;
  pointsNamesList = rhs.pointsNamesList;
  latList = rhs.latList;
  lonList = rhs.lonList;
  heightList = rhs.heightList;

  ninjaTime = rhs.ninjaTime;
  if(rhs.ninjaTimeZone.get() == NULL)   //If pointer is NULL
      ninjaTimeZone.reset();
  else if(rhs.ninjaTimeZone->to_posix_string().empty()) //If pointer is good, but posix string is empty
      ninjaTimeZone.reset();
  else  //Else it is a good time zone pointer, so set accordingly
      ninjaTimeZone = boost::local_time::time_zone_ptr (new boost::local_time::posix_time_zone(rhs.ninjaTimeZone->to_posix_string()));

  airTemp = rhs.airTemp;
  cloudCover = rhs.cloudCover;
  latitude = rhs.latitude;
  longitude = rhs.longitude;
  numberCPUs = rhs.numberCPUs;
  outputBufferClipping = rhs.outputBufferClipping;
  writeAtmFile = rhs.writeAtmFile;
  googOutFlag = rhs.googOutFlag;
  googSpeedScaling = rhs.googSpeedScaling;
  googLineWidth = rhs.googLineWidth;
  wxModelGoogOutFlag = rhs.wxModelGoogOutFlag;
  wxModelGoogSpeedScaling = rhs.wxModelGoogSpeedScaling;
  wxModelGoogLineWidth = rhs.wxModelGoogLineWidth;
  shpOutFlag = rhs.shpOutFlag;
  pdfOutFlag = rhs.pdfOutFlag;
  pdfDEMFileName = rhs.pdfDEMFileName;
  pdfResolution = rhs.pdfResolution;
  pdfLineWidth  = rhs.pdfLineWidth ;
  pdfUnits = rhs.pdfUnits;
  pdfFile = rhs.pdfFile;
  pdfBaseType = rhs.pdfBaseType;
  pdfWidth = rhs.pdfWidth;
  pdfHeight = rhs.pdfHeight;
  pdfDPI = rhs.pdfDPI;
  asciiOutFlag = rhs.asciiOutFlag;
  wxModelShpOutFlag = rhs.wxModelShpOutFlag;
  wxModelAsciiOutFlag = rhs.wxModelAsciiOutFlag;
  txtOutFlag = rhs.txtOutFlag;
  volVTKOutFlag = rhs.volVTKOutFlag;
  kmlFile = rhs.kmlFile;
  kmzFile = rhs.kmzFile;
  wxModelKmlFile = rhs.wxModelKmlFile;
  wxModelKmzFile = rhs.wxModelKmzFile;
  kmzResolution = rhs.kmzResolution;
  kmzUnits = rhs.kmzUnits;
  shpFile = rhs.shpFile;
  dbfFile = rhs.dbfFile;
  wxModelShpFile = rhs.wxModelShpFile;
  wxModelDbfFile = rhs.wxModelDbfFile;
  shpResolution = rhs.shpResolution;
  shpUnits = rhs.shpUnits;


  cldFile = rhs.cldFile;
  velFile = rhs.velFile;
  wxModelCldFile = rhs.wxModelCldFile;
  wxModelVelFile = rhs.wxModelVelFile;
  velResolution = rhs.velResolution;
  velOutputFileDistanceUnits = rhs.velOutputFileDistanceUnits;
  angFile = rhs.angFile;
  atmFile = rhs.atmFile;
  wxModelAngFile = rhs.wxModelAngFile;
  angResolution = rhs.angResolution;
  angOutputFileDistanceUnits = rhs.angOutputFileDistanceUnits;
  legFile = rhs.legFile;
  dateTimeLegFile = rhs.dateTimeLegFile;
  volVTKFile = rhs.volVTKFile;
  keepOutGridsInMemory = rhs.keepOutGridsInMemory;
  customOutputPath = rhs.customOutputPath;
  
#ifdef NINJA_SPEED_TESTING
  speedDampeningRatio = rhs.speedDampeningRatio;
#endif
  
  downDragCoeff = rhs.downDragCoeff;
  downEntrainmentCoeff = rhs.downDragCoeff;
  upDragCoeff = rhs.upDragCoeff;
  upEntrainmentCoeff = rhs.upEntrainmentCoeff;
#ifdef FRICTION_VELOCITY
  frictionVelocityFlag = rhs.frictionVelocityFlag; 
  frictionVelocityCalculationMethod = rhs.frictionVelocityCalculationMethod;
  ustarFile = rhs.ustarFile;
#endif
#ifdef EMISSIONS
  dustFlag = rhs.dustFlag;
  dustFilename = rhs.dustFilename;
  dustFileOut = rhs.dustFileOut;
  geotiffOutFlag = rhs.geotiffOutFlag;
  dustFile = rhs.dustFile;
  ustarFile = rhs.ustarFile;
#endif
  
#ifdef STABILITY
  stabilityFlag = rhs.stabilityFlag;
  alphaStability = rhs.alphaStability;
#endif
  
  outputPath = rhs.outputPath;

  //class crap
  dem = rhs.dem;
  surface = rhs.surface;

#ifdef _OPENMP
  omp_set_nested(false);
  omp_set_dynamic(false);
#endif //_OPENMP

}

/**
 * Comparison operator.
 * For use in detecting duplicate runs. Only a subset
 * of inputs (those necessary to ID duplicate runs from
 * the GUI) are checked.
 * @param rhs WindNinjaInputs object to compare with.
 * @return true if objects are equal, otherwise false.
 */
bool WindNinjaInputs::operator==(const WindNinjaInputs &rhs)
{
    if( inputSpeed == rhs.inputSpeed &&
        inputSpeedUnits == rhs.inputSpeedUnits &&
        inputDirection == rhs.inputDirection &&
        airTemp == rhs.airTemp &&
        airTempUnits == rhs.airTempUnits &&
        cloudCover == rhs.cloudCover &&
        cloudCoverUnits == rhs.cloudCoverUnits &&
        ninjaTime == rhs.ninjaTime &&
        diurnalWinds == rhs.diurnalWinds)
    {
        return true;
    }
    else{
        return false;
    }
}

/**
 * Equals operator.
 * @param rhs WindNinjaInputs object to set equal to.
 * @return A referece to a copied rhs.
 */
WindNinjaInputs &WindNinjaInputs::operator=(const WindNinjaInputs &rhs)
{
  if(&rhs != this)
    {
      //ninjaCom stuff
      Com = NULL;   //must be set to null!
      armySize = rhs.armySize;
      hSpdMemDs = rhs.hSpdMemDs;
      hDirMemDs = rhs.hDirMemDs;
      hDustMemDs = rhs.hDustMemDs;
      
      vegetation = rhs.vegetation;

      initializationMethod = rhs.initializationMethod;
      forecastFilename = rhs.forecastFilename;
      inputSpeedUnits = rhs.inputSpeedUnits;
      outputSpeedUnits = rhs.outputSpeedUnits;
      inputSpeed = rhs.inputSpeed;
      inputDirection = rhs.inputDirection;
      inputWindHeightUnits = rhs.inputWindHeightUnits;
      inputWindHeight = rhs.inputWindHeight;
      outputWindHeightUnits = rhs.outputWindHeightUnits;
      outputWindHeight = rhs.outputWindHeight;
      stations = rhs.stations;
      wxStationFilename = rhs.wxStationFilename;
      stationsScratch = rhs.stationsScratch;
      stationsOldInput = rhs.stationsOldInput;
      stationsOldOutput = rhs.stationsOldOutput;
      matchWxStations = rhs.matchWxStations;
      outer_relax = rhs.outer_relax;
      CPLDebug("NINJA", "Setting NINJA_POINT_MATCH_OUT_RELAX to %lf", outer_relax);
      diurnalWinds = rhs.diurnalWinds;
#ifdef EMISSIONS
      dustFlag = rhs.dustFlag;
      dustFilename = rhs.dustFilename;
      dustFileOut = rhs.dustFileOut;
      geotiffOutFlag = rhs.geotiffOutFlag;
      dustFile = rhs.dustFile;
      ustarFile = rhs.ustarFile;
#endif
#ifdef NINJAFOAM
      nIterations = rhs.nIterations;
      meshCount = rhs.meshCount;
      ninjafoamMeshChoice = rhs.ninjafoamMeshChoice;
      nonEqBc = rhs.nonEqBc;
      stlFile = rhs.stlFile;
#endif
      outputPointsFilename = rhs.outputPointsFilename;
      inputPointsFilename = rhs.inputPointsFilename;
      pointsNamesList = rhs.pointsNamesList;
      latList = rhs.latList;
      lonList = rhs.lonList;
      heightList = rhs.heightList;

      ninjaTime = rhs.ninjaTime;
      if(rhs.ninjaTimeZone.get() == NULL)   //If pointer is NULL
          ninjaTimeZone.reset();
      else if(rhs.ninjaTimeZone->to_posix_string().empty()) //If pointer is good, but posix string is empty
          ninjaTimeZone.reset();
      else  //Else it is a good time zone pointer, so set accordingly
          ninjaTimeZone = boost::local_time::time_zone_ptr (new boost::local_time::posix_time_zone(rhs.ninjaTimeZone->to_posix_string()));

      airTemp = rhs.airTemp;
      cloudCover = rhs.cloudCover;
      latitude = rhs.latitude;
      longitude = rhs.longitude;
      numberCPUs = rhs.numberCPUs;
      outputBufferClipping = rhs.outputBufferClipping;
      writeAtmFile = rhs.writeAtmFile;
      googOutFlag = rhs.googOutFlag;
      googSpeedScaling = rhs.googSpeedScaling;
      googLineWidth = rhs.googLineWidth;
      wxModelGoogOutFlag = rhs.wxModelGoogOutFlag;
      wxModelGoogSpeedScaling = rhs.wxModelGoogSpeedScaling;
      wxModelGoogLineWidth = rhs.wxModelGoogLineWidth;
      shpOutFlag = rhs.shpOutFlag;
      asciiOutFlag = rhs.asciiOutFlag;
      wxModelShpOutFlag = rhs.wxModelShpOutFlag;
      wxModelAsciiOutFlag = rhs.wxModelAsciiOutFlag;
      txtOutFlag = rhs.txtOutFlag;
      volVTKOutFlag = rhs.volVTKOutFlag;
      kmlFile = rhs.kmlFile;
      kmzFile = rhs.kmzFile;
      wxModelKmlFile = rhs.wxModelKmlFile;
      wxModelKmzFile = rhs.wxModelKmzFile;
      kmzResolution = rhs.kmzResolution;
      kmzUnits = rhs.kmzUnits;
      shpFile = rhs.shpFile;
      dbfFile = rhs.dbfFile;
      wxModelShpFile = rhs.wxModelShpFile;
      wxModelDbfFile = rhs.wxModelDbfFile;
      shpResolution = rhs.shpResolution;
      shpUnits = rhs.shpUnits;
      pdfOutFlag = rhs.pdfOutFlag;
      pdfDEMFileName = rhs.pdfDEMFileName;
      pdfResolution = rhs.pdfResolution;
      pdfLineWidth  = rhs.pdfLineWidth ;
      pdfUnits = rhs.pdfUnits;
      pdfFile = rhs.pdfFile;
      pdfBaseType = rhs.pdfBaseType;
      pdfWidth = rhs.pdfWidth;
      pdfHeight = rhs.pdfHeight;
      pdfDPI = rhs.pdfDPI;
      cldFile = rhs.cldFile;
      velFile = rhs.velFile;
      wxModelCldFile = rhs.wxModelCldFile;
      wxModelVelFile = rhs.wxModelVelFile;
      velResolution = rhs.velResolution;
      velOutputFileDistanceUnits = rhs.velOutputFileDistanceUnits;
      angFile = rhs.angFile;
      atmFile = rhs.atmFile;
      wxModelAngFile = rhs.wxModelAngFile;
      angResolution = rhs.angResolution;
      angOutputFileDistanceUnits = rhs.angOutputFileDistanceUnits;
      legFile = rhs.legFile;
      dateTimeLegFile = rhs.dateTimeLegFile;
      volVTKFile = rhs.volVTKFile;
      keepOutGridsInMemory = rhs.keepOutGridsInMemory;
      customOutputPath = rhs.customOutputPath;
      
#ifdef NINJA_SPEED_TESTING
      speedDampeningRatio = rhs.speedDampeningRatio;
#endif

      downDragCoeff = rhs.downDragCoeff;
      downEntrainmentCoeff = rhs.downDragCoeff;
      upDragCoeff = rhs.upDragCoeff;
      upEntrainmentCoeff = rhs.upEntrainmentCoeff;
      
#ifdef STABILITY
      stabilityFlag = rhs.stabilityFlag;
      alphaStability = rhs.alphaStability;
#endif
#ifdef FRICTION_VELOCITY
      frictionVelocityFlag = rhs.frictionVelocityFlag; 
      frictionVelocityCalculationMethod = rhs.frictionVelocityCalculationMethod;
      ustarFile = rhs.ustarFile;
#endif

      outputPath = rhs.outputPath;

      //class crap
      dem = rhs.dem;
      surface = rhs.surface;

#ifdef _OPENMP
      omp_set_nested(false);
      omp_set_dynamic(false);
#endif //_OPENMP

    }
  return *this;
}


