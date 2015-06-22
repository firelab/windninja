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
const char * OutputWriter::OGR_FILE  = "/tmp/out.shp";


OutputWriter::OutputWriter ()
{
    OGRRegisterAll();
    GDALAllRegister();
    hSrcDS       = NULL;
    hDstDS       = NULL;
    hDriver      = NULL;
    pafScanline  = NULL;
    hLayer       = NULL;
    hFieldDefn   = NULL;
    hDataSource  = NULL;
    hOGRDriver   = NULL;
    papszOptions = NULL;
    hSpdMemDs    = NULL;
    hDirMemDs    = NULL;
    hDustMemDs   = NULL;
    hSrcSRS      = NULL;
    hDestSRS     = NULL;
    hTransform   = NULL;
    colors       = NULL;
    split_vals   = NULL;

    ///_createDefaultStyles();
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
        GDALClose( hDataSource );
    }
    if( NULL != hSrcSRS )
    {
        OSRDestroySpatialReference( hSrcSRS );
    }
    if( NULL != hDestSRS )
    {
        OSRDestroySpatialReference( hDestSRS );
    }
    if( NULL != hTransform )
    {
        OCTDestroyCoordinateTransformation( hTransform );
    }
    //_destroyDefaultStyles();
    if( NULL != split_vals )
    {
        delete [] split_vals;
    }

    return;
}		/* -----  end of method OutputWriter::~OutputWriter  ----- */

void OutputWriter::_createDefaultStyles()
{
    //_destroyDefaultStyles(); 
    linewidth = 1.0;
    colors    = new Style*[ NCOLORS ];
    colors[0] = new Style( "blue"  , 255, 255,  0,   0, linewidth );
    colors[1] = new Style( "green" , 255, 0,  255,   0, linewidth );
    colors[2] = new Style( "yellow", 255, 0,  255, 255, linewidth );
    colors[3] = new Style( "orange", 255, 0,  127, 255, linewidth );
    colors[4] = new Style( "red"   , 255, 0,    0, 255, linewidth );

    return;
}
void OutputWriter::_destroyDefaultStyles()
{
    if( NULL != colors )
    {
        for( int i = 0; i < NCOLORS; i ++ )
        {
            delete colors[i];
        }
        delete [] colors;
    }
    colors = NULL;
}

#ifdef EMISSIONS
void OutputWriter::setDustGrid(AsciiGrid<double> &d)
{
    dust = d;
    return;
}		/* -----  end of method OutputWriter::setDustGrid  ----- */
#endif

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

void OutputWriter::setMemDs(GDALDatasetH hSpdMemDs, 
              GDALDatasetH hDirMemDs, 
              GDALDatasetH hDustMemDs)
{
    this->hSpdMemDs  = hSpdMemDs;
    this->hDirMemDs  = hDirMemDs;
    this->hDustMemDs = hDustMemDs;
}

    bool
OutputWriter::write (std::string outputFilename, std::string driver)
{

    if( 0 == driver.compare( "PDF" ) )
    {
        _writePDF(outputFilename);
    }
    else if( 0 == driver.compare( "GTiff" ) )
    {
        /*------------------------------------------*/
        /*  Loop over spd, dir, dust grids          */
        /*------------------------------------------*/
        std::string outFilename;
    
        for(int grid=0; grid<3; grid++){
            outFilename = outputFilename;
            if(grid == 0){
                outFilename.insert(outFilename.find(".tif"), "_spd");
            }
            else if(grid == 1){
                outFilename.insert(outFilename.find(".tif"), "_dir");
            }
#ifdef EMISSIONS
            else if(grid == 2){
                outFilename.insert(outFilename.find(".tif"), "_dust");
            }
#endif

            if(outFilename.find("spd.tif") != outFilename.npos){
                _writeGTiff(outFilename, hSpdMemDs);
            }
            else if(outFilename.find("dir.tif") != outFilename.npos){
                _writeGTiff(outFilename, hDirMemDs);
            }
            else if(outFilename.find("dust.tif") != outFilename.npos){
                 _writeGTiff(outFilename, hDustMemDs);
            }

           //_writeGTiff(outFilename);
        }
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
    const char* pszSrcWkt = (char*) spd.prjString.c_str();
    hSrcSRS = OSRNewSpatialReference( pszSrcWkt );

    hOGRDriver = GDALGetDriverByName( "ESRI Shapefile" );
    if( NULL == hOGRDriver )
    {
        throw std::runtime_error("OutputWriter: Failed to get OGR Memory driver");
    }

    hDataSource = GDALCreate( hOGRDriver, OGR_FILE, 0, 0, 0, GDT_Unknown, NULL );
    if( NULL == hDataSource )
    {
        throw std::runtime_error("OutputWriter: Failed to create OGR Memory datasource");
    }
    
    //Create a new layer for the wind features
    hLayer = GDALDatasetCreateLayer( hDataSource, "Wind Vectors" , hSrcSRS, 
                                 wkbLineString, NULL );
    if( hLayer == NULL )
    {
        throw std::runtime_error("OutputWriter: Failed to create wind vector layer");
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
    return ;

}		/* -----  end of method OutputWriter::createOGRFields  ----- */ 


/* --------------------------------------------------------------------------*/
/** 
 * @brief Creates a GeoPDF file with the given name and opens the dataset
 * for modification.
 * @pre The base raster DEM file is specified
 * @post hTransform contains coordinate transformation (sim SRS -> dem SRS)
 * @post hDstDS is open and writable
 * @post hLayer created with associated speed, direction fields
 * 
 * @Param outputfn desired output filename
 * 
 * @Returns  True if successful. 
 */
/* ----------------------------------------------------------------------------*/
bool OutputWriter::_createPDFFromDEM(std::string outputfn)
{
    const char *pszDriverName = "PDF"; 
    hSrcDS = GDALOpenEx( demFile.c_str(), 0, NULL, NULL, NULL );

    if( NULL == hSrcDS )
    {
        std::cout << "opening base PDF file failed\n";
        throw std::runtime_error("OutputWriter: Failed to open PDF base DEM");
    }

    const char* pszSrcWkt = (char*) spd.prjString.c_str();
    /*
    const char* pszDstWkt = GDALGetProjectionRef( hSrcDS );
    hSrcSRS = OSRNewSpatialReference( pszSrcWkt );
    hDestSRS = OSRNewSpatialReference( pszDstWkt );
    hTransform = OCTNewCoordinateTransformation( hSrcSRS, hDestSRS );
    if( NULL == hTransform )
    {
        throw std::runtime_error("OutputWriter: Failed to create coordinate" \
                                 "transformation for PDF output");
    }
  */
    hDriver = GDALGetDriverByName( "PDF" );

    if( NULL == hDriver )
    {
        throw std::runtime_error("OutputWriter: Failed to get OGR PDF driver");
    }

    hDstDS = GDALCreateCopy( hDriver, outputfn.c_str(), hSrcDS, FALSE, 
                             papszOptions, NULL, NULL );
    if( NULL == hDstDS )
    {
        throw std::runtime_error("OutputWriter: Error creating output file");
    }
    double adfGeoTransform[6];
    adfGeoTransform[0] = spd.get_xllCorner();
    adfGeoTransform[1] = spd.get_cellSize();
    adfGeoTransform[2] = 0;
    adfGeoTransform[3] = spd.get_yllCorner()+(spd.get_nRows()*spd.get_cellSize());
    adfGeoTransform[4] = 0;
    adfGeoTransform[5] = -spd.get_cellSize();
    
    char* pszDstWKT = (char*)spd.prjString.c_str();
    GDALSetProjection(hDstDS, pszDstWKT);
    GDALSetGeoTransform(hDstDS, adfGeoTransform);
    GDALClose(hSrcDS);
    

    hLayer = GDALDatasetCreateLayer( hDstDS, "Wind", NULL, 
                                     wkbLineString, NULL );
    if( NULL == hLayer )
    {
        throw std::runtime_error("OutputWriter: Failed to create wind vector layer");
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
    return true;

}

    bool
OutputWriter::_writePDF (std::string outputfn)
{
    int ncols = spd.get_nCols();
    int nrows = spd.get_nRows();
    double x     = 0,     y = 0;
    double interval;

    //@TODO: move to _createSplits( scaleParam ) (and deleteSplits)
    if( NULL != split_vals )
    {
        delete [] split_vals;
    }
    split_vals = new double[NCOLORS];
    interval = spd.get_maxValue()/(float)NCOLORS;
    for(int i = 0;i < NCOLORS;i++)
    {
        split_vals[i] = i * interval;
    }

    //_createPDFFromDEM( outputfn );
    _createOGRFileWithFields();
    //Add the features to the OGR datasource given by the dir and spd grids
    for( int i = 0; i < nrows; i++ )
    {
        for( int j = 0; j < ncols; j++ )
        {
            OGRFeatureH hFeature = OGR_F_Create( OGR_L_GetLayerDefn( hLayer ) );
            OGRGeometryH hLine   = OGR_G_CreateGeometry( wkbLineString );
            WN_Arrow     arrow;

            spd.get_cellPosition(i, j, &x, &y);
            arrow = WN_Arrow( x, y, spd(i,j), dir(i,j), spd.get_cellSize(),
                              split_vals, NCOLORS);

            OGR_F_SetFieldInteger( hFeature, OGR_F_GetFieldIndex(hFeature, SPEED), 
                                   spd(i,j) );       
            OGR_F_SetFieldInteger( hFeature, OGR_F_GetFieldIndex(hFeature, DIR), 
                                   (int)dir(i,j)+0.5);
            

            arrow.asGeometry( hLine );
            OGR_F_SetGeometry( hFeature, hLine );
            OGR_G_AssignSpatialReference( hLine, hSrcSRS );
            OGR_F_SetStyleString( hFeature, "PEN(c:#FF0000,w:5px)" );

            if( OGR_L_CreateFeature( hLayer, hFeature ) != OGRERR_NONE )
            {
                throw std::runtime_error("OutputWriter: error creating features");
            } 
            OGR_G_DestroyGeometry( hLine );
            OGR_F_Destroy( hFeature );
        }
    }
    OGR_L_SyncToDisk( hLayer );
    // GDALClose( hDataSource );
    // hDataSource = NULL;
    hSrcDS = GDALOpenEx( demFile.c_str(), 0, NULL, NULL, NULL );

    if( NULL == hSrcDS )
    {
        std::cout << "opening base PDF file failed\n";
        throw std::runtime_error("OutputWriter: Failed to open PDF base DEM");
    }
    double adfGeoTransform[6];
    adfGeoTransform[0] = spd.get_xllCorner();
    adfGeoTransform[1] = spd.get_cellSize();
    adfGeoTransform[2] = 0;
    adfGeoTransform[3] = spd.get_yllCorner()+(spd.get_nRows()*spd.get_cellSize());
    adfGeoTransform[4] = 0;
    adfGeoTransform[5] = -spd.get_cellSize();
    
    char* pszDstWKT = (char*)spd.prjString.c_str();
    GDALSetProjection(hSrcDS, pszDstWKT);
    GDALSetGeoTransform(hSrcDS, adfGeoTransform);

    hDriver = GDALGetDriverByName( "PDF" );

    if( NULL == hDriver )
    {
        throw std::runtime_error("OutputWriter: Failed to get OGR PDF driver");
    }

    papszOptions = CSLAddNameValue( papszOptions, "OGR_DATASOURCE", OGR_FILE ); 
    papszOptions = CSLAddNameValue( papszOptions, "OGR_DISPLAY_LAYER_NAMES", "Wind_Vectors");	
    papszOptions = CSLAddNameValue( papszOptions, "LAYER_NAME", demFile.c_str() );
    hDstDS = GDALCreateCopy( hDriver, outputfn.c_str(), hSrcDS, FALSE, 
                             papszOptions, NULL, NULL );
    if( NULL == hDstDS )
    {
        throw std::runtime_error("OutputWriter: Error creating output file");
    }
    
    GDALClose(hSrcDS);
    GDALClose(hDstDS);
    hSrcDS = NULL;
    hDstDS = NULL;
    GDALClose( hDataSource );
    hDataSource = NULL;
 
    //OCTDestroyCoordinateTransformation( hTransform );
    if( NULL != papszOptions )
    {
        CSLDestroy( papszOptions );
        papszOptions = NULL;
    }
    GDALDeleteDataset( hOGRDriver, OGR_FILE );

    return true;
}		/* -----  end of method OutputWriter::_writePDF  ----- */

bool OutputWriter::_writeGTiff (std::string filename, GDALDatasetH &hMemDS)
{
    CPLSetConfigOption( "GDAL_CACHEMAX", "1024" );
    
    int nXSize = spd.get_nCols();
    int nYSize = spd.get_nRows();
    
    double *padfScanline;
    padfScanline = new double[nXSize];
    
    if(runNumber == 0)
    {
        /*------------------------------------------*/
        /* Set dataset metadata                     */
        /*------------------------------------------*/
                  
        double adfGeoTransform[6];
        adfGeoTransform[0] = spd.get_xllCorner();
        adfGeoTransform[1] = spd.get_cellSize();
        adfGeoTransform[2] = 0;
        adfGeoTransform[3] = spd.get_yllCorner()+(spd.get_nRows()*spd.get_cellSize());
        adfGeoTransform[4] = 0;
        adfGeoTransform[5] = -spd.get_cellSize();
        
        char* pszDstWKT = (char*)spd.prjString.c_str();
        GDALSetProjection(hMemDS, pszDstWKT);
        GDALSetGeoTransform(hMemDS, adfGeoTransform);
        
        GDALSetMetadataItem(hMemDS, "TIFFTAG_DATETIME", ninjaTime.c_str(), NULL );
        
        GDALRasterBandH hBand = GDALGetRasterBand( hMemDS, 1 );
        
        GDALSetRasterNoDataValue(hBand, -9999.0);
        GDALSetMetadataItem(hBand, "DT", "0", NULL ); // offset in hours
        

        for(int i=nYSize-1; i>=0; i--)
        {
            for(int j=0; j<nXSize; j++)
            {   
                if(filename.find("spd.tif") != filename.npos){
                    padfScanline[j] = spd.get_cellValue(nYSize-1-i, j);
                }
                else if(filename.find("dir.tif") != filename.npos){
                    padfScanline[j] = dir.get_cellValue(nYSize-1-i, j);
                }
#ifdef EMISSIONS
                else if(filename.find("dust.tif") != filename.npos){
                    padfScanline[j] = dust.get_cellValue(nYSize-1-i, j);
                }
#endif
                else{
                    return false;
                }
                
            }
            GDALRasterIO(hBand, GF_Write, 0, i, nXSize, 1, padfScanline, nXSize,
                             1, GDT_Float64, 0, 0);
        }
    }
    else if(runNumber < maxRunNumber){
        /*------------------------------------------*/
        /*  Add a new band                          */
        /*------------------------------------------*/
        
        CPLErr eErr = GDALAddBand(hMemDS, GDT_Float64, NULL);
        if(eErr != 0){
            return false;
        }
        
        GDALRasterBandH hBand = GDALGetRasterBand( hMemDS, GDALGetRasterCount(hMemDS) );
        
        const char* startTime = GDALGetMetadataItem( hMemDS, "TIFFTAG_DATETIME", NULL );
        
        // calculate hours since startTime 
        std::string s(ninjaTime);
        std::string s0(startTime);
        
        s.erase(s.length()-4); //get rid of tz
        s0.erase(s0.length()-4); //get rid of tz
               
        boost::posix_time::ptime t(boost::posix_time::time_from_string(s));
        boost::posix_time::ptime t0(boost::posix_time::time_from_string(s0));
        
        boost::posix_time::time_duration tdiff = t - t0;
        
        int hdiff = tdiff.hours();

        std::string h(boost::lexical_cast<std::string>(hdiff));
        
        CPLDebug( "GTIFF", "offset in hours, DT = %s", h.c_str() );
        
        GDALSetRasterNoDataValue(hBand, -9999.0);
        
        GDALSetMetadataItem( hBand, "DT", h.c_str(), NULL ); // offset in hours since first band

        for(int i=nYSize-1; i>=0; i--)
        {
            for(int j=0; j<nXSize; j++)
            {
                if(filename.find("spd.tif") != filename.npos){
                    padfScanline[j] = spd.get_cellValue(nYSize-1-i, j);
                }
                else if(filename.find("dir.tif") != filename.npos){
                    padfScanline[j] = dir.get_cellValue(nYSize-1-i, j);
                }
#ifdef EMISSIONS
                else if(filename.find("dust.tif") != filename.npos){
                    padfScanline[j] = dust.get_cellValue(nYSize-1-i, j);
                }
#endif
                else{
                    return false;
                }
            }
            GDALRasterIO(hBand, GF_Write, 0, i, nXSize, 1, padfScanline, nXSize,
                            1, GDT_Float64, 0, 0); 
        }
    }
    else{
        /*------------------------------------------*/
        /*  Write the tif to disk                   */
        /*------------------------------------------*/
        //get start time
        const char* startTime = GDALGetMetadataItem(hMemDS, "TIFFTAG_DATETIME", NULL);

        CPLErr eErr = GDALAddBand(hMemDS, GDT_Float64, NULL);
        if(eErr != 0){
            return false;
        }
        
        //copy MEM to GTiff format
        hDriver = GDALGetDriverByName( "GTiff" );

        // write current grid to last band in hDstDS        
        GDALRasterBandH hBand = GDALGetRasterBand( hMemDS, GDALGetRasterCount(hMemDS) );
  
        // calculate hours since startTime 
        std::string s(ninjaTime);
        std::string s0(startTime);
        
        s.erase(s.length()-4); //get rid of tz
        s0.erase(s0.length()-4); //get rid of tz
               
        boost::posix_time::ptime t(boost::posix_time::time_from_string(s));
        boost::posix_time::ptime t0(boost::posix_time::time_from_string(s0));
        
        boost::posix_time::time_duration tdiff = t - t0;
        
        int hdiff = tdiff.hours();

        std::string h(boost::lexical_cast<std::string>(hdiff));
        
        CPLDebug( "GTIFF", "offset in hours, DT = %s", h.c_str() );
        
        GDALSetRasterNoDataValue(hBand, -9999.0);
        
        GDALSetMetadataItem( hBand, "DT", h.c_str(), NULL ); // offset in hours since first band

        for(int i=nYSize-1; i>=0; i--)
        {
            for(int j=0; j<nXSize; j++)
            {
                if(filename.find("spd.tif") != filename.npos){
                    padfScanline[j] = spd.get_cellValue(nYSize-1-i, j);
                }
                else if(filename.find("dir.tif") != filename.npos){
                    padfScanline[j] = dir.get_cellValue(nYSize-1-i, j);
                }
#ifdef EMISSIONS
                else if(filename.find("dust.tif") != filename.npos){
                    padfScanline[j] = dust.get_cellValue(nYSize-1-i, j);
                }
#endif
                else{
                    return false;
                }
            }
            GDALRasterIO(hBand, GF_Write, 0, i, nXSize, 1, padfScanline, nXSize,
                            1, GDT_Float64, 0, 0); 
        }
        
        
        GDALDriverH hGtiffDriver = GDALGetDriverByName( "GTiff" );
        papszOptions = CSLAddString( papszOptions, "INTERLEAVE=BAND");	
        papszOptions = CSLAddString( papszOptions, "BIGTIFF=YES" );        
        hDstDS = GDALCreateCopy(hGtiffDriver, filename.c_str(), hMemDS, FALSE, papszOptions, NULL, NULL);
        
        //close MEM dataset
        if( hMemDS != NULL ){
            GDALClose( hMemDS );
            hMemDS = NULL;
        }
    
        //close GTiff dataset
        if( hDstDS != NULL ){
            GDALClose( hDstDS );
            hDstDS = NULL;
        }
    }

    delete [] padfScanline;

    return true;
}		/* -----  end of method OutputWriter::_writeGTiff  ----- */
