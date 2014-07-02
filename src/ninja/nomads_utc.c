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

#include "nomads_utc.h"

int NomadsUtcCompare( const void *a, const void *b )
{
    nomads_utc *aa = (nomads_utc*)a;
    nomads_utc *bb = (nomads_utc*)b;
    if( aa->ts->tm_year < bb->ts->tm_year )
        return -1;
    else if( bb->ts->tm_year < aa->ts->tm_year )
        return 1;
    else if( aa->ts->tm_mon < bb->ts->tm_mon )
        return -1;
    else if( bb->ts->tm_mon < aa->ts->tm_mon )
        return 1;
    else if( aa->ts->tm_mday < bb->ts->tm_mday )
        return -1;
    else if( bb->ts->tm_mday < aa->ts->tm_mday )
        return 1;
    else if( aa->ts->tm_hour < bb->ts->tm_hour )
        return -1;
    else if( bb->ts->tm_hour < aa->ts->tm_hour )
        return 1;
    else if( aa->ts->tm_min < bb->ts->tm_min )
        return -1;
    else if( bb->ts->tm_min < aa->ts->tm_min )
        return 1;
    else if( aa->ts->tm_sec < bb->ts->tm_sec)
        return -1;
    else if( bb->ts->tm_sec < aa->ts->tm_sec)
        return 1;
    else
        return 0;
}

void NomadsUtcCreate( nomads_utc **u )
{
    (*u) = (nomads_utc*)malloc( sizeof( nomads_utc ) );
    (*u)->ts = (struct tm*)malloc( sizeof( struct tm ) );
    NomadsUtcNow( *u );
    memset( (*u)->s, 0, NOMADS_UTC_STRFTIME_SIZE );
}

void NomadsUtcFree( nomads_utc *u )
{
    free( u->ts );
    free( u );
}

void NomadsUtcNow( nomads_utc *u )
{
    struct tm *tmp;
    if( !u )
        return;
    time( &(u->t) );
    tmp = gmtime( &(u->t) );
    memcpy( u->ts, tmp, sizeof( struct tm ) );
    u->t = timegm( u->ts );
}

void NomadsUtcAddHours( nomads_utc *u, int nHours )
{
    struct tm *tmp;
    if( !u )
        return;
    u->t += nHours * 3600;
    tmp = gmtime( &(u->t) );
    memcpy( u->ts, tmp, sizeof( struct tm ) );
}

void NomadsUtcCopy( nomads_utc *dst, const nomads_utc *src )
{
    if( !dst || !src )
        return;
    dst->t = src->t;
    memcpy( dst->ts, src->ts, sizeof( struct tm ) );
    if( src->s )
        strncpy( dst->s, src->s, NOMADS_UTC_STRFTIME_SIZE );
}

/*
** Return an internal handle to a static buffer that represents a time as
** defined by frmt.  Not to be free'd or modified in any way by the caller.
*/
const char * NomadsUtcStrfTime( nomads_utc *u, const char *frmt )
{
    strftime( u->s, NOMADS_UTC_STRFTIME_SIZE, frmt, u->ts );
    return u->s;
}

void NomadsUtcStrpTime( nomads_utc *u, const char *s, const char *frmt )
{
    strptime( s, frmt, u->ts );
}

