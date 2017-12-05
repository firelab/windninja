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

#ifndef OPENFOAM_SETFIELDS_H
#define OPENFOAM_SETFIELDS_H

#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>

class openFoam_setFields
{

public:

    /*
     * This is going to be similar to fvSchemes, in how it takes in data and outputs it,
     * but there will also be a separate class for generating a smoke source
     * Or could put that class stuff in here. We shall see.
     *
     * Okay, in the end, this all got put into this single class. Just follow the format for setupDesiredValues,
     * checkModel_modelName, and runModel_modelName, adding in the model to runModel. This allows you to add
     * whatever model you want. Might need to do some adjustment to the organization depending on how complex
     * the models become, but it looks like it works plenty fine for now :)
     */

    openFoam_setFields();

    void setupDesiredValues(std::string simulationType);

    std::vector<std::string> get_fieldNames();
    std::vector<std::string> get_fieldTypes();
    std::vector<std::string> get_fieldDefaultValues();
    std::vector< std::vector<std::string> > get_fieldRegions();
    std::vector< std::vector< std::vector<std::string> > > get_fieldRegionInfo();
    std::vector< std::vector< std::vector<std::string> > > get_fieldRegionValues();

private:

    void addField(std::string fieldName, std::string fieldType, std::string defaultValue, std::vector<std::string> regionModelTypes, std::vector< std::vector< std::vector<std::string> > > modelInputs);

    // right now, this just puts them in separate region stuff, but it would be nice to group stuff that is similar
    // so that it outputs in the same region if it has the same locations/values, whatever is needed for grouping
    void runModel(std::vector<std::string> regionModelTypes, std::vector<std::vector<std::vector<std::string> > > modelInputs);

    // disallow changes to the values once values start to be pulled out
    bool finishedAddingValues;
    // first vector layer is fieldNames
    // second vector layer is fieldRegions
    // each region has differing numbers of info and values
    std::vector<std::string> fieldNames;
    std::vector<std::string> fieldTypes;
    std::vector<std::string> fieldDefaultValues;
    std::vector< std::vector<std::string> > fieldRegions;
    std::vector< std::vector< std::vector<std::string> > > fieldRegionInfo; // this one will be freakin tricky
    std::vector< std::vector< std::vector<std::string> > > fieldRegionValues;

    size_t current_fieldName;
    size_t current_fieldRegion;

    // the list of models and their input checkers
    bool checkModelFill(std::vector<std::string> modelInput, std::string variableName, double size, bool shouldBeNumeric, bool canBeNegative, bool canBeZero);
    bool checkModel_boxToCell(std::vector<std::vector<std::string> > modelInputs);
    void runModel_boxToCell(std::vector<std::vector<std::string> > modelInputs);
    bool checkModel_plume(std::vector<std::vector<std::string> > modelInputs);
    void runModel_plume(std::vector<std::vector<std::string> > modelInputs);
    bool checkModel_patchToFace(std::vector<std::vector<std::string> > modelInputs);
    void runModel_patchToFace(std::vector<std::vector<std::string> > modelInputs);

    double infValues;   // value to know whether to allow any number of size, but zero, for the number of variables in a model

    // utility functions
    bool isNumeric(std::string s);
    bool isNegative(std::string s);
    bool isZero(std::string s);
    double stringToDbl(std::string s);
    std::string dblToString(double n);

};

#endif // OPENFOAM_SETFIELDS_H
