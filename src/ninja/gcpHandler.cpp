/******************************************************************************
*
* $Id$
*
* Project:  WindNinja
* Purpose:  Handle GCP fetching for archived HRRR and other GCP related issues
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


gcpHandler::gcpHandler() {
}

void gcpHandler::generateTIFFfromGCP() {
   if (setenv("GS_OAUTH2_PRIVATE_KEY_FILE", "", 1) != 0) {
           std::cerr << "Failed to set GS_OAUTH2_PRIVATE_KEY_FILE environment variable.\n";
           return;
   }
   if (setenv("GS_OAUTH2_CLIENT_EMAIL", "", 1) != 0) {
           std::cerr << "Failed to set GS_OAUTH2_CLIENT_EMAIL environment variable.\n";
           return;
   }

   GDALAllRegister();
   std::cout << "GS_OAUTH2_PRIVATE_KEY_FILE: " << getenv("GS_OAUTH2_PRIVATE_KEY_FILE") << "\n";

   // Define input and output file paths
   const char *inputFile = "/vsigs/high-resolution-rapid-refresh/hrrr.20210525/conus/hrrr.t01z.wrfprsf00.grib2";
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


   // Perform the translation


   GDALDataset *inputDataset;


   GDALDataset *outputDataset; 
   inputDataset = (GDALDataset *)GDALOpen(inputFile, GA_ReadOnly);
   if (inputDataset == nullptr) {
       fprintf(stderr, "Failed to open input dataset.\n");
       return 1;
   }

   outputDataset = GDALTranslate(outputFile, inputDataset, translateOptions, nullptr);
   if (outputDataset == nullptr) {
       fprintf(stderr, "GDALTranslate failed.\n");
       GDALClose(inputDataset);
       return 1;
   }


   // Clean up
   GDALClose(outputDataset);
   GDALClose(inputDataset);
   GDALTranslateOptionsFree(translateOptions);

   return 0;

}



