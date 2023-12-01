/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Command line parser and model run for WindNinja
 * Author:   Kyle Shannon <ksshannon@gmail.com>
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

#ifndef CLI_H
#define CLI_H
#ifndef Q_MOC_RUN
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#endif

#include "ninjaArmy.h"
#include "ninja.h"
#include "ninja_conv.h"
#include "ninja_version.h"
#include "gdal_util.h"
#include "ninjaUnits.h"
#include "gdal_util.h"
#include "fetch_factory.h"
namespace po = boost::program_options;

#include <iostream>
#include <iterator>

//#include <QDateTime>

int windNinjaCLI(int argc, char* argv[]);

void conflicting_options(const po::variables_map& vm, const char* opt1, const char* opt2);

void option_dependency(const po::variables_map& vm, const char* for_what, const char* required_option);

void verify_option_set(const po::variables_map& vm, const char* optn);

// this should be used instead of direct 'variables_map["key"].as<T>()' calls since otherwise a single typo
// in the key literal results in undefined behavior that can corrupt memory miles away. 
// Alternatively keys could be defined/used as constants to catch this at compile time
template<typename T>
inline T option_val (const po::variables_map& vm, const char* key) {
    const po::variable_value &vv = vm[key];
    if (vv.empty()) {
        throw logic_error(std::string("no value for option '") + key + "' set.\n");
    } else {
        return vv.as<T>();
    }
}

//bool checkArgs(string arg1, string arg2, string arg3);

#endif /* CLI_H */
