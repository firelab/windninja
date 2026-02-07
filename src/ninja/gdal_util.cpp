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

#ifndef PI
#define PI 3.14159
#endif

/** Fetch the max value of a dataset.
 * Fetch the max value from any valid GDAL dataset
 * @param poDS a pointer to a valid GDAL Dataset
 * @return max value 
 */
double GDALGetMax( GDALDataset *poDS )
{
    GDALRasterBand *poBand = poDS->GetRasterBand( 1 );
    double adfMinMax[2];

    GDALComputeRasterMinMax((GDALRasterBandH)poBand, FALSE, adfMinMax);

    return adfMinMax[1];
}

/** Fetch the min value of a dataset.
 * Fetch the min value from any valid GDAL dataset
 * @param poDS a pointer to a valid GDAL Dataset
 * @return min value 
 */
double GDALGetMin( GDALDataset *poDS )
{
    GDALRasterBand *poBand = poDS->GetRasterBand( 1 );
    double adfMinMax[2];

    GDALComputeRasterMinMax((GDALRasterBandH)poBand, FALSE, adfMinMax);

    return adfMinMax[0];
}

/** Fetch the center of a domain.
 * Fetch the center of a domain from any valid GDAL dataset
 * @param poDS a pointer to a valid GDAL Dataset
 * @param x the longitude of the center
 * @param y the latitude of the center
 * @return true on valid population of the double*
 */
bool GDALGetCenter( GDALDataset *poDS, double *longitude, double *latitude )
{
    GDALDatasetH hDS = (GDALDatasetH)poDS;
    assert(hDS);
    assert(longitude);
    assert(latitude);
    bool rc = true;

    const char *pszPrj = GDALGetProjectionRef(hDS);
    if(pszPrj == NULL) {
        return false;
    }

    OGRSpatialReferenceH hSrcSRS, hTargetSRS;
    hSrcSRS = OSRNewSpatialReference(pszPrj);
    hTargetSRS = OSRNewSpatialReference(NULL);
    if(hSrcSRS == NULL || hTargetSRS == NULL) {
        OSRDestroySpatialReference(hSrcSRS);
        OSRDestroySpatialReference(hTargetSRS);
        return false;
    }
    OSRImportFromEPSG(hTargetSRS, 4326);

#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    OSRSetAxisMappingStrategy(hTargetSRS, OAMS_TRADITIONAL_GIS_ORDER);
    OSRSetAxisMappingStrategy(hSrcSRS, OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */

    OGRCoordinateTransformationH hCT;
    hCT = OCTNewCoordinateTransformation(hSrcSRS, hTargetSRS);
    if(hCT == NULL) {
        OSRDestroySpatialReference(hSrcSRS);
        OSRDestroySpatialReference(hTargetSRS);
        return false;
    }

    int nX = GDALGetRasterXSize(hDS);
    int nY = GDALGetRasterYSize(hDS);

    double adfGeoTransform[6];
    if(GDALGetGeoTransform(hDS, adfGeoTransform) != CE_None) {
        OCTDestroyCoordinateTransformation(hCT);
        OSRDestroySpatialReference(hSrcSRS);
        OSRDestroySpatialReference(hTargetSRS);
        return false;
    }

    double x = adfGeoTransform[0] + adfGeoTransform[1] * (nX / 2) +
      adfGeoTransform[2] * (nY / 2);
    double y = adfGeoTransform[3] + adfGeoTransform[4] * (nX / 2) +
      adfGeoTransform[5] * (nY / 2);

    rc = OCTTransform(hCT, 1, &x, &y, 0);
    if(rc) {
        *longitude = x;
        *latitude = y;
    }
    OCTDestroyCoordinateTransformation(hCT);
    OSRDestroySpatialReference(hSrcSRS);
    OSRDestroySpatialReference(hTargetSRS);
    return rc;
}

/** Fetch the center of a domain.
 *
 * Fetch the center of a domain from any valid GDAL dataset
 * in the coordinates of the dataset or in the coordinates of an output CRS/spatial reference
 *
 * @param poDS a pointer to a valid GDAL Dataset
 * @param dfX the X coordinate of the center point, to be filled
 * @param dfY the Y coordinate of the center point, to be filled
 * @param pszDstWkt the output spatial reference to which the center point of the dataset should be warped to, as an OGC well-known text representation of the spatial reference.
 *        If NULL is passed, the center point of the dataset is not warped and is left in the coordinates of the dataset.
 * @return true on valid population of the double* center point dfX and dfY values
 */
bool GDALGetCenter( GDALDataset *poDS, double *dfX, double *dfY, const char *pszDstWkt )
{
    GDALDatasetH hDS = (GDALDatasetH)poDS;
    assert(hDS);
    assert(dfX);
    assert(dfY);
    bool rc = true;

    int nX = GDALGetRasterXSize(hDS);
    int nY = GDALGetRasterYSize(hDS);

    double adfGeoTransform[6];
    if(GDALGetGeoTransform(hDS, adfGeoTransform) != CE_None)
    {
        return false;
    }

    double x = adfGeoTransform[0] + adfGeoTransform[1] * (nX / 2) + adfGeoTransform[2] * (nY / 2);
    double y = adfGeoTransform[3] + adfGeoTransform[4] * (nX / 2) + adfGeoTransform[5] * (nY / 2);

    // pszDstWkt == NULL, return the center point in the coordinates of the input dataset
    if(pszDstWkt == NULL)
    {
        *dfX = x;
        *dfY = y;
        return true;
    }

    // pszDstWkt != NULL, attempt to reproject the center point to the output spatial reference

    const char *pszSrcWkt = GDALGetProjectionRef(hDS);
    if(pszSrcWkt == NULL)
    {
        return false;
    }

    OGRSpatialReferenceH hSrcSRS, hTargetSRS;

    hSrcSRS = OSRNewSpatialReference(pszSrcWkt);
    hTargetSRS = OSRNewSpatialReference(NULL);
    if(hSrcSRS == NULL || hTargetSRS == NULL)
    {
        OSRDestroySpatialReference(hSrcSRS);
        OSRDestroySpatialReference(hTargetSRS);
        return false;
    }

    OSRImportFromWkt( hTargetSRS, (char**)&pszDstWkt );
    if(hTargetSRS == NULL)
    {
        OSRDestroySpatialReference(hSrcSRS);
        OSRDestroySpatialReference(hTargetSRS);
        return false;
    }

#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    OSRSetAxisMappingStrategy(hTargetSRS, OAMS_TRADITIONAL_GIS_ORDER);
    OSRSetAxisMappingStrategy(hSrcSRS, OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */

    OGRCoordinateTransformationH hCT;
    hCT = OCTNewCoordinateTransformation(hSrcSRS, hTargetSRS);
    if(hCT == NULL)
    {
        OSRDestroySpatialReference(hSrcSRS);
        OSRDestroySpatialReference(hTargetSRS);
        return false;
    }

    rc = OCTTransform(hCT, 1, &x, &y, 0);
    if(rc)
    {
        *dfX = x;
        *dfY = y;
    }
    OCTDestroyCoordinateTransformation(hCT);
    OSRDestroySpatialReference(hSrcSRS);
    OSRDestroySpatialReference(hTargetSRS);
    return rc;
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
#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    oTargetSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */

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

/** Fetch the bounds of an image
 *
 * Fetch the bounds of an image from any valid GDAL dataset
 * in the coordinates of the dataset or in the coordinates of an output CRS/spatial reference
 *
 * @param poDS a pointer to a valid GDAL Dataset
 * @param bounds a pointer to a double[4], in n, e, s, w order, to be filled
 * @param pszDstWkt the output spatial reference to which the bounds of the dataset should be warped to, as an OGC well-known text representation of the spatial reference.
 *        If NULL is passed, the bounds of the dataset is not warped and is left in the coordinates of the dataset.
 * @return true on valid population of bounds in n, e, s, w order
 */
bool GDALGetBounds( GDALDataset *poDS, double *bounds, const char *pszDstWkt )
{
    if( poDS == NULL )
    {
        return false;
    }

    int xSize = poDS->GetRasterXSize();
    int ySize = poDS->GetRasterYSize();

    double adfGeoTransform[6];
    poDS->GetGeoTransform( adfGeoTransform );

    double north = adfGeoTransform[3] + adfGeoTransform[4] * 0     + adfGeoTransform[5] * 0;
    double east  = adfGeoTransform[0] + adfGeoTransform[1] * xSize + adfGeoTransform[2] * 0;
    double south = adfGeoTransform[3] + adfGeoTransform[4] * 0     + adfGeoTransform[5] * ySize;
    double west  = adfGeoTransform[0] + adfGeoTransform[1] * 0     + adfGeoTransform[2] * 0;

    // pszDstWkt == NULL, return the bounds in the coordinates of the input dataset
    if(pszDstWkt == NULL)
    {
        bounds[0] = north;
        bounds[1] = east;
        bounds[2] = south;
        bounds[3] = west;
        return true;
    }

    // pszDstWkt != NULL, attempt to reproject the bounds to the output spatial reference

    char* pszSrcWkt;
    if( poDS->GetProjectionRef() == NULL )
    {
        return false;
    }
    pszSrcWkt = (char*)poDS->GetProjectionRef();

    OGRSpatialReference oSourceSRS, oTargetSRS;

    oSourceSRS.importFromWkt( &pszSrcWkt );
    oTargetSRS.importFromWkt( (char**)&pszDstWkt );

#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    oSourceSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    oTargetSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */

    OGRCoordinateTransformation *poCT;
    poCT = OGRCreateCoordinateTransformation( &oSourceSRS, &oTargetSRS );
    if( poCT == NULL )
    {
        return false;
    }

    if( !poCT->Transform( 1, &east, &north ) )
    {
        OGRCoordinateTransformation::DestroyCT( poCT );
        return false;
    }
    if( !poCT->Transform( 1, &west, &south ) )
    {
        OGRCoordinateTransformation::DestroyCT( poCT );
        return false;
    }

    // note, by only warping and tracking two of the four points, this is truncating or expanding the bounds in the new coordinate system
    bounds[0] = north;
    bounds[1] = east;
    bounds[2] = south;
    bounds[3] = west;

    OGRCoordinateTransformation::DestroyCT( poCT );
    return true;
}

/** Calculate the angle between the N-S grid lines in the DS and true north for the center point of the DS.
 *  Where the angle is defined as going FROM true north TO the y coordinate grid line of the DS.
 * @param poDS a pointer to a valid GDALDataset
 * @param angle the computed angle from north
 * @return true on success false on failure.
 */
bool GDALCalculateAngleFromNorth( GDALDataset *poDS, double &angleFromNorth )
{
    double x1, y1; //center point of DEM in lat/lon
    double x2, y2; //point due north of center point in lat/lon
    double boundsLonLat[4]; //bounds of the DS in lat/lon

    CPLDebug( "COORD_TRANSFORM_ANGLES", "GDALCalculateAngleFromNorth()");
    CPLDebug( "COORD_TRANSFORM_ANGLES", "pszSrcWkt = \"%s\"", GDALGetProjectionRef( poDS ) );

    if(!GDALGetCenter( poDS, &x1, &y1 ))
    {
        return false;
    }

    x2 = x1;

    //add 1/4 size of the DEM extent in y direction
    if(!GDALGetBounds( poDS, boundsLonLat ))
    {
        return false;
    }

    y2 = y1 + 0.25*(boundsLonLat[0] - boundsLonLat[2]);

    CPLDebug( "COORD_TRANSFORM_ANGLES", "x1, y1 = %lf, %lf", x1, y1 );
    CPLDebug( "COORD_TRANSFORM_ANGLES", "x2, y2 = %lf, %lf", x2, y2 );

    //project the two lat/lon points to projected DEM coordinates
    if(!GDALPointFromLatLon(x1, y1, poDS, "WGS84"))
    {
        return false;
    }

    if(!GDALPointFromLatLon(x2, y2, poDS, "WGS84"))
    {
        return false;
    }

    CPLDebug( "COORD_TRANSFORM_ANGLES", "x1, y1 = %lf, %lf", x1, y1 );
    CPLDebug( "COORD_TRANSFORM_ANGLES", "x2, y2 = %lf, %lf", x2, y2 );

    //compute angle of the line formed between projected (x1,y1) to (x2,y2) (projected true north)
    //and projected (x1,y1) to (x1,y2) (y coordinate gridline of the projected CRS).
    //call the line going from (x1,y1) to (x2,y2) in the projected CRS "b", the line formed
    //by our points (x1,y1) to (x1,y2) "a", and the angle between a and b "theta"
    //cos(theta) = a dot b /(|a||b|)
    //a dot b = axbx + ayby
    //|a| = sqrt(ax^2 + ay^2) and |b| = sqrt(bx^2 + by^2)

    double ax, ay, bx, by; //denote x,y vector components of lines "a" and "b", derived from component length between startpoints and endpoints of lines "a" and "b"
    double adotb; //a dot b
    double mag_a, mag_b; //|a| and |b|

    ax = x1 - x1;
    ay = y2 - y1;
    bx = x2 - x1;
    by = y2 - y1;
    CPLDebug( "COORD_TRANSFORM_ANGLES", "a = (%lf,%lf), b = (%lf,%lf)", ax, ay, bx, by );

    adotb = ax*bx + ay*by;
    mag_a = sqrt(ax*ax + ay*ay);
    mag_b = sqrt(bx*bx + by*by);

    angleFromNorth = acos(adotb/(mag_a * mag_b)); //compute angle in radians

    // add sign to the angle, ax should equal 0, ay should equal by, so should just be checking the sign of bx
    // if bx is positive, the arrow b is pointed right from a, the rotation going FROM b TO a is counter clockwise, so the angle is negative
    // if bx is negative, the arrow b is pointed  left from a, the rotation going FROM b TO a is clockwise, so the angle is positive (we don't need to do anything)
    // also, going FROM b TO a is equivalent to going FROM true north TO the y coordinate grid line of the DS.
    if( bx > 0 )
    {
        angleFromNorth = -1*angleFromNorth;
    }

    CPLDebug( "COORD_TRANSFORM_ANGLES", "angleFromNorth in radians = %lf", angleFromNorth );
    //convert the result from radians to degrees
    angleFromNorth *= 180.0 / PI;
    CPLDebug( "COORD_TRANSFORM_ANGLES", "angleFromNorth in degrees = %lf", angleFromNorth );

    return true;
}

/** Calculate the angle between the y coordinate grid lines of a source dataset and an output spatial reference.
 *  Where the angle is defined, as going FROM the source dataset spatial reference TO the output spatial reference,
 *  so, going FROM the y coordinate grid line of the DS TO the y coordinate grid line of the output spatial reference.
 * @param poSrcDS a pointer to a valid GDAL Dataset, from which the input spatial reference is obtained
 * @param coordinateTransformAngle the computed angle between the y coordinate grid lines of the two datasets, to be filled
 * @param pszDstWkt the output spatial reference to which the angle should be calculated for, as an OGC well-known text representation of the spatial reference.
 * @return true on success false on failure.
 */
bool GDALCalculateCoordinateTransformationAngle( GDALDataset *poSrcDS, double &coordinateTransformAngle, const char *pszDstWkt )
{
    double x1, y1; //center point of the poSrcDS, in the projection of the poSrcDS, to be transformed to the pszDstWkt projection
    double x2, y2; //point straight out in the direction of the y coordinate grid line from the center point of the poSrcDS, in the projection of the poSrcDS, to be transformed to the pszDstWkt projection
    double bounds[4]; //bounds of the poSrcDS, used to calculate y2

    CPLDebug( "COORD_TRANSFORM_ANGLES", "GDALCalculateCoordinateTransformationAngle()");
    CPLDebug( "COORD_TRANSFORM_ANGLES", "pszSrcWkt = \"%s\"", GDALGetProjectionRef( poSrcDS ) );
    CPLDebug( "COORD_TRANSFORM_ANGLES", "pszDstWkt = \"%s\"", pszDstWkt );

    //get the center of the poSrcDS, in the projection of the poSrcDS
    if(!GDALGetCenter( poSrcDS, &x1, &y1, NULL ))
    {
        return false;
    }

    x2 = x1;

    //add 1/4 size of the poSrcDS extent in y direction, in the projection of the poSrcDS
    if(!GDALGetBounds( poSrcDS, bounds, NULL ))
    {
        return false;
    }

    y2 = y1 + 0.25*(bounds[0] - bounds[2]);

    CPLDebug( "COORD_TRANSFORM_ANGLES", "x1, y1 = %lf, %lf", x1, y1 );
    CPLDebug( "COORD_TRANSFORM_ANGLES", "x2, y2 = %lf, %lf", x2, y2 );

    //project the two points FROM the projection of the poSrcDS TO the pszDstWkt projection
    if(!GDALTransformPoint(x1, y1, poSrcDS, pszDstWkt))
    {
        return false;
    }

    if(!GDALTransformPoint(x2, y2, poSrcDS, pszDstWkt))
    {
        return false;
    }

    CPLDebug( "COORD_TRANSFORM_ANGLES", "x1, y1 = %lf, %lf", x1, y1 );
    CPLDebug( "COORD_TRANSFORM_ANGLES", "x2, y2 = %lf, %lf", x2, y2 );

    //compute angle of the line formed between projected (x1,y1) to (x2,y2) (y coordinate gridline of poSrcDS, in the pszDstWkt projection coordinates)
    //and projected (x1,y1) to (x1,y2) (y coordinate gridline of the pszDstWkt projection coordinate system, in the pszDstWkt projection coordinates).
    //call the line going from (x1,y1) to (x2,y2) in the projected CRS "b", the line formed
    //by our points (x1,y1) to (x1,y2) "a", and the angle between a and b "theta"
    //cos(theta) = a dot b /(|a||b|)
    //a dot b = axbx + ayby
    //|a| = sqrt(ax^2 + ay^2) and |b| = sqrt(bx^2 + by^2)

    double ax, ay, bx, by; //denote x,y vector components of lines "a" and "b", derived from component length between startpoints and endpoints of lines "a" and "b"
    double adotb; //a dot b
    double mag_a, mag_b; //|a| and |b|

    ax = x1 - x1;
    ay = y2 - y1;
    bx = x2 - x1;
    by = y2 - y1;
    CPLDebug( "COORD_TRANSFORM_ANGLES", "a = (%lf,%lf), b = (%lf,%lf)", ax, ay, bx, by );

    adotb = ax*bx + ay*by;
    mag_a = sqrt(ax*ax + ay*ay);
    mag_b = sqrt(bx*bx + by*by);

    coordinateTransformAngle = acos(adotb/(mag_a * mag_b)); //compute angle in radians

    // add sign to the angle, ax should equal 0, ay should equal by, so should just be checking the sign of bx
    // if bx is positive, the arrow b is pointed right from a, the rotation going FROM b TO a is counter clockwise, so the angle is negative
    // if bx is negative, the arrow b is pointed  left from a, the rotation going FROM b TO a is clockwise, so the angle is positive (we don't need to do anything)
    // also, going FROM b TO a is equivalent to going FROM the y coordinate grid line of the DS TO the y coordinate grid line of the output spatial reference.
    if( bx > 0 )
    {
        coordinateTransformAngle = -1*coordinateTransformAngle;
    }

    CPLDebug( "COORD_TRANSFORM_ANGLES", "coordinateTransformAngle in radians = %lf", coordinateTransformAngle );
    //convert the result from radians to degrees
    coordinateTransformAngle *= 180.0 / PI;
    CPLDebug( "COORD_TRANSFORM_ANGLES", "coordinateTransformAngle in degrees = %lf", coordinateTransformAngle );

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
#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    oTargetSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */

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

/** perform a coordinate transformation on a point, from one coordinate system to another
 *
 * perform a coordinate transformation on a point, from one coordinate system to another
 * where the input coordinate system is specified by a valid GDAL dataset
 * and the output coordinate system is specified by a CRS/spatial reference.
 *
 * @param dfX the X coordinate of the point to be transformed, to be filled
 * @param dfY the Y coordinate of the point to be transformed, to be filled
 * @param poSrcDS a pointer to a valid GDAL Dataset, from which the input spatial reference is obtained
 * @param pszDstWkt the output spatial reference to which the point of interest should be warped to, as an OGC well-known text representation of the spatial reference.
 * @return true on valid population of the transformed point
 */
bool GDALTransformPoint( double &dfX, double &dfY, GDALDataset *poSrcDS, const char *pszDstWkt )
{
    if( poSrcDS == NULL )
    {
        return false;
    }

    char* pszSrcWkt;
    if( poSrcDS->GetProjectionRef() == NULL )
    {
        return false;
    }
    pszSrcWkt = (char*)poSrcDS->GetProjectionRef();

    OGRSpatialReference oSourceSRS, oTargetSRS;

    oSourceSRS.importFromWkt( &pszSrcWkt );
    oTargetSRS.importFromWkt( (char**)&pszDstWkt );

#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    oSourceSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    oTargetSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */

    OGRCoordinateTransformation *poCT;
    poCT = OGRCreateCoordinateTransformation( &oSourceSRS, &oTargetSRS );
    if( poCT == NULL )
    {
        return false;
    }

    if( !poCT->Transform( 1, &dfX, &dfY ) )
    {
        OGRCoordinateTransformation::DestroyCT( poCT );
        return false;
    }

    OGRCoordinateTransformation::DestroyCT( poCT );
    return true;
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

#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    oSourceSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    oTargetSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */

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

#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    oSourceSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    oTargetSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */

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

bool OGRPointToLatLon(double &x, double &y, OGRDataSourceH hDS,
                      const char *datum) {
    char *pszPrj = NULL;

    const OGRSpatialReference *poSrcSRS;
    OGRSpatialReference oSourceSRS, oTargetSRS;
    OGRCoordinateTransformation *poCT;

    if (hDS == NULL) {
        return false;
    }

    OGRLayer *poLayer;

    poLayer = (OGRLayer *)OGR_DS_GetLayer(hDS, 0);
    poLayer->ResetReading();

    poSrcSRS = poLayer->GetSpatialRef();
    if (poSrcSRS == NULL) {
        return false;
    }
    oSourceSRS = *poSrcSRS;
    oTargetSRS.SetWellKnownGeogCS(datum);

    #ifdef GDAL_COMPUTE_VERSION
    #if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
        oSourceSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
        oTargetSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    #endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
    #endif /* GDAL_COMPUTE_VERSION */
    poCT = OGRCreateCoordinateTransformation(&oSourceSRS, &oTargetSRS);

    if (poCT == NULL) {
        return false;
    }

    if (!poCT->Transform(1, &x, &y)) {
        OGRCoordinateTransformation::DestroyCT(poCT);
        return false;
    }
    OGRCoordinateTransformation::DestroyCT(poCT);
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

    double longitude = 0;
    double latitude = 0;
    if( !GDALGetCenter( poDS, &longitude, &latitude ) ) return 0;

    return GetUTMZoneInEPSG( longitude, latitude );
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

    // Check for wrapped longitude
    lon = fmod( lon, 360.0 );

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
    if(poDS == NULL)
    {
        fprintf(stderr, "Invalid GDAL Dataset Handle, cannot fill no data\n");
        return -1;
    }
    int nPixels, nLines;
    nPixels = poDS->GetRasterXSize();
    nLines = poDS->GetRasterYSize();

    GDALRasterBand *poBand;

    poBand = poDS->GetRasterBand(nBand);

    GDALFillNodata(poBand, NULL, nSearchPixels, 0, 0, NULL, NULL, NULL);

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

#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    oSourceSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    oTargetSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */

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
    GDALClose( hDS );
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
    int bContains = FALSE;
    if( pszWkt == NULL || pszFile == NULL )
    {
        return FALSE;
    }
    CPLDebug( "WINDNINJA", "Checking for containment of %s in %s:%s",
              pszWkt, pszFile, pszLayer ? pszLayer : "" );
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
    GDALClose( hDS );
    return bContains;
}

// PCM - more GDAL utility functions. To distinguish them from the GDAL libs we us a 'gdal' prefix

bool gdalHasGeographicSRS (const char* filename) {
    bool isGeographic = false;
    GDALDatasetH hDS = (GDALDatasetH) GDALOpen(filename, GA_ReadOnly);
    CPLAssert(hDS);

    const char *pszPrj = GDALGetProjectionRef(hDS);
    if (pszPrj == "") {
      isGeographic = true;
    }
    GDALClose(hDS);

    return isGeographic;
}

// return UTM zone 1-60 or -1 if illegal input
int gdalGetUtmZone (double lat, double lon) {

    // handle special cases (Svalbard/Norway)
    if (lat > 55 && lat < 64 && lon > 2 && lon < 6) {
        return 32;
    }

    if (lat > 71) {
        if (lon >= 6 && lon < 9) {
            return 31;
        }
        if ((lon >= 9 && lon < 12) || (lon >= 18 && lon < 21)) {
            return 33;
        }
        if ((lon >= 21 && lon < 24) || (lon >= 30 && lon < 33)) {
            return 35;
        }
    }

    if (lon >= -180 && lon <= 180) {
        return ((int)((lon + 180.0) / 6.0) % 60) + 1;
    } else if (lon > 180 && lon < 360) {
        return ((int)(lon / 6.0) % 60) + 1;
    }

    return -1;
}

/**
 * @brief Warp a dataset into a GeoTiff with UTM projection.
 *
 * Warp a given GDAL dataset to a GDAL dataset with the appropriate UTM projection and write 
 * to disk as a GeoTiff with a specified file name.
 *
 * @param filename the output filename to be written to disk
 * @param hSrcDS the source dataset
 * @param hDstDS the destination dataset
 * @return true on success, false on failure
 *
 */
bool GDALWarpToUtm (const char* filename, GDALDatasetH& hSrcDS, GDALDatasetH& hDstDS)
{
    /* parse options */
    GDALResampleAlg eAlg = GRA_NearestNeighbour;

    CPLSetConfigOption("GTIFF_DIRECT_IO", "YES");

    GDALDriverH hDriver;

    hDriver = GDALGetDriverByName("GTiff");

    const char *pszSrcWKT, *pszDstWKT = NULL;
    pszSrcWKT = GDALGetProjectionRef(hSrcDS);

    OGRSpatialReference oDstSRS;

    int nUtmZone = GDALGetUtmZone( (GDALDataset*)hSrcDS );

    oDstSRS.importFromEPSG(nUtmZone);
    oDstSRS.exportToWkt((char**)&pszDstWKT);

    void *hTransformArg;

    hTransformArg =
        GDALCreateGenImgProjTransformer(hSrcDS, pszSrcWKT, NULL, pszDstWKT,
                                        FALSE, 0, 1);

    double adfDstGeoTransform[6];
    int nPixels=0, nLines=0;
    CPLErr eErr;

    /* Silence warnings with regard to exceeding limits */
    CPLPushErrorHandler(CPLQuietErrorHandler);
    eErr = GDALSuggestedWarpOutput(hSrcDS, 
                                   GDALGenImgProjTransform, hTransformArg, 
                                   adfDstGeoTransform, &nPixels, &nLines);
    CPLPopErrorHandler();
    if(eErr != CE_None)
    {
        CPLError( CE_Failure, CPLE_AppDefined, "GDALSuggestedWarpOutput failed." );
        return false;
    }
    GDALDestroyGenImgProjTransformer(hTransformArg);

    hDstDS = GDALCreate(hDriver, filename, nPixels, nLines, 
                        GDALGetRasterCount(hSrcDS), GDT_Float32, NULL);

    if(hDstDS == NULL)
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Failed to create gdal dataset." );
        return false;
    }

    GDALSetProjection(hDstDS, pszDstWKT);
    GDALSetGeoTransform(hDstDS, adfDstGeoTransform);

    GDALRasterBandH hSrcBand;
    GDALRasterBandH hDstBand;

    hSrcBand = GDALGetRasterBand(hSrcDS, 1);
    hDstBand = GDALGetRasterBand(hDstDS, 1);

    int nBandCount = GDALGetRasterCount( hDstDS );

    double dfNoData = GDALGetRasterNoDataValue(hSrcBand, NULL);

    GDALSetRasterNoDataValue(hDstBand, dfNoData);

    GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();

    psWarpOptions->hSrcDS = hSrcDS;
    psWarpOptions->hDstDS = hDstDS;

    psWarpOptions->nBandCount = 1;
    psWarpOptions->padfDstNoDataReal =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );
    psWarpOptions->padfDstNoDataImag =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );
    psWarpOptions->padfDstNoDataReal[0] = dfNoData;
    psWarpOptions->padfDstNoDataImag[0] = dfNoData;
    psWarpOptions->panSrcBands = 
        (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
    psWarpOptions->panSrcBands[0] = 1;
    psWarpOptions->panDstBands = 
        (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
    psWarpOptions->panDstBands[0] = 1;

    psWarpOptions->pTransformerArg = 
        GDALCreateGenImgProjTransformer( hSrcDS, 
                                         GDALGetProjectionRef(hSrcDS), 
                                         hDstDS,
                                         GDALGetProjectionRef(hDstDS), 
                                         FALSE, 0.0, 1 );

    psWarpOptions->pfnTransformer = GDALGenImgProjTransform;

    psWarpOptions->eResampleAlg = eAlg;

    GDALWarpOperation oOperation;

    oOperation.Initialize( psWarpOptions );
    eErr = oOperation.ChunkAndWarpImage( 0, 0, 
                                         GDALGetRasterXSize( hDstDS ), 
                                         GDALGetRasterYSize( hDstDS ) );

    GDALDestroyGenImgProjTransformer( psWarpOptions->pTransformerArg );
    GDALDestroyWarpOptions( psWarpOptions );

    if( eErr != CE_None )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Could not warp downloaded DEM." );
        CPLFree((void*)pszDstWKT);
        GDALClose(hDstDS);
        GDALClose(hSrcDS);
        return false;
    }

    //fill no data from warping
    int nNoDataCount = 0;
    if(GDALHasNoData((GDALDataset*)hDstDS, 1))
    {
        nNoDataCount = GDALFillBandNoData((GDALDataset*)hDstDS, 1, 100);
    }

    double *padfScanline;
    padfScanline = (double *) CPLMalloc(sizeof(double)*nPixels);
    nNoDataCount = 0;
    for(int i = 0;i < nLines;i++)
    {
        GDALRasterIO(hDstBand, GF_Read, 0, i, nPixels, 1, 
                     padfScanline, nPixels, 1, GDT_Float64, 0, 0);
        for(int j = 0; j < nPixels;j++)
        {
            if(CPLIsEqual(padfScanline[j], dfNoData))
                nNoDataCount++;
        }
    }
    if(nNoDataCount > 0)
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Failed to fill all no data values." );
        return false;
    }

    CPLFree((void*)padfScanline);
    CPLFree((void*)pszDstWKT);

    CPLSetConfigOption("GTIFF_DIRECT_IO", "NO");

    return true;
}

// turn provided data set into a GeoTiff with UTM projection
// returning NULL indicates error
GDALDataset* gdalWarpToUtm (const char* filename, GDALDataset* pSrcDS) {
    GDALDataset* pDstDS = NULL;
    double lat, lon;
    if (pSrcDS != NULL && GDALGetCenter( pSrcDS, &lon, &lat)) {
        int utmZone = gdalGetUtmZone(lat,lon);
        GDALDriverH hDriver = GDALGetDriverByName("GTiff"); // built-in

        const char *pszSrcWKT = pSrcDS->GetProjectionRef();
        if (strlen(pszSrcWKT) > 0) {
            char *pszDstWKT = NULL;

            OGRSpatialReference oSRS;
            oSRS.SetUTM(utmZone, lat > 0);
            oSRS.SetWellKnownGeogCS("WGS84");
            oSRS.exportToWkt( &pszDstWKT);

            void* hTransformArg = GDALCreateGenImgProjTransformer( pSrcDS, pszSrcWKT, NULL, pszDstWKT, FALSE, 0, 1 );
            if (hTransformArg) {
                double adfDstGeoTransform[6];
                int nPixels = pSrcDS->GetRasterXSize(); 
                int nLines =  pSrcDS->GetRasterYSize();
                GDALSuggestedWarpOutput( pSrcDS, GDALGenImgProjTransform, hTransformArg, adfDstGeoTransform, &nPixels, &nLines );
                GDALDestroyGenImgProjTransformer( hTransformArg );

                GDALDataType eDT = GDALGetRasterDataType(GDALGetRasterBand(pSrcDS,1));
                pDstDS = (GDALDataset*) GDALCreate( hDriver, filename, nPixels, nLines, GDALGetRasterCount(pSrcDS), eDT, NULL );
                if (pDstDS) {
                    pDstDS->SetProjection(pszDstWKT);
                    pDstDS->SetGeoTransform(adfDstGeoTransform);
                }
            }
        }
    }

    return pDstDS;
}

/**
 * @brief Warp a single band of a GDAL dataset into an output VRT GDAL dataset, using an output spatial reference defined by a WKT
 * using GDALAutoCreateWarpedVRT().
 *
 * Warp a single band of a given source GDAL dataset into an output VRT GDAL dataset.
 * Where the source dataset is warped FROM the source dataset spatial reference TO the output spatial reference,
 * and the output spatial reference is defined by a WKT.
 *
 * note that the only pszWarpOptions used are panSrcBands as the source band number, panDstBands the destination band number of 1,
 * and both padfDstNoDataReal and padfDstNoDataImag set to the NoDataValue of the dataset (or -9999 if no NoDataValue in the dataset).
 *
 * @param hSrcDS a pointer to a valid source GDAL dataset, to be warped
 * @param band the specific band number within the source GDAL dataset to be warped
 * @param hDstDS the destination GDAL dataset, as a VRT, to be filled
 * @param pszDstWkt the output spatial reference to which the source GDAL dataset should be warped to, as an OGC well-known text representation of the spatial reference.
 * @return true on success, false on failure
 */
bool GDALWarpToWKT_GDALAutoCreateWarpedVRT( GDALDatasetH& hSrcDS, int band, GDALDatasetH& hDstDS, const char *pszDstWkt )
{
    //if(hDstDS != NULL)  // this didn't work as a good checking method
    //{
    //    CPLError( CE_Failure, CPLE_AppDefined, "GDALWarpToWKT_GDALAutoCreateWarpedVRT() input hDstDS is NOT NULL, it was input pre-filled!" );
    //    return false;
    //}

    int nBandCount = GDALGetRasterCount( hSrcDS );
    if( band <= 0 )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "GDALWarpToWKT_GDALAutoCreateWarpedVRT() input band is <= 0." );
        return false;
    }
    if( band > nBandCount )
    {
        CPLError( CE_Failure, CPLE_AppDefined, "GDALWarpToWKT_GDALAutoCreateWarpedVRT() input band %i is > nBands of the dataset.", band );
        return false;
    }

    GDALDatasetH hBand = GDALGetRasterBand( hSrcDS, band );
    int bSuccess = false;
    double dfNoData = GDALGetRasterNoDataValue( hBand, &bSuccess );
    if( bSuccess == false )
    {
        dfNoData = -9999.0;
    }

    const char *pszSrcWkt;
    pszSrcWkt = GDALGetProjectionRef( hSrcDS );

    GDALWarpOptions *psWarpOptions;
    psWarpOptions = GDALCreateWarpOptions();

    psWarpOptions->nBandCount = nBandCount;

    psWarpOptions->panSrcBands =
        (int *) CPLMalloc(sizeof(int) * nBandCount );
    psWarpOptions->panSrcBands[0] = band;

    psWarpOptions->panDstBands =
        (int *) CPLMalloc(sizeof(int) * nBandCount );
    psWarpOptions->panDstBands[0] = 1;

    psWarpOptions->padfDstNoDataReal =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );
    psWarpOptions->padfDstNoDataImag =
        (double*) CPLMalloc( sizeof( double ) * nBandCount );
    for( int i = 0; i < nBandCount; i++ )
    {
        psWarpOptions->padfDstNoDataReal[i] = dfNoData;
        psWarpOptions->padfDstNoDataImag[i] = dfNoData;
    }

    psWarpOptions->papszWarpOptions = CSLSetNameValue( psWarpOptions->papszWarpOptions, "INIT_DEST", "NO_DATA" );
    if( bSuccess == false )  // if GDALGetRasterNoDataValue( hBand, &bSuccess ) fails to return that a NO_DATA value is in the source dataset
    {
        psWarpOptions->papszWarpOptions = CSLSetNameValue( psWarpOptions->papszWarpOptions, "INIT_DEST", boost::lexical_cast<std::string>(dfNoData).c_str() );
    }


    hDstDS = GDALAutoCreateWarpedVRT( hSrcDS, pszSrcWkt, pszDstWkt,
                                      GRA_NearestNeighbour, 1.0,
                                      psWarpOptions );

    if(hDstDS == NULL)
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Warp operation failed!" );
        return false;
    }

    GDALDestroyWarpOptions( psWarpOptions );

    return true;
}


