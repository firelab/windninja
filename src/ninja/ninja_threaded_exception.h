/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Handle threading exceptions
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

#ifndef NINJA_THREADED_EXCEPTION_H_
#define NINJA_THREADED_EXCEPTION_H_

#include <exception>
#include <string>
#include <vector>

#include "wxModelInitialization.h"

/*
** Mimic exception handling, attempting to bypass OpenMP shortcomings regarding
** exception handling
*/

#define NO_EXCEPTION            0
#define STD_RUNTIME_EXC         1
#define STD_BAD_ALLOC_EXC       2
#define STD_LOGIC_EXC           3
#define STD_EXC                 4
#define STD_UNKNOWN_EXC         5
#define NINJA_CANCEL_USER_EXC   101
#define NINJA_BAD_FORECAST_EXC  102

void NinjaRethrowThreadedException( std::vector<int> anErrors,
                                    std::vector<std::string> asMessages,
                                    int nThreads );

#endif /* NINJA_THREADED_EXCEPTION_H_ */

