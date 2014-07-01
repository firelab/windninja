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

#ifndef NOMADS_UTC_INCLUDED_
#define NOMADS_UTC_INCLUDED_

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_INTERNAL_TIME_GM
time_t timegm( const struct tm *tm );
#endif
#ifdef USE_INTERNAL_STRPTIME
char * strptime( const char * __restrict,
                 const char * __restrict,
                 struct tm * __restrict);
#endif

#define NOMADS_UTC_STRFTIME_SIZE 4096

typedef struct nomads_utc
{
    time_t t;
    struct tm *ts;
    char s[NOMADS_UTC_STRFTIME_SIZE];
} nomads_utc;

void NomadsUtcCreate( nomads_utc **u );
void NomadsUtcFree( nomads_utc *u );
void NomadsUtcNow( nomads_utc *u );
void NomadsUtcAddHours( nomads_utc *u, int nHours );
void NomadsUtcCopy( nomads_utc *dst, const nomads_utc *src );
int NomadsUtcCompare( const void *a, const void *b );
const char * NomadsUtcStrfTime( nomads_utc *u, const char *frmt );
void NomadsUtcStrpTime( nomads_utc *u, const char *s, const char *frmt );

#ifdef __cplusplus
}
#endif

#endif /* NOMADS_UTC_INCLUDED_ */

