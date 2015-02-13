/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Executable for converting xyz output from OpenFOAM 
 * Author:   Natalie Wagenbrenner <nwagenbrenner@fs.fed.us>
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
 
#include "ninja_init.h"
#include "ninjafoam.h"

void Usage()
{ 
    printf("convert_output input_file output_file dem_file\n");
    exit(1);
}

int ReplaceKey(std::string &s, std::string k, std::string v)
{
    int i, n;
    i = s.find(k);
    if( i != std::string::npos )
    {
        n = k.length();
        s.replace(i, n, v);
        return TRUE;
    }
    else
        return FALSE;
}

int ReplaceKeys(std::string &s, std::string k, std::string v, int n)
{
    int rc = FALSE;
    int c = 0;
    do
    {
        rc = ReplaceKey(s, k, v);
        c++;
    } while(rc && c < n);
    return rc;
}

static int TransformGeoToPixelSpace( double *adfInvGeoTransform, double dfX,
                                     double dfY, int *iPixel, int *iLine )
{
    *iPixel = (int) floor( adfInvGeoTransform[0] +
                           adfInvGeoTransform[1] * dfX +
                           adfInvGeoTransform[2] * dfY );
    *iLine  = (int) floor( adfInvGeoTransform[3] +
                           adfInvGeoTransform[4] * dfX +
                           adfInvGeoTransform[5] * dfY );
    return NINJA_SUCCESS;
}

int WriteOutputFiles(std::string inputFile, std::string outFile, std::string dem_filename)
{
    /*-------------------------------------------------------------------*/
    /* convert output from xyz to speed and direction                    */
    /*-------------------------------------------------------------------*/

    AsciiGrid<double> foamU, foamV;
    
    /*
    ** Note that fin is a normal FILE used with VSI*, not VSI*L.  This is for
    ** the VSIFGets functions.
    */
    FILE *fin;
    VSILFILE *fout, *fvrt;
    char buf[512];
    int rc;
    const char *pszVrtFile;
    const char *pszVrt;
    const char *pszVrtMem;
    const char *pszMem;
    std::string s;

    pszMem = CPLSPrintf( "output.raw" );
    pszVrtMem = CPLStrdup( CPLSPrintf( "output.vrt") );

    fin = VSIFOpen( inputFile.c_str(), "r" ); 
    fout = VSIFOpenL( pszMem, "w" ); 
    fvrt = VSIFOpenL( pszVrtMem, "w" ); 
    pszVrtFile = CPLSPrintf( "CSV:%s", pszMem );

    pszVrt = CPLSPrintf( NINJA_FOAM_OGR_VRT, "output", pszVrtFile, "output" );

    VSIFWriteL( pszVrt, strlen( pszVrt ), 1, fvrt );
    VSIFCloseL( fvrt );
    
    buf[0] = '\0';
    /*
    ** eat the first line
    */
    VSIFGets( buf, 512, fin );
    /*
    ** fix the header
    */
    VSIFGets( buf, 512, fin );
    s = buf;

    ReplaceKeys( s, "#", "", 1 );
    ReplaceKeys( s, "  ", "", 1 );
    ReplaceKeys( s, "  ", ",", 5 );
    ReplaceKeys( s, "  ", "", 1 );
    VSIFWriteL( s.c_str(), s.size(), 1, fout );
    
    /*
    ** sanitize the data.
    */
    while( VSIFGets( buf, 512, fin ) != NULL )
    {
        s = buf;
        ReplaceKeys( s, " ", ",", 5 );
        VSIFWriteL( s.c_str(), s.size(), 1, fout );
    }
    VSIFClose( fin );
    VSIFCloseL( fout );
    
    /*-------------------------------------------------------------------*/
    /* sample cloud                                                      */
    /*-------------------------------------------------------------------*/
    
    OGRDataSourceH hDS = NULL;
    OGRLayerH hLayer = NULL;
    OGRFeatureH hFeature = NULL;
    OGRFeatureDefnH hFeatDefn = NULL;
    OGRGeometryH hGeometry = NULL;
    GDALDatasetH hGriddedDS = NULL;

    double adfGeoTransform[6], adfInvGeoTransform[6];

    hDS = OGROpen( pszVrtMem, FALSE, NULL );
    if( hDS == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Invalid in memory datasource in NinjaFoam" );
        return NINJA_E_FILE_IO;
    }

    hLayer = OGR_DS_GetLayer( hDS, 0 );
    if( hLayer == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to extract a valid layer for NinjaFoam resampling" );
        return NINJA_E_OTHER;
    }
    double dfX, dfY, dfU, dfV;
    int nPoints, nXSize, nYSize;
    double dfXMax, dfYMax, dfXMin, dfYMin, dfCellSize;
    
    /* get some info from the DEM */
    GDALDatasetH hDem;
    hDem = GDALOpen(dem_filename.c_str(), GA_ReadOnly);
    if(hDem == NULL)
    {
        fprintf(stderr, "Failed to open DEM\n");
        return 1;
    }
    AsciiGrid<double> dem; 
    GDAL2AsciiGrid( (GDALDataset *)hDem, 1, dem );
    GDALClose(hDem);

    dfXMin = dem.get_xllCorner();
    dfXMax = dem.get_xllCorner() + dem.get_xDimension();
    dfYMin = dem.get_yllCorner();
    dfYMax = dem.get_yllCorner() + dem.get_yDimension();
    dfCellSize = dem.get_cellSize();
    
    nPoints = OGR_L_GetFeatureCount( hLayer, TRUE );

    /* Get DEM/output specs */
    nXSize = dem.get_nCols();
    nYSize = dem.get_nRows();

    GDALDriverH hDriver = GDALGetDriverByName( "GTiff" );
    const char *pszGridFilename;
    pszGridFilename = CPLStrdup( CPLSPrintf( "foam.tif" ) );
    hGriddedDS = GDALCreate( hDriver, pszGridFilename, nXSize, nYSize, 2,
                             GDT_Float64, NULL );
    GDALRasterBandH hUBand, hVBand;
    hUBand = GDALGetRasterBand( hGriddedDS, 1 );
    hVBand = GDALGetRasterBand( hGriddedDS, 2 );
    GDALSetRasterNoDataValue( hUBand, -9999 );
    GDALSetRasterNoDataValue( hVBand, -9999 );

    /* Set the projection from the DEM */
    rc = GDALSetProjection( hGriddedDS, dem.prjString.c_str() );

    adfGeoTransform[0] = dfXMin;
    adfGeoTransform[1] = dfCellSize;
    adfGeoTransform[2] = 0;
    adfGeoTransform[3] = dfYMax;
    adfGeoTransform[4] = 0;
    adfGeoTransform[5] = -dfCellSize;

    GDALSetGeoTransform( hGriddedDS, adfGeoTransform );
    rc = GDALInvGeoTransform( adfGeoTransform, adfInvGeoTransform );

    int i = 0;
    int nUIndex, nVIndex;
    int nPixel, nLine;
    OGR_L_ResetReading( hLayer );
    hFeatDefn = OGR_L_GetLayerDefn( hLayer );
    nUIndex = OGR_FD_GetFieldIndex( hFeatDefn, "U" );
    nVIndex = OGR_FD_GetFieldIndex( hFeatDefn, "V" );
    while( (hFeature = OGR_L_GetNextFeature( hLayer )) != NULL )
    {
        hGeometry = OGR_F_GetGeometryRef( hFeature );
        dfX = OGR_G_GetX( hGeometry, 0 );
        dfY = OGR_G_GetY( hGeometry, 0 );
        dfU = OGR_F_GetFieldAsDouble( hFeature, nUIndex );
        dfV = OGR_F_GetFieldAsDouble( hFeature, nVIndex );
        TransformGeoToPixelSpace( adfInvGeoTransform, dfX, dfY, &nPixel, &nLine );
        GDALRasterIO( hUBand, GF_Write, nPixel, nLine, 1, 1, &dfU,
                      1, 1, GDT_Float64, 0, 0 );
        GDALRasterIO( hVBand, GF_Write, nPixel, nLine, 1, 1, &dfV,
                      1, 1, GDT_Float64, 0, 0 );
        i++;
    }
    OGR_DS_Destroy( hDS );

    GDAL2AsciiGrid( (GDALDataset *)hGriddedDS, 1, foamU );
    GDAL2AsciiGrid( (GDALDataset *)hGriddedDS, 2, foamV );

    AsciiGrid<double> foamSpd( foamU );
    AsciiGrid<double> foamDir( foamU );

    for(int i=0; i<foamU.get_nRows(); i++)
    {
        for(int j=0; j<foamU.get_nCols(); j++)
        {
            wind_uv_to_sd(foamU(i,j), foamV(i,j), &(foamSpd)(i,j), &(foamDir)(i,j));
        }
    }

    /*-------------------------------------------------------------------*/
    /* write output files                                                */
    /*-------------------------------------------------------------------*/
    
    /* write asc files */
	foamDir.write_Grid(outFile, 0);
	foamSpd.write_Grid(outFile, 2);

	/* write kmz files */
    KmlVector ninjaKmlFiles;
    
    ninjaKmlFiles.setKmlFile(outFile + ".kml");
	ninjaKmlFiles.setKmzFile(outFile + ".kmz");
	ninjaKmlFiles.setDemFile(dem_filename);

	ninjaKmlFiles.setLegendFile(outFile + ".bmp");
	//ninjaKmlFiles.setDateTimeLegendFile("out_kml_time.bmp", "ninjatime.bmp");
	ninjaKmlFiles.setSpeedGrid(foamSpd, velocityUnits::metersPerSecond);
	ninjaKmlFiles.setDirGrid(foamDir);

    ninjaKmlFiles.setLineWidth(1.0);
	//ninjaKmlFiles.setTime("ninjatime");

	if(ninjaKmlFiles.writeKml(KmlVector::equal_interval))
	{
		if(ninjaKmlFiles.makeKmz())
			ninjaKmlFiles.removeKmlFile();
	}
	
	/*-------------------------------------------------------------------*/
    /* clean up                                                          */
    /*-------------------------------------------------------------------*/
	
	if(pszMem){
        VSIUnlink(pszMem);
    }
    if(pszVrtMem){
        VSIUnlink(pszVrtMem);
    }
    if(pszGridFilename){
        VSIUnlink(pszGridFilename);
    }

	return true;
}

int main( int argc, char* argv[] )
{
    NinjaInitialize();
    /*  parse input arguments  */
    if( argc != 4 )
    {
        cout << "Invalid arguments!" << endl;
        cout << "convert_output [input filename] [output filename] [dem filename]" << endl;
        return 1;
    }
    std::string input_file = std::string( argv[1] );
    std::string output_file = std::string( argv[2] );
    std::string dem_filename = std::string( argv[3] );
    
    cout<<"input filename = "<<input_file<<endl;
    cout<<"output filename = "<<output_file<<endl;
    cout<<"dem filename = "<<dem_filename<<endl;
    cout<<"Writing files..."<<endl;

    //Convert an xyz output file from OpenFOAM and convert to kmz AND raster with name 'output_file'
    WriteOutputFiles( input_file, output_file, dem_filename );
    
    return 0;
}

