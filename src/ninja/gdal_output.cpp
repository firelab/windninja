/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  Arbitrary output writer for any GDAL driver
 * Author:   Kyle Shannon <kyle@pobox.com>
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

#include "gdal_output.h"

/*
** TODO(kyle): handle color schemes
*/

static int writeLegend(const char *pszFilename, double splits[5], double max,
                       const char *pszUnits) {
  const int width = 180;
  const int height = int(width / 0.75);
  const int arrowLength = 40;
  const int arrowHeadLength = 10;

  int textHeight = 12;
  int titleTextHeight = int(1.2 * textHeight);
  int titleX, titleY;
  int x1, x2, x3, x4;
  double x;
  int y1, y2, y3, y4;
  double y;
  int textX;
  int textY;

  BMP legend;
  RGBApixel white, red, orange, yellow, green, blue;
  RGBApixel colors[5];
  legend.SetSize(width, height);
  legend.SetBitDepth(8);
  /* Initialize legend to black explicitly */
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      legend(i, j)->Alpha = 0;
      legend(i, j)->Blue = 0;
      legend(i, j)->Green = 0;
      legend(i, j)->Red = 0;
    }
  }
  white.Red = 255;
  white.Green = 255;
  white.Blue = 255;
  white.Alpha = 0;
  // RGBApixel red, orange, yellow, green, blue;
  red.Red = 255;
  red.Green = 0;
  red.Blue = 0;
  red.Alpha = 0;

  orange.Red = 255;
  orange.Green = 127;
  orange.Blue = 0;
  orange.Alpha = 0;

  yellow.Red = 255;
  yellow.Green = 255;
  yellow.Blue = 0;
  yellow.Alpha = 0;

  green.Red = 0;
  green.Green = 255;
  green.Blue = 0;
  green.Alpha = 0;

  blue.Red = 0;
  blue.Green = 0;
  blue.Blue = 255;
  blue.Alpha = 0;

  colors[0] = red;
  colors[1] = orange;
  colors[2] = yellow;
  colors[3] = green;
  colors[4] = blue;

  if (splits[4] >= 100) {
    textHeight = 10;
  }
  x = 0.05;
  y = 0.30;

  titleX = x * width;
  titleY = (y / 3) * height;
  PrintString(legend, CPLSPrintf("Wind Speed (%s)", pszUnits), titleX, titleY,
              titleTextHeight, white);

  const char *pszHiLow = 0;

  for (int i = 0; i < 5; i++) {
    x1 = int(width * x);
    x2 = x1 + arrowLength;
    y1 = int(height * y);
    y2 = y1;

    x3 = x2 - arrowHeadLength;
    y3 = y2 + arrowHeadLength;

    x4 = x2 - arrowHeadLength;
    y4 = y2 - arrowHeadLength;

    textX = x2 + 10;
    textY = y2 - int(textHeight * 0.5);

    DrawLine(legend, x1, y1, x2, y2, colors[i]);
    DrawLine(legend, x2, y2, x3, y3, colors[i]);
    DrawLine(legend, x2, y2, x4, y4, colors[i]);

    if (i == 0) {
      pszHiLow = CPLSPrintf("%.2f - %.2f", splits[4], max);
    } else if (i == 4) {
      pszHiLow = CPLSPrintf("%.2f - %.2f", 0, splits[1] - 0.01);
    } else {
      pszHiLow =
          CPLSPrintf("%.2f - %.2f", splits[5 - i - 1], splits[5 - i] - 0.01);
    }
    PrintString(legend, pszHiLow, textX, textY, textHeight, white);
    y += 0.15;
  }
  legend.WriteToFile(pszFilename);
  return 0;
}

static OGRGeometryH drawArrow(double x, double y, double s, double d,
                              double scale, double edge) {
  (void)scale;
  (void)edge;
  (void)s;

  double theta;
  double xPoint, yPoint;
  double xTip, yTip;
  double xTail, yTail;
  double xCenter, yCenter;
  double xHeadRight, yHeadRight;
  double xHeadLeft, yHeadLeft;
  double xScale, yScale;

  double cSize = edge;

  yScale = 0.5;
  xScale = yScale * 0.40;

  xCenter = x;
  yCenter = y;

  theta = d + 180.0;
  if (theta > 360.0) {
    theta -= 360.0;
  }
  theta = 360.0 - theta;

  theta = theta * (acos(-1.0) / 180.0);

  xPoint = 0;
  yPoint = (cSize * yScale);

  // compute tip coordinates
  xTip = (xPoint * cos(theta)) - (yPoint * sin(theta));
  yTip = (xPoint * sin(theta)) + (yPoint * cos(theta));
  // compute tail coordinates
  xTail = -xTip;
  yTail = -yTip;

  // compute right and left coordinates for head
  xPoint = (cSize * xScale);
  yPoint = (cSize * yScale) - (cSize * xScale);

  xHeadRight = (xPoint * cos(theta)) - (yPoint * sin(theta));
  yHeadRight = (xPoint * sin(theta)) + (yPoint * cos(theta));

  xPoint = -(cSize * xScale);
  yPoint = (cSize * yScale) - (cSize * xScale);
  xHeadLeft = (xPoint * cos(theta)) - (yPoint * sin(theta));
  yHeadLeft = (xPoint * sin(theta)) + (yPoint * cos(theta));

  // shift to global coordinates
  xTip += xCenter;
  yTip += yCenter;
  xTail += xCenter;
  yTail += yCenter;
  xHeadRight += xCenter;
  yHeadRight += yCenter;
  xHeadLeft += xCenter;
  yHeadLeft += yCenter;
  // Create a geometry that maps to an arrow.  Use multiline, although thos
  // may
  // change.
  OGRGeometryH hLine = OGR_G_CreateGeometry(wkbLineString);
  OGRGeometryH hGeom = OGR_G_CreateGeometry(wkbMultiLineString);

  OGR_G_AddPoint(hLine, xTail, yTail, 0.0);
  OGR_G_AddPoint(hLine, xTip, yTip, 0.0);

  OGR_G_AddGeometry(hGeom, hLine);
  OGR_G_DestroyGeometry(hLine);

  hLine = OGR_G_CreateGeometry(wkbLineString);

  OGR_G_AddPoint(hLine, xHeadLeft, yHeadLeft, 0.0);
  OGR_G_AddPoint(hLine, xTip, yTip, 0.0);

  OGR_G_AddGeometry(hGeom, hLine);
  OGR_G_DestroyGeometry(hLine);

  hLine = OGR_G_CreateGeometry(wkbLineString);

  OGR_G_AddPoint(hLine, xTip, yTip, 0.0);
  OGR_G_AddPoint(hLine, xHeadRight, yHeadRight, 0.0);

  OGR_G_AddGeometry(hGeom, hLine);
  OGR_G_DestroyGeometry(hLine);

  return hGeom;
}

int NinjaGDALOutput(const char *pszDriver, const char *pszFilename, int nFlags,
                    AsciiGrid<double> &spd, AsciiGrid<double> &dir,
                    char **papszOptions) {
  // Driver to create the output
  GDALDriverH hDrv = 0;
  // If hDrv only supports CreateCopy, use a working driver.
  GDALDriverH hWorkDrv = 0;
  // Output dataset
  GDALDatasetH hDS = 0;
  // Output layer(s)
  OGRLayerH hLayer = 0;
  // Features for the wind field
  OGRFeatureH hFeat = 0;
  // Either points or vectors for the output vector layer
  OGRGeometryH hGeom = 0;

  OGRFieldDefnH hFieldDefn = 0;

  // Source SRS
  OGRSpatialReferenceH hSRS = 0;
  // Destination SRS, if needed
  OGRSpatialReferenceH hDstSRS = 0;
  OGRCoordinateTransformationH hCT = 0;

  int bCreateCopy = FALSE;

  int bTransform = EQUAL(pszDriver, "LIBKML") && spd.prjString != "";

  int rc = 0;

  const char *pszMDI = 0;

  /* If the grids don't align, return early */
  if (!dir.checkForCoincidentGrids(spd)) {
    return 1;
  }

  /*
  ** TODO(kyle): handle AAIGrid(or other single band formats) early,
  ** pszFilename is a stub?
  */
  if (pszDriver == "AAIGrid" && nFlags & NINJA_OUTPUT_RASTER &&
      !(nFlags & NINJA_OUTPUT_VECTOR)) {
  }

  hDrv = GDALGetDriverByName(pszDriver);
  if (hDrv == 0) {
    return 1;
  }

  pszMDI = GDALGetMetadataItem(hDrv, GDAL_DCAP_VECTOR, 0);
  if (pszMDI == 0 && (nFlags & NINJA_OUTPUT_VECTOR)) {
    return 1;
  }
  if ((nFlags & NINJA_OUTPUT_VECTOR) && !CSLTestBoolean(pszMDI)) {
    return 1;
  }

  pszMDI = GDALGetMetadataItem(hDrv, GDAL_DCAP_RASTER, 0);
  if (pszMDI == 0 && (nFlags & NINJA_OUTPUT_RASTER)) {
    return 1;
  }
  if ((nFlags & NINJA_OUTPUT_RASTER) && !CSLTestBoolean(pszMDI)) {
    return 1;
  }

  pszMDI = GDALGetMetadataItem(hDrv, GDAL_DCAP_CREATE, 0);
  if (pszMDI == 0) {
  }

  if (GDALGetMetadataItem(hDrv, GDAL_DCAP_CREATE, 0) &&
      CSLTestBoolean(GDALGetMetadataItem(hDrv, GDAL_DCAP_CREATE, 0))) {
    hWorkDrv = hDrv;
  } else if (GDALGetMetadataItem(hDrv, GDAL_DCAP_CREATECOPY, 0) &&
             CSLTestBoolean(
                 GDALGetMetadataItem(hDrv, GDAL_DCAP_CREATECOPY, 0))) {
    hWorkDrv = GDALGetDriverByName("MEM");
    bCreateCopy = TRUE;
  } else {
    return 1;
  }
  hDS = GDALCreate(hWorkDrv, pszFilename, spd.get_nCols(), spd.get_nRows(), 3,
                   GDT_Byte, 0);
  if (hDS == 0) {
    return rc;
  }

  hSRS = OSRNewSpatialReference(spd.prjString.c_str());
  if (bTransform) {
    hDstSRS = OSRNewSpatialReference(0);
    rc = OSRImportFromEPSG(hDstSRS, 4326);
    if (rc != OGRERR_NONE) {
      // cleanup
      GDALClose(hDS);
      return 1;
    }
    hCT = OCTNewCoordinateTransformation(hSRS, hDstSRS);
  } else {
    hDstSRS = OSRClone(hSRS);
  }

  double splits[5];
  spd.divide_gridData(splits, 5);

  if (nFlags & NINJA_OUTPUT_RASTER) {
    char *pabyRed = (char *)malloc(spd.get_nCols() * spd.get_nRows());
    assert(pabyRed);
    char *pabyGreen = (char *)malloc(spd.get_nCols() * spd.get_nRows());
    assert(pabyGreen);
    char *pabyBlue = (char *)malloc(spd.get_nCols() * spd.get_nRows());
    assert(pabyBlue);
    double s;
    for (int i = 0; i < spd.get_nRows(); i++) {
      for (int j = 0; j < spd.get_nCols(); j++) {
        s = spd.get_cellValue(i, j);
        if (s <= splits[1]) {
          pabyRed[i * j + i] = 0;
          pabyGreen[i * j + i] = 0;
          pabyBlue[i * j + i] = 255;
        } else if (s <= splits[3]) {
          pabyRed[i * j + i] = 0;
          pabyGreen[i * j + i] = 255;
          pabyBlue[i * j + i] = 0;
        } else if (s <= splits[3]) {
          pabyRed[i * j + i] = 255;
          pabyGreen[i * j + i] = 255;
          pabyBlue[i * j + i] = 0;
        } else if (s <= splits[4]) {
          pabyRed[i * j + i] = 255;
          pabyGreen[i * j + i] = 127;
          pabyBlue[i * j + i] = 0;
        } else {
          pabyRed[i * j + i] = 255;
          pabyGreen[i * j + i] = 0;
          pabyBlue[i * j + i] = 0;
        }
      }
    }
    GDALRasterBandH hBand = GDALGetRasterBand(hDS, 1);
    assert(hBand);
    rc = GDALRasterIO(hBand, GF_Write, 0, 0, spd.get_nCols(), spd.get_nRows(),
                      (void *)pabyRed, spd.get_nCols(), spd.get_nRows(),
                      GDT_Byte, 0, 0);
    assert(rc == 0);
    hBand = GDALGetRasterBand(hDS, 2);
    assert(hBand);
    rc = GDALRasterIO(hBand, GF_Write, 0, 0, spd.get_nCols(), spd.get_nRows(),
                      (void *)pabyGreen, spd.get_nCols(), spd.get_nRows(),
                      GDT_Byte, 0, 0);
    assert(rc == 0);
    hBand = GDALGetRasterBand(hDS, 3);
    assert(hBand);
    rc = GDALRasterIO(hBand, GF_Write, 0, 0, spd.get_nCols(), spd.get_nRows(),
                      (void *)pabyBlue, spd.get_nCols(), spd.get_nRows(),
                      GDT_Byte, 0, 0);
    assert(rc == 0);
    if (bCreateCopy) {
      GDALDatasetH hRDS =
          GDALCreateCopy(hDrv, pszFilename, hDS, FALSE, 0, 0, 0);
      assert(hRDS);
      GDALClose(hRDS);
    }
  }

  if (nFlags & NINJA_OUTPUT_VECTOR) {
    // Handle the KML special options
    char **papszKMLOptions = 0;
    if (EQUAL(pszDriver, "LIBKML")) {
      // Don't create a root doc.kml, this aligns with our old format
      CPLSetConfigOption("LIBKML_USE_DOC.KML", "NO");
      papszKMLOptions = CSLAddNameValue(papszKMLOptions, "ADD_REGION", "YES");
      papszKMLOptions =
          CSLAddNameValue(papszKMLOptions, "SO_HREF", "legend.png");
      papszKMLOptions = CSLAddNameValue(papszKMLOptions, "SO_NAME", "Legend");
      papszKMLOptions = CSLAddNameValue(papszKMLOptions, "SO_OVERLAY_X", "0");
      papszKMLOptions = CSLAddNameValue(papszKMLOptions, "SO_OVERLAY_Y", "1");
      papszKMLOptions =
          CSLAddNameValue(papszKMLOptions, "SO_OVERLAY_XUNITS", "fraction");
      papszKMLOptions =
          CSLAddNameValue(papszKMLOptions, "SO_OVERLAY_YUNITS", "fraction");
      papszKMLOptions = CSLAddNameValue(papszKMLOptions, "SO_SCREEN_X", "0");
      papszKMLOptions = CSLAddNameValue(papszKMLOptions, "SO_SCREEN_Y", "1");
      papszKMLOptions =
          CSLAddNameValue(papszKMLOptions, "SO_SCREEN_XUNITS", "fraction");
      papszKMLOptions =
          CSLAddNameValue(papszKMLOptions, "SO_SCREEN_YUNITS", "fraction");
      papszOptions = CSLMerge(papszKMLOptions, papszOptions);
    }

    const char *pszLayerName = "Wind Speed";
    if (nFlags & NINJA_OUTPUT_ARROWS) {
      hLayer = GDALDatasetCreateLayer(hDS, pszLayerName, hDstSRS, wkbLineString,
                                      papszOptions);
    } else {
      hLayer = GDALDatasetCreateLayer(hDS, pszLayerName, hDstSRS, wkbPoint,
                                      papszOptions);
    }
    if (hLayer == 0) {
      GDALClose(hDS);
      return 1;
    }

    hFieldDefn = OGR_Fld_Create("name", OFTString);
    rc = OGR_L_CreateField(hLayer, hFieldDefn, TRUE);
    if (rc != OGRERR_NONE) {
      GDALClose(hDS);
      return 1;
    }
    OGR_Fld_Destroy(hFieldDefn);

    const char *apszFieldDefn[] = {"spd", "dir", 0};

    for (int i = 0; apszFieldDefn[i] != 0; i++) {
      hFieldDefn = OGR_Fld_Create(apszFieldDefn[i], OFTReal);
      rc = OGR_L_CreateField(hLayer, hFieldDefn, TRUE);
      if (rc != OGRERR_NONE) {
        GDALClose(hDS);
        return 1;
      }
      OGR_Fld_Destroy(hFieldDefn);
    }

    double x, y, s, d;
    for (int i = 0; i < spd.get_nRows(); i++) {
      for (int j = 0; j < spd.get_nCols(); j++) {
        hFeat = OGR_F_Create(OGR_L_GetLayerDefn(hLayer));
        s = spd.get_cellValue(i, j);
        d = dir.get_cellValue(i, j);
        OGR_F_SetFieldString(hFeat, OGR_F_GetFieldIndex(hFeat, "name"),
                             CPLSPrintf("cell %d, %d", i, j));
        OGR_F_SetFieldDouble(hFeat, OGR_F_GetFieldIndex(hFeat, "spd"), s);
        OGR_F_SetFieldDouble(hFeat, OGR_F_GetFieldIndex(hFeat, "dir"), d);
        if (s <= splits[1]) {
          OGR_F_SetStyleString(hFeat, "PEN(c:#0000ff;w:10px);");
        } else if (s <= splits[2]) {
          OGR_F_SetStyleString(hFeat, "PEN(c:#00ff00;w:10px);");
        } else if (s <= splits[3]) {
          OGR_F_SetStyleString(hFeat, "PEN(c:#ffff00;w:10px);");
        } else if (s <= splits[4]) {
          OGR_F_SetStyleString(hFeat, "PEN(c:#ffa500;w:10px);");
        } else {
          OGR_F_SetStyleString(hFeat, "PEN(c:#ff0000;w:10px);");
        }
        spd.get_cellPosition(i, j, &x, &y);
        if (nFlags & NINJA_OUTPUT_ARROWS) {
          hGeom = drawArrow(x, y, s, d, 1.0, spd.get_cellSize());
        } else {
          hGeom = OGR_G_CreateGeometry(wkbPoint);
          OGR_G_SetPoint_2D(hGeom, 0, x, y);
        }
        if (bTransform) {
          OGR_G_Transform(hGeom, hCT);
        }
        OGR_F_SetGeometry(hFeat, hGeom);
        if (OGR_L_CreateFeature(hLayer, hFeat) != OGRERR_NONE) {
          OGR_G_DestroyGeometry(hGeom);
          GDALClose(hDS);
          return 1;
        }
        OGR_G_DestroyGeometry(hGeom);
        OGR_F_Destroy(hFeat);
      }
    }
  }
  OSRDestroySpatialReference(hSRS);
  OSRDestroySpatialReference(hDstSRS);
  OCTDestroyCoordinateTransformation(hCT);
  GDALClose(hDS);
  /* After we close the dataset, insert the support files */
  if (EQUAL(pszDriver, "LIBKML") &&
      EQUAL(CPLGetExtension(pszFilename), "kmz")) {
    rc = writeLegend("legend.bmp", splits, spd.get_maxValue(), "mph");
    if (rc != 0) {
      return rc;
    }
    // Copy the bmp into the kmz.  Use GDALCreateCopy to make a PNG, smaller
    // and better for the web.
    GDALDatasetH hBMP = GDALOpen("legend.bmp", GA_ReadOnly);
    GDALDriverH hPNGDrv = GDALGetDriverByName("PNG");
    assert(hPNGDrv);
    const char *pszLgd = CPLSPrintf("/vsizip/%s/legend.png", pszFilename);
    GDALDatasetH hPNG = GDALCreateCopy(hPNGDrv, pszLgd, hBMP, FALSE, 0, 0, 0);
    assert(hPNG);
    GDALClose(hPNG);
    VSIUnlink("legend.bmp");
  }
  return 0;
}
