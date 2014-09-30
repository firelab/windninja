/******************************************************************************
*
* Filename: OutputWriter.cpp
*
* Project:  WindNinja 
* Purpose:  Class to handle output of WindNinja simulations to various GDAL
*           formats
* Author:   Levi Malott, lmnn3@mst.edu 
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

#include "OutputWriter.h"
#include <iostream>


const char * OutputWriter::SPEED     = "speed";
const char * OutputWriter::DIR       = "dir";
const char * OutputWriter::AV_DIR    = "AV_dir";
const char * OutputWriter::AM_DIR    = "AM_dir";
const char * OutputWriter::QGIS_DIR  = "QGIS_dir";
const char * OutputWriter::OGR_FILE  = "/vsimem/out.ogr";


OutputWriter::OutputWriter ()
{
    OGRRegisterAll();
    GDALAllRegister();
    hSrcDS = NULL;
    hDstDS = NULL;
    hDriver = NULL;
    pafScanline = NULL;
    hLayer = NULL;
    hFieldDefn = NULL;
    hDataSource = NULL;
    hOGRDriver = NULL;
    papszOptions = NULL;
}  /* -----  end of method OutputWriter::OutputWriter  (constructor)  ----- */


OutputWriter::~OutputWriter ()
{
    if( NULL != hSrcDS )
    {
        GDALClose(hSrcDS);
    }
    if( NULL != hDstDS )
    {
        GDALClose(hDstDS);
    }
    if( NULL != papszOptions )
    {
        CSLDestroy( papszOptions );
    }
    if( NULL != hDataSource )
    {
        OGR_DS_Destroy( hDataSource );
    }

    return;
}		/* -----  end of method OutputWriter::~OutputWriter  ----- */



    void
OutputWriter::setSpeedGrid ( AsciiGrid<double> &s)
{
    spd = s;
    return;
}		/* -----  end of method OutputWriter::setSpeedGrid  ----- */


    void
OutputWriter::setDirGrid ( AsciiGrid<double> &d )
{
    dir = d;
    return;
}		/* -----  end of method OutputWriter::setDirGrid  ----- */

    bool
OutputWriter::write (std::string outputFilename, std::string driver)
{

    if( 0 == driver.compare( "PDF" ) )
    {
        _writePDF(outputFilename);
    }
    else if( 0 == driver.compare( "GTiff" ) )
    {
        _writeGTiff(outputFilename);
    }
    else
    {
        throw std::runtime_error("OutputWriter: unrecognized output format");
    }
    return true;
}		/* -----  end of method OutputWriter::write  ----- */


    void
OutputWriter::_createOGRFileWithFields ()
{
    hOGRDriver = OGRGetDriverByName( "Memory" );
    if( NULL == hOGRDriver )
    {
        throw std::runtime_error("OutputWriter: Failed to get OGR Memory driver");
    }

    hDataSource = OGR_Dr_CreateDataSource( hOGRDriver, OGR_FILE, NULL );
    if( NULL == hDataSource )
    {
        throw std::runtime_error("OutputWriter: Failed to create OGR Memory datasource");
    }
    
    //Create a new layer for the wind features
    hLayer = OGR_DS_CreateLayer( hDataSource, "Points", NULL, wkbPoint, NULL );
    if( hLayer == NULL )
    {
        throw std::runtime_error("OutputWriter: Failed to create point attribute layer");
    }
    
    hFieldDefn = OGR_Fld_Create( SPEED, OFTReal );
    if( OGRERR_NONE != OGR_L_CreateField( hLayer, hFieldDefn, TRUE ) )
    {
        throw std::runtime_error("OutputWriter: Creating SPEED field failed");
    }
    OGR_Fld_Destroy(hFieldDefn);

    hFieldDefn = OGR_Fld_Create( DIR, OFTInteger );
    if( OGRERR_NONE != OGR_L_CreateField( hLayer, hFieldDefn, TRUE ) )
    {
        throw std::runtime_error("OutputWriter: Create DIR field failed");
    }
    OGR_Fld_Destroy(hFieldDefn);

    hFieldDefn = OGR_Fld_Create( AV_DIR, OFTInteger );
    if( OGRERR_NONE != OGR_L_CreateField( hLayer, hFieldDefn, TRUE ) )
    {
        throw std::runtime_error("OutputWriter: Create AV_DIR field failed");
    }
    OGR_Fld_Destroy(hFieldDefn);

    hFieldDefn = OGR_Fld_Create( AM_DIR, OFTInteger );
    if( OGRERR_NONE != OGR_L_CreateField( hLayer, hFieldDefn, TRUE ) )
    {
        throw std::runtime_error("OutputWriter: Create AM_DIR field failed");
    }
    OGR_Fld_Destroy(hFieldDefn);

    hFieldDefn = OGR_Fld_Create( QGIS_DIR, OFTInteger );
    if( OGRERR_NONE != OGR_L_CreateField( hLayer, hFieldDefn, TRUE ) )
    {
        throw std::runtime_error("OutputWriter: Create QGIS_DIR field failed");
    }
    OGR_Fld_Destroy(hFieldDefn);

    return ;
}		/* -----  end of method OutputWriter::createOGRFields  ----- */ 


    bool
OutputWriter::_writePDF (std::string filename)
{
    const char *pszDriverName = "PDF";
    int ncols, nrows;
    int nXSize, nYSize;
    double x, y, z;
    double mapDir, viewDir, qgisDir;
    x = y = z = mapDir = viewDir = qgisDir = 0;

    _createOGRFileWithFields();

    //Add the features to the OGR datasource given by the dir and spd grids
    for( int i = 0; i < spd.get_nRows(); i++ )
    {
        for( int j = 0; j < spd.get_nCols(); j++ )
        {
            OGRFeatureH hFeature;
            OGRGeometryH hPt;

            spd.get_cellPosition(i, j, &x, &y);
            cout << x << "," << y << endl;
            mapDir    = dir(i,j) + 180.0;
            viewDir   = qgisDir = mapDir;

            if( qgisDir > 360.0 )
            {
                qgisDir -= 360.0;
                mapDir  -= 360.0;
                viewDir -= 360.0;
            }

            viewDir = 360.0 - viewDir;
            mapDir  = -90.0;
            if( mapDir < 0.0 )
            {
                mapDir += 360.0;
            }

            hFeature = OGR_F_Create( OGR_L_GetLayerDefn( hLayer ) );
            OGR_F_SetFieldDouble( hFeature, OGR_F_GetFieldIndex(hFeature, SPEED), spd(i,j) );
            OGR_F_SetFieldInteger( hFeature, OGR_F_GetFieldIndex(hFeature, DIR), 
                                   (int)dir(i,j)+0.5);
            OGR_F_SetFieldInteger( hFeature, OGR_F_GetFieldIndex(hFeature, AV_DIR), 
                                   (int)viewDir+0.5);
            OGR_F_SetFieldInteger( hFeature, OGR_F_GetFieldIndex(hFeature, AM_DIR), 
                                   (int)mapDir+0.5);
            OGR_F_SetFieldInteger( hFeature, OGR_F_GetFieldIndex(hFeature, QGIS_DIR),
                                   (int)qgisDir+0.5);

            hPt = OGR_G_CreateGeometry(wkbPoint);
            OGR_G_SetPoint_2D(hPt, 0, x, y);

            OGR_F_SetGeometry( hFeature, hPt );
            OGR_G_DestroyGeometry( hPt );
            OGR_F_Destroy( hFeature );
        }
    }
    hDriver = GDALGetDriverByName( pszDriverName );
    
    if( hDriver == NULL )
    {
        throw std::runtime_error("OutputWriter: obtaining output driver failed");
    }


    hSrcDS = GDALOpenShared( demFile.c_str(), GA_ReadOnly );

    if( hSrcDS == NULL )
    {
        throw std::runtime_error("OutputWriter: failed to open background file");
    }

    papszOptions = CSLSetNameValue( papszOptions, "OGR_DATASOURCE", OGR_FILE );

    hDstDS = GDALCreateCopy( hDriver, filename.c_str(), hSrcDS, FALSE, 
                             papszOptions, NULL, NULL );


    GDALClose(hSrcDS);
    hSrcDS = NULL;
    if( NULL == hDstDS )
    {
        throw std::runtime_error("OutputWriter: Error creating output file");
    }
    else
    {
        GDALClose(hDstDS);
        hDstDS = NULL;
    }
    CSLDestroy( papszOptions );
    papszOptions = NULL;
    OGR_DS_Destroy( hDataSource );
    hDataSource = NULL;

    return true;
}		/* -----  end of method OutputWriter::_writePDF  ----- */

bool OutputWriter::_writeGTiff (std::string filename)
{
    // Silence error if file does not yet exist
    CPLPushErrorHandler(CPLQuietErrorHandler);
    hDstDS = GDALOpen(filename.c_str(), GA_ReadOnly);
    CPLPopErrorHandler();
    //if file doesn't already exist, create it
    if(hDstDS == NULL)
    {
        const char *pszDriverName = "GTiff";
        hDriver = GDALGetDriverByName( pszDriverName );
    
        int nXSize = spd.get_nCols();
        int nYSize = spd.get_nRows();
        int nBands = 1;

        hDstDS = GDALCreate(hDriver, filename.c_str(), nXSize, nYSize, 
                        nBands, GDT_Float32, NULL);
                        
        double adfGeoTransform[6];
        adfGeoTransform[0] = spd.get_xllCorner();
        adfGeoTransform[1] = spd.get_cellSize();
        adfGeoTransform[2] = 0;
        adfGeoTransform[3] = spd.get_yllCorner()+(spd.get_nRows()*spd.get_cellSize());
        adfGeoTransform[4] = 0;
        adfGeoTransform[5] = -(spd.get_cellSize());
        
        char* pszDstWKT = (char*)spd.prjString.c_str();
        GDALSetProjection(hDstDS, pszDstWKT);
        GDALSetGeoTransform(hDstDS, adfGeoTransform);
        
        GDALRasterBandH hBand = GDALGetRasterBand( hDstDS, 1 );
        
        double *padfScanline;
        padfScanline = new double[nXSize];

        for(int i=nYSize-1; i>=0; i--)
        {
            for(int j=0; j<nXSize; j++)
            {
                padfScanline[j] = spd.get_cellValue(nYSize-1-i, j);
                
                GDALRasterIO(hBand, GF_Write, 0, i, nXSize, 1, padfScanline, nXSize,
                             1, GDT_Float64, 0, 0);    
            }
        }
    }
    else{
        //if file already exists, just add a new band
    }
    
    GDALClose(hDstDS);
    hDstDS = NULL;

    return true;
}		/* -----  end of method OutputWriter::_writeGTiff  ----- */