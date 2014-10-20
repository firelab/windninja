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

#ifndef NINJA_UTC_TIME_H_
#define NINJA_UTC_TIME_H_

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef USE_INTERNAL_TIME_GM
#ifdef __cplusplus
extern "C" {
#endif
time_t timegm(const struct tm *tm);
#ifdef __cplusplus
}
#endif
#endif

#define NINJA_UTC_FRMT_SIZE 512

#define IS_LEAP_YEAR(x) ((((x) % 4 == 0) && ((x) % 100 != 0)) || ((x) % 400 == 0))

static const int NinjaDaysInMonth[] = { 31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31 };

class NinjaUtcTime
{

public:
    NinjaUtcTime();
    NinjaUtcTime( int nYear, int nMonth, int nDay,
                  int nHour, int nMinute, int nSecond );
    ~NinjaUtcTime();
    int IsLeapYear() const;
    int GetYear();
    int GetMonth();
    int GetDay();
    int GetHour();
    int GetMinute();
    int GetSecond();

    void AddHours( int nHours );
    const char *StrFTime( const char *format );

private:
    struct tm *ts;
    time_t t;
    char szFmt[NINJA_UTC_FRMT_SIZE];
};

#endif /* NINJA_UTC_TIME_H_ */

