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

#include "cpl_error.h"
#include "cpl_string.h"
#include "cpl_vsi.h"
#include "cpl_http.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NOMADS_OK  0
#define NOMADS_ERR 1

/*
** XXX: Document me.
*/
#define NOMADS_URL_FILTER_TMPLT      "file=%s&lev_10_m_above_ground=on&" \
                                     "lev_2_m_above_ground=on&" \
                                     "lev_entire_atmosphere_(considered_as_a_single_layer)=on" \
                                     "&var_TCDC=on&var_TMP=on&var_UGRD=on&var_VGRD=on&" \
                                     "subregion=&leftlon=%lf&rightlon=%lf&toplat=%lf&bottomlat=%lf" \
                                     "&dir=/%s"
#define NOMADS_URL_CGI        "http://nomads.ncep.noaa.gov/cgi-bin/"

#define NOMADS_GENERIC_VAR_LIST      "TCDC,TMP,UGRD,VGRD"

#define NOMADS_GENERIC_LEVELS_LIST   "10_m_above_ground,2_m_above_ground," \
                                     "entire_atmosphere_(considered_as_a_single_layer)" 

#define NOMADS_SUBREGION             "&subregion=&leftlon=%lf&rightlon=%lf&toplat=%lf&bottomlat=%lf"

#define NOMADS_GENERIC_NAM_DIR         "nam.%s"
#define NOMADS_GENERIC_NAM_DATE        "%Y%m%d"
#define NOMADS_GENERIC_NAM_FCST_HOURS  "0:18:6"
#define NOMADS_GENERIC_NAM_RUN_HOURS   "0:36:3,39:86:6"

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
** NOMADS_HUMAN_READABLE  9       Human readable name
**
** The models are listed in the same order as found on the web page.  Models
** not yet implemented (or may never be) are marked with XXX.
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
#define NOMADS_HUMAN_READABLE       9

static const char *apszNomadsKeys[][10] =
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
      "0:12:12",
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      "0:192:3",
      /* Variable list */
      NOMADS_GENERIC_VAR_LIST,
      /* Levels list */
      "convective_cloud_layer,10_m_above_ground,2_m_above_ground,",
      /* Human readable name */
      "Global Forecast System, 0.5deg" },
    /*
    ** HIRES Alaska
    */
    { /* Name key of the model */
      "hires_alaska",
      /* name of the perl script for the grib filter */
      "filter_hiresak.pl",
      /* File naming format */
      "akarw.t%02dz.awpregf%02d.grib2",
      /* Directory formats, string is date formatted as folowing */
      "hiresw.%s",
      /* Date format for directory */
      NOMADS_GENERIC_NAM_DATE,
      /* Forecast hours, start:stop:stride */
      "6:6:0",
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      "0:48:1",
      /* Variable list */
      NOMADS_GENERIC_VAR_LIST,
      /* Levels list */
      NOMADS_GENERIC_LEVELS_LIST,
      /* Human readable name */
      "HIRES Alaska, 4-5km" },
    /*
    ** HIRES CONUS
    ** Check on what this is.
    */
    { /* Name key of the model */
      "hires_conus",
      /* name of the perl script for the grib filter */
      "filter_hiresconus.pl",
      /* File naming format */
      "conusarw.t%02dz.awp5kmf%02d.grib2",
      /* Directory formats, string is date formatted as folowing */
      "hiresw.%s",
      /* Date format for directory */
      NOMADS_GENERIC_NAM_DATE,
      /* Forecast hours, start:stop:stride */
      "0:12:12",
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      "0:48:1",
      /* Variable list */
      NOMADS_GENERIC_VAR_LIST,
      /* Levels list */
      NOMADS_GENERIC_LEVELS_LIST,
      /* Human readable name */
      "HIRES CONUS, 5km" },
    /*
    ** HIRES CONUS NMM
    */
    { /* Name key of the model */
      "hires_conus_nmm",
      /* name of the perl script for the grib filter */
      "filter_hiresconus.pl",
      /* File naming format */
      "conusnmmb.t%02dz.awp5kmf%02d.grib2",
      /* Directory formats, string is date formatted as folowing */
      "hiresw.%s",
      /* Date format for directory */
      NOMADS_GENERIC_NAM_DATE,
      /* Forecast hours, start:stop:stride */
      "0:12:12",
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      "0:48:1",
      /* Variable list */
      NOMADS_GENERIC_VAR_LIST,
      /* Levels list */
      NOMADS_GENERIC_LEVELS_LIST,
      /* Human readable name */
      "HIRES CONUS NMM, 5km" },
    /* XXX: HIRES Guam */
    /* XXX: HIRES Hawaii */
    /* XXX: HIRES Puerto Rico */
    /*
    ** NAM ALASKA
    */
    { /* Name key of the model */
      "nam_alaska",
      /* name of the perl script for the grib filter */
      "filter_nam_ak.pl",
      /* File naming format */
      "nam.t%02dz.awak3d%02d.grb2.tm00",
      /* Directory formats, string is date formatted as folowing */
      NOMADS_GENERIC_NAM_DIR,
      /* Date format for directory */
      NOMADS_GENERIC_NAM_DATE,
      /* Forecast hours, start:stop:stride */
      NOMADS_GENERIC_NAM_FCST_HOURS,
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      NOMADS_GENERIC_NAM_RUN_HOURS,
      /* Variable list */
      NOMADS_GENERIC_VAR_LIST,
      /* Levels list */
      NOMADS_GENERIC_LEVELS_LIST,
      /* Human readable name */
      "NAM Alaska, 11.25km" },
    /*
    ** NAM CONUS
    */
    { /* Name key of the model */
      "nam_conus",
      /* name of the perl script for the grib filter */
      "filter_nam.pl",
      /* File naming format */
      "nam.t%02dz.awphys%02d.grb2.tm00",
      /* Directory formats, string is date formatted as folowing */
      NOMADS_GENERIC_NAM_DIR,
      /* Date format for directory */
      NOMADS_GENERIC_NAM_DATE,
      /* Forecast hours, start:stop:stride */
      NOMADS_GENERIC_NAM_FCST_HOURS,
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      "0:36:1,39:86:3",
      /* Variable list */
      NOMADS_GENERIC_VAR_LIST,
      /* Levels list */
      NOMADS_GENERIC_LEVELS_LIST,
      /* Human readable name */
      "NAM CONUS, 12km" },
    /*
    ** NAM North America
    */
    { /* Name key of the model */
      "nam_north_america",
      /* name of the perl script for the grib filter */
      "filter_nam_na.pl",
      /* File naming format */
      "nam.t%02dz.awip32%02d.tm00.grib2",
      /* Directory formats, string is date formatted as folowing */
      NOMADS_GENERIC_NAM_DIR,
      /* Date format for directory */
      NOMADS_GENERIC_NAM_DATE,
      /* Forecast hours, start:stop:stride */
      NOMADS_GENERIC_NAM_FCST_HOURS,
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      NOMADS_GENERIC_NAM_RUN_HOURS,
      /* Variable list */
      NOMADS_GENERIC_VAR_LIST,
      /* Levels list */
      NOMADS_GENERIC_LEVELS_LIST,
      /* Human readable name */
      "NAM North America, 32km" },
    /* XXX: NAM Caribbean/Central America */
    /* XXX: NAM Pacific */
    /* XXX: NAM Alaska NEST */
    /*
    ** NAM CONUS NEST
    */
    { /* Name key of the model */
      "nam_conus_nest",
      /* name of the perl script for the grib filter */
      "filter_nam_conusnest.pl",
      /* File naming format */
      "nam.t%02dz.conusnest.hiresf%02d.t00.grib2",
      /* Directory formats, string is date formatted as folowing */
      "hiresw.%s",
      /* Date format for directory */
      NOMADS_GENERIC_NAM_DATE,
      /* Forecast hours, start:stop:stride */
      "0:12:12",
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      "0:48:1",
      /* Variable list */
      NOMADS_GENERIC_VAR_LIST,
      /* Levels list */
      NOMADS_GENERIC_LEVELS_LIST,
      /* Human readable name */
      "NAM CONUS NEST, 5km" },
    /* XXX: NAM Hawaii NEST */
    /* XXX: NAM Puerto Rico NEST */
#ifdef NOMADS_EXPER_FORECASTS
    /* XXX: Alaska RTMA */
    /*
    ** CONUS RTMA
    */
    { /* Name key of the model */
      "rtma_conus",
      /* name of the perl script for the grib filter */
      "filter_rtma2p5.pl",
      /* File naming format */
      "rtma2p5.t%02dz.2dvaranl_ndfd.grb2",
      /* Directory formats, string is date formatted as folowing */
      "rtma2p5.%s",
      /* Date format for directory */
      NOMADS_GENERIC_NAM_DATE,
      /* Forecast hours, start:stop:stride */
      "0:23:1",
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      "0:0:1",
      /* Variable list */
      "TMP,UGRD,VGRD",
      /* Levels list */
      "10_m_above_ground,2_m_above_ground",
      /* Human readable name */
      "RTMA CONUS, 2.5km" },
    /* XXX: Guam RTMA */
    /* XXX: Hawaii RTMA */
    /* XXX: Puerto Rico RTMA */
#endif
    /*
    ** RAP
    */
    { /* Name key of the model */
      "rap",
      /* name of the perl script for the grib filter */
      "filter_rap.pl",
      /* File naming format */
      "rap.t%02dz.awp130pgrbf%02d.grib2",
      /* Directory formats, string is date formatted as folowing */
      "rap.%s",
      /* Date format for directory */
      NOMADS_GENERIC_NAM_DATE,
      /* Forecast hours, start:stop:stride */
      "0:23:1",
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      "0:18:1",
      /* Variable list */
      NOMADS_GENERIC_VAR_LIST,
      /* Levels list */
      NOMADS_GENERIC_LEVELS_LIST,
      /* Human readable name */
      "Rapid Update, 13km" },
    /* XXX: RAP North America */
#ifdef NOMADS_EXPER_FORECASTS
    /*
    ** NARRE
    */
    { /* Name key of the model */
      "narre",
      /* name of the perl script for the grib filter */
      "filter_narre.pl",
      /* File naming format */
      "narre.t%02dz.mean.grd130.f%02d.grib2",
      /* Directory formats, string is date formatted as folowing */
      "narre.%s/ensprod",
      /* Date format for directory */
      "%Y%m%d",
      /* Forecast hours, start:stop:stride */
      "0:24:1",
      /* Forecast run hours, start:stop:stride,start:stop:stride,... */
      "1:12:1",
      /* Variable list */
      "UGRD,VGRD",
      /* Levels list */
      "10_m_above_ground",
      /* Human readable name */
      "North American Reanalysis" },
#endif /* NOMADS_EXPER_FORECASTS */
    { NULL, NULL, NULL }
};


int NomadsFetch( const char *pszModelKey, int nHours, double *padfBbox,
                 const char *pszDstVsiPath );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NOMADS_C_CLIENT_H_ */

