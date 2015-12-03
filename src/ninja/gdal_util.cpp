/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  utility functions for gdaldatasets
 * Author:   Kyle Shannon <ksshannon@gmail.com>
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

#include "gdal_util.h"

#include "ninja_conv.h"

/** Fetch the center of a domain.
 * Fetch the center of a domain from any valid GDAL dataset
 * @param poDS a pointer to a valid GDAL Dataset
 * @param centerLonLat a pointer to a double size of double * 2
 * @return true on valid population of the double*
 */
bool GDALGetCenter( GDALDataset *poDS, double *centerLonLat )
{
    char* pszPrj;
    double adfGeoTransform[6];
    int xSize, ySize;
    double xCenter, yCenter;
    double lon, lat;

    OGRSpatialReference oSourceSRS, oTargetSRS;
    OGRCoordinateTransformation *poCT;

    if( poDS == NULL )
	return false;

    xSize = poDS->GetRasterXSize( );
    ySize = poDS->GetRasterYSize( );

    if( poDS->GetGeoTransform( adfGeoTransform ) != CE_None )
	return false;

    if( poDS->GetProjectionRef(  ) == NULL )
	return false;
    else
	pszPrj = (char*)poDS->GetProjectionRef();

    oSourceSRS.importFromWkt( &pszPrj );
    oTargetSRS.SetWellKnownGeogCS( "WGS84" );

    poCT = OGRCreateCoordinateTransformation( &oSourceSRS, &oTargetSRS );
    if( poCT == NULL )
	return false;

    xCenter = xSize / 2;
    yCenter = ySize / 2;

    lon = adfGeoTransform[0] + adfGeoTransform[1] * xCenter
	+ adfGeoTransform[2] * yCenter;

    lat = adfGeoTransform[3] + adfGeoTransform[4] * xCenter
	+ adfGeoTransform[5] * yCenter;

    if( !poCT->Transform( 1, &lon, &lat ) ) {
	OGRCoordinateTransformation::DestroyCT( poCT );
	return false;
    }

    centerLonLat[0] = lon;
    centerLonLat[1] = lat;

    OGRCoordinateTransformation::DestroyCT( poCT );
    return true;
}

/** Fetch the longitude/latitude bounds of an image
 * @param poDS a pointer to a valid GDAL Dataset
 * @param boundsLonLat a pointer to a double[4] n, e, s, w order
 * @return true on valid population of boundsLonLat in n, e, s, w order
 */
bool GDALGetBounds( GDALDataset *poDS, double *boundsLonLat )
{
    char* pszPrj;
    double adfGeoTransform[6];
    int xSize, ySize;
    double nLat, eLon, sLat, wLon;

    OGRSpatialReference oSourceSRS, oTargetSRS;
    OGRCoordinateTransformation *poCT;

    if( poDS == NULL )
	return false;

    xSize = poDS->GetRasterXSize ();
    ySize = poDS->GetRasterYSize();

    poDS->GetGeoTransform( adfGeoTransform );

    if( poDS->GetProjectionRef() == NULL )
	return false;
    else
	pszPrj = (char*)poDS->GetProjectionRef();

    oSourceSRS.importFromWkt( &pszPrj );
    oTargetSRS.SetWellKnownGeogCS( "WGS84" );

    poCT = OGRCreateCoordinateTransformation( &oSourceSRS, &oTargetSRS );
    if( poCT == NULL )
	return false;

    nLat = adfGeoTransform[3] + adfGeoTransform[4] * 0 + adfGeoTransform[5] * 0;
    eLon = adfGeoTransform[0] + adfGeoTransform[1] * xSize + adfGeoTransform[2] * 0;
    sLat = adfGeoTransform[3] + adfGeoTransform[4] * 0 + adfGeoTransform[5] * ySize;
    wLon = adfGeoTransform[0] + adfGeoTransform[1] * 0 + adfGeoTransform[2] * 0;

    if( !poCT->Transform( 1, &eLon, &nLat ) ) {
	OGRCoordinateTransformation::DestroyCT( poCT );
	return false;
    }

    boundsLonLat[0] = nLat;
    boundsLonLat[1] = eLon;

    if( !poCT->Transform( 1, &wLon, &sLat ) ) {
	OGRCoordinateTransformation::DestroyCT( poCT );
	return false;
    }

    boundsLonLat[2] = sLat;
    boundsLonLat[3] = wLon;

    OGRCoordinateTransformation::DestroyCT( poCT );
    return true;
}

/** Test the spatial reference of an image.
 * @param poDS a pointer to a valid GDALDataset
 * @return true if the spatial reference is valid
 */
bool GDALTestSRS( GDALDataset *poDS )
{
    char* pszPrj;
    OGRSpatialReference oSourceSRS, oTargetSRS;
    OGRCoordinateTransformation *poCT;

    if( poDS == NULL )
	return false;

    if( poDS->GetProjectionRef() == NULL )
	return false;
    else
	pszPrj = (char*) poDS->GetProjectionRef();

    oSourceSRS.importFromWkt( &pszPrj );
    oTargetSRS.SetWellKnownGeogCS( "WGS84" );

    poCT = OGRCreateCoordinateTransformation( &oSourceSRS, &oTargetSRS );

    if( poCT == NULL )
	return false;
    OGRCoordinateTransformation::DestroyCT( poCT );
    return true;
}

/** Check for no data values in a band for an image
 * @param poDS a pointer to a valid GDALDataset
 * @param band an integer representation of which band in the image
 * @return true if the band contains any no data values
 * @warning May be cpu intensive on large images
 */
bool GDALHasNoData( GDALDataset *poDS, int band )
{
    bool hasNDV = false;
    //check if poDS is NULL #lm
    int ncols = poDS->GetRasterXSize();
    int nrows = poDS->GetRasterYSize();

    double nDV;

    GDALRasterBand *poBand = poDS->GetRasterBand( band );
    if( poBand == NULL )
	return false;

    int pbSuccess = 0;
    nDV = poBand->GetNoDataValue( &pbSuccess );
    if( pbSuccess == false )
	nDV = -9999.0;

    double *padfScanline;
    padfScanline = new double[ncols];
    for( int i = 0;i < nrows;i++ ) {
	poBand->RasterIO( GF_Read, 0, i, ncols, 1, padfScanline, ncols, 1,
			  GDT_Float64, 0, 0 );
	for( int j = 0;j < ncols;j++ ) {
	    if( CPLIsEqual( (float)padfScanline[j], (float)nDV ) )
            {
		hasNDV = true;
                goto done;
            }
	}
    }
done:
    delete[] padfScanline;

    return hasNDV;
}

bool GDAL2AsciiGrid( GDALDataset *poDS, int band, AsciiGrid<double> &grid )
{

    if( poDS == NULL )
	return false;
    //get some info from the ds

    int nXSize = poDS->GetRasterXSize();
    int nYSize = poDS->GetRasterYSize();

    double adfGeoTransform[6];
    poDS->GetGeoTransform( adfGeoTransform );

    //find llcorner
    double adfCorner[2];
    adfCorner[0] = adfGeoTransform[0];
    adfCorner[1] = adfGeoTransform[3] + ( nYSize * adfGeoTransform[5] );

    //check for non-square cells
    /*
    if( abs( adfGeoTransform[1] ) - abs( adfGeoTransform[5] ) > 0.0001 )
	CPLDebug( "GDAL2AsciiGrid", "Non-uniform cells" );
    */

    GDALRasterBand *poBand;
    poBand = poDS->GetRasterBand( band );

    //not used yet
    //GDALDataType eDataType = poBand->GetRasterDataType();

    int pbSuccess = 0;
    double dfNoData = poBand->GetNoDataValue( &pbSuccess );
    if( pbSuccess == false )
	dfNoData = -9999.0;

    //reallocate all memory and set header info
    grid.set_headerData( nXSize, nYSize,
			 adfCorner[0], adfCorner[1],
			 adfGeoTransform[1], dfNoData, dfNoData );
    //set the data
    double *padfScanline;
    padfScanline = new double[nXSize];
    for(int i = nYSize - 1;i >= 0;i--) {

	poBand->RasterIO(GF_Read, 0, i, nXSize, 1, padfScanline, nXSize, 1,
			 GDT_Float64, 0, 0);

	for(int j = 0;j < nXSize;j++) {
	    grid.set_cellValue(nYSize - 1 - i, j, padfScanline[j]);

	}
    }

    delete [] padfScanline;

    //try to get the projection info
    std::string prj = poDS->GetProjectionRef();
    grid.set_prjString( prj );

    return true;

    /**************************************************
     *
     * AsciiGrid is templated, but we don't support all
     * types, this could be sticky, or we could just
     * ignore it and try to fill in whatever they give
     * us and throw if we can't, maybe scale?
     *
     * GDT_Unknown 	Unknown or unspecified type
     * GDT_Byte 	Eight bit unsigned integer
     * GDT_UInt16 	Sixteen bit unsigned integer
     * GDT_Int16 	Sixteen bit signed integer
     * GDT_UInt32 	Thirty two bit unsigned integer
     * GDT_Int32 	Thirty two bit signed integer
     * GDT_Float32 	Thirty two bit floating point
     * GDT_Float64 	Sixty four bit floating point
     * GDT_CInt16 	Complex Int16
     * GDT_CInt32 	Complex Int32
     * GDT_CFloat32 	Complex Float32
     * GDT_CFloat64 	Complex Float64
     *
     **************************************************/

}

bool GDALPointFromLatLon( double &x, double &y, GDALDataset *poSrcDS,
			  const char *datum )
{
    char* pszPrj;

    OGRSpatialReference oSourceSRS, oTargetSRS;
    OGRCoordinateTransformation *poCT;

    if( poSrcDS == NULL )
	return false;

    if( poSrcDS->GetProjectionRef() == NULL )
	return false;
    else
	pszPrj = (char*)poSrcDS->GetProjectionRef();

    oSourceSRS.SetWellKnownGeogCS( datum );
    oTargetSRS.importFromWkt( &pszPrj );

    poCT = OGRCreateCoordinateTransformation( &oSourceSRS, &oTargetSRS );
    if( poCT == NULL )
	return false;

    if( !poCT->Transform( 1, &x, &y ) ) {
	OGRCoordinateTransformation::DestroyCT( poCT );
	return false;
    }
    OGRCoordinateTransformation::DestroyCT( poCT );
    return true;
}

bool GDALPointToLatLon( double &x, double &y, GDALDataset *poSrcDS,
				const char *datum )
{
    char* pszPrj = NULL;

    OGRSpatialReference oSourceSRS, oTargetSRS;
    OGRCoordinateTransformation *poCT;

    if( poSrcDS == NULL )
	return false;

    if( poSrcDS->GetProjectionRef() == NULL )
	return false;
    else
	pszPrj = (char*)poSrcDS->GetProjectionRef();

    oSourceSRS.importFromWkt( &pszPrj );
    oTargetSRS.SetWellKnownGeogCS( datum );

    poCT = OGRCreateCoordinateTransformation( &oSourceSRS, &oTargetSRS );

    if( poCT == NULL )
	return false;

    if( !poCT->Transform( 1, &x, &y ) ) {
	OGRCoordinateTransformation::DestroyCT( poCT );
	return false;
    }
    OGRCoordinateTransformation::DestroyCT( poCT );
    return true;
}

bool OGRPointToLatLon( double &x, double &y, OGRDataSource *poSrcDS,
				const char *datum )
{
    char* pszPrj = NULL;

    OGRSpatialReference oSourceSRS, oTargetSRS;
    OGRCoordinateTransformation *poCT;

    if( poSrcDS == NULL )
	return false;
	
	OGRLayer *poLayer;
    
    poLayer = poSrcDS->GetLayer(0);
    poLayer->ResetReading();
    
    OGRSpatialReference *poSrcSRS;
	
    if( poLayer->GetSpatialRef() == NULL )
        return false;
    else{
        poSrcSRS = poLayer->GetSpatialRef();
        poSrcSRS->exportToWkt( &pszPrj );
    }

    oSourceSRS.importFromWkt( &pszPrj );
    
    oTargetSRS.SetWellKnownGeogCS( datum );

    poCT = OGRCreateCoordinateTransformation( &oSourceSRS, &oTargetSRS );

    if( poCT == NULL )
	return false;

    if( !poCT->Transform( 1, &x, &y ) ) {
	OGRCoordinateTransformation::DestroyCT( poCT );
	return false;
    }
    OGRCoordinateTransformation::DestroyCT( poCT );
    return true;
}

/**
 * Fetch the UTM zone for a GDALDataset
 *
 * @param Dataset to query
 * @return int representation of the zone, ie 32612
 *
 */
int GDALGetUtmZone( GDALDataset *poDS )
{
    if( poDS == NULL )
        return 0;

    double centerLonLat[2];
    if( !GDALGetCenter( poDS, centerLonLat ) )
        return 0;
    double lon = centerLonLat[0];
    double lat = centerLonLat[1];

    return GetUTMZoneInEPSG( lon, lat );
}

/**
 * @brief Get the UTM zone for a longitude and a latitude.
 *
 * Calclulate the utm zone with special cases for a given lon/lat
 *
 * @param lon the point longitude
 * @param lat the point latitude
 * @return an integer representation of the utm zone suitable for
 *         use in a OGRSpatialReference.ImportFromEPSG() call.
 *
 * I couldn't find the original link, but this discussion references the source:
 * http://postgis.17.n6.nabble.com/function-to-find-UTM-zone-SRID-from-POINT-td3520775.html
 *
 */

int GetUTMZoneInEPSG( double lon, double lat )
{
    int wkid;
    int baseValue;

    // Southern hemisphere if latitude is less than 0
    if ( lat < 0 )
    {
        baseValue = 32700;
    }
    // Otherwise, Northern hemisphere
    else
    {
        baseValue = 32600;
    }

    // Perform standard calculation lat/long --> UTM Zone WKID calculation
    // and adjust it later for special cases.
    wkid = baseValue + (int)floor( ( lon + 186 ) / 6 );

    // Make sure longitude 180 is in zone 60
    if ( lon == 180 )
    {
        wkid = baseValue + 60;
    }
    // Special zone for Norway
    else if ( lat >= 56.0 && lat < 64.0
             && lon >= 3.0 && lon < 12.0 )
    {
        wkid = baseValue + 32;
    }
    // Special zones for Svalbard
    else if ( lat >= 72.0 && lat < 84.0 )
    {
        if ( lon >= 0.0 && lon < 9.0 )
        {
            wkid = baseValue + 31;
        }
        else if ( lon >= 9.0 && lon < 21.0 )
        {
            wkid = baseValue + 33;
        }
        else if ( lon >= 21.0 && lon < 33.0 )
        {
            wkid = baseValue + 35;
        }
        else if ( lon >= 33.0 && lon < 42.0 )
        {
            wkid = baseValue + 37;
        }
    }

    return wkid;
}

int GDALFillBandNoData(GDALDataset *poDS, int nBand, int nSearchPixels)
{
    (void)nBand;
    (void)nSearchPixels;
    if(poDS == NULL)
    {
        fprintf(stderr, "Invalid GDAL Dataset Handle, cannot fill no data\n");
        return -1;
    }
    int nPixels, nLines;
    nPixels = poDS->GetRasterXSize();
    nLines = poDS->GetRasterYSize();

    GDALRasterBand *poBand;

    poBand = poDS->GetRasterBand(1);

    GDALFillNodata(poBand, NULL, 100, 0, 0, NULL, NULL, NULL);

    double dfNoData = poBand->GetNoDataValue(NULL);

    double *padfScanline;
    padfScanline = (double *) CPLMalloc(sizeof(double)*nPixels);
    int nNoDataCount = 0;
    for(int i = 0;i < nLines;i++)
    {
        GDALRasterIO(poBand, GF_Read, 0, i, nPixels, 1,
                     padfScanline, nPixels, 1, GDT_Float64, 0, 0);
        for(int j = 0; j < nPixels;j++)
        {
            if(CPLIsEqual(padfScanline[j], dfNoData))
                nNoDataCount++;
        }
    }

    CPLFree(padfScanline);

    return nNoDataCount;
}

int GDALGetCorners( GDALDataset *poDS, double corners[8] )
{
    //corners is northeast x, northeast y, southeast x, southeast y,
    //southwest x, southwest y, northwest x, northwest y
    double adfGeoTransform[6];
    poDS->GetGeoTransform(adfGeoTransform);
    int xSize, ySize;
    xSize = poDS->GetRasterXSize();
    ySize = poDS->GetRasterYSize();
    double north, east, south, west;
    north = adfGeoTransform[3] + adfGeoTransform[4] * 0 + adfGeoTransform[5] * 0;
    east = adfGeoTransform[0] + adfGeoTransform[1] * xSize + adfGeoTransform[2] * 0;
    south = adfGeoTransform[3] + adfGeoTransform[4] * 0 + adfGeoTransform[5] * ySize;
    west = adfGeoTransform[0] + adfGeoTransform[1] * 0 + adfGeoTransform[2] * 0;
    corners[0] = east;
    corners[1] = north;
    corners[2] = east;
    corners[3] = south;
    corners[4] = west;
    corners[5] = south;
    corners[6] = west;
    corners[7] = north;

    GDALPointToLatLon(corners[0], corners[1], poDS, "WGS84");
    GDALPointToLatLon(corners[2], corners[3], poDS, "WGS84");
    GDALPointToLatLon(corners[4], corners[5], poDS, "WGS84");
    GDALPointToLatLon(corners[6], corners[7], poDS, "WGS84");
    return 0;
}

/**
 * \brief Get the timezone for a given point.
 *
 * Open the world timezone shape file in the data path.  Assume the GDAL is
 * built with GEOS enabled.  If it isn't build with GEOS support this will
 * possibly return a number of geometries.  It should be a rare case.  The
 * OGR_L_SetSpatialFilter() function returns an intersection with the geometry
 * envelope if GEOS support is disabled.  We are also assuming we have vsizip
 * support to help keep distribution sizes smaller.
 *
 * \param dfX X coordinate for the query
 * \param dfY Y coordinate for the query
 * \param pszWkt OGC well-known text representation of the spatial reference.
 *        If NULL is passed, WGS84 is assumed, and the point is not warped.
 * \return An empty string on failure, the standard timezone representation if
 *         we succeed, such as "America/Boise"
 */

std::string FetchTimeZone( double dfX, double dfY, const char *pszWkt )
{
    CPLDebug( "WINDNINJA", "Fetching timezone for  %lf,%lf", dfX, dfY );
    if( pszWkt != NULL )
    {
        OGRSpatialReference oSourceSRS, oTargetSRS;
        OGRCoordinateTransformation *poCT;

        oSourceSRS.SetWellKnownGeogCS( "WGS84" );
        oTargetSRS.importFromWkt( (char**)&pszWkt );

        poCT = OGRCreateCoordinateTransformation( &oSourceSRS, &oTargetSRS );
        if( poCT == NULL )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "OGR coordinate transformation failed" );
            return std::string();
        }
        if( !poCT->Transform( 1, &dfX, &dfY ) )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "OGR coordinate transformation failed" );
            return std::string();
        }
        OGRCoordinateTransformation::DestroyCT( poCT );
    }
    OGRGeometryH hGeometry = OGR_G_CreateGeometry( wkbPoint );
    OGR_G_SetPoint_2D( hGeometry, 0, dfX, dfY );

    OGRDataSourceH hDS;
    OGRLayerH hLayer;
    OGRFeatureH hFeature;

    std::string oTzFile = FindDataPath( "tz_world.zip" );
    oTzFile = "/vsizip/" + oTzFile + "/world/tz_world.shp";

    hDS = OGROpen( oTzFile.c_str(), 0, NULL );
    if( hDS == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to open datasource: %s", oTzFile.c_str() );
        return std::string();
    }
    hLayer = OGR_DS_GetLayer( hDS, 0 );
    OGR_L_SetSpatialFilter( hLayer, hGeometry );
    OGR_L_ResetReading( hLayer );
    int nMaxTries = 5;
    int nTries = 0;
    OGRGeometryH hBufferGeometry;
    do
    {
        if( nTries == 0 )
        {
            hBufferGeometry = OGR_G_Clone( hGeometry );
        }
        else
        {
            hBufferGeometry = OGR_G_Buffer( hGeometry, 0.2 * nTries, 30 );
        }
        OGR_L_SetSpatialFilter( hLayer, hBufferGeometry );
        hFeature = OGR_L_GetNextFeature( hLayer );
        OGR_G_DestroyGeometry( hBufferGeometry );
        nTries++;
    }
    while( hFeature == NULL && nTries < nMaxTries );
    std::string oTimeZone;
    if( hFeature == NULL )
    {
        oTimeZone = std::string();
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to find timezone" );
    }
    else
    {
        oTimeZone = std::string( OGR_F_GetFieldAsString( hFeature, 0 ) );
    }
    OGR_F_Destroy( hFeature );
    OGR_G_DestroyGeometry( hGeometry );
    OGR_DS_Destroy( hDS );
    return oTimeZone;
}
/**
 * \brief Convenience function to check if a geometry is contained in a OGR
 *        datasource for a given layer.
 *
 * The passed geometry is a wkt representation of a geometry of type GeomType.
 * pszFile is opened, and the passed geometry is queried against all
 * geometries in pszLayer.  If the passed geometry is contained in *any* of the
 * geomtries in the layer, TRUE is returned.  FALSE is returned otherwise,
 * including errors.  The SRS of all geometries is assumed to be the same.
 *
 * \param pszWkt Well-known text representation of a geometry.
 * \param pszFile File to open
 * \param pszLayer Layer to extract geometry from, if NULL, use layer 0.
 * \return TRUE if pszWkt is contained in any geometry in pszLayer, FALSE
 *         otherwise, include errors
 */
int NinjaOGRContain(const char *pszWkt, const char *pszFile,
                    const char *pszLayer)
{
    CPLDebug( "WINDNINJA", "Checking for containment of %s in %s:%s",
              pszWkt, pszFile, pszLayer );
    int bContains = FALSE;
    if( pszWkt == NULL || pszFile == NULL )
    {
        return FALSE;
    }
    OGRGeometryH hTestGeometry = NULL;
    int err = OGR_G_CreateFromWkt( (char**)&pszWkt, NULL, &hTestGeometry );
    if( hTestGeometry == NULL || err != CE_None )
    {
        return FALSE;
    }
    OGRDataSourceH hDS = OGROpen( pszFile, 0, NULL );
    if( hDS == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failed to open datasource: %s", pszFile );
        OGR_G_DestroyGeometry( hTestGeometry );
        bContains = FALSE;
        return bContains;
    }
    OGRLayerH hLayer;
    if( pszLayer == NULL )
    {
        hLayer = OGR_DS_GetLayer( hDS, 0 );
    }
    else
    {
        hLayer = OGR_DS_GetLayerByName( hDS, pszLayer );
    }
    OGRFeatureH hFeature;
    if( hLayer != NULL )
    {
        OGRGeometryH hGeometry;
        OGR_L_ResetReading( hLayer );
        while( ( hFeature = OGR_L_GetNextFeature( hLayer ) ) != NULL )
        {
            hGeometry = OGR_F_GetGeometryRef( hFeature );
            if( OGR_G_Contains( hGeometry, hTestGeometry ) )
            {
                bContains = TRUE;
                OGR_F_Destroy( hFeature );
                break;
            }
            OGR_F_Destroy( hFeature );
        }
    }
    OGR_G_DestroyGeometry( hTestGeometry );
    OGR_DS_Destroy( hDS );
    return bContains;
}

