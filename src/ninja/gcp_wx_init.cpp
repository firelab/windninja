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

std::string GCPWxModel::getForecastReadable( const char bySwapWithSpace )
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
  GDALDatasetH hDS = GDALOpen(demFile.c_str(), GA_ReadOnly);
  double demBounds[4];
  if (!GDALGetBounds((GDALDataset *)hDS, demBounds))
  {
    throw badForecastFile("Could not download weather forecast, invalid projection for the DEM");
  }

  double adfNESW[4];
  ComputeWxModelBuffer((GDALDataset *)hDS, adfNESW);
  GDALClose(hDS);

  std::string buffer[] = {
      to_string(adfNESW[3]),
      to_string(adfNESW[0]),
      to_string(adfNESW[1]),
      to_string(adfNESW[2])
  };

  if (CPLGetConfigOption("GS_OAUTH2_PRIVATE_KEY_FILE", NULL) != NULL)
  {
    privateKey = CPLGetConfigOption("GS_OAUTH2_PRIVATE_KEY_FILE", NULL);
  }

  if (CPLGetConfigOption("GS_OAUTH2_CLIENT_EMAIL", NULL) != NULL)
  {
    clientEmail = CPLGetConfigOption("GS_OAUTH2_CLIENT_EMAIL", NULL);
  }

         // Parse start and end datetime
  boost::posix_time::ptime startDateTime(startDate, boost::posix_time::duration_from_string(starthours + ":00:00"));
  boost::posix_time::ptime endDateTime(endDate, boost::posix_time::duration_from_string(endhours + ":00:00"));

         // Create output directory
  std::string path(CPLGetDirname(demFile.c_str()));
  std::string fileName(CPLGetFilename(demFile.c_str()));
  std::string startDateStr = boost::gregorian::to_iso_string(startDateTime.date());
  std::string outFolder = path + "/" + getForecastReadable("-") + "-" + fileName + "/" + startDateStr + "T" + starthours + "00" + "/";
  VSIStatBufL sStat;
  memset(&sStat, 0, sizeof(VSIStatBufL));
  if (VSIStatL(outFolder.c_str(), &sStat) != 0 || !VSI_ISDIR(sStat.st_mode))
  {
    VSIMkdirRecursive(outFolder.c_str(), 0777);
  }

         // Temporary zip file
  const char *pszTmpFile = CPLGenerateTempFilename("GCPWX_FCST");
  pszTmpFile = CPLSPrintf("%s", CPLFormFilename(NULL, pszTmpFile, ".zip"));
  pszTmpFile = CPLStrdup(pszTmpFile);

  std::vector<std::string> filePathsToZip;

  for (boost::posix_time::ptime dt = startDateTime; dt <= endDateTime; dt += boost::posix_time::hours(1))
  {
    std::string dateStr = boost::gregorian::to_iso_string(dt.date());
    std::stringstream hourSS;
    hourSS << std::setw(2) << std::setfill('0') << dt.time_of_day().hours();
    std::string hourStr = hourSS.str();

    std::string srcFile = "/vsigs/high-resolution-rapid-refresh/hrrr." + dateStr +
                          "/conus/hrrr.t" + hourStr + "z.wrfsfcf00.grib2";

    std::string outFile = outFolder + "hrrr." + dateStr + "t" + hourStr + "z." + "wrfsfcf00.grib2";
    std::string idxFile = "https://storage.googleapis.com/high-resolution-rapid-refresh/hrrr." + dateStr +
                          "/conus/hrrr.t" + hourStr + "z.wrfsfcf00.grib2.idx";

    std::vector<std::string> variables = getVariableList();
    std::vector<int> bands = findBands(idxFile, variables);
    std::vector<const char *> options = getOptions(bands, variables, buffer);

    GDALTranslateOptions *transOptions = GDALTranslateOptionsNew(options.data(), NULL);
    GDALDataset *srcDataset = (GDALDataset *)GDALOpen(srcFile.c_str(), GA_ReadOnly);
    if (!srcDataset)
    {
      CPLDebug("GCP", "Failed to open input dataset for %s", srcFile.c_str());
      continue;
    }

    GDALDataset *outDataset = GDALTranslate(outFile.c_str(), srcDataset, transOptions, NULL);
    GDALClose(srcDataset);
    GDALTranslateOptionsFree(transOptions);

    if (!outDataset)
    {
      CPLDebug("GCP", "GDALTranslate Failed for %s", outFile.c_str());
      continue;
    }

    GDALClose(outDataset);
    filePathsToZip.push_back(outFile);
  }

         // Create directory using startDate
  std::string zipFolder = outFolder;

  if (VSIStatL(zipFolder.c_str(), &sStat) != 0 || !VSI_ISDIR(sStat.st_mode))
  {
    VSIMkdir(zipFolder.c_str(), 0777);
  }

         // Final path for the zip file
  std::string zipFilePath = zipFolder + startDateStr + ".zip";

         // Construct virtual ZIP path (for writing)
  std::string zipVirtualPath = "/vsizip/" + zipFilePath;

         // Create the zip archive
  for (const std::string &filePath : filePathsToZip)
  {
    // Extract the filename to use as internal zip entry name
    std::string fileNameOnly(CPLGetFilename(filePath.c_str()));

           // Build the internal path inside the zip archive
    std::string zipEntryPath = zipVirtualPath + "/" + fileNameOnly;

           // Read from original file
    VSILFILE *fpSrc = VSIFOpenL(filePath.c_str(), "rb");
    if (!fpSrc)
    {
      CPLDebug("GCP", "Failed to open source file for zipping: %s", filePath.c_str());
      continue;
    }

    VSIFSeekL(fpSrc, 0, SEEK_END);
    vsi_l_offset fileSize = VSIFTellL(fpSrc);
    VSIFSeekL(fpSrc, 0, SEEK_SET);

    std::vector<char> buffer(fileSize);
    if (VSIFReadL(buffer.data(), 1, fileSize, fpSrc) != fileSize)
    {
      CPLDebug("GCP", "Failed to read complete file for zipping: %s", filePath.c_str());
      VSIFCloseL(fpSrc);
      continue;
    }

    VSIFCloseL(fpSrc);

           // Write to zip archive
    VSILFILE *fpZip = VSIFOpenL(zipEntryPath.c_str(), "wb");
    if (!fpZip)
    {
      CPLDebug("GCP", "Failed to create zip entry: %s", zipEntryPath.c_str());
      continue;
    }

    if (VSIFWriteL(buffer.data(), 1, fileSize, fpZip) != fileSize)
    {
      CPLDebug("GCP", "Failed to write data to zip entry: %s", zipEntryPath.c_str());
    }

    VSIFCloseL(fpZip);
  }

  for (const std::string &filePath : filePathsToZip)
  {
    if (VSIUnlink(filePath.c_str()) != 0)
    {
      CPLDebug("GCP", "Failed to delete temporary file: %s", filePath.c_str());
    }
    else
    {
      CPLDebug("GCP", "Deleted temporary file: %s", filePath.c_str());
    }
  }

  CPLDebug("GCP", "Created zip archive at %s", zipFilePath.c_str());

  return zipFilePath;


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

std::vector<int> GCPWxModel::findBands(std::string filename, std::vector<std::string> variables)
{
  CPLHTTPResult* result = CPLHTTPFetch(filename.c_str(), nullptr);
  if (!result || result->nDataLen == 0 || result->pabyData == nullptr) \
    {
      CPLDebug("IDXParse", "Failed to fetch data from: %s", filename);
      CPLHTTPDestroyResult(result);
      return;
    }

  std::string data(reinterpret_cast<char*>(result->pabyData), result->nDataLen);
  std::istringstream stream(data);
  std::string line;

  std::vector<int> bands;

  // Example line in file: 1:1000000:d=20210525 t=01z :UGRD:10 m above ground:anl:
  while (std::getline(stream, line))
  {
    for (const std::string& var : variables)
    {
      if (line.find(var) != std::string::npos)
      {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
          int band = std::stoi(line.substr(0, colonPos));
          bands.push_back(band);
          CPLDebug("IDXParse", "Field '%s' is at band %d", var.c_str(), band);
        }
      }
    }
  }
  CPLHTTPDestroyResult(result);
  return bands;
}

std::vector<const char *> GCPWxModel::getOptions(std::vector<int> bands, std::vector<std::string> variables, std::string buffer[])
{
  std::vector<std::string> options;
  for (int band : bands) {
    options.push_back("-b");
    options.push_back(std::to_string(band));
  }

  options.push_back("-projwin");
  options.push_back(buffer[0]);
  options.push_back(buffer[1]);
  options.push_back(buffer[2]);
  options.push_back(buffer[3]);
  options.push_back("-projwin_srs");
  options.push_back("EPSG:4326");

  options.push_back("-of");
  options.push_back("GRIB");

  std::vector<const char*> cstrOptions;
  for (const auto& s : options) {
    cstrOptions.push_back(s.c_str());
  }
  cstrOptions.push_back(nullptr);
  return cstrOptions;
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








