/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Nomads C client
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

#ifndef NOMADS_C_CLIENT_H_
#define NOMADS_C_CLIENT_H_

#include "nomads_utc.h"

#include "cpl_port.h"
#include "cpl_error.h"
#include "cpl_http.h"
#include "cpl_multiproc.h"
#include "cpl_progress.h"
#include "cpl_string.h"
#include "cpl_vsi.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NOMADS_OK  0
#define NOMADS_ERR 1

#define SKIP_DOT_AND_DOTDOT(a) if(EQUAL(a,"..")||EQUAL(a,".")) continue

/*
** XXX: Document me.
*/

#define NOMADS_IP                    "140.90.101.62"
#define NOMADS_HOST                  "nomads.ncep.noaa.gov"

#define NOMADS_URL_CGI_HOST         "http://" NOMADS_HOST "/cgi-bin/"
#define NOMADS_URL_CGI_IP           "http://" NOMADS_IP "/cgi-bin/"

#define NOMADS_URL_CGI               "http://nomads.ncep.noaa.gov/cgi-bin/"

#define NOMADS_GENERIC_VAR_LIST      "TCDC,TMP,UGRD,VGRD"

#define NOMADS_GENERIC_LEVELS_LIST   "10_m_above_ground,2_m_above_ground," \
                                     "entire_atmosphere_(considered_as_a_single_layer)" 

#define NOMADS_SUBREGION             "&subregion=&leftlon=%lf&rightlon=%lf&toplat=%lf&bottomlat=%lf"

#define NOMADS_GENERIC_DIR           "nam.%s"
#define NOMADS_GENERIC_DATE          "%Y%m%d"
#define NOMADS_GENERIC_FCST_HOURS    "0:18:6"
#define NOMADS_GENERIC_RUN_HOURS     "0:36:3,39:86:6"

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
** NOMADS_GRID            10      NWS Grid used for the model
** NOMADS_HUMAN_READABLE  11      Human readable name
**
** The models are listed in the same order as found on the web page.  Models
** not yet implemented (or may never be) are marked with XXX.
**
** XXX: Do not forget commas after the constants defined above!!!
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
    */
    { /* Name key of the model */
      "gfs",
      /* name of the perl script for the grib filter */
      "filter_gfs_hd.pl",
      /* File naming format */
      "gfs.t%02dz.mastergrb2f%02d",
      /* Directory formats, string is date formatted as folowing */
      "gfs.%s%02d/master",
      /* Date format for directory */
      "%Y%m%d",
      /* Forecast hours, start:stop:stride */
      NOMADS_GENERIC_FCST_HOURS,
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      "0:192:3",
      /* Variable list */
      NOMADS_GENERIC_VAR_LIST,
      /* Levels list */
      "convective_cloud_layer,10_m_above_ground,2_m_above_ground,",
      /* Horizontal grid resolution */
      "0.5 deg",
      /* Human readable name */
      "GFS" },
    /*
    ** HIRES Alaska
    */
    {
      "hires_alaska",
      "filter_hiresak.pl",
      "akarw.t%02dz.awpregf%02d.grib2",
      "hiresw.%s",
      NOMADS_GENERIC_DATE,
      "6:6:0",
      "0:48:1",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "5 km",
      "HIRES Alaska" },
    /*
    ** HIRES CONUS
    ** Check on what this is.
    */
    {
      "hires_conus",
      "filter_hiresconus.pl",
      "conusarw.t%02dz.awp5kmf%02d.grib2",
      "hiresw.%s",
      NOMADS_GENERIC_DATE,
      "0:12:12",
      "0:48:1",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "5 km",
      "HIRES CONUS" },
    /*
    ** HIRES CONUS NMM
    */
    {
      "hires_conus_nmm",
      "filter_hiresconus.pl",
      "conusnmmb.t%02dz.awp5kmf%02d.grib2",
      "hiresw.%s",
      NOMADS_GENERIC_DATE,
      "0:12:12",
      "0:48:1",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "5 km",
      "HIRES CONUS NMM" },
    /* XXX: HIRES Guam */
    /* XXX: HIRES Hawaii */
    /* XXX: HIRES Puerto Rico */
    /*
    ** NAM ALASKA
    */
    {
      "nam_alaska",
      "filter_nam_ak.pl",
      "nam.t%02dz.awak3d%02d.grb2.tm00",
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
      "nam.t%02dz.awphys%02d.grb2.tm00",
      NOMADS_GENERIC_DIR,
      NOMADS_GENERIC_DATE,
      NOMADS_GENERIC_FCST_HOURS,
      "0:36:1,39:86:3",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "12 km",
      "NAM CONUS" },
    /*
    ** NAM North America
    */
    {
      "nam_north_america",
      "filter_nam_na.pl",
      "nam.t%02dz.awip32%02d.tm00.grib2",
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
      "nam_alaska_nest",
      "filter_nam_alaskanest.pl",
      "nam.t%02dz.alaskanest.hiresf%02d.tm00.grib2",
      NOMADS_GENERIC_DIR,
      NOMADS_GENERIC_DATE,
      NOMADS_GENERIC_FCST_HOURS,
      "0:36:1,39:60:3",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "6 km",
      "NAM Alaska NEST" },
    /*
    ** NAM CONUS NEST
    */
    {
      "nam_conus_nest",
      "filter_nam_conusnest.pl",
      "nam.t%02dz.conusnest.hiresf%02d.tm00.grib2",
      NOMADS_GENERIC_DIR,
      NOMADS_GENERIC_DATE,
      "0:12:12",
      "0:36:1,39:60:3",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "4 km",
      "NAM CONUS NEST" },
    /* XXX: NAM Hawaii NEST */
    /* XXX: NAM Puerto Rico NEST */
#ifdef NOMADS_EXPER_FORECASTS
    /* XXX: Alaska RTMA */
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
      "TMP,UGRD,VGRD",
      "10_m_above_ground,2_m_above_ground",
      "2.5 km",
      "RTMA CONUS" },
    /* XXX: Guam RTMA */
    /* XXX: Hawaii RTMA */
    /* XXX: Puerto Rico RTMA */
#endif
    /*
    ** RAP
    */
    {
      "rap",
      "filter_rap.pl",
      "rap.t%02dz.awp130pgrbf%02d.grib2",
      "rap.%s",
      NOMADS_GENERIC_DATE,
      "0:23:1",
      "0:18:1",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "13 km",
      "RAP CONUS" },
    /*
    ** RAP North America
    */
    {
      "rap_north_america",
      "filter_rap32.pl",
      "rap.t%02dz.awipf%02d.grib2",
      "rap.%s",
      NOMADS_GENERIC_DATE,
      "0:23:1",
      "0:18:1",
      NOMADS_GENERIC_VAR_LIST,
      NOMADS_GENERIC_LEVELS_LIST,
      "32 km",
      "RAP North America" },
#ifdef NOMADS_EXPER_FORECASTS
    /*
    ** NARRE
    */
    {
      "narre",
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

int NomadsFetch( const char *pszModelKey, int nHours, double *padfBbox,
                 const char *pszDstVsiPath, char ** papszOptions,
                 GDALProgressFunc pfnProgress );
const char ** NomadsFindModel( const char *pszKey );

static void * hNomadsMutex;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NOMADS_C_CLIENT_H_ */

