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

#ifndef OPENFOAM_FVSOLUTIONS_H
#define OPENFOAM_FVSOLUTIONS_H

#include <vector>
#include <iostream>

class openFoam_fvSolutions
{
public:

    /*
     * This is going to be similar to fvSchemes, in that it will expect to group different parts together
     * under a similar heading. For example, the solvers section has as many sections as there are headings
     * each heading with a similar structure no matter the heading. It is similar for the potentialFlow,
     * PIMPLE, SIMPLE, and relaxationFactors sections. The difference is that while the different headings for
     * a given section are similar for that heading, they are not similar between sections.
     * So technically this could be broken up into more classes as well, or separate headers for each section,
     * but kept separate instead of being grouped into a single vector, but I feel like it would be better
     * to just group them all here.
     *
     * Turns out that while it is possible to group them all into a single section vector, they have just
     * enough differences between sections/headers that the complexity goes up by quite a bit. To add to the
     * confusion, just increasing the number of vector layers by one from fvSchemes makes it a heck of a ton
     * more difficult to work with the vectors. You have to be super careful about not accessing values before
     * a given layer in the vectors have been formed, which involves a ton of what seems like excess resizing.
     * It also doesn't help that not all the sections will be used, adding in more resizing complexity.
     */

    openFoam_fvSolutions();

    void setupDesiredValues(std::string simulationType);

    std::vector<std::string> get_sections();
    std::vector< std::vector<std::string> > get_headers();
    std::vector< std::vector< std::vector<std::string> > > get_keyWords();
    std::vector< std::vector< std::vector<std::string> > > get_whitespace();
    std::vector< std::vector< std::vector<std::string> > > get_keyWordValues();

private:

    // doesn't matter if the added sections aren't filled, it matters that they start in the right order
    bool check_sectionPlacement();
    // information that is outside of a header as keyWord and keyWordValue will be grouped in a header called "noheader"
    void addToSection(std::string section, std::string header, std::string keyWord, std::string keyWordValue);
    // this section could possibly use more debugging
    bool validateHeaders();
    bool validateKeyWords();
    void resizeForOutput();
    void calculateWhiteSpace();
    void calculateBiggestString();

    unsigned int biggestString;
    unsigned int minimumWhiteSpace;
    bool finishedAddingValues;
    std::vector<std::string> fvSolution_sections;
    std::vector< std::vector<std::string> > fvSolution_headers;
    std::vector< std::vector< std::vector<std::string> > > fvSolution_keyWords;
    std::vector< std::vector< std::vector<std::string> > > fvSolution_whitespace;
    std::vector< std::vector< std::vector<std::string> > > fvSolution_keyWordValues;

};

#endif // OPENFOAM_FVSOLUTIONS_H
