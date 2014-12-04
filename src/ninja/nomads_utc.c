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

static void NomadsUpdateTimeStruct( nomads_utc *u )
{
    struct tm *tmp;
    if( !u )
        return;
    tmp = gmtime( &(u->t) );
    memcpy( u->ts, tmp, sizeof( struct tm ) );
    u->t = timegm( u->ts );
}

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
    if( !u )
        return;
    free( u->ts );
    free( u );
}

void NomadsUtcNow( nomads_utc *u )
{
    struct tm *tmp;
    if( !u )
        return;
    time( &(u->t) );
    NomadsUpdateTimeStruct( u );
}

void NomadsUtcFromTimeT( nomads_utc *u, time_t t )
{
    if( !u )
        return;
    u->t = t;
    NomadsUpdateTimeStruct( u );
}

int NomadsUtcFromIsoFrmt( nomads_utc *u, const char *s )
{
    char *p, *t;
    int i, rc;
    int dt[3];
    if( !u || !s )
        return 1;
    if( strlen( s ) != strlen( "YYYYmmddTHH:MM:SS" ) )
    {
        return 1;
    }
    rc = 0;
    t = strdup( s );
    /*
    ** Start from the rear, check for seconds, minutes, hours, 'T', day, month,
    ** year.  If anything is missing, fail.  This has to be a full ISO string,
    ** minus the timezone and daylight qualifiers, as this is UTC only.
    **
    ** YYYYmmddTHH:MM:SS
    **
    */

    /* Time half */
    memset( dt, 0, sizeof( int ) * 3 );
    p = strrchr( t, ':' );
    i = 0;
    while( p && i < 3 )
    {
        if( strlen( p ) > 2 )
        {
            dt[i] = atoi( p + 1 );
        }
        else
        {
            rc = 1;
            goto cleanup;
        }
        *p = '\0';
        p = strrchr( t, ':' );
        i++;
    }
    if( i < 2 )
    {
        rc = 1;
        goto cleanup;
    }
    p = strrchr( t, 'T' );
    if( !p )
    {
        rc = 1;
        goto cleanup;
    }
    if( strlen( p ) > 2 )
    {
        dt[i++] = atoi( p + 1 );
    }
    else
    {
        rc = 1;
        goto cleanup;
    }
    *p = '\0';
    u->ts->tm_sec = dt[0];
    u->ts->tm_min = dt[1];
    u->ts->tm_hour = dt[2];

    memset( dt, 0, sizeof( int ) * 3 );
    if( strlen( t ) != strlen( "YYYYmmdd" ) )
    {
        rc = 1;
        goto cleanup;
    }
    p -= 2;
    i = 0;
    while( i < 2 && p != t)
    {
        dt[i] = atoi( p );
        *p = '\0';
        p -= 2;
        i++;
    }
    dt[2] = atoi( t );

    u->ts->tm_mday = dt[0];
    u->ts->tm_mon = dt[1] - 1;
    u->ts->tm_year = dt[2] - 1900;

    /* Simple sanity check */
    if( u->ts->tm_sec < 0 || u->ts->tm_sec > 61 ||
        u->ts->tm_min < 0 || u->ts->tm_min > 59 ||
        u->ts->tm_hour < 0 || u->ts->tm_hour > 23 ||
        u->ts->tm_mday < 1 || u->ts->tm_mday > 31 ||
        u->ts->tm_mon < 0 || u->ts->tm_mon > 11 ||
        u->ts->tm_year < 0 )
    {
        rc = 1;
    }

cleanup:
    free( t );
    return rc;
}

void NomadsUtcAddHours( nomads_utc *u, int nHours )
{
    if( !u )
        return;
    NomadsUtcAddSeconds( u, (time_t)(nHours * 3600) );
}

void NomadsUtcAddSeconds( nomads_utc *u, time_t nSeconds )
{
    if( !u )
        return;
    u->t += nSeconds;
    NomadsUpdateTimeStruct( u );
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
    if( !u )
        return NULL;
    strftime( u->s, NOMADS_UTC_STRFTIME_SIZE, frmt, u->ts );
    return u->s;
}

