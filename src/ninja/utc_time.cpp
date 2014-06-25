/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Simple utilities to handle utc dates/times
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

#include "utc_time.h"

/**
 * \brief construct an empty UTC Time.
 */
NinjaUtcTime::NinjaUtcTime()
{
    ts = (struct tm*)malloc( sizeof( struct tm ) );
    time( &t );
    struct tm *tmp = gmtime( &t );
    memcpy( ts, tmp, sizeof( struct tm ) );
    t = timegm( ts );
}

/**
 * \brief construct a UTC time with provided parameters
 *
 * \param nYear year
 * \param nMonth month
 * \param nDay day
 * \param nHour hour
 * \param nMinute minute
 */

NinjaUtcTime::NinjaUtcTime( int nYear, int nMonth, int nDay, int nHour,
                            int nMinute, int nSecond )
{
    ts = (struct tm*)malloc( sizeof( struct tm ) );
    ts->tm_year = nYear - 1900;
    ts->tm_mon = nMonth - 1;
    ts->tm_mday = nDay;
    ts->tm_hour = nHour;
    ts->tm_mday = nDay;
    ts->tm_min = nMinute;
    ts->tm_sec = nSecond;
    t = timegm( ts );
}

NinjaUtcTime::~NinjaUtcTime()
{
    free( ts );
}

int NinjaUtcTime::GetYear()
{
    return ts->tm_year + 1900;
}

int NinjaUtcTime::GetMonth()
{
    return ts->tm_mon + 1;
}

int NinjaUtcTime::GetDay()
{
    return ts->tm_mday;
}

int NinjaUtcTime::GetHour()
{
    return ts->tm_hour;
}

int NinjaUtcTime::GetMinute()
{
    return ts->tm_min;
}

int NinjaUtcTime::GetSecond()
{
    return ts->tm_sec;
}

void NinjaUtcTime::AddHours( int nHours )
{
    t += nHours * 3600;
    struct tm *tmp = gmtime( &t );
    memcpy( ts, tmp, sizeof( struct tm ) );
}

/*
** Return a string representation of the object.  Not to be freed.
*/
const char * NinjaUtcTime::StrFTime( const char *format )
{
    szFmt[0] = '\0';
    size_t n = strftime( szFmt, NINJA_UTC_FRMT_SIZE, format, ts );
    return szFmt;
}
