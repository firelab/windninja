/******************************************************************************
*
* $Id$
*
* Project:  WindNinja
* Purpose:  Handle GCP fetching for weather model data
* Author:   Mason Willman <mason.willman@usda.gov>
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

#include "gcp_wx_init.h"

GCPWxModel::GCPWxModel(GCPWxModel const&A) : wxModelInitialization(A) {

}


GCPWxModel::GCPWxModel()
{
  ppszModelData = NULL;
  pfnProgress = NULL;
}


GCPWxModel::GCPWxModel( std::string filename)
{
  wxModelFileName = filename;
  pfnProgress = NULL;
  ppszModelData = NULL;
}

GCPWxModel::~GCPWxModel()
{

}

/**
 *@brief Returns horizontal grid resolution of the model
 *@return return 3
 */
double GCPWxModel::getGridResolution()
{
  return 3;
}

std::string GCPWxModel::getForecastReadable()
{
  return "PAST-CAST-GCP-HRRR-CONUS-3KM";
}

std::string GCPWxModel::getForecastIdentifier()
{
  return "PAST-CAST-GCP-HRRR-CONUS-3KM";
}

bool GCPWxModel::identify( std::string fileName )
{

  if (fileName.find("PAST-CAST-GCP-HRRR-CONUS-3KM") != std::string::npos)
  {
    return true;
  }
  else
  {
    return false;
  }
}

std::vector<blt::local_date_time>
GCPWxModel::getTimeList(const char *pszVariable, blt::time_zone_ptr timeZonePtr)
{

  if (aoCachedTimes.size() > 0)
    return aoCachedTimes;

  if (wxModelFileName.empty())
  {
    throw badForecastFile("ZIP file path is empty");
  }

         // Point to the ZIP file using GDAL VSI
  std::string vsiZipPath = "/vsizip/" + wxModelFileName;

         // Read contents of the ZIP archive
  char **papszFileList = VSIReadDir(vsiZipPath.c_str());
  if (!papszFileList || CSLCount(papszFileList) == 0)
  {
    throw badForecastFile("No files found in ZIP archive");
  }

  std::vector<blt::local_date_time> aoTimeList;
  char **papszTimeList = NULL;

  for (int i = 0; papszFileList[i] != NULL; ++i)
  {
    SKIP_DOT_AND_DOTDOT(papszFileList[i]);

    std::string filePath = vsiZipPath + "/" + std::string(papszFileList[i]);
    GDALDatasetH hDS = GDALOpen(filePath.c_str(), GA_ReadOnly);
    if (!hDS)
    {
      CPLDebug("GCP", "Failed to open GRIB file: %s", filePath.c_str());
      continue;
    }

    int nBandCount = GDALGetRasterCount(hDS);
    for (int j = 0; j < nBandCount; ++j)
    {
      GDALRasterBandH hBand = GDALGetRasterBand(hDS, j + 1);
      const char *pszValidTime = GDALGetMetadataItem(hBand, "GRIB_VALID_TIME", NULL);
      if (pszValidTime && CSLFindString(papszTimeList, pszValidTime) == -1)
      {
        papszTimeList = CSLAddString(papszTimeList, pszValidTime);
        CPLDebug("GCP", "Found valid time: %s", pszValidTime);
      }
    }

    GDALClose(hDS);
  }

  // Convert time strings to local_date_time
  for (int i = 0; i < CSLCount(papszTimeList); ++i)
  {
    time_t nValidTime = static_cast<time_t>(atoi(papszTimeList[i]));

    bpt::ptime epoch(boost::gregorian::date(1970, 1, 1));
    bpt::ptime utc_time = epoch + bpt::seconds(nValidTime);
    blt::local_date_time local_time(utc_time, timeZonePtr);
    aoTimeList.push_back(local_time);
  }

  CSLDestroy(papszFileList);
  CSLDestroy(papszTimeList);

  aoCachedTimes = aoTimeList;
  return aoCachedTimes;
}


std::string GCPWxModel::fetchForecast(std::string demFile, int nhours)
{
  if (pfnProgress) {
    pfnProgress(0.0, "Downloading files...", nullptr);
  }

  GDALDatasetH hDS = GDALOpen(demFile.c_str(), GA_ReadOnly);
  double demBounds[4];
  if (!GDALGetBounds((GDALDataset *)hDS, demBounds)) {
    throw badForecastFile("Could not download weather forecast, invalid projection for the DEM");
  }

  double adfNESW[4];
  ComputeWxModelBuffer((GDALDataset *)hDS, adfNESW);
  GDALClose(hDS);

  std::string buffer[] = {
      boost::lexical_cast<std::string>(adfNESW[3]),
      boost::lexical_cast<std::string>(adfNESW[0]),
      boost::lexical_cast<std::string>(adfNESW[1]),
      boost::lexical_cast<std::string>(adfNESW[2])
  };

  privateKey = CPLGetConfigOption("GS_OAUTH2_PRIVATE_KEY_FILE", NULL);
  clientEmail = CPLGetConfigOption("GS_OAUTH2_CLIENT_EMAIL", NULL);

  boost::posix_time::ptime startDateTime(startDate, boost::posix_time::duration_from_string(starthours + ":00:00"));
  boost::posix_time::ptime endDateTime(endDate, boost::posix_time::duration_from_string(endhours + ":00:00"));
  std::string path(CPLGetDirname(demFile.c_str()));
  std::string fileName(CPLGetFilename(demFile.c_str()));
  std::string startDateStr = boost::gregorian::to_iso_string(startDateTime.date());

  std::string outFolder = path + "/" + getForecastReadable() + "-" + fileName + "/" + startDateStr + "T" + starthours + "00/";
  VSIMkdirRecursive(outFolder.c_str(), 0777);

  std::string tmp = outFolder + "/tmp/";
  VSIMkdirRecursive(tmp.c_str(), 0777);

  std::vector<std::string> fileBands;
  std::vector<std::string> variables = getVariableList();
  std::vector<boost::posix_time::ptime> validTimes;

  for (boost::posix_time::ptime dt = startDateTime; dt <= endDateTime; dt += boost::posix_time::hours(1))
  {
    std::string dateStr = boost::gregorian::to_iso_string(dt.date());
    std::stringstream hourSS;
    hourSS << std::setw(2) << std::setfill('0') << dt.time_of_day().hours();
    std::string hourStr = hourSS.str();

    std::string idxUrl = "https://storage.googleapis.com/high-resolution-rapid-refresh/hrrr." + dateStr +
                         "/conus/hrrr.t" + hourStr + "z.wrfsfcf00.grib2.idx";
    std::string localIdxPath = tmp + "hrrr." + dateStr + "t" + hourStr + "z.idx";

    VSILFILE *fpRemote = VSIFOpenL(("/vsicurl/" + idxUrl).c_str(), "r");
    if (!fpRemote) {
      CPLDebug("IDXCache", "Failed to open remote idx file: %s", idxUrl.c_str());
      continue;
    }

    VSILFILE *fpLocal = VSIFOpenL(localIdxPath.c_str(), "w");
    if (!fpLocal) {
      CPLDebug("IDXCache", "Failed to create local idx file: %s", localIdxPath.c_str());
      VSIFCloseL(fpRemote);
      continue;
    }

    char buf[8192];
    size_t nRead = 0;
    while ((nRead = VSIFReadL(buf, 1, sizeof(buf), fpRemote)) > 0)
      VSIFWriteL(buf, 1, nRead, fpLocal);

    VSIFCloseL(fpRemote);
    VSIFCloseL(fpLocal);

    std::string bands = findBands(localIdxPath, variables);
    fileBands.push_back(bands);
    validTimes.push_back(dt);

    VSIUnlink(localIdxPath.c_str());
  }

  std::vector<std::vector<const char*>> options = getOptions(fileBands, buffer);
  const int MAX_CONCURRENT = 4;
  std::vector<void*> threadHandles;

  int i = 0;
  for (size_t dt = 0; dt < validTimes.size(); ++dt)
  {
    if ((int)threadHandles.size() >= MAX_CONCURRENT)
    {
      CPLJoinThread(threadHandles.front());
      threadHandles.erase(threadHandles.begin());
    }

    ThreadParams* params = new ThreadParams{validTimes[dt], tmp, options, i};

    void* handle = CPLCreateJoinableThread(ThreadFunc, params);
    if (!handle) {
      CPLDebug("GCP", "Failed to create thread");
      delete params;
      break;
    }

    threadHandles.push_back(handle);
    i++;
  }

  for (auto& handle : threadHandles)
    CPLJoinThread(handle);

  threadHandles.clear();

  std::string zipFolder = outFolder;
  std::string zipFilePath = zipFolder + startDateStr + "T" + starthours + "00" + ".zip";
  std::string zipVirtualPath = "/vsizip/" + zipFilePath;

  char** fileList = VSIReadDir(tmp.c_str());
  if (!fileList) {
    throw std::runtime_error("Failed to read temporary directory: " + tmp);
  }

  for (int i = 0; fileList[i] != nullptr; ++i) {
    std::string fileNameOnly = fileList[i];
    if (fileNameOnly == "." || fileNameOnly == "..") continue;

    std::string filePath = tmp + fileNameOnly;
    std::string zipEntryPath = zipVirtualPath + "/" + fileNameOnly;

    VSILFILE* fpSrc = VSIFOpenL(filePath.c_str(), "rb");
    if (!fpSrc) {
      CPLDebug("GCP", "Failed to open source file for zipping: %s", filePath.c_str());
      continue;
    }

    VSIFSeekL(fpSrc, 0, SEEK_END);
    vsi_l_offset fileSize = VSIFTellL(fpSrc);
    VSIFSeekL(fpSrc, 0, SEEK_SET);

    std::vector<char> buffer(fileSize);
    if (VSIFReadL(buffer.data(), 1, fileSize, fpSrc) != fileSize) {
      CPLDebug("GCP", "Failed to read complete file for zipping: %s", filePath.c_str());
      VSIFCloseL(fpSrc);
      continue;
    }
    VSIFCloseL(fpSrc);

    VSILFILE* fpZip = VSIFOpenL(zipEntryPath.c_str(), "wb");
    if (!fpZip) {
      CPLDebug("GCP", "Failed to create zip entry: %s", zipEntryPath.c_str());
      continue;
    }

    if (VSIFWriteL(buffer.data(), 1, fileSize, fpZip) != fileSize) {
      CPLDebug("GCP", "Failed to write data to zip entry: %s", zipEntryPath.c_str());
    }

    VSIFCloseL(fpZip);
  }
  VSIRmdirRecursive(tmp.c_str());
  CSLDestroy(fileList);

  CPLDebug("GCP", "Created zip archive at %s", zipFilePath.c_str());

  return zipFilePath;
}



// Thread function (must take void* and return void)
static void GCPWxModel::ThreadFunc(void* pData)
{
  ThreadParams* params = static_cast<ThreadParams*>(pData);
  boost::posix_time::ptime dt = params->dt;
  std::string outPath = params->outPath;
  int i = params->i;
  auto options = params->options;

  std::string dateStr = boost::gregorian::to_iso_string(dt.date());
  std::stringstream hourSS;
  hourSS << std::setw(2) << std::setfill('0') << dt.time_of_day().hours();
  std::string hourStr = hourSS.str();

  std::string srcFile = "/vsigs/high-resolution-rapid-refresh/hrrr." + dateStr +
                        "/conus/hrrr.t" + hourStr + "z.wrfsfcf00.grib2";
  std::string outFile = outPath + "hrrr." + dateStr + "t" + hourStr + "z." + "wrfsfcf00.grib2";

  GDALTranslateOptions *transOptions = GDALTranslateOptionsNew(options[i].data(), NULL);
  GDALDataset *srcDataset = (GDALDataset *)GDALOpen(srcFile.c_str(), GA_ReadOnly);
  if (!srcDataset)
  {
    CPLDebug("GCP", "Failed to open input dataset for %s", srcFile.c_str());
    GDALTranslateOptionsFree(transOptions);
    delete params;
    return;
  }

  GDALDataset *outDataset = GDALTranslate(outFile.c_str(), srcDataset, transOptions, NULL);
  GDALClose(srcDataset);
  GDALTranslateOptionsFree(transOptions);

  if (!outDataset)
  {
    CPLDebug("GCP", "GDALTranslate Failed for %s", outFile.c_str());
  }
  else
  {
    GDALClose(outDataset);
  }
  delete params;
}

std::vector<std::string> GCPWxModel::getVariableList()
{  
  std::vector<std::string> varList;
  varList.push_back( "TMP:2 m above ground" );
  varList.push_back( "UGRD:10 m above ground" );
  varList.push_back( "VGRD:10 m above ground" );
  varList.push_back( "TCDC:entire atmosphere" );
  return varList;
}

std::string GCPWxModel::findBands(std::string idxFilePath, std::vector<std::string> variables)
{
  std::ifstream idxFile(idxFilePath.c_str());
  if (!idxFile.is_open())
  {
    CPLDebug("IDXParse", "Failed to open cached .idx file: %s", idxFilePath.c_str());
    return "";
  }

  std::vector<int> bandSet;
  std::string line;
  while (std::getline(idxFile, line))
  {
    for (int i = 0; i < variables.size(); ++i)
    {
      const std::string& var = variables[i];
      if (line.find(var) != std::string::npos)
      {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
          std::string bandStr = line.substr(0, colonPos);
          std::istringstream iss(bandStr);
          int band;
          if (iss >> band)
          {
            bandSet.push_back(band);
            CPLDebug("IDXParse", "Field '%s' is at band %d", var.c_str(), band);
          }
          else
          {
            CPLDebug("IDXParse", "Could not parse band number in line: %s", line.c_str());
          }
        }
      }
    }
  }
  idxFile.close();

  std::stringstream bandOptions;
  for (int i = 0; i < bandSet.size(); ++i)
  {
    bandOptions << "-b " << bandSet[i] << " ";
  }
  return bandOptions.str();
}


std::vector<std::vector<const char*>> GCPWxModel::getOptions(const std::vector<std::string>& bands, const std::string buffer[4])
{
  std::vector<std::vector<const char*>> allOptions;

  for (size_t i = 0; i < bands.size(); ++i)
  {
    std::vector<std::string> strOptions;

           // Tokenize the band string (e.g., "-b 1 -b 2")
    std::istringstream iss(bands[i]);
    std::string token;
    while (iss >> token)
    {
      strOptions.push_back(token);
    }

           // Append projwin parameters
    strOptions.push_back("-projwin");
    strOptions.push_back(buffer[0]);
    strOptions.push_back(buffer[1]);
    strOptions.push_back(buffer[2]);
    strOptions.push_back(buffer[3]);

    strOptions.push_back("-projwin_srs");
    strOptions.push_back("EPSG:4326");

    strOptions.push_back("-of");
    strOptions.push_back("GRIB");

           // Convert std::string to char* safely
    std::vector<const char*> cstrArgs;
    for (size_t j = 0; j < strOptions.size(); ++j)
    {
      cstrArgs.push_back(const_cast<const char*>(strOptions[j].c_str()));
    }
    cstrArgs.push_back(nullptr); // Null-terminated for GDAL

    allOptions.push_back(cstrArgs); // Store each option list
  }

  return allOptions;
}

void GCPWxModel::setDateTime(boost::gregorian::date date1, boost::gregorian::date date2, std::string hours1, std::string hours2)
{
  startDate = date1;
  endDate = date2;
  starthours = hours1;
  endhours = hours2;
}

void GCPWxModel::setSurfaceGrids(WindNinjaInputs& input,
                                 AsciiGrid<double>& airGrid,
                                 AsciiGrid<double>& cloudGrid,
                                 AsciiGrid<double>& uGrid,
                                 AsciiGrid<double>& vGrid,
                                 AsciiGrid<double>& wGrid)
{
  std::vector<blt::local_date_time> timeList( getTimeList( NULL, input.ninjaTimeZone ) );
  if (timeList.empty()) {
    throw badForecastFile("No forecast time steps found.");
  }

  GDALDatasetH hSrcDS = nullptr, hVrtDS = nullptr;
  const char* pszSrcWkt = nullptr;
  const char* pszDstWkt = input.dem.prjString.c_str();
  GDALWarpOptions* psWarpOptions = nullptr;
  double dfNoData = -9999.0;
  bool bHaveTemp = false, bHaveCloud = false, bNeedNextCloud = false;

         // Extract date string: YYYYMMDD
  std::ostringstream dateStrStream;
  dateStrStream << boost::gregorian::to_iso_string(input.ninjaTime.date());
  std::string dateStr = dateStrStream.str();

         // Extract hour string: HH (zero-padded)
  std::ostringstream hourStrStream;
  hourStrStream << std::setw(2) << std::setfill('0') << input.ninjaTime.time_of_day().hours();
  std::string hourStr = hourStrStream.str();

         // Build the GRIB2 filename: "hrrr.t20z.wrfsubhf00.grib2"
  std::string grib2FileName = "hrrr." + dateStr + "t" + hourStr + "z.wrfsfcf00.grib2";

         // Combine into VSI path
  std::string vsiPath = "/vsizip/" + input.forecastFilename + "/" + grib2FileName;
  const char *pszForecastFile = vsiPath.c_str();
  if( !pszForecastFile )
  {
    throw badForecastFile( "Could not find forecast associated with " \
                          "requested time step" );
  }

  hSrcDS = GDALOpenShared(pszForecastFile, GA_ReadOnly);
  if (!hSrcDS) {
    throw badForecastFile("Failed to open forecast dataset.");
  }

  int nBandCount = GDALGetRasterCount(hSrcDS);
  GDALRasterBandH hBand = GDALGetRasterBand(hSrcDS, 1);
  int bSuccess;
  dfNoData = GDALGetRasterNoDataValue(hBand, &bSuccess);
  if (!bSuccess) dfNoData = -9999.0;

  // Setup warp options
  psWarpOptions = GDALCreateWarpOptions();
  psWarpOptions->padfDstNoDataReal = (double*)CPLMalloc(sizeof(double) * nBandCount);
  psWarpOptions->padfDstNoDataImag = (double*)CPLMalloc(sizeof(double) * nBandCount);
  for (int i = 0; i < nBandCount; ++i) {
    psWarpOptions->padfDstNoDataReal[i] = dfNoData;
    psWarpOptions->padfDstNoDataImag[i] = dfNoData;
  }

  pszSrcWkt = GDALGetProjectionRef(hSrcDS);
  hVrtDS = GDALAutoCreateWarpedVRT(hSrcDS, pszSrcWkt, pszDstWkt, GRA_NearestNeighbour, 1.0, psWarpOptions);

         // Extract desired bands
  for (int i = 0; i < nBandCount; ++i) {
    hBand = GDALGetRasterBand(hVrtDS, i + 1);
    const char* pszElement = GDALGetMetadataItem(hBand, "GRIB_ELEMENT", nullptr);
    const char* pszShortName = GDALGetMetadataItem(hBand, "GRIB_SHORT_NAME", nullptr);
    if (!pszElement || !pszShortName) continue;

    if (EQUAL(pszElement, "TMP")) {
      GDAL2AsciiGrid((GDALDataset*)hVrtDS, i + 1, airGrid);
      airGrid.set_noDataValue(dfNoData);
      airGrid.replaceNan(dfNoData);
      bHaveTemp = true;
    }
    else if (EQUAL(pszElement, "UGRD")) {
      GDAL2AsciiGrid((GDALDataset*)hVrtDS, i + 1, uGrid);
      uGrid.set_noDataValue(dfNoData);
      uGrid.replaceNan(dfNoData);
    }
    else if (EQUAL(pszElement, "VGRD")) {
      GDAL2AsciiGrid((GDALDataset*)hVrtDS, i + 1, vGrid);
      vGrid.set_noDataValue(dfNoData);
      vGrid.replaceNan(dfNoData);
    }
    else if (EQUAL(pszElement, "TCDC")) {
      GDAL2AsciiGrid((GDALDataset*)hVrtDS, i + 1, cloudGrid);
      cloudGrid.set_noDataValue(dfNoData);
      cloudGrid.replaceNan(dfNoData);
      bHaveCloud = true;
    }
  }

  GDALClose(hSrcDS);
  GDALClose(hVrtDS);

  if (!bHaveCloud && bNeedNextCloud) {
    // Try loading cloud data from next time step
    bpt::ptime epoch(boost::gregorian::date(1970, 1, 1));
    time_t t = (timeList[1].utc_time() - epoch).total_seconds();
    const char* pszNextFcst = FindForecast(input.forecastFilename.c_str(), t);
    hSrcDS = GDALOpenShared(pszNextFcst, GA_ReadOnly);
    CPLFree((void*)pszNextFcst);

    if (hSrcDS) {
      pszSrcWkt = GDALGetProjectionRef(hSrcDS);
      hVrtDS = GDALAutoCreateWarpedVRT(hSrcDS, pszSrcWkt, pszDstWkt, GRA_NearestNeighbour, 1.0, psWarpOptions);
      for (int i = 0; i < GDALGetRasterCount(hVrtDS); ++i) {
        hBand = GDALGetRasterBand(hVrtDS, i + 1);
        const char* pszElement = GDALGetMetadataItem(hBand, "GRIB_ELEMENT", nullptr);
        if (EQUAL(pszElement, "TCDC")) {
          GDAL2AsciiGrid((GDALDataset*)hVrtDS, i + 1, cloudGrid);
          cloudGrid.set_noDataValue(dfNoData);
          cloudGrid.replaceNan(dfNoData);
          bHaveCloud = true;
          break;
        }
      }
      GDALClose(hSrcDS);
      GDALClose(hVrtDS);
    }

    if (!bHaveCloud) {
      CPLError(CE_Warning, CPLE_AppDefined, "No cloud data found. Setting cloud grid to 0.");
      cloudGrid.set_headerData(uGrid);
      cloudGrid = 0.0;
    }
  }
  else if (!bHaveCloud) {
    cloudGrid.set_headerData(uGrid);
    cloudGrid = 0.0;
  }

  cloudGrid /= 100.0;
  airGrid += 273.15;  // Celsius to Kelvin
  wGrid.set_headerData(uGrid);
  wGrid = 0.0;

  GDALDestroyWarpOptions(psWarpOptions);
}

char* GCPWxModel::FindForecast(const char* pszFilePath, time_t nTime)
{
  VSIStatBufL sStat;
  VSIStatL(pszFilePath, &sStat);

  const char* pszPath = nullptr;
  if (VSI_ISDIR(sStat.st_mode))
    pszPath = CPLStrdup(pszFilePath);
  else if (strstr(wxModelFileName.c_str(), ".zip"))
    pszPath = CPLStrdup(CPLSPrintf("/vsizip/%s", wxModelFileName.c_str()));
  else
    pszPath = CPLStrdup(CPLGetPath(pszFilePath));

  char** papszFileList = VSIReadDir(pszPath);
  if (!papszFileList || CSLCount(papszFileList) == 0)
  {
    CPLFree((void*)pszPath);
    return nullptr;
  }

  for (int i = 0; papszFileList[i] != nullptr; ++i)
  {
    std::string filename = papszFileList[i];

           // Skip dotfiles
    if (filename == "." || filename == "..")
      continue;

           // Try to match pattern: 20250512.hrrr.t15z.wrfnatf00.grib2
    size_t posDate = filename.find('.');
    size_t posHour = filename.find(".hrrr.t");
    size_t posZ = filename.find('z', posHour);

    if (posDate == std::string::npos || posHour == std::string::npos || posZ == std::string::npos)
      continue;

    std::string dateStr = filename.substr(0, posDate); // "20250512"
    std::string hourStr = filename.substr(posHour + 7, posZ - (posHour + 7)); // "15"

    if (dateStr.length() != 8 || hourStr.length() != 2)
      continue;

           // Build datetime string and convert to time_t
    std::string datetimeStr = dateStr + hourStr;

    struct tm tmTime = {};
    strptime(datetimeStr.c_str(), "%Y%m%d%H", &tmTime);
    time_t fileTime = mktime(&tmTime);

    if (fileTime == nTime)
    {
      std::string fullPath = std::string(pszPath) + "/" + filename;
      CPLFree((void*)pszPath);
      CSLDestroy(papszFileList);
      return CPLStrdup(fullPath.c_str());
    }
  }

  CPLFree((void*)pszPath);
  CSLDestroy(papszFileList);
  return nullptr;
}


void GCPWxModel::checkForValidData()
{
  return;
}

int GCPWxModel::getEndHour()
{
  return NULL;
}

int GCPWxModel::getStartHour()
{
  return NULL;
}


void GCPWxModel::set3dGrids( WindNinjaInputs &input, Mesh const& mesh )
{
  return;
}








