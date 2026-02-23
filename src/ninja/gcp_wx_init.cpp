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

GCPWxModel::GCPWxModel(GCPWxModel const&A) : wxModelInitialization(A) 
{

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
   return "PASTCAST-GCP-HRRR-CONUS-3-KM";
}

std::string GCPWxModel::getForecastIdentifier()
{
   return "PASTCAST-GCP-HRRR-CONUS-3-KM";
}

bool GCPWxModel::identify( std::string fileName )
{
    if (fileName.find("PASTCAST-GCP-HRRR-CONUS-3-KM") != std::string::npos)
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
            ostringstream os;
            os << "Failed to open file: " << filePath << "\n";
            throw badForecastFile( os.str() );
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

    // Sort the list to ensure chronological order
    std::sort(aoTimeList.begin(), aoTimeList.end());

    CSLDestroy(papszFileList);
    CSLDestroy(papszTimeList);

    aoCachedTimes = aoTimeList;
    return aoCachedTimes;
}

std::string GCPWxModel::fetchForecast(std::string demFile, int nhours)
{
    if(CPLGetConfigOption("GS_SECRET_ACCESS_KEY", NULL) == NULL || CPLGetConfigOption("GS_ACCESS_KEY_ID", NULL) == NULL)
    {
        if(CPLGetConfigOption("GS_OAUTH2_PRIVATE_KEY_FILE", NULL) == NULL || CPLGetConfigOption("GS_OAUTH2_CLIENT_EMAIL", NULL) == NULL)
        {
            throw std::runtime_error(
                "Missing required GCS credentials. One of the following pairs of environment variables must be set:\n"
                "GS_SECRET_ACCESS_KEY and GS_ACCESS_KEY_ID \n"
                "                OR \n"
                "GS_OAUTH2_PRIVATE_KEY_FILE and GS_OAUTH2_CLIENT_EMAIL"
            );
        }
    }

    if (pfnProgress)
    {
        pfnProgress(0.0, "Starting download...", NULL);
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

    boost::posix_time::ptime startDateTime(startDate, boost::posix_time::duration_from_string(starthours + ":00:00"));
    boost::posix_time::ptime endDateTime(endDate, boost::posix_time::duration_from_string(endhours + ":00:00"));
    std::string path(CPLGetDirname(demFile.c_str()));
    std::string fileName(CPLGetFilename(demFile.c_str()));
    std::string startDateStr = boost::gregorian::to_iso_string(startDateTime.date());

    std::string identifier = path + "/" + getForecastReadable() + "-" + fileName + "/";
    std::string outFolder = identifier + startDateStr + "T" + starthours + "00/";
    std::string tmp = outFolder + "tmp/";
    VSIMkdir(identifier.c_str(), 0777);
    VSIMkdir(outFolder.c_str(), 0777);
    VSIMkdir(tmp.c_str(), 0777);

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
            CPLError(CE_Warning, CPLE_AppDefined, "GCP, Failed to open remote idx file: %s", idxUrl.c_str());
            continue;
        }

        VSILFILE *fpLocal = VSIFOpenL(localIdxPath.c_str(), "w");
        if (!fpLocal) {
            CPLError(CE_Warning, CPLE_AppDefined, "GCP, Failed to create local idx file: %s", localIdxPath.c_str());
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

    if(validTimes.size() == 0)
    {
        throw std::runtime_error("Failed to open any remote idx files.\nLikely input times are out of available data range.");
    }

    std::vector<std::vector<std::string>> options = getOptions(fileBands, buffer);

#ifdef _OPENMP
    double startTime = omp_get_wtime();
#endif

    int nrc;

    if( pfnProgress )
    {
        if( pfnProgress( 0.1,
                         CPLSPrintf( "Downloading file 1 out of %d...\n This may take a few minutes...", validTimes.size() ),
                         NULL ) )
        {
            CPLError( CE_Failure, CPLE_UserInterrupt, "Cancelled by user." );
            nrc = GCP_ERR;
            return "";
        }
    }

    /*
    // try single threaded download
    int rc = 0;
    for (size_t i = 0; i < validTimes.size(); i++)
    {
    rc = fetchData( validTimes[i], tmp, options, i );
    }
    */

    // try multi threaded download
    const int MAX_CONCURRENT = 4;
    std::vector<CPLJoinableThread*> threadHandles;

    int i = 0;
    for (size_t dt = 0; dt < validTimes.size(); ++dt)
    {
        if ((int)threadHandles.size() >= MAX_CONCURRENT)
        {
            CPLJoinThread(threadHandles.front());
            threadHandles.erase(threadHandles.begin());
        }

        ThreadParams* params = new ThreadParams();
        params->dt = validTimes[dt];
        params->outPath = tmp;
        params->options = options;
        params->i = i;

        CPLJoinableThread* handle = CPLCreateJoinableThread(ThreadFunc, params);
        if (!handle) {
            CPLError(CE_Failure, CPLE_AppDefined, "GCP, Failed to create thread");
            delete params;
            break;
        }

        threadHandles.push_back(handle);
        i++;
    }

    for(int i = 0; i < threadHandles.size(); i++)
    {
        if( pfnProgress )
        {
            if( pfnProgress( 0.1+(0.9*(double)i / threadHandles.size()),
                             CPLSPrintf( "Downloading file %d out of %d...\n This may take a few minutes...", i+1, threadHandles.size() ),
                             NULL ) )
            {
                CPLError( CE_Failure, CPLE_UserInterrupt, "Cancelled by user." );
                nrc = GCP_ERR;
                return "";
            }
        }
        CPLJoinThread(threadHandles[i]);
    }

    threadHandles.clear();

#ifdef _OPENMP
    double endTime = omp_get_wtime();
    std::cout << "weather model raw data download time was " << endTime-startTime << " seconds." << std::endl;
#endif

    std::string zipFolder = outFolder;
    std::string zipFilePath = zipFolder + startDateStr + "T" + starthours + "00" + ".zip";
    std::string zipVirtualPath = "/vsizip/" + zipFilePath;

    if( CPLCheckForFile((char*)zipFilePath.c_str(), NULL) )
    {
        CPLUnlinkTree(zipFilePath.c_str());
    }

    char** fileList = VSIReadDir(tmp.c_str());
    if (!fileList) {
        throw std::runtime_error("Failed to read temporary directory: " + tmp);
    }

    int numFiles = 0;
    for(int fileIdx = 0; fileList[fileIdx] != nullptr; fileIdx++)
    {
        std::string fileName = fileList[fileIdx];
        if(fileName != "." && fileName != "..")
        {
            numFiles++;
        }
    }
    if(numFiles == 0)
    {
        throw badForecastFile("Failed to download any forecast files.");
    }

    for (int i = 0; fileList[i] != nullptr; ++i) {
        std::string fileNameOnly = fileList[i];
        if (fileNameOnly == "." || fileNameOnly == "..") continue;

        std::string filePath = tmp + fileNameOnly;
        std::string zipEntryPath = zipVirtualPath + "/" + fileNameOnly;

        VSILFILE* fpSrc = VSIFOpenL(filePath.c_str(), "rb");
        if (!fpSrc) {
            ostringstream os;
            os << "GCP, Failed to open source file for zipping: " << filePath << "\n";
            throw badForecastFile( os.str() );
        }

        VSIFSeekL(fpSrc, 0, SEEK_END);
        vsi_l_offset fileSize = VSIFTellL(fpSrc);
        VSIFSeekL(fpSrc, 0, SEEK_SET);

        std::vector<char> buffer(fileSize);
        if (VSIFReadL(buffer.data(), 1, fileSize, fpSrc) != fileSize) {
            VSIFCloseL(fpSrc);
            ostringstream os;
            os << "GCP, Failed to read complete file for zipping: " << filePath << "\n";
            throw badForecastFile( os.str() );
        }
        VSIFCloseL(fpSrc);

        VSILFILE* fpZip = VSIFOpenL(zipEntryPath.c_str(), "wb");
        if (!fpZip) {
            ostringstream os;
            os << "GCP, Failed to create zip entry: " << zipEntryPath << "\n";
            throw badForecastFile( os.str() );
        }

        if (VSIFWriteL(buffer.data(), 1, fileSize, fpZip) != fileSize) {
            VSIFCloseL(fpZip);
            ostringstream os;
            os << "GCP, Failed to write data to zip entry: " << zipEntryPath << "\n";
            throw badForecastFile( os.str() );
        }
        VSIFCloseL(fpZip);
    }
    NinjaUnlinkTree(tmp.c_str());
    CSLDestroy(fileList);

    CPLDebug("GCP", "Created zip archive at %s", zipFilePath.c_str());

    if( nrc == GCP_OK && pfnProgress )
    {
        pfnProgress( 1.0, NULL, NULL );
    }

    return zipFilePath;
}

int GCPWxModel::fetchData( boost::posix_time::ptime dt, std::string outPath, std::vector<std::vector<std::string>> options, int i )
{
    std::string dateStr = boost::gregorian::to_iso_string(dt.date());
    std::stringstream hourSS;
    hourSS << std::setw(2) << std::setfill('0') << dt.time_of_day().hours();
    std::string hourStr = hourSS.str();

    std::string srcFile = "/vsigs/high-resolution-rapid-refresh/hrrr." + dateStr +
                        "/conus/hrrr.t" + hourStr + "z.wrfsfcf00.grib2";
    std::string outFile = outPath + "hrrr." + dateStr + "t" + hourStr + "z." + "wrfsfcf00.tif";

    std::vector<const char*> cstrArgs;
    for (size_t kk = 0; kk < options[i].size(); ++kk)
    {
        cstrArgs.push_back(const_cast<const char*>(options[i][kk].c_str()));
    }
    cstrArgs.push_back(nullptr); // Null-terminated for GDAL

    GDALTranslateOptions *transOptions = GDALTranslateOptionsNew((char**)cstrArgs.data(), NULL);

    GDALDatasetH hSrcDS = GDALOpen(srcFile.c_str(), GA_ReadOnly);
    if (!hSrcDS)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "GCP, Failed to open input dataset for %s", srcFile.c_str());
        GDALTranslateOptionsFree(transOptions);
        return GCP_ERR;
    }

    GDALDatasetH hOutDS = GDALTranslate(outFile.c_str(), hSrcDS, transOptions, NULL);
    GDALClose(hSrcDS);
    GDALTranslateOptionsFree(transOptions);

    if (!hOutDS)
    {
        CPLError(CE_Failure, CPLE_AppDefined, "GCP, GDALTranslate failed for %s", outFile.c_str());
        return GCP_ERR;
    }
    GDALClose(hOutDS);

    return GCP_OK;
}

void GCPWxModel::ThreadFunc(void* pData)
{
    ThreadParams* params = static_cast<ThreadParams*>(pData);

    int rc = 0;
    rc = GCPWxModel::fetchData( params->dt, params->outPath, params->options, params->i );

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
        CPLError(CE_Warning, CPLE_AppDefined, "GCP, Failed to open cached .idx file: %s", idxFilePath.c_str());
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
                        CPLDebug("GCP", "Field '%s' is at band %d", var.c_str(), band);
                    }
                    else
                    {
                        CPLError(CE_Warning, CPLE_AppDefined, "GCP, Could not parse band number in line: %s", line.c_str());
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

std::vector<std::vector<std::string>> GCPWxModel::getOptions(const std::vector<std::string>& bands, const std::string buffer[4])
{
    std::vector<std::vector<std::string>> allOptions;

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
        strOptions.push_back("GTIFF");

        allOptions.push_back(strOptions); // Store each option list
    }

    return allOptions;
}

void GCPWxModel::setDateTime(boost::gregorian::date date1, boost::gregorian::date date2, std::string hours1, std::string hours2)
{
    startDate = date1;
    endDate = date2;
    starthours = hours1;
    endhours = hours2;

    // now do some checks of the inputs
    //
    // inputs are already in UTC time, so using a standard boost::posix_time::ptime is good enough,
    // no need for a boost::local_time::local_date_time or a boost::local_time::time_zone_ptr

    boost::posix_time::ptime startDateTime(startDate, boost::posix_time::duration_from_string(starthours + ":00:00"));
    boost::posix_time::ptime endDateTime(endDate, boost::posix_time::duration_from_string(endhours + ":00:00"));

    boost::posix_time::ptime minDateTime(boost::gregorian::date(2014, 7, 30), boost::posix_time::hours(18));

    // the max time should actually be 1 minus the hour of the current time, and 59 minutes, not the current time
    // will be more accurate across dates/times edge cases if the math is done right on the starting time structure
    boost::posix_time::ptime currentLocalTime_UTC = boost::posix_time::second_clock::universal_time();
    boost::posix_time::ptime maxDateTime = currentLocalTime_UTC - boost::posix_time::hours(1);

    if(startDateTime < minDateTime || endDateTime > maxDateTime)
    {
        throw std::runtime_error(
            "PASTCAST Datetime must be within the allowed range\n(from " +
            boost::posix_time::to_simple_string(minDateTime) + " UTC to " +
            boost::posix_time::to_simple_string(maxDateTime) + " UTC)."
        );
    }
    if(startDateTime > endDateTime)
    {
        throw std::runtime_error("Start datetime cannot be after stop datetime.");
    }
    boost::posix_time::time_duration maxRange = endDateTime - startDateTime;
    if(maxRange.hours() > 14 * 24)
    {
        throw std::runtime_error("Datetime range must not exceed 14 days.");
    }
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

    // Build the filename: "hrrr.20250520t20z.wrfsubhf00.tif"
    // Build the filename: "hrrr.20250520t20z.wrfsfcf00.tif"
    std::string filename = "hrrr." + dateStr + "t" + hourStr + "z.wrfsfcf00.tif";

    // Combine into VSI path
    std::string vsiPath = "/vsizip/" + input.forecastFilename + "/" + filename;
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

    // compute the coordinateTransformationAngle, the angle between the y coordinate grid lines of the pre-warped and warped datasets,
    // going FROM the y coordinate grid line of the pre-warped dataset TO the y coordinate grid line of the warped dataset
    // in this case, going FROM weather model projection coordinates TO dem projection coordinates
    double coordinateTransformationAngle = 0.0;
    if( CSLTestBoolean(CPLGetConfigOption("DISABLE_COORDINATE_TRANSFORMATION_ANGLE_CALCULATIONS", "FALSE")) == false )
    {
        // direct calculation of FROM wx TO dem, already has the appropriate sign
        if(!GDALCalculateCoordinateTransformationAngle( (GDALDataset*)hSrcDS, coordinateTransformationAngle, pszDstWkt ))  // this is FROM wx TO dem
        {
            printf("Warning: Unable to calculate coordinate transform angle for the wxModel.");
        }
    }

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

    //use the coordinateTransformationAngle to correct the angles of the output dataset
    //to convert from the original dataset projection angles to the warped dataset projection angles
    if( CSLTestBoolean(CPLGetConfigOption("DISABLE_COORDINATE_TRANSFORMATION_ANGLE_CALCULATIONS", "FALSE")) == false )
    {
        // need an intermediate spd and dir set of ascii grids
        AsciiGrid<double> speedGrid;
        AsciiGrid<double> dirGrid;
        speedGrid.set_headerData(uGrid);
        dirGrid.set_headerData(uGrid);
        for(int i=0; i<uGrid.get_nRows(); i++)
        {
            for(int j=0; j<uGrid.get_nCols(); j++)
            {
                if( uGrid(i,j) == uGrid.get_NoDataValue() || vGrid(i,j) == vGrid.get_NoDataValue() )
                {
                    speedGrid(i,j) = speedGrid.get_NoDataValue();
                    dirGrid(i,j) = dirGrid.get_NoDataValue();
                } else
                {
                    wind_uv_to_sd(uGrid(i,j), vGrid(i,j), &(speedGrid)(i,j), &(dirGrid)(i,j));
                }
            }
        }

        // use the coordinateTransformationAngle to correct each spd,dir, u,v dataset for the warp
        for(int i=0; i<dirGrid.get_nRows(); i++)
        {
            for(int j=0; j<dirGrid.get_nCols(); j++)
            {
                if( speedGrid(i,j) != speedGrid.get_NoDataValue() && dirGrid(i,j) != dirGrid.get_NoDataValue() )
                {
                    dirGrid(i,j) = wrap0to360( dirGrid(i,j) - coordinateTransformationAngle ); //convert FROM wxModel projection coordinates TO dem projected coordinates
                    // always recalculate the u and v grids from the corrected dir grid, the changes need to go together
                    wind_sd_to_uv(speedGrid(i,j), dirGrid(i,j), &(uGrid)(i,j), &(vGrid)(i,j));
                }
            }
        }

        // cleanup the intermediate grids
        speedGrid.deallocate();
        dirGrid.deallocate();
    }

    GDALDestroyWarpOptions(psWarpOptions);
}

char* GCPWxModel::FindForecast(const char* pszFilePath, time_t nTime)
{
    return NULL;
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
