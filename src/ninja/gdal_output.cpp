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
  // Create a geometry that maps to an arrow.  Use multiline, although thos may
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
  GDALDriverH hDrv = 0;
  GDALDatasetH hDS = 0;
  OGRLayerH hLayer = 0;
  OGRFeatureH hFeat = 0;
  OGRGeometryH hGeom = 0;

  OGRFieldDefnH hFieldDefn = 0;

  OGRSpatialReferenceH hSRS = 0;

  int rc = 0;

  /* If the grids don't align, return early */
  if (!dir.checkForCoincidentGrids(spd)) {
    return 1;
  }

  hDrv = GDALGetDriverByName(pszDriver);
  if (hDrv == 0) {
    return 1;
  }

  hDS = GDALCreate(hDrv, pszFilename, 0, 0, 0, GDT_Byte, 0);
  if (hDS == 0) {
    return rc;
  }

  hSRS = OSRNewSpatialReference(spd.prjString.c_str());

  if (nFlags & NINJA_OUTPUT_ARROWS) {
    hLayer =
        GDALDatasetCreateLayer(hDS, "wind", hSRS, wkbLineString, papszOptions);
  } else {
    hLayer = GDALDatasetCreateLayer(hDS, "wind", hSRS, wkbPoint, papszOptions);
  }
  if (hLayer == 0) {
    GDALClose(hDS);
    return 1;
  }

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

  double splits[5];
  spd.divide_gridData(splits, 5);

  double x, y, s, d;
  for (int i = 0; i < spd.get_nRows(); i++) {
    for (int j = 0; j < spd.get_nCols(); j++) {
      hFeat = OGR_F_Create(OGR_L_GetLayerDefn(hLayer));
      s = spd.get_cellValue(i, j);
      d = dir.get_cellValue(i, j);
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
      }
      OGR_G_SetPoint_2D(hGeom, 0, x, y);
      OGR_F_SetGeometry(hFeat, hGeom);
      if (OGR_L_CreateFeature(hLayer, hFeat) != OGRERR_NONE) {
        GDALClose(hDS);
        return 1;
      }
      // OGR_F_SetStyleTable(hFeat, hStyleTable);
      OGR_G_DestroyGeometry(hGeom);
      OGR_F_Destroy(hFeat);
    }
  }
  GDALClose(hDS);
  return 0;
}
