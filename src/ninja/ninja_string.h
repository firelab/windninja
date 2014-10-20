/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  String class for WindNinja
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

#ifndef NINJA_STRING_H_
#define NINJA_STRING_H_

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define NINJA_STRING_OK    0
#define NINJA_STRING_ERR   1

#define NINJA_STRING_START 0
#define NINJA_STRING_END   1

#define NINJA_STRING_FWD   0
#define NINJA_STRING_REV   1

typedef struct _ninjastr
{
    /* character array */
    char *data;
    /* current size */
    size_t size;
    /* capacity */
    size_t capacity;
    /* iterator */
    size_t i;
    /* status */
    unsigned int err;
} ninjastr;

int ninjastr_create(ninjastr **str, const char *data, size_t hint);
int ninjastr_free(ninjastr *str);
int ninjastr_size(ninjastr *str);
int ninjastr_capacity(ninjastr *str);
char ninjastr_index(ninjastr *str, int i);
int ninjastr_seek(ninjastr *str, int whence, size_t i);
char ninjastr_next(ninjastr *str, int way);
int ninjastr_err(ninjastr *str);
int ninjastr_equal(ninjastr *str1, ninjastr *str2);
int ninjastr_equaln(ninjastr *str1, ninjastr *str2);

#endif /* NINJA_STRING_H_ */

