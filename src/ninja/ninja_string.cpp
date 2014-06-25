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

#include <stdio.h>
#include "ninja_string.h"

int ninjastr_create(ninjastr **str, const char *data, size_t hint)
{
    int len = strlen(data) + 1;
    int size = len > hint ? len : hint;
    *str = (ninjastr*)malloc(sizeof(ninjastr));
    if(*str == NULL)
        return NINJA_STRING_ERR;
    (*str)->data = (char*)malloc(size);
    if((*str)->data == NULL)
    {
        free(*str);
        *str = NULL;
        return NINJA_STRING_ERR;
    }
    if(data)
        strcpy((*str)->data, data);
    else
        (*str)->data[0] = '\0';
    (*str)->size = len - 1;
    (*str)->capacity = size;
    (*str)->err = NINJA_STRING_OK;
    (*str)->i = 0;
    return NINJA_STRING_OK;
}

void ninja_free(ninjastr *str)
{
    free(str->data);
    free(str);
}

int ninjastr_set(ninjastr *str, const char *data)
{
    if(str == NULL)
        return NINJA_STRING_ERR;
    if(data == NULL)
    {
        str->data[0] = '\0';
        return NINJA_STRING_OK;
    }
    int len = strlen(data);
    if(len > str->capacity)
    {
        str->data = (char*)realloc(str->data, len + 1);
        str->capacity = len + 1;
    }
    str->size = len;
    strcpy(str->data, data);
    str->i = 0;
    str->err = NINJA_STRING_OK;
    return NINJA_STRING_OK;
}


int ninjastr_size(ninjastr *str)
{
    if(str == NULL)
        return 0;
    return str->size;
}

int ninjastr_capacity(ninjastr *str)
{
    if(str == NULL)
        return 0;
    return str->capacity;
}

int ninjastr_seek(ninjastr *str, int whence, size_t i)
{
    if(str == NULL)
        return NINJA_STRING_ERR;
    if(i < 0 || i > str->size)
        return NINJA_STRING_ERR;
    if(whence == NINJA_STRING_START)
        str->i = i;
    else if(whence == NINJA_STRING_END)
        str->i = str->size - i - 1;
    return NINJA_STRING_OK;
}

char ninjastr_index(ninjastr *str, int i)
{
    if(str == NULL)
        return '\0';
    if(i > str->size)
        return '\0';
    return str->data[i];
}

char ninjastr_next(ninjastr *str, int way)
{
    if(str == NULL)
        return '\0';
    if(str->i < 0 || str->i > str->size)
        return '\0';
    char c;
    if(way == NINJA_STRING_FWD)
        c = str->data[str->i++];
    else if(way == NINJA_STRING_REV)
        c = str->data[str->i--];
    return c;
}

int ninjastr_err(ninjastr *str)
{
    if(str == NULL)
        return NINJA_STRING_ERR;
    return str->err;
}

int ninjastr_equal(ninjastr *str1, ninjastr *str2)
{
    if(str1 == NULL && str2 == NULL)
        return 1;
    else if( str1 == NULL || str2 == NULL )
        return 0;
    return strcmp(str1->data, str2->data) == 0;
}

int ninjastr_equaln(ninjastr *str1, ninjastr *str2, int n)
{
    if(str1 == NULL && str2 == NULL)
        return 1;
    else if( str1 == NULL || str2 == NULL )
        return 0;
    return strncmp(str1->data, str2->data, n) == 0;
}

int main()
{
    ninjastr *s;
    ninjastr_create(&s, "Kyle is mars.", 0);
    printf("%s\n", s->data);
    char c;
    while((c = ninjastr_next(s, NINJA_STRING_FWD)) != '\0')
        printf("%c\n", c);
    while((c = ninjastr_next(s, NINJA_STRING_FWD)) != '\0')
        printf("%c\n", c);
    ninjastr_seek(s, NINJA_STRING_END, 0 );
    while((c = ninjastr_next(s, NINJA_STRING_REV)) != '\0')
        printf("%c\n", c);
    ninja_free(s);
    return 0;
}

