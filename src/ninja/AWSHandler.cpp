/******************************************************************************
*
* $Id$
*
* Project:  WindNinja
* Purpose:  Handle AWS fetching for archived HRRR and other AWS related issues
* Author:   Rui Zhang <ruizhangslc2017@gmail.com>
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

#include "gcpHandler.h"


AWSHandler::AWSHandler() {
}

void AWSHandler::generateTIFFfromAWS() {
   if (setenv("AWS_ACCESS_KEY_ID", "", 1) != 0) {
       std::cerr << "Failed to set AWS_ACCESS_KEY_ID environment variable.\n";
       return;
   }
   if (setenv("AWS_SECRET_ACCESS_KEY", "", 1) != 0) {
       std::cerr << "Failed to set AWS_SECRET_ACCESS_KEY environment variable.\n";
       return;
   }
   
   CPLSetConfigOption( "CPL_VSIL_USE_TEMP_FILE_FOR_RANDOM_WRITE", "YES");

   CPLSetConfigOption( "GDAL_READDIR_LIMIT_ON_OPEN", "10000");
   GDALAllRegister();
   std::cout << "AWS_ACCESS_KEY_ID: " << getenv("AWS_ACCESS_KEY_ID") << "\n";

   // Define input and output file paths on S3
   const char *inputFile = "/vsis3/noaa-hrrr-bdp-pds/hrrr.20210525/conus/hrrr.t01z.wrfprsf00.grib2";
   const char *outputFile = "out.tif";


   // Set up GDALTranslateOptions
   const char *options[] = {
       "-b", "697",
       "-projwin", "-113.55", "45.00", "-112.55", "44.00",
       "-projwin_srs", "EPSG:4326",
       "-of", "GTiff",
       nullptr
   };

   GDALTranslateOptions *translateOptions;
   translateOptions = GDALTranslateOptionsNew(options, nullptr);

   GDALDataset *inputDataset;
   GDALDataset *outputDataset; 


   inputDataset = (GDALDataset *)GDALOpen(inputFile, GA_ReadOnly);
   if (inputDataset == nullptr) {
       fprintf(stderr, "Failed to open input dataset.\n");
       return;
   }


   outputDataset = GDALTranslate(outputFile, inputDataset, translateOptions, nullptr);
   if (outputDataset == nullptr) {
       fprintf(stderr, "GDALTranslate failed.\n");
       GDALClose(inputDataset);
       return;
   }

   GDALClose(outputDataset);
   GDALClose(inputDataset);
   GDALTranslateOptionsFree(translateOptions);

   return;
}


