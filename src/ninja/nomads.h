/******************************************************************************
 *
 * Project:  WindNinja
 * Purpose:  Nomads C client
 * Author:   Kyle Shannon <kyle at pobox dot com>
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

#ifndef NOMADS_C_CLIENT_H_
#define NOMADS_C_CLIENT_H_

#include "nomads_utc.h"

#include <assert.h>

#include "cpl_port.h"
#include "cpl_error.h"
#include "cpl_http.h"
#include "cpl_multiproc.h"
#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,10,0)
#include "cpl_progress.h"
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,10,0) */
#else /* GDAL_COMPUTE_VERSION */
#include "gdal.h"
#endif /* GDAL_COMPUTE_VERSION */

#include "gdal_alg.h"
#include "gdalwarper.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NOMADS_OK  0
#define NOMADS_ERR 1

/*
** Helper for bbox ordering
*/
#define NOMADS_XMIN 0
#define NOMADS_XMAX 1
#define NOMADS_YMIN 3
#define NOMADS_YMAX 2

#ifndef SKIP_DOT_AND_DOTDOT
#define SKIP_DOT_AND_DOTDOT(a) if(EQUAL(a,"..")||EQUAL(a,".")) continue
#endif

/*
** We can't do threaded download on older GDAL versions (pre-1.10.0) due to
** lack of an API.  Handle that here.
*/
#ifdef NOMADS_ENABLE_ASYNC
 #ifdef GDAL_COMPUTE_VERSION
  #if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(1,10,0)
   #undef NOMADS_ENABLE_ASYNC
  #endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(1,10,0) */
 #else /* GDAL_COMPUTE_VERSION */
  #undef NOMADS_ENABLE_ASYNC
 #endif /* GDAL_COMPUTE_VERSION */
#endif /* NOMADS_ENABLE_ASYNC */

/*
** In earlier versions of GDAL, the thread api has an issue with CURL failing
** to lookup the ip from the hostname using DNS.  This is an attempt to work
** around that.  It has been fixed in GDAL 1.11.x.  Older versions may want to
** try defining NOMADS_USE_IP.
*/
#define NOMADS_IP                        "140.90.101.62"
#define NOMADS_HOST                      "nomads.ncep.noaa.gov"

/* Host for NOMADS */
#define NOMADS_URL_CGI_HOST              "https://" NOMADS_HOST "/cgi-bin/"
#define NOMADS_URL_CGI_IP                "https://" NOMADS_IP "/cgi-bin/"

/*
** cgi path, ignoring the IP issues, not used but left in for reference.  Use
** the two constant urls above.
*/
#define NOMADS_URL_CGI                   "https://nomads.ncep.noaa.gov/cgi-bin/"

/*
** NAM based defaults.  Most models are derived or based on NAM, so we can use
** NAM variables and levels for many submodels.  Thes #defines are variable
** names and heights for NAM.  Use when possible.
*/
#define NOMADS_GENERIC_VAR_LIST          "TCDC,TMP,UGRD,VGRD"
#define NOMADS_GENERIC_3D_VAR_LIST       "DZDT,HGT," NOMADS_GENERIC_VAR_LIST
#define NOMADS_GENERIC_LEVELS_LIST       "2_m_above_ground,10_m_above_ground," \
                                         "entire_atmosphere_(considered_as_a_single_layer)"
#define NOMADS_GENERIC_3D_LEVELS_LIST    "surface,2_m_above_ground," \
                                         "10_m_above_ground,1000_mb,975_mb," \
                                         "950_mb,925_mb,900_mb,875_mb,850_mb," \
                                         "825_mb,800_mb,775_mb,750_mb,725_mb," \
                                         "700_mb,675_mb,650_mb,625_mb,600_mb," \
                                         "575_mb,550_mb,525_mb,500_mb," \
                                         "entire_atmosphere_(considered_as_a_single_layer)"

/* arg list for bounding box */
#define NOMADS_SUBREGION                 "&subregion=&leftlon=%lf&rightlon=%lf&toplat=%lf&bottomlat=%lf"

/* More NAM based defaults */
#define NOMADS_GENERIC_DIR               "nam.%s"
#define NOMADS_GENERIC_DATE              "%Y%m%d"
#define NOMADS_GENERIC_FCST_HOURS        "0:18:6"
#define NOMADS_GENERIC_RUN_HOURS         "0:36:3,39:86:6"
#define NOMADS_NEST_RUN_HOURS            "0:60:1"
/*
** The extension for all NAM models.  This changed in TIN 16-41, so we move it
** to a metadata item.
*/
#define NOMADS_NAM_FILE_EXT              ".tm00.grib2"

/*
** The following list of metadata is for models residing at
** http://nomads.ncep.noaa.gov.  The indices contain the following info
** (documented in GFS as well):
**
** Convenience index      Index   Description
** NOMADS_NAME            0       Name key of the model
** NOMADS_FILTER_BIN      1       name of the perl script for the grib filter
** NOMADS_FILE_NAME_FRMT  2       File naming format
** NOMADS_DIR_FRMT        3       Directory formats, string is date formatted 
**                                as folowing
** NOMADS_DIR_DATE_FRMT   4       Date format for directory
** NOMADS_FCST_HOURS      5       Forecast hours, start:stop:stride
** NOMADS_FCST_RUN_HOURS  6       Forecast run hours, in the format of
**                                start:stop:stride,start:stop:stride,...
** NOMADS_VARIABLES       7       Variable list
** NOMADS_LEVELS          8       Levels list
** NOMADS_GRID_RES        9       Horizontal resolution, as "value unit"
**                                ("0.5 deg" or "12 km")
** NOMADS_HUMAN_READABLE  10     Human readable name
**
** The models are listed in the same order as found on the web page.  Models
** not yet implemented (or may never be) are marked with XXX.
**
** XXX: Do not forget commas after the constants defined above!!! XXX
*/

#define NOMADS_NAME                 0
#define NOMADS_FILTER_BIN           1
#define NOMADS_FILE_NAME_FRMT       2
#define NOMADS_DIR_FRMT             3
#define NOMADS_DIR_DATE_FRMT        4
#define NOMADS_FCST_HOURS           5
#define NOMADS_FCST_RUN_HOURS       6
#define NOMADS_VARIABLES            7
#define NOMADS_LEVELS               8
#define NOMADS_GRID_RES             9
#define NOMADS_HUMAN_READABLE       10

static const char *apszNomadsKeys[][11] =
{
    /*
    ** GFS
    **
    ** Note that there are 3 resolutions available.  Depending on the
    ** situation, we may want to use one over the other.
    */
    { /* Name key of the model */
      "gfs_global",
      /* name of the perl script for the grib filter */
#if defined(NOMADS_GFS_0P5DEG)
      "filter_gfs_0p50.pl",
#elif defined(NOMADS_GFS_1P0DEG)
      "filter_gfs_1p00.pl",
#else /* NOMADS_GFS_0.25DEG is default */
      /*
      ** The hourly output has a different script in nomads, but this one seems
      ** to work too.  The official hourly is:
      **
      ** filter_gfs_0p25_1hr.pl
      */
#if defined(NOMADS_GFS_0P5DEG) || defined(NOMADS_GFS_1P0DEG)
      "filter_gfs_0p25_1hr.pl",
#else
      "filter_gfs_0p25.pl",
#endif /* defined(NOMADS_GFS_0P5DEG) || defined(NOMADS_GFS_1P0DEG) */
#endif
      /* File naming format */
#if defined(NOMADS_GFS_0P5DEG)
      "gfs.t%02dz.pgrb2full.0p50.f%03d",
#elif defined(NOMADS_GFS_1P0DEG)
      "gfs.t%02dz.pgrb2.1p00.f%03d",
#else /* NOMADS_GFS_0.25DEG is default */
      "gfs.t%02dz.pgrb2.0p25.f%03d",
#endif
      /* Directory formats, string is date formatted as folowing */
      "gfs.%s/%02d",
      /* Date format for directory */
      "%Y%m%d",
      /* Forecast hours, start:stop:stride */
      NOMADS_GENERIC_FCST_HOURS,
#if defined(NOMADS_GFS_0P5DEG) || defined(NOMADS_GFS_1P0DEG)
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      "0:240:3,252:384:12",
#else
      /* Hourly for 1/4 degree */
      "0:120:1,123:240:3,252:384:12",
#endif /* defined(NOMADS_GFS_0P5DEG) || defined(NOMADS_GFS_1P0DEG) */
      /* Variable list */
      NOMADS_GENERIC_VAR_LIST,
      /* Level list. XXX Note entire_atmosphere instead of the default */
      "2_m_above_ground,10_m_above_ground,entire_atmosphere",
      /* Horizontal grid resolution */
#if defined(NOMADS_GFS_0P5DEG)
      "0.5 deg",
#elif defined(NOMADS_GFS_1P0DEG)
      "1.0 deg",
#else /* NOMADS_GFS_0.25DEG is default */
      "0.25 deg",
#endif
      /* Human readable name */
      "GFS Global" },
    /* XXX: Climate Forecast System Flux (CFS)??? */
    /*
    ** HIRES Alaska
    */
    {
      "hires_arw_alaska",
      "filter_hiresak.pl",
      "hiresw.t%02dz.arw_5km.f%02d.ak.grib2",
      "hiresw.%s",
      NOMADS_GENERIC_DATE,
      "6:6:0",
      "0:48:1",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "5 km",
      "HIRES ARW Alaska" },
    /*
    ** HIRES Alaska NMM
    */
    {
      "hires_nmm_alaska",
      "filter_hiresak.pl",
      "hiresw.t%02dz.nmmb_5km.f%02d.ak.grib2",
      "hiresw.%s",
      NOMADS_GENERIC_DATE,
      "6:6:0",
      "0:48:1",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "5 km",
      "HIRES NMM Alaska" },
    /*
    ** HIRES CONUS
    */
    {
      "hires_arw_conus",
      "filter_hiresconus.pl",
      "hiresw.t%02dz.arw_5km.f%02d.conus.grib2",
      "hiresw.%s",
      NOMADS_GENERIC_DATE,
      "0:12:12",
      "0:48:1",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "5 km",
      "HIRES ARW CONUS" },
    /*
    ** HIRES CONUS NMM
    */
    {
      "hires_nmm_conus",
      "filter_hiresconus.pl",
      "hiresw.t%02dz.nmmb_5km.f%02d.conus.grib2",
      "hiresw.%s",
      NOMADS_GENERIC_DATE,
      "0:12:12",
      "0:48:1",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "5 km",
      "HIRES NMM CONUS" },
    /* XXX: HIRES Guam */
    /* XXX: HIRES Hawaii */
    /* XXX: HIRES Puerto Rico */
    /*
    ** NAM ALASKA
    */
    {
      "nam_alaska",
      "filter_nam_ak.pl",
      "nam.t%02dz.awak3d%02d" NOMADS_NAM_FILE_EXT,
      NOMADS_GENERIC_DIR,
      NOMADS_GENERIC_DATE,
      NOMADS_GENERIC_FCST_HOURS,
      NOMADS_GENERIC_RUN_HOURS,
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "11.25 km",
      "NAM Alaska" },
    /*
    ** NAM CONUS
    */
    {
      "nam_conus",
      "filter_nam.pl",
      "nam.t%02dz.awphys%02d" NOMADS_NAM_FILE_EXT,
      NOMADS_GENERIC_DIR,
      NOMADS_GENERIC_DATE,
      NOMADS_GENERIC_FCST_HOURS,
      "0:36:1,39:86:3",
#ifdef NOMADS_ENABLE_3D
      NOMADS_GENERIC_3D_VAR_LIST,
      NOMADS_GENERIC_3D_LEVELS_LIST,
#else
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
#endif
      "12 km",
      "NAM CONUS" },
    /*
    ** NAM North America
    */
    {
      "nam_north_america",
      "filter_nam_na.pl",
      "nam.t%02dz.awip32%02d" NOMADS_NAM_FILE_EXT,
      NOMADS_GENERIC_DIR,
      NOMADS_GENERIC_DATE,
      NOMADS_GENERIC_FCST_HOURS,
      NOMADS_GENERIC_RUN_HOURS,
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "32 km",
      "NAM North America" },
    /* XXX: NAM Caribbean/Central America */
    /* XXX: NAM Pacific */
    /*
    ** NAM Alaska NEST
    */
    {
      "nam_nest_alaska",
      "filter_nam_alaskanest.pl",
      "nam.t%02dz.alaskanest.hiresf%02d" NOMADS_NAM_FILE_EXT,
      NOMADS_GENERIC_DIR,
      NOMADS_GENERIC_DATE,
      NOMADS_GENERIC_FCST_HOURS,
      NOMADS_NEST_RUN_HOURS,
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "3 km",
      "NAM NEST Alaska" },
    /*
    ** NAM CONUS NEST
    */
    {
      "nam_nest_conus",
      "filter_nam_conusnest.pl",
      "nam.t%02dz.conusnest.hiresf%02d" NOMADS_NAM_FILE_EXT,
      NOMADS_GENERIC_DIR,
      NOMADS_GENERIC_DATE,
      NOMADS_GENERIC_FCST_HOURS,
      NOMADS_NEST_RUN_HOURS,
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "3 km",
      "NAM NEST CONUS" },
    /* XXX: NAM Hawaii NEST */
    /* XXX: NAM Puerto Rico NEST */
#ifdef NOMADS_EXPER_FORECASTS
    /* Alaska RTMA */
    {
      "rtma_ak",
      "filter_akrtma.pl",
      "akrtma.t%02dz.2dvaranl_ndfd.grb2",
      "akrtma.%s",
      NOMADS_GENERIC_DATE,
      "0:23:1",
      "0:0:1",
      "TMP,UGRD,VGRD",
      "10_m_above_ground,2_m_above_ground",
      "2.5 km",
      "RTMA ALASKA" },
#endif /* NOMADS_EXPER_FORECASTS */
#ifdef NOMADS_RTMA
    /*
    ** CONUS RTMA
    */
    {
      "rtma_conus",
      "filter_rtma2p5.pl",
      "rtma2p5.t%02dz.2dvaranl_ndfd.grb2",
      "rtma2p5.%s",
      NOMADS_GENERIC_DATE,
      "0:23:1",
      "0:0:1",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "2.5 km",
      "RTMA CONUS" },
#endif /* NOMADS_RTMA */
#ifdef NOMADS_EXPER_FORECASTS
    /* Guam RTMA */
    {
      "rtma_gu",
      "filter_gurtma.pl",
      "gurtma.t%02dz.2dvaranl_ndfd.grb2",
      "gurtma.%s",
      NOMADS_GENERIC_DATE,
      "0:23:1",
      "0:0:1",
      "TMP,UGRD,VGRD",
      "10_m_above_ground,2_m_above_ground",
      "2.5 km",
      "RTMA GUAM" },
    /* Hawaii RTMA */
    {
      "rtma_hi",
      "filter_hirtma.pl",
      "hirtma.t%02dz.2dvaranl_ndfd.grb2",
      "hirtma.%s",
      NOMADS_GENERIC_DATE,
      "0:23:1",
      "0:0:1",
      "TMP,UGRD,VGRD",
      "10_m_above_ground,2_m_above_ground",
      "2.5 km",
      "RTMA HAWAII" },
    /* Puerto Rico RTMA */
    {
      "rtma_pr",
      "filter_prrtma.pl",
      "prrtma.t%02dz.2dvaranl_ndfd.grb2",
      "prrtma.%s",
      NOMADS_GENERIC_DATE,
      "0:23:1",
      "0:0:1",
      "TMP,UGRD,VGRD",
      "10_m_above_ground,2_m_above_ground",
      "2.5 km",
      "RTMA PUERTO RICO" },
#endif /* NOMADS_EXPER_FORECASTS */
    /*
    ** HRRR Alaska
    */
    {
      "hrrr_alaska",
      "filter_hrrrak_2d.pl",
      "hrrr.t%02dz.wrfsfcf%02d.ak.grib2",
      "hrrr.%s/alaska",
      NOMADS_GENERIC_DATE,
      "0:23:1",
      "0:18:1",
      NOMADS_GENERIC_VAR_LIST,
      "2_m_above_ground,10_m_above_ground," \
      "entire_atmosphere",
      "3 km",
      "HRRR ALASKA" },
    /*
    ** HRRR Conus
    */
    {
      "hrrr_conus",
      "filter_hrrr_2d.pl",
      "hrrr.t%02dz.wrfsfcf%02d.grib2",
      "hrrr.%s/conus",
      NOMADS_GENERIC_DATE,
      "0:23:1",
      "0:18:1",
      NOMADS_GENERIC_VAR_LIST,
      "2_m_above_ground,10_m_above_ground," \
      "entire_atmosphere",
      "3 km",
      "HRRR CONUS" },
    /*
    ** RAP
    */
    {
      "rap_conus",
      "filter_rap.pl",
      "rap.t%02dz.awp130pgrbf%02d.grib2",
      "rap.%s",
      NOMADS_GENERIC_DATE,
      "0:23:1",
      "0:18:1",
      NOMADS_GENERIC_VAR_LIST,
      /*
      ** The August 2016 TIN changed the level for cloud cover to
      ** entire_atmosphere.  Request both levels so if they change it, we won't
      ** fail.
      */
      NOMADS_GENERIC_LEVELS_LIST ",entire_atmosphere",
      "13 km",
      "RAP CONUS" },
    /*
    ** RAP North America
    */
    {
      "rap_north_america",
      "filter_rap32.pl",
      "rap.t%02dz.awip32f%02d.grib2",
      "rap.%s",
      NOMADS_GENERIC_DATE,
      "0:23:1",
      "0:18:1",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST ",entire_atmosphere",
      "32 km",
      "RAP North America" },
#ifdef NOMADS_EXPER_FORECASTS
    /*
    ** NARRE
    */
    {
      "narr",
      "filter_narre.pl",
      "narre.t%02dz.mean.grd130.f%02d.grib2",
      "narre.%s/ensprod",
      "%Y%m%d",
      "0:24:1",
      "1:12:1",
      "UGRD,VGRD",
      "10_m_above_ground",
      "11 km",
      "NARR" },
#endif /* NOMADS_EXPER_FORECASTS */
    { NULL, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL, NULL,
      NULL }
};

typedef struct NomadsThreadData
{
    const char *pszUrl;
    const char *pszFilename;
    int nErr;
} NomadsThreadData;

int NomadsFetch( const char *pszModelKey,  const char *pszRefTime, 
                 int nHours, int nStride, double *padfBbox,
                 const char *pszDstVsiPath, char ** papszOptions,
                 GDALProgressFunc pfnProgress );
const char ** NomadsFindModel( const char *pszKey );

char * NomadsFormName( const char *pszKey, char pszSpacer );

void NomadsFree( void *p );

#ifdef NOMADS_INTERNAL_VRT
GDALDatasetH
NomadsAutoCreateWarpedVRT(GDALDatasetH hSrcDS,
                          const char *pszSrcWKT,
                          const char *pszDstWKT,
                          GDALResampleAlg eResampleAlg,
                          double dfMaxError,
                          const GDALWarpOptions *psOptionsIn);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NOMADS_C_CLIENT_H_ */

