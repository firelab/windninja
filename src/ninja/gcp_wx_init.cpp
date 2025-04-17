/******************************************************************************
*
* $Id$
*
* Project:  WindNinja
* Purpose:  Handle GCP fetching for archived HRRR and other GCP related issues
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
#include <string>
#include <vector>
#include "cpl_http.h"
#include "nomads_wx_init.h"

int GCPWxModel::CheckFileName( const char *pszFile, const char *pszFormat )
{

}

GCPWxModel::GCPWxModel(GCPWxModel const&A) : wxModelInitialization(A) {

}


GCPWxModel::GCPWxModel()
{

}

GCPWxModel::GCPWxModel( std::string filename )
{

}

GCPWxModel::GCPWxModel( const char *pszModelKey )
{

}

GCPWxModel::~GCPWxModel()
{

}

const char ** GCPWxModel::FindModelKey( const char *pszFilename )
{

}

bool GCPWxModel::identify( std::string fileName )
{

}

int GCPWxModel::getEndHour()
{

}

int GCPWxModel::getStartHour()
{

}

/**
 *@brief Returns horizontal grid resolution of the model
 *@return return grid resolution (in km unless < 1, then degrees)
 */
double GCPWxModel::getGridResolution()
{
  double resolution = -1.0;

  if(getForecastReadable('-').find("3-KM") != getForecastReadable('-').npos)
    resolution = 3.0;

  return resolution;
}

std::string GCPWxModel::fetchForecast( std::string demFile, int nHours )
{
  if( !ppszModelData )
  {
    throw badForecastFile( "Model not found" );
  }

  //Check to make sure the DEM is good.
  //This check is very similar to wxModelInitialization::fetchForecast
  //Line ~388-394
  GDALDatasetH hDS = GDALOpen( demFile.c_str(), GA_ReadOnly );
  double demBounds[4];
  if(!GDALGetBounds((GDALDataset*)hDS,demBounds))//Cast GDALDatasetH as a GdalDataset*
  {
    throw badForecastFile("Could not download weather forecast, invalid "
                          "projection for the DEM");
  }

  double adfNESW[4], adfWENS[4];
  ComputeWxModelBuffer( (GDALDataset*)hDS, adfNESW );
  /* Different order for nomads */
  adfWENS[0] = adfNESW[3];
  adfWENS[1] = adfNESW[1];
  adfWENS[2] = adfNESW[0];
  adfWENS[3] = adfNESW[2];

  GDALClose( hDS );

  std::string upperLeftX = std::to_string(adfWENS[0]);
  std::string upperLeftY = std::to_string(adfWENS[2]);
  std::string lowerRightX = std::to_string(adfWENS[1]);
  std::string lowerRightY = std::to_string(adfWENS[3]);

  if(CPLGetConfigOption("GS_OAUTH2_PRIVATE_KEY_FILE", NULL) != NULL)
  {
    privateKey = CPLGetConfigOption("GS_OAUTH2_PRIVATE_KEY_FILE", NULL);
    CPLDebug("GCP", "Setting private key file to %s", privateKey);
  }

  if(CPLGetConfigOption("GS_OAUTH2_CLIENT_EMAIL", NULL) != NULL)
  {
    clientEmail = CPLGetConfigOption("GS_OAUTH2_CLIENT_EMAIL", NULL);
    CPLDebug("GCP", "Setting client email to %s", clientEmail);
  }

  const char *srcFile = "/vsigs/high-resolution-rapid-refresh/hrrr.20210525/conus/hrrr.t01z.wrfsfcf00.grib2";
  const char *outFile = "hrrr.t01z.wrfsfcf00.grib2";
  const char* idxFile = "https://storage.googleapis.com/high-resolution-rapid-refresh/hrrr.20210525/conus/hrrr.t01z.wrfsfcf00.grib2.idx";

  std::string variables[] =
  {
      "TMP:2 m above ground",
      "UGRD:10 m above ground",
      "VGRD:10 m above ground",
      "TCDC:entire atmosphere"
  };

  CPLHTTPResult* result = CPLHTTPFetch(idxFile, nullptr);
  if (!result || result->nDataLen == 0 || result->pabyData == nullptr) \
  {
    CPLDebug("IDXParse", "Failed to fetch data from: %s", idxFile);
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

  std::vector<std::string> optionStrs;

  // Add all -b band options
  for (int band : bands) {
    optionStrs.push_back("-b");
    optionStrs.push_back(std::to_string(band));
  }

  // Add spatial cropping options
  optionStrs.push_back("-projwin");
  optionStrs.push_back(upperLeftX);
  optionStrs.push_back(upperLeftY);
  optionStrs.push_back(lowerRightX);
  optionStrs.push_back(lowerRightY);

  optionStrs.push_back("-projwin_srs");
  optionStrs.push_back("EPSG:4326");

  optionStrs.push_back("-of");
  optionStrs.push_back("GRIB");

  // Convert to const char* array
  std::vector<const char*> options;
  for (const auto& s : optionStrs) {
    options.push_back(s.c_str());
  }
  options.push_back(nullptr);

  GDALTranslateOptions *transOptions = GDALTranslateOptionsNew(options.data(), NULL);

  GDALDataset *srcDataset = (GDALDataset *)GDALOpen(srcFile, GA_ReadOnly);
  if (srcDataset == NULL) {
    CPLDebug("GCP", "Failed to open input dataset");
    return "";
  }

  GDALDataset *outDataset = GDALTranslate(outFile, srcDataset, transOptions, NULL);
  if (outDataset == NULL) {
    GDALClose(srcDataset);
    CPLDebug("GCP", "GDALTranslate Failed");
    return "";
  }

  GDALClose(outDataset);
  GDALClose(srcDataset);
  GDALTranslateOptionsFree(transOptions);

  return std::string(outFile);
}

std::vector<std::string> GCPWxModel::getVariableList()
{

}


std::string GCPWxModel::getForecastIdentifier()
{
  return "TEST";
}

std::string GCPWxModel::getForecastReadable( const char bySwapWithSpace )
{

}

static int NomadsCompareStrings( const void *a, const void *b )
{

}

std::vector<blt::local_date_time> GCPWxModel::getTimeList( const char *pszVariable, blt::time_zone_ptr timeZonePtr )
{

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

int GCPWxModel::ClipNoData( GDALRasterBandH hBand, double dfNoData,
                              int *pnRowsToCull, int *pnColsToCull )
{

}

GDALRasterBandH GCPWxModel::FindBand( GDALDatasetH hDS, const char *pszVar,
                                        const char *pszHeight )
{

}






