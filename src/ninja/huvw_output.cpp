/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  utility functions to store and output 3D (u,v,w,h) wind data
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

#include "huvw_output.h"


/**
 * create (optional) 3d (h,u,v,w,spd) output grid dataset 
 * we store the redundant spd value as a separate layer so that we can easily create contour maps from the data set
 * called from ninja::prepareOutput()
 */
GDALDataset* createHuvwDS (const char* filename, const char* descr, const char* prjString, int nCols, int nRows, double xllCorner, double yllCorner, double cellSize)
{
    GDALDriver* pDriver = GetGDALDriverManager()->GetDriverByName( "GTiff"); // should be built-in format
    char **papszOptions = nullptr;
    papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "ZSTD" );
    //papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "LZW" );
    //papszOptions = CSLSetNameValue( papszOptions, "PREDICTOR", "3" );

    GDALDataset* pDS = pDriver->Create( filename, nCols, nRows, 5, GDT_Float32, papszOptions);

    if (pDS) {
        gdalSetSrs( pDS, nCols,nRows, xllCorner,yllCorner, cellSize, prjString);
        pDS->SetMetadataItem("info", descr);

        GDALRasterBand *pBand = pDS->GetRasterBand(1);
        pBand->SetMetadataItem("name", "H");
        pBand->SetMetadataItem("info", "height of wind vector (terrain elevation + wind height above ground)");
        pBand->SetMetadataItem("unit", "[m]");
        pBand->SetNoDataValue(-9999);

        pBand = pDS->GetRasterBand(2);
        pBand->SetMetadataItem("name", "U");
        pBand->SetMetadataItem("info", "horizontal East component of wind vector");
        pBand->SetMetadataItem("unit", "[m/sec]");
        pBand->SetNoDataValue(-9999);

        pBand = pDS->GetRasterBand(3);
        pBand->SetMetadataItem("name", "V");
        pBand->SetMetadataItem("info", "horizontal North component of wind vector");
        pBand->SetMetadataItem("unit", "[m/sec]");
        pBand->SetNoDataValue(-9999);

        pBand = pDS->GetRasterBand(4);
        pBand->SetMetadataItem("name", "W");
        pBand->SetMetadataItem("info", "vertical component of wind vector");
        pBand->SetMetadataItem("unit", "[m/sec]");
        pBand->SetNoDataValue(-9999);

        pBand = pDS->GetRasterBand(5);
        pBand->SetMetadataItem("name", "Speed");
        pBand->SetMetadataItem("info", "wind speed");
        pBand->SetMetadataItem("unit", "[m/sec]");
        pBand->SetNoDataValue(-9999);
    } else {
        cerr << "failed to create huvw output data set\n"; // should we throw a runtime_error here?
    }

    return pDS;
}

/**
 * set scanlines for d (h,u,v,w,spd) output grid dataset 
 * called from ninja::interp_uvw()
 */
void setHuvwScanlines(GDALDataset* pDS, int rowIdx, int nCols, float* hRow, float* uRow, float* vRow, float* wRow, float* spdRow)
{
    pDS->GetRasterBand(1)->RasterIO( GF_Write, 0,rowIdx, nCols,1, hRow, nCols,1, GDT_Float32, 0,0,nullptr); 
    pDS->GetRasterBand(2)->RasterIO( GF_Write, 0,rowIdx, nCols,1, uRow, nCols,1, GDT_Float32, 0,0,nullptr); 
    pDS->GetRasterBand(3)->RasterIO( GF_Write, 0,rowIdx, nCols,1, vRow, nCols,1, GDT_Float32, 0,0,nullptr); 
    pDS->GetRasterBand(4)->RasterIO( GF_Write, 0,rowIdx, nCols,1, wRow, nCols,1, GDT_Float32, 0,0,nullptr);
    pDS->GetRasterBand(5)->RasterIO( GF_Write, 0,rowIdx, nCols,1, spdRow, nCols,1, GDT_Float32, 0,0,nullptr);
}
