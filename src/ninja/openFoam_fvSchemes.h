/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for writing native solver polyMesh files
 * Author:   Loren Atwood <pianotocador@gmail.com>
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

#ifndef OPENFOAM_FVSCHEMES_H
#define OPENFOAM_FVSCHEMES_H

#include <vector>
#include <iostream>

class openFoam_fvSchemes
{
public:

    /*
     * Because each fvScheme is so similar, the idea is to create a container for each
     * type of fvSchemes that contains the default for each scheme, the different names needed for an fvScheme
     * and the values needed for each fvScheme name. This will group them all together so it is a nice easy
     * function call with little code to setup and pull out all the different schemes for writing an fvSchemes file
     * In addition, this also figures out the needed amount of whitespace to output all the values in a nice order
     */

    openFoam_fvSchemes();

    void setupDesiredValues(std::string simulationType);

    // what if output were a single vector of vectors of vectors that holds all this stuff together
    // would work, but would be harder to tell what information was getting received and what it would be for
    std::vector<std::string> get_types();
    std::vector<std::string> get_defaultwhitespace();
    std::vector<std::string> get_defaultvalues();
    std::vector< std::vector<std::string> > get_nondefaultnames();
    std::vector< std::vector<std::string> > get_nondefaultwhitespace();
    std::vector< std::vector<std::string> > get_nondefaultvalues();

private:

    // the idea here is that while the user can fill in the types in any order,
    // the output order must always be the same, so the order is actually fixed
    // so this is checking to make sure some programmer kept the order fixed inside the program
    bool check_typePlacement();
    void add_default(std::string type, std::string defaultvalue);
    void add_nondefault(std::string type, std::string name, std::string value);
    void calculateWhiteSpace();
    void calculateBiggestString();

    unsigned int biggestString;
    unsigned int minimumWhiteSpace;
    // disallow changes to the values once values start to be pulled out
    bool finishedAddingValues;
    std::vector<std::string> fvScheme_types;
    std::vector<std::string> fvScheme_defaultwhitespace;
    std::vector<std::string> fvScheme_defaultvalues;
    std::vector< std::vector<std::string> > fvScheme_nondefaultnames;
    std::vector< std::vector<std::string> > fvScheme_nondefaultwhitespace;
    std::vector< std::vector<std::string> > fvScheme_nondefaultvalues;

};

#endif // OPENFOAM_FVSCHEMES_H
