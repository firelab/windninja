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

const char * OutputWriter::SPEED     = "speed";
const char * OutputWriter::DIR       = "dir";
const char * OutputWriter::AV_DIR    = "AV_dir";
const char * OutputWriter::AM_DIR    = "AM_dir";
const char * OutputWriter::QGIS_DIR  = "QGIS_dir";
const char * OutputWriter::OGR_FILE  = "/tmp/out.shp";
const char * OutputWriter::LEGEND_FILE = "/tmp/legend.bmp";


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
    linewidth    = 1.0;

    _createDefaultStyles();
    
}  /* -----  end of method OutputWriter::OutputWriter  (constructor)  ----- */


OutputWriter::~OutputWriter ()
{
    
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

    _destroyDefaultStyles();
    _deleteSplits();
    _destroyOptions();
    _closeOGRFile();
    _closeDataSets();
    return;
}		/* -----  end of method OutputWriter::~OutputWriter  ----- */

void OutputWriter::_createDefaultStyles()
{
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

void OutputWriter::setLineWidth( const float w )
{
   linewidth = ( w >= 0.0f ) ? w : -w; 
   if( areEqual( linewidth, 0.0f ) )
   {
       linewidth = 1.0f;
   }
   _destroyDefaultStyles();
   _createDefaultStyles();
   
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

std::string OutputWriter::_getStyleFromSpeed( const double & spd )
{
    std::string style = "none";
    
    for ( int i = 1; i < NCOLORS; ++i )
    {
        if( spd <= split_vals[i] )
        {
            style = colors[i-1]->asOGRStyleString();
            break;
        }
    }
    if( "none" ==  style)
    {
        style = colors[NCOLORS - 1]->asOGRStyleString();
    }
    return style;
}

void OutputWriter::_closeDataSets()
{
    if( NULL != hSrcDS )
    {
        GDALClose(hSrcDS);
        hSrcDS = NULL;
    }
    if( NULL != hDstDS )
    {
        GDALClose(hDstDS);
        hDstDS = NULL;
    }
    _closeOGRFile(); 

}

void OutputWriter::_destroyOptions()
{
    if( NULL != papszOptions )
    {
        CSLDestroy( papszOptions );
        papszOptions = NULL;
    }
}

void OutputWriter::_closeOGRFile()
{
    if( NULL != hDataSource )
    {
        OGR_DS_Destroy( hDataSource );
        hDataSource = NULL;
    }
}

void OutputWriter::_deleteSplits()
{
    if( NULL != split_vals )
    {
        delete [] split_vals;
        split_vals = NULL;
    }
}

void OutputWriter::_createSplits()
{
    _deleteSplits(); 
    double interval = 0.0;

    split_vals = new double[NCOLORS];
    interval = spd.get_maxValue()/(float)NCOLORS;
    for(int i = 0;i < NCOLORS;i++)
    {
        split_vals[i] = i * interval;
    }


}
bool OutputWriter::_createLegend()
{
	//make bitmap
	int legendWidth = LGND_WIDTH;
	int legendHeight = LGND_HEIGHT;
	BMP legend;

	std::string legendStrings[NCOLORS];
	ostringstream os;

	double maxxx = spd.get_maxValue();

	for(int i = 0;i < NCOLORS; i++)
	{
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
		if(i == 0)
			os << split_vals[NCOLORS - 1] << " - " << maxxx;
		else if(i == NCOLORS)
			os << "0.00 - " << split_vals[1] - 0.01;
		else if(i != 0)
			os << split_vals[NCOLORS - i - 1] << " - " << split_vals[NCOLORS - i] - 0.01;

        legendStrings[i] = os.str();
		os.str("");
	}
	legend.SetSize(legendWidth,legendHeight);
	legend.SetBitDepth(8);

    //black legend
	for(int i = 0;i < legendWidth;i++)
	{
		for(int j = 0;j < legendHeight;j++)
		{
			legend(i,j)->Alpha = 0;
			legend(i,j)->Blue = 0;
			legend(i,j)->Green = 0;
			legend(i,j)->Red = 0;
		}
    }
    
	//for white text
	RGBApixel white;
	white.Red = 255;
	white.Green = 255;
	white.Blue = 255;
	white.Alpha = 0;

	RGBApixel lcolors[NCOLORS];
	//RGBApixel red, orange, yellow, green, blue;
	lcolors[0].Red = 255;
    lcolors[0].Green = 0;
    lcolors[0].Blue = 0;
    lcolors[0].Alpha = 0;

    lcolors[1].Red = 255;
    lcolors[1].Green = 127;
    lcolors[1].Blue = 0;
    lcolors[1].Alpha = 0;

    lcolors[2].Red = 255;
    lcolors[2].Green = 255;
    lcolors[2].Blue = 0;
    lcolors[2].Alpha = 0;

    lcolors[3].Red = 0;
    lcolors[3].Green = 255;
    lcolors[3].Blue = 0;
    lcolors[3].Alpha = 0;

    lcolors[4].Red = 0;
    lcolors[4].Green = 0;
    lcolors[4].Blue = 255;
    lcolors[4].Alpha = 0;

	int arrowLength = 40;	//pixels;
	int arrowHeadLength = 10; // pixels;
	int textHeight = 12;	//pixels- 10 for maximum speed of "999.99 - 555.55";
							//12 for normal double digits
	if(split_vals[NCOLORS-1] >= 100)
		textHeight = 10;
	int titleTextHeight = int(1.2 * textHeight);
	int titleX, titleY;

    int x1, x2, x3, x4;
	double x;
	int y1, y2, y3, y4;
	double y;

	int textX;
	int textY;

	x = 0.05;
	y = 0.30;


	titleX = x * legendWidth;
	titleY = (y / 3) * legendHeight;

    
    //TODO: Add support for configuring wind speed units
    PrintString(legend,"Wind Speed (mph)", titleX, titleY, titleTextHeight, white);
	for(int i = 0;i < NCOLORS;i++)
	{
		x1 = int(legendWidth * x);
		x2 = x1 + arrowLength;
		y1 = int(legendHeight * y);
		y2 = y1;

		x3 = x2 - arrowHeadLength;
		y3 = y2 + arrowHeadLength;

		x4 = x2 - arrowHeadLength;
		y4 = y2 - arrowHeadLength;

		textX = x2 + 10;
		textY = y2 - int(textHeight * 0.5);


		DrawLine(legend, x1, y1, x2, y2, lcolors[i]);
		DrawLine(legend, x2, y2, x3, y3, lcolors[i]);
		DrawLine(legend, x2, y2, x4, y4, lcolors[i]);

		PrintString(legend, legendStrings[i].c_str(), textX, textY, textHeight, white);


		y += 0.15;
	}

	legend.WriteToFile( LEGEND_FILE );

    return true;

}

void OutputWriter::_destroyLegend()
{
    GDALDriverH hLegendDrv = GDALGetDriverByName( "BMP" );
    GDALDeleteDataset( hLegendDrv, LEGEND_FILE );
    return;
}

void OutputWriter::_openSrcDataSet()
{
    hSrcDS = GDALOpen( demFile.c_str(), GA_ReadOnly );
    if( NULL == hSrcDS )
    {
        throw std::runtime_error("OutputWriter: Failed to open PDF base DEM");
    }
    return;
}

/* --------------------------------------------------------------------------*/
/** 
 * @brief Creates an OGR datasource for holding vector features
 *
 * @pre base DEM file is specified
 * @pre speed grid is instantiated
 * 
 * @post hTransform contains coordinate transformation from speed grid to raster SRS
 * @post The source raster datasource is open
 * @post The OGR datasource is populated with features from simulation data
 */
/* ----------------------------------------------------------------------------*/
    void
OutputWriter::_createOGRFile()
{
    int ncols = spd.get_nCols();
    int nrows = spd.get_nRows();
    double x  = 0, y = 0;

    _openSrcDataSet(); 

    const char* pszSrcWkt = (char*) spd.prjString.c_str();
    const char* pszDstWkt = GDALGetProjectionRef( hSrcDS );
    hSrcSRS = OSRNewSpatialReference( pszSrcWkt );
    hDestSRS   = OSRNewSpatialReference( pszDstWkt );
    hTransform = OCTNewCoordinateTransformation( hSrcSRS, hDestSRS );

    GDALGetGeoTransform( hSrcDS, adfGeoTransform );
    GDALClose( hSrcDS );

    if( NULL == hTransform )
    {
        throw std::runtime_error("OutputWriter: Failed to create coordinate" \
                                 "transformation for PDF output");
    }
    hOGRDriver = OGRGetDriverByName( "ESRI Shapefile" );
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
    hLayer = OGR_DS_CreateLayer( hDataSource, "Wind Vectors" , hDestSRS, 
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

    hFieldDefn = OGR_Fld_Create( "OGR_STYLE", OFTString );
    OGR_Fld_SetWidth( hFieldDefn, 32 );
    if( OGRERR_NONE != OGR_L_CreateField( hLayer, hFieldDefn, TRUE ) )
    {
        throw std::runtime_error("OutputWriter: Creating SPEED field failed");
    }
    OGR_Fld_Destroy(hFieldDefn);

    //Add the features to the OGR datasource given by the dir and spd grids
    std::string style;
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
            OGR_G_Transform( hLine, hTransform );
            OGR_F_SetGeometry( hFeature, hLine );
            
            style = _getStyleFromSpeed( spd(i,j) );
            
            OGR_F_SetStyleString( hFeature, style.c_str() ); 
            OGR_F_SetFieldString( hFeature, OGR_F_GetFieldIndex(hFeature, "OGR_STYLE"), 
                                  style.c_str() );


            if( OGR_L_CreateFeature( hLayer, hFeature ) != OGRERR_NONE )
            {
                throw std::runtime_error("OutputWriter: error creating features");
            } 
            OGR_G_DestroyGeometry( hLine );
            OGR_F_Destroy( hFeature );
        }
    }
    
    _closeOGRFile();

    return ;

}		/* -----  end of method OutputWriter::createOGRFields  ----- */ 


/* --------------------------------------------------------------------------*/
/** 
 * @brief Creates a new PDF file from the base DEM and simulation output
 * 
 * @Param outputfn - specifies the name of the output PDF file
 *
 * @pre OGR datasource and source raster are created/opened
 * @post All datasources are closed, OGR datasource is deleted from system
 * @post Output PDF is created
 * 
 * @Returns True is successful. 
 */
/* ----------------------------------------------------------------------------*/
    bool
OutputWriter::_writePDF (std::string outputfn)
{
    _createSplits();
    _createOGRFile();
    _createLegend();
    _openSrcDataSet();

    std::string tf_logo_path = FindDataPath( "tf-logo.png" );
    unsigned int out_x_size = GDALGetRasterXSize( hSrcDS );
    unsigned int out_y_size = GDALGetRasterYSize( hSrcDS );
    hDriver = GDALGetDriverByName( "PDF" );

    if( NULL == hDriver )
    {
        throw std::runtime_error("OutputWriter: Failed to get OGR PDF driver");
    }

    const char * EXTRA_IMG_FRMT = "%s,%d,%d,%f";
    std::string extra_img_lgnd = CPLSPrintf( EXTRA_IMG_FRMT, LEGEND_FILE, 
                                             0, out_y_size-LGND_HEIGHT, 1.0f );
    std::string extra_img_logo = CPLSPrintf( EXTRA_IMG_FRMT, tf_logo_path.c_str(), 0, 0, 1.0f );
    std::string extra_imgs     = extra_img_lgnd + "," + extra_img_logo;

    papszOptions = CSLAddNameValue( papszOptions, "OGR_DATASOURCE", OGR_FILE ); 
    papszOptions = CSLAddNameValue( papszOptions, "OGR_DISPLAY_LAYER_NAMES", "Wind_Vectors");	
    papszOptions = CSLAddNameValue( papszOptions, "LAYER_NAME", demFile.c_str() );
    papszOptions = CSLAddNameValue( papszOptions, "EXTRA_IMAGES", extra_imgs.c_str() );
    papszOptions = CSLAddNameValue( papszOptions, "TILED", "YES" );
    papszOptions = CSLAddNameValue( papszOptions, "PREDICTOR", "2" );
    hDstDS = GDALCreateCopy( hDriver, outputfn.c_str(), hSrcDS, FALSE, 
                             papszOptions, NULL, NULL );
    if( NULL == hDstDS )
    {
        throw std::runtime_error("OutputWriter: Error creating output file");
    }
    
    _closeDataSets();
    _destroyOptions();
    _destroyLegend();
    
    OGR_Dr_DeleteDataSource( hOGRDriver, OGR_FILE );

    return true;
}		/* -----  end of method OutputWriter::_writePDF  ----- */

bool OutputWriter::_writeGTiff (std::string filename, GDALDatasetH &hMemDS)
{
    CPLSetConfigOption( "GDAL_CACHEMAX", "1024" );
    
    int nXSize = spd.get_nCols();
    int nYSize = spd.get_nRows();
    
    double *padfScanline;
    padfScanline = new double[nXSize];
    CPLErr eErr = CE_None;
    
    if(runNumber == 0)
    {
        /*------------------------------------------*/
        /* Set dataset metadata                     */
        /*------------------------------------------*/
                  
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
            eErr = GDALRasterIO(hBand, GF_Write, 0, i, nXSize, 1, padfScanline, nXSize,
                                1, GDT_Float64, 0, 0);
            assert( eErr == CE_None );
        }
    }
    else if(runNumber < maxRunNumber){
        /*------------------------------------------*/
        /*  Add a new band                          */
        /*------------------------------------------*/
        
        eErr = GDALAddBand(hMemDS, GDT_Float64, NULL);
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

        CPLErr eErr = CE_None;
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
            eErr = GDALRasterIO(hBand, GF_Write, 0, i, nXSize, 1, padfScanline, nXSize,
                                1, GDT_Float64, 0, 0); 
            assert( eErr == CE_None );
        }
    }
    else{
        /*------------------------------------------*/
        /*  Write the tif to disk                   */
        /*------------------------------------------*/
        //get start time
        const char* startTime = GDALGetMetadataItem(hMemDS, "TIFFTAG_DATETIME", NULL);

        eErr = GDALAddBand(hMemDS, GDT_Float64, NULL);
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

        CPLErr eErr;

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
            eErr = GDALRasterIO(hBand, GF_Write, 0, i, nXSize, 1, padfScanline, nXSize,
                                1, GDT_Float64, 0, 0); 
            assert( eErr == CE_None );
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
}

