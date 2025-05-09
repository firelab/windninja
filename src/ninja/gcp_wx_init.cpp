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

std::vector<blt::local_date_time>
GCPWxModel::getTimeList(const char *pszVariable, blt::time_zone_ptr timeZonePtr)
{

}


std::string GCPWxModel::fetchForecast(std::string demFile, int nhours)
{
  if (!ppszModelData)
  {
    throw badForecastFile("Model not found");
  }

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
  std::string outFolder = path + "/test/";
  VSIStatBufL sStat;
  memset(&sStat, 0, sizeof(VSIStatBufL));
  if (VSIStatL(outFolder.c_str(), &sStat) != 0 || !VSI_ISDIR(sStat.st_mode))
  {
    VSIMkdir(outFolder.c_str(), 0777);
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

    std::string outFile = outFolder + "hrrr_" + dateStr + "_" + hourStr + ".grib2";
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
  // Create the zip archive using GDAL's /vsizip/ virtual file system
  std::string zipPath = outFolder + "pastcast.zip";
  std::string zipVSIPath = "/vsizip/" + zipPath;

  for (const std::string &file : filePathsToZip)
  {
    std::string filenameOnly = CPLGetFilename(file.c_str());
    std::string zipEntryPath = zipVSIPath + "/" + filenameOnly;

    VSILFILE *in = VSIFOpenL(file.c_str(), "rb");
    VSILFILE *out = VSIFOpenL(zipEntryPath.c_str(), "wb");

    if (!in || !out)
    {
      CPLDebug("GCP", "Failed to open file for zipping: %s", file.c_str());
      if (in) VSIFCloseL(in);
      if (out) VSIFCloseL(out);
      continue;
    }

    char buffer[8192];
    size_t nRead;
    while ((nRead = VSIFReadL(buffer, 1, sizeof(buffer), in)) > 0)
    {
      VSIFWriteL(buffer, 1, nRead, out);
    }

    VSIFCloseL(in);
    VSIFCloseL(out);
  }
  wxModelFileName = zipPath;

  return zipPath;

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

std::string GCPWxModel::getForecastIdentifier()
{
  return "PAST-CAST-ARCHIVED-HRRR-CONUS-3KM";
}


char * GCPWxModel::NomadsFindForecast( const char *pszFilePath,
                                        time_t nTime )
{

}

void GCPWxModel::setSurfaceGrids( WindNinjaInputs &input,
                                    AsciiGrid<double> &airGrid,
                                    AsciiGrid<double> &cloudGrid,
                                    AsciiGrid<double> &uGrid,
                                    AsciiGrid<double> &vGrid,
                                    AsciiGrid<double> &wGrid )
{
  std::vector<blt::local_date_time> timeList( getTimeList( NULL, input.ninjaTimeZone ) );
  int i;
  GDALDatasetH hSrcDS, hVrtDS;
  GDALDatasetH hBand;
  const char *pszSrcWkt, *pszDstWkt;
  GDALWarpOptions *psWarpOptions;
  int bSuccess;
  double dfNoData;
  int nBandCount;
  int bNeedNextCloud = FALSE;
  AsciiGrid<double> speedGrid;
  AsciiGrid<double> directionGrid;
  bool blendCheck;

  /*
  ** We need to find the correct file in the directory.  It may not be the
  ** filename.
  */
  const char *pszForecastFile = NULL;
  for( i = 0; i < (int)timeList.size(); i++ )
  {
    if( timeList[i] == input.ninjaTime )
    {
      bpt::ptime epoch( boost::gregorian::date( 1970, 1, 1 ) );
      bpt::time_duration::sec_type t;
      t = (input.ninjaTime.utc_time() - epoch).total_seconds();
      pszForecastFile =
          NomadsFindForecast( input.forecastFilename.c_str(), (time_t)t );
      if( i == 0 && timeList.size() > 1 )
      {
        bNeedNextCloud = TRUE;
      }
      break;
    }

  }
  if( !pszForecastFile )
  {
    throw badForecastFile( "Could not find forecast associated with " \
                          "requested time step" );
  }

  hSrcDS = GDALOpenShared( pszForecastFile, GA_ReadOnly );
  if( hSrcDS == NULL ) {
    throw badForecastFile( "Could not find forecast associated with " \
                          "requested time step" );
  }
  CPLFree( (void*) pszForecastFile );
  pszForecastFile = NULL;
  nBandCount = GDALGetRasterCount( hSrcDS );
  hBand = GDALGetRasterBand( hSrcDS, 1 );
  dfNoData = GDALGetRasterNoDataValue( hBand, &bSuccess );
  if( bSuccess == FALSE )
  {
    dfNoData = -9999.0;
  }

  psWarpOptions = GDALCreateWarpOptions();
  psWarpOptions->padfDstNoDataReal =
      (double*) CPLMalloc( sizeof( double ) * nBandCount );
  psWarpOptions->padfDstNoDataImag =
      (double*) CPLMalloc( sizeof( double ) * nBandCount );
  for( i = 0; i < nBandCount; i++ )
  {
    psWarpOptions->padfDstNoDataReal[i] = dfNoData;
    psWarpOptions->padfDstNoDataImag[i] = dfNoData;
  }

  pszSrcWkt = GDALGetProjectionRef( hSrcDS );
  pszDstWkt = input.dem.prjString.c_str();
#ifdef NOMADS_INTERNAL_VRT
  hVrtDS = NomadsAutoCreateWarpedVRT( hSrcDS, pszSrcWkt, pszDstWkt,
                                     GRA_NearestNeighbour, 1.0,
                                     psWarpOptions );
#else
  hVrtDS = GDALAutoCreateWarpedVRT( hSrcDS, pszSrcWkt, pszDstWkt,
                                   GRA_NearestNeighbour, 1.0,
                                   psWarpOptions );
#endif

  const char *pszElement;
  const char *pszComment;
  const char *pszShortName;
  int bHaveTemp, bHaveCloud;
  bHaveTemp = FALSE;
  bHaveCloud = FALSE;
  for( i = 0; i < nBandCount; i++ )
  {
    hBand = GDALGetRasterBand( hVrtDS, i + 1 );
    /*
    ** Check the shortname and make sure it's valid.  HRRR TCDC uses
    ** entire_atmosphere instead of
    ** entire_atmosphere_(considered_as_a_single_layer), so we account for
    ** this with the 0-RESERVED(10).
    */
    pszShortName = GDALGetMetadataItem( hBand, "GRIB_SHORT_NAME", NULL );
    if( !EQUAL( "10-HTGL", pszShortName ) &&
        !EQUAL( "2-HTGL", pszShortName ) &&
        !EQUAL( "0-EATM", pszShortName ) &&
        !EQUAL( "0-CCY", pszShortName ) &&
        !EQUAL( "0-RESERVED(10)", pszShortName ) &&
        !EQUAL( "0-HCY", pszShortName ))
    {
      continue;
    }

    pszElement = GDALGetMetadataItem( hBand, "GRIB_ELEMENT", NULL );
    if( !pszElement )
    {
      throw badForecastFile( "Could not fetch proper band" );
    }
    if( EQUAL( pszElement, "TMP" ) )
    {
      GDAL2AsciiGrid( (GDALDataset*)hVrtDS, i + 1, airGrid );
      if( CPLIsNan( dfNoData ) )
      {
        airGrid.set_noDataValue( -9999.0 );
        airGrid.replaceNan( -9999.0 );
      }
      bHaveTemp = TRUE;
    }
    else if( EQUAL( pszElement, "UGRD" ) )
    {
      GDAL2AsciiGrid( (GDALDataset*)hVrtDS, i + 1, uGrid );
      if( CPLIsNan( dfNoData ) )
      {
        uGrid.set_noDataValue( -9999.0 );
        uGrid.replaceNan( -9999.0 );
      }
    }
    else if( EQUAL( pszElement, "VGRD" ) )
    {
      GDAL2AsciiGrid( (GDALDataset*)hVrtDS, i + 1, vGrid );
      if( CPLIsNan( dfNoData ) )
      {
        vGrid.set_noDataValue( -9999.0 );
        vGrid.replaceNan( -9999.0 );
      }
    }
    else if( EQUAL( pszElement, "TCDC" ) )
    {
      GDAL2AsciiGrid( (GDALDataset*)hVrtDS, i + 1, cloudGrid );
      if( CPLIsNan( dfNoData ) )
      {
        cloudGrid.set_noDataValue( -9999.0 );
        cloudGrid.replaceNan( -9999.0 );
      }
      bHaveCloud = TRUE;
    }
    else if( EQUAL( pszElement, "T" ) )
    {
      pszComment = GDALGetMetadataItem( hBand, "GRIB_COMMENT", NULL );

      if( EQUAL( pszComment, "Temperature [stddev]")) {
        continue;
      }

      GDAL2AsciiGrid( (GDALDataset*)hVrtDS, i + 1, airGrid );
      if( CPLIsNan( dfNoData ) )
      {
        airGrid.set_noDataValue( -9999.0 );
        airGrid.replaceNan( -9999.0 );
      }
      bHaveTemp = TRUE;
    }
    else if( EQUAL( pszElement, "WindSpd" ) )
    {
      blendCheck = true;
      pszComment = GDALGetMetadataItem( hBand, "GRIB_COMMENT", NULL );

      if( EQUAL( pszComment, "Wind speed [stddev]")) {
        continue;
      }

      GDAL2AsciiGrid( (GDALDataset*)hVrtDS, i + 1, speedGrid );
      if( CPLIsNan( dfNoData ) )
      {
        speedGrid.set_noDataValue( -9999.0 );
        speedGrid.replaceNan( -9999.0 );
      }
    }
    else if( EQUAL( pszElement, "WindDir" ) )
    {
      blendCheck = true;
      GDAL2AsciiGrid( (GDALDataset*)hVrtDS, i + 1, directionGrid );
      if( CPLIsNan( dfNoData ) )
      {
        directionGrid.set_noDataValue( -9999.0 );
        directionGrid.replaceNan( -9999.0 );
      }
    }
  }
  GDALClose( hSrcDS );
  GDALClose( hVrtDS );
  if( !bHaveCloud )
  {
    /*
    ** If we don't have cloud cover, and we are on the first time step, we
    ** may be in a strange discretization situation.  GFS 0th time step
    ** doesn't have time averaged cloud cover.  Let's go forward in time
    ** and copy the cloud cover from the 1st time.  Issue a warning.
    **
    ** Note that GDAL handles thread safety *in* the GRIB driver by
    ** acquiring a mutex in the driver.  We should be fine here, as long as
    ** we use GDALOpenShared().
    */
    if( bNeedNextCloud == TRUE )
    {
      GDALDatasetH hDSNextCloud;
      const char *pszNextFcst;
      bpt::ptime epoch( boost::gregorian::date( 1970, 1, 1 ) );
      bpt::time_duration::sec_type t;
      t = (timeList[1].utc_time() - epoch).total_seconds();
      pszNextFcst =
          NomadsFindForecast( input.forecastFilename.c_str(), (time_t)t );
      hSrcDS = GDALOpenShared( pszNextFcst, GA_ReadOnly );
      if( hSrcDS == NULL )
      {
        throw badForecastFile( "Could not load cloud data." );
      }
      pszSrcWkt = GDALGetProjectionRef( hSrcDS );
#ifdef NOMADS_VRT
      hVrtDS = NomadsAutoCreateWarpedVRT( hSrcDS, pszSrcWkt, pszDstWkt,
                                         GRA_NearestNeighbour, 1.0,
                                         psWarpOptions );
#else
      hVrtDS = GDALAutoCreateWarpedVRT( hSrcDS, pszSrcWkt, pszDstWkt,
                                       GRA_NearestNeighbour, 1.0,
                                       psWarpOptions );
#endif
      if( hVrtDS == NULL )
      {
        throw badForecastFile( "Could not load cloud data." );
      }
      int j = 0;
      for( j = 0; j < GDALGetRasterCount( hVrtDS ); j++ )
      {
        hBand = GDALGetRasterBand( hVrtDS, j + 1 );
        pszElement = GDALGetMetadataItem( hBand, "GRIB_ELEMENT", NULL );
        if( EQUAL( pszElement, "TCDC" ) )
        {
          break;
        }
        hBand = NULL;
      }
      if( hBand == NULL )
      {
        /*
         ** This is okay for models that have no cloud cover data at
         ** all, for example HRRR subhourly.  Before incorporation of
         ** the HRRR subhourly, this threw a bad forecast file, now we
         ** just set the cloud cover to 0 and issue a warning.
         */
        CPLFree( (void*)pszNextFcst );
        GDALClose( hSrcDS );
        GDALClose( hVrtDS );
        goto noCloudOK;
      }
      GDAL2AsciiGrid( (GDALDataset*)hVrtDS, j + 1, cloudGrid );
      dfNoData = GDALGetRasterNoDataValue( hBand, &bSuccess );
      if( bSuccess == FALSE )
      {
        dfNoData = -9999.0;
      }
      if( CPLIsNan( dfNoData ) )
      {
        cloudGrid.set_noDataValue( -9999.0 );
        cloudGrid.replaceNan( -9999.0 );
      }
      CPLFree( (void*)pszNextFcst );
      GDALClose( hSrcDS );
      GDALClose( hVrtDS );
      CPLError( CE_Warning, CPLE_AppDefined, "Could not load cloud data "
                                            "from 0th time step, using time step 1." );
    }
    else
    {
    noCloudOK:
      CPLError( CE_Warning, CPLE_AppDefined, "Could not load cloud data " \
               "from the forecast file, setting to 0." );

      cloudGrid.set_headerData( uGrid );
      cloudGrid = 0.0;
    }
  }
  cloudGrid /= 100.0;
  airGrid += 273.15;

  if(EQUAL(pszKey, "nbm_conus")) {
    uGrid.set_headerData(speedGrid);
    vGrid.set_headerData(speedGrid);
    wGrid.set_headerData(speedGrid);

    for(int i=0; i<speedGrid.get_nRows(); i++)
    {
      for(int j=0; j<speedGrid.get_nCols(); j++)
      {
        if( speedGrid(i,j) == speedGrid.get_NoDataValue() || directionGrid(i,j) == directionGrid.get_NoDataValue() ) {
          uGrid(i,j) = uGrid.get_NoDataValue();
          vGrid(i,j) = vGrid.get_NoDataValue();
        }
        else
          wind_sd_to_uv(speedGrid(i,j), directionGrid(i,j), &(uGrid)(i,j), &(vGrid)(i,j));
      }
    }
  }

  wGrid = 0.0;

  speedGrid.deallocate();
  directionGrid.deallocate();
  GDALDestroyWarpOptions( psWarpOptions );
}

void GCPWxModel::checkForValidData()
{
  return;
}


bool GCPWxModel::identify( std::string fileName )
{
  return GCPWxModel::FindModelKey( fileName.c_str() ) ? TRUE : FALSE;
}

int GCPWxModel::getEndHour()
{
  if( !ppszModelData )
  {
    return 0;
  }
  char **papszTokens = NULL;
  int nHour = 0;
  int nCount = 0;
  papszTokens = CSLTokenizeString2( ppszModelData[0],
                                   ":,", 0 );
  nCount = CSLCount( papszTokens );
  nHour = atoi( papszTokens[nCount - 2] );
  CSLDestroy( papszTokens );
  return nHour;
}

int GCPWxModel::getStartHour()
{
  if( !ppszModelData )
  {
    return 0;
  }
  char **papszTokens = NULL;
  int nHour = 0;
  int nCount = 0;
  papszTokens = CSLTokenizeString2( ppszModelData[0],
                                   ":,", 0 );
  nCount = CSLCount( papszTokens );
  if( nCount == 0 )
    return 0;
  nHour = atoi( papszTokens[0] );
  CSLDestroy( papszTokens );
  return nHour;
}


#define NOMADS_NON_PRES 4

void GCPWxModel::set3dGrids( WindNinjaInputs &input, Mesh const& mesh )
{
#ifdef NOMADS_ENABLE_3D
  if( ppszModelData == NULL )
    return;
  int g, h, i, j, k, n;
  GDALDatasetH hDS, hVrtDS;
  GDALRasterBandH hBand;
  const char *pszSrcWkt, *pszDstWkt;
  GDALWarpOptions *psWarpOptions;
  int nLayerCount;

  char **papszLevels =
      CSLTokenizeString2( ppszModelData[NOMADS_LEVELS], ",", 0 );
  nLayerCount = CSLCount( papszLevels );
  if( nLayerCount < 4 )
  {
    CSLDestroy( papszLevels );
    return;
  }
  std::vector<blt::local_date_time> timeList( getTimeList( NULL, input.ninjaTimeZone ) );
  /*
  ** We need to find the correct file in the directory.  It may not be the
  ** filename.
  */
  const char *pszForecastFile = NULL;
  for( i = 0; i < (int)timeList.size(); i++ )
  {
    if( timeList[i] == input.ninjaTime )
    {
      bpt::ptime epoch( boost::gregorian::date( 1970, 1, 1 ) );
      bpt::time_duration::sec_type t;
      t = (input.ninjaTime.utc_time() - epoch).total_seconds();
      pszForecastFile =
          NomadsFindForecast( input.forecastFilename.c_str(), (time_t)t );
      break;
    }
  }
  if( !pszForecastFile )
  {
    throw badForecastFile( "Could not find forecast associated with " \
                          "requested time step" );
  }

  hDS = GDALOpenShared( pszForecastFile, GA_ReadOnly );
  CPLFree( (void*)pszForecastFile );
  int nBandCount = GDALGetRasterCount( hDS );
  hBand = GDALGetRasterBand( hDS, 1 );
  int bSuccess;
  double dfNoData = GDALGetRasterNoDataValue( hBand, &bSuccess );
  if( !bSuccess )
    dfNoData = -9999.0;
  psWarpOptions = GDALCreateWarpOptions();
  psWarpOptions->padfDstNoDataReal =
      (double*) CPLMalloc( sizeof( double ) * nBandCount );
  psWarpOptions->padfDstNoDataImag =
      (double*) CPLMalloc( sizeof( double ) * nBandCount );
  for( i = 0; i < nBandCount; i++ )
  {
    psWarpOptions->padfDstNoDataReal[i] = dfNoData;
    psWarpOptions->padfDstNoDataImag[i] = dfNoData;
  }
  psWarpOptions->papszWarpOptions =
      CSLSetNameValue( psWarpOptions->papszWarpOptions,
                      "INIT_DEST", "-9999.0" );

  pszSrcWkt = GDALGetProjectionRef( hDS );
  pszDstWkt = input.dem.prjString.c_str();
#ifdef NOMADS_VRT
  hVrtDS = NomadsAutoCreateWarpedVRT( hDS, pszSrcWkt, pszDstWkt,
                                     GRA_NearestNeighbour, 1.0,
                                     psWarpOptions );
#else
  hVrtDS = GDALAutoCreateWarpedVRT( hDS, pszSrcWkt, pszDstWkt,
                                   GRA_NearestNeighbour, 1.0,
                                   psWarpOptions );
#endif

  int nSkipRows, nSkipCols;
  hBand = GDALGetRasterBand( hVrtDS, 1 );
  ClipNoData( hBand, dfNoData, &nSkipRows, &nSkipCols );
  nSkipRows++;
  nSkipCols++;

  int nXSize, nYSize;
  double dfXOrigin, dfYOrigin;
  double dfDeltaX, dfDeltaY;
  double adfGeoTransform[6];
  GDALGetGeoTransform( hVrtDS, adfGeoTransform );
  dfXOrigin = adfGeoTransform[0];
  dfYOrigin = adfGeoTransform[3];
  dfDeltaX = adfGeoTransform[1];
  dfDeltaY = adfGeoTransform[5];
  nXSize = GDALGetRasterXSize( hVrtDS );
  nYSize = GDALGetRasterYSize( hVrtDS );
  int nXSubSize ,nYSubSize;
  nXSubSize = nXSize - nSkipCols * 2;
  nYSubSize = nYSize - nSkipRows * 2;
  double *padfData = (double*)CPLMalloc( sizeof( double ) * nXSubSize );
  /* Subtract the surface layers */
  nLayerCount -= NOMADS_NON_PRES;
  oArray.allocate( nYSubSize, nXSubSize, nLayerCount );
  /* We assume our levels are in order from the groud up in the level list */
  int rc;
  i = 0;
  h = 0;
  while( h < nLayerCount && i < nLayerCount + NOMADS_NON_PRES )
  {
    if( strstr( papszLevels[i], "_m_above_ground" ) ||
        strstr( papszLevels[i], "surface" ) ||
        strstr( papszLevels[i], "entire_atmosphere" ) )
    {
      i++;
      continue;
    }

    hBand = FindBand( hVrtDS, "HGT", papszLevels[i] );
    if( hBand == NULL )
    {
      i++;
      continue;
    }
    n = 0;
    for( j = nYSize - nSkipRows - 1; j >= nSkipRows; j-- )
    {
      rc = GDALRasterIO( hBand, GF_Read, nSkipCols, j, nXSubSize, 1,
                        padfData, nXSubSize, 1, GDT_Float64, 0, 0 );
      for( k = 0; k < nXSubSize; k++ )
      {
        oArray( n, k, h ) = padfData[k];
      }
      n++;
    }
    h++;
    i++;
  }
  double xOffset, yOffset;
  double xllNinja, yllNinja, xllWxModel, yllWxModel;
  input.dem.get_cellPosition( 0, 0, &xllNinja, &yllNinja );
  xllWxModel = dfXOrigin + nSkipCols * dfDeltaX;
  yllWxModel = dfYOrigin + ((nYSize - nSkipRows) * dfDeltaY);

  xllWxModel += dfDeltaX / 2;
  yllWxModel += dfDeltaY / 2;

  xOffset = xllWxModel - xllNinja;
  yOffset = yllWxModel - yllNinja;

  wxMesh.buildFrom3dWeatherModel( input, oArray, dfDeltaX,
                                 nYSubSize, nXSubSize, nLayerCount,
                                 xOffset, yOffset );
  volVTK vtk;
  vtk.writeMeshVolVTK(wxMesh.XORD, wxMesh.YORD, wxMesh.ZORD,
                      wxMesh.ncols, wxMesh.nrows, wxMesh.nlayers,
                      "wxMesh.vtk");
  Mesh m2 = mesh;
  vtk.writeMeshVolVTK(m2.XORD, m2.YORD, m2.ZORD,
                      m2.ncols, m2.nrows, m2.nlayers,
                      "mackay.vtk");

  /* u,v,w,t */
  wxFields[0] = &wxU3d;
  wxFields[1] = &wxV3d;
  wxFields[2] = &wxW3d;
  wxFields[3] = &wxAir3d;
  wxFields[4] = &wxCloud3d;
  fields[0] = &u3d;
  fields[1] = &v3d;
  fields[2] = &w3d;
  fields[3] = &air3d;
  h = 0;

  static const char *apszVarList[] = { "UGRD", "VGRD", "DZDT", "TMP", NULL };
  while( apszVarList[h] != NULL )
  {
    wxFields[h]->allocate( &wxMesh );
    i = 0;
    g = 0;
    while( g < nLayerCount  && i < nLayerCount + NOMADS_NON_PRES )
    {
      if( strstr( papszLevels[i], "_m_above_ground" ) ||
          strstr( papszLevels[i], "surface" ) ||
          strstr( papszLevels[i], "entire_atmosphere" ) )
      {
        i++;
        continue;
      }
      hBand = FindBand( hVrtDS, apszVarList[h], papszLevels[i] );
      if( hBand == NULL )
      {
        i++;
        continue;
      }
      n = 0;
      for( j = nYSize - nSkipRows - 1; j >= nSkipRows; j-- )
      {
        rc = GDALRasterIO( hBand, GF_Read, nSkipCols, j, nXSubSize, 1,
                          padfData, nXSubSize, 1, GDT_Float64, 0, 0 );
        for( k = 0; k < nXSubSize; k++ )
        {
          (*(wxFields[h]))( n, k, g ) = padfData[k];
        }
        n++;
      }
      g++;
      i++;
    }
    h++;
  }
  wxCloud3d.allocate( &wxMesh );

  CPLFree( (void*)padfData );
  CSLDestroy( papszLevels );
  GDALClose( hDS );
  GDALClose( hVrtDS );

  for( i = 0; i < 4; i++ )
  {
    fields[i]->allocate( &mesh );
    wxFields[i]->interpolateScalarData((*(fields[i])), mesh, input);
  }
#endif /* NOMADS_ENABLE_3D */
  return;
}

const char ** GCPWxModel::FindModelKey( const char *pszFilename )
{

}

std::string GCPWxModel::getForecastReadable( const char bySwapWithSpace )
{

}






