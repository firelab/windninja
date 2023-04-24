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

/////////////////////// REMOVE

static std::string gzipped (std::string& fileName)
{
    std::string fn = "/vsigzip/";
    fn += fileName;
    fn += ".gz";

    return fn;
}

//--- writing grid data as JSON (source SRS agnostic)

static void printData (GDALRasterBand* pBand, int nCols, int nRows, VSILFILE* fout, const char* varName)
{
    std::unique_ptr<float[]> scanLine(new float[nCols]);
    int maxCol = nCols-1;

    VSIFPrintfL(fout,"\"%s\":[\n", varName);

    for (int i=0; i<nRows; i++) {
        if (pBand->RasterIO(GF_Read, 0, i, nCols,1, scanLine.get(), nCols,1,GDT_Float32, 0,0,nullptr ) != CE_None) {
            cerr << "error reading band:scanline " << pBand->GetBand() << ":" << i << "\n";
            break;
        }
        
        if (i >0) VSIFPrintfL( fout, ",\n");
        for (int j=0; j<maxCol; j++) {
            VSIFPrintfL( fout, "%.2f,", scanLine[j]);
        }
        VSIFPrintfL( fout, "%.2f", scanLine[maxCol]);
    }

    VSIFPrintfL(fout,"\n],\n");
}

/**
 * this is gridded data in JSON, loosely based on AAIGRID but for all u,v,w,h bands
 * we add the WKT projection spec so that we could turn it into  
 */
void writeHuvwJsonGrid (GDALDataset* pDS, std::string& fname)
{
    std::string fn = gzipped(fname);

    cout << "writing JSON 3D grid to " << fn << "\n";
    int nCols = pDS->GetRasterXSize();
    int nRows = pDS->GetRasterYSize();

    std::string wkt = pDS->GetProjectionRef();
    wkt = std::regex_replace(wkt, std::regex("\""), "\\\"");

    double a[6];
    pDS->GetGeoTransform(a);
    double xllCorner = a[0];
    double cellSize = a[1];
    double yllCorner = a[3] - nRows*cellSize;

    VSILFILE *fout = VSIFOpenL( fn.c_str(), "w");
    if (fout) {
        VSIFPrintfL(fout, "{\n\"ncols\":%d,\n\"nrows\":%d,\n", nCols, nRows);
        VSIFPrintfL(fout, "\"xllcorner\":%f,\n\"yllcorner\":%f,\n\"cellsize\":%f,\n", xllCorner, yllCorner, cellSize);
        VSIFPrintfL(fout, "\"wkt\":\"%s\",\n", wkt.c_str());
        VSIFPrintfL(fout, "\"NODATA_value\":-9999.0,\n");

        printData( pDS->GetRasterBand(1), nCols, nRows, fout, "h");
        printData( pDS->GetRasterBand(2), nCols, nRows, fout, "u");
        printData( pDS->GetRasterBand(3), nCols, nRows, fout, "v");
        printData( pDS->GetRasterBand(4), nCols, nRows, fout, "w");

        VSIFPrintfL(fout,"\n}\n");
        VSIFCloseL(fout);
    } else {
        cerr << "failed to write " << fn << "\n";
    }
}

//--- writing vector data output

// returns length of wind vector in percent of cell size
static double scaleFactor (float u, float v, float w, double& spd)
{
    //spd = std::sqrt( u*u + v*v + w*w);
    spd = std::sqrt( u*u + v*v);
    // we could emphasize high vertical (w) wind components here to avoid ortho view angle distortion

    if (spd < 2.2352) return 0.25;  // < 5mph
    if (spd < 4.4704) return 0.5;  // < 10mph
    if (spd < 8.9408) return 0.75;  // < 20mph
    return 0.95; // >= 20mph
}

typedef bool (*pv_func_t)(VSILFILE* fout, OGRCoordinateTransformation*,bool,double,double,double,float,float,float,float);


static void printVectors (VSILFILE* fout, GDALDataset* pDS, int tgtEpsg, pv_func_t pv, bool isHuvw0=false)
{
    if (pDS->GetRasterCount() < 4) {
        cerr << "invalid HUVW dataset (wrong raster count)\n";
        return;
    }

    const OGRSpatialReference *srcSRS = pDS->GetSpatialRef();
    OGRSpatialReference tgtSRS;
    tgtSRS.importFromEPSG( tgtEpsg);

    if (!srcSRS) {
        cerr << "no SRS in HUVW dataset\n";
        return;
    }

    OGRCoordinateTransformation* pTrans = OGRCreateCoordinateTransformation( srcSRS, &tgtSRS);
    if (!pTrans) {
        cerr << "failed to create coordinate transformation for HUVW dataset\n";
        return;
    }

    int nCols = pDS->GetRasterXSize();
    int nRows = pDS->GetRasterYSize();

    double a[6];
    if (pDS->GetGeoTransform(a) != CE_None){
        OCTDestroyCoordinateTransformation( pTrans);
        cerr << "error retrieving HUVW geo transform\n";
        return;
    }
    double cellSize = a[1]; // we assume a[1] == a[5] for HUVW dataset
    std::unique_ptr<float[]> h(new float[nCols]), u(new float[nCols]), v(new float[nCols]), w(new float[nCols]);

    GDALRasterBand* pH = pDS->GetRasterBand(1);

    int baseIdx = isHuvw0 ? 6 : 2;
    GDALRasterBand* pU = pDS->GetRasterBand(baseIdx);
    GDALRasterBand* pV = pDS->GetRasterBand(baseIdx+1);
    GDALRasterBand* pW = pDS->GetRasterBand(baseIdx+2);

    double cx2 = a[1] / 2;
    double cy2 = a[5] / 2;

    for (int i=0; i<nRows; i++) {
        if ((pH->RasterIO(GF_Read, 0, i, nCols,1, h.get(), nCols,1,GDT_Float32, 0,0,nullptr ) != CE_None)
             || (pU->RasterIO(GF_Read, 0, i, nCols,1, u.get(), nCols,1,GDT_Float32, 0,0,nullptr ) != CE_None)
             || (pV->RasterIO(GF_Read, 0, i, nCols,1, v.get(), nCols,1,GDT_Float32, 0,0,nullptr ) != CE_None)
             || (pW->RasterIO(GF_Read, 0, i, nCols,1, w.get(), nCols,1,GDT_Float32, 0,0,nullptr ) != CE_None) ) {
            cerr << "error reading HUVW grid line " << i << "\n";
            return;
        }

        for (int j=0; j<nCols; j++) {
            // the grid values are for the respective grid cell centers. note there is no rotation
            double x = a[0] + (a[1] * j) + cx2;
            double y = a[3] + (a[5] * i) + cy2;

            if (!pv( fout, pTrans, (i==0 && j==0), cellSize, x, y, h[j], u[j], v[j], w[j])){
                return;
            }
        }
    }
}

//--- JSON and GeoJSON output

static bool printJsonVector (VSILFILE* fout, OGRCoordinateTransformation* pTrans, bool isFirst, double cellSize, double x, double y, float h, float u, float v, float w)
{
    double spd = 0; // [m/sec]
    double s = scaleFactor(u,v,w, spd) * cellSize; // length of display vector in [m]

    double f = s / spd;
    double su = u * f;
    double sv = v * f;
    double sw = w * f;

    double xs[] = { x, x + su };
    double ys[] = { y, y + sv };

    double h0 = h;
    double h1 = h0 + sw;
    double spdMph = spd * 2.23693629; // m/sec -> mph

    if (pTrans->Transform( 2, xs, ys, nullptr, nullptr)) {
        if (!isFirst) VSIFPrintfL(fout, ",\n");

        VSIFPrintfL(fout, "{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[%.5f,%.5f,%.0f]},\"properties\":{\"spd\":%.1f}},\n", 
                    ys[0], xs[0], h0, spdMph);
        VSIFPrintfL(fout, "{\"type\":\"Feature\",\"geometry\":{\"type\":\"LineString\",\"coordinates\":[[%.5f,%.5f,%.0f],[%.5f,%.5f,%.0f]]},\"properties\":{\"spd\":%.1f}}", 
                    ys[0], xs[0], h0, ys[1],xs[1],h1, spdMph);

        return true;
    } else {
        cerr << "HUVW coordinate transformation failed\n";
        return false;
    }
}

static void printJsonVectors (VSILFILE* fout, GDALDataset* pDS)
{
    printVectors(fout,pDS, 4326, printJsonVector);
}

// note that pDS is the huvw grid dataset in UTM
void writeHuvwJsonVectors (GDALDataset* pDS, std::string& fname)
{
    std::string fn = gzipped(fname);
    cout << "writing HUVW GeoJSON vectors to " << fn << "\n";

    VSILFILE *fout = VSIFOpenL( fn.c_str(), "w");
    if (fout) {
        VSIFPrintfL(fout, "{\n\"type\":\"FeatureCollection\",\n");
        VSIFPrintfL(fout, "\"features\": [\n");

        printJsonVectors(fout,pDS);

        VSIFPrintfL(fout,"\n]\n}\n");
        VSIFCloseL(fout);
    } else {
        cerr << "failed to write " << fname << "\n";
    }
}

//--- ECEF wind-vectors as CSV

static bool printCsvVector (VSILFILE* fout, OGRCoordinateTransformation* pTrans, bool isFirst, 
                            double cellSize, double x, double y, float h, float u, float v, float w)
{
    double spd = 0; // [m/sec]
    double s = scaleFactor(u,v,w, spd) * cellSize; // length of display vector in [m]

    double f = s / spd;
    double su = u * f;
    double sv = v * f;
    double sw = w * f;

    double xs[] = { x, x + su };
    double ys[] = { y, y + sv };
    double zs[] = { h, h + sw };

    //double spdMph = spd * 2.23693629; // m/sec -> mph

    if (pTrans->Transform( 2, xs, ys, zs, nullptr)) {
        VSIFPrintfL(fout, "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.2f\n", xs[0], ys[0], zs[0], xs[1], ys[1], zs[1], spd);
        return true;
    } else {
        cerr << "HUVW coordinate transformation failed\n";
        return false;
    }
}

static void printCsvVectors (VSILFILE* fout, GDALDataset* pDS, bool isHuvw0)
{
    int nCols = pDS->GetRasterXSize();
    int nRows = pDS->GetRasterYSize();

    VSIFPrintfL(fout, "# length:%d\n", nCols*nRows); // comment prefix line to let clients pre-allocate data
    VSIFPrintfL(fout, "x0,y0,z0, x1,y1,z1, spd m/sec\n");

    printVectors(fout,pDS, 4978, printCsvVector, isHuvw0);
}


void writeHuvwCsvVectors (GDALDataset* pDS, std::string& fname, bool isHuvw0)
{
    std::string fn = gzipped(fname);
    cout << "writing HUVW ECEF vectors to " << fn << "\n";

    VSILFILE *fout = VSIFOpenL( fn.c_str(), "w");
    if (fout) {
        printCsvVectors(fout,pDS, isHuvw0);
        VSIFCloseL(fout);
    } else {
        cerr << "failed to write " << fname << "\n";
    }
}

//--- wind vectors as single array CSV


static void printCsvGrid (VSILFILE* fout, GDALDataset* pDS, bool isHuvw0=false)
{
    int nCols = pDS->GetRasterXSize();
    int nRows = pDS->GetRasterYSize();

    double a[6];
    if (pDS->GetGeoTransform(a) != CE_None){
        cerr << "error retrieving HUVW geo transform\n";
        return;
    }

    double x0 = a[0];
    double cx = a[1];
    double y0 = a[3];
    double cy = a[5];

    double cx2 = a[1] / 2;
    double cy2 = a[5] / 2;

    std::unique_ptr<float[]> hs(new float[nCols]), us(new float[nCols]), vs(new float[nCols]), ws(new float[nCols]);

    GDALRasterBand* pH = pDS->GetRasterBand(1);

    int baseIdx = isHuvw0 ? 6 : 2;
    GDALRasterBand* pU = pDS->GetRasterBand(baseIdx);
    GDALRasterBand* pV = pDS->GetRasterBand(baseIdx+1);
    GDALRasterBand* pW = pDS->GetRasterBand(baseIdx+2);

    VSIFPrintfL(fout, "# nx:%d, x0:%f, dx:%f, ny:%d, y0:%f, dy:%f\n", nCols, x0, cx, nRows, y0, cy);
    VSIFPrintfL(fout, "h, u,v,w, spd m/sec\n");

    for (int i=0; i<nRows; i++) {
        if ((pH->RasterIO(GF_Read, 0, i, nCols,1, hs.get(), nCols,1,GDT_Float32, 0,0,nullptr ) != CE_None)
             || (pV->RasterIO(GF_Read, 0, i, nCols,1, vs.get(), nCols,1,GDT_Float32, 0,0,nullptr ) != CE_None)
             || (pW->RasterIO(GF_Read, 0, i, nCols,1, ws.get(), nCols,1,GDT_Float32, 0,0,nullptr ) != CE_None)
             || (pU->RasterIO(GF_Read, 0, i, nCols,1, us.get(), nCols,1,GDT_Float32, 0,0,nullptr ) != CE_None) ) {
            cerr << "error reading HUVW grid line " << i << "\n";
            return;
        }

        for (int j=0; j<nCols; j++) {
            float u = us[j];
            float v = vs[j];
            float w = ws[j];
            float spd = std::sqrt( u*u + v*v + w*w);

            VSIFPrintfL(fout, "%.1f,%.1f,%.1f,%.1f,%.1f\n", hs[j], u, v, w, spd);
        }
    }
}

void writeHuvwCsvGrid (GDALDataset* pDS, std::string& fname, bool isHuvw0)
{
    std::string fn = gzipped(fname);
    cout << "writing HUVW CSV grid to " << fn << "\n";

    VSILFILE *fout = VSIFOpenL( fn.c_str(), "w");
    if (fout) {
        printCsvGrid(fout,pDS, isHuvw0);
        VSIFCloseL(fout);
    } else {
        cerr << "failed to write " << fname << "\n";
    }
}
