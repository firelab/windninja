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

#include "openFoam_setFields.h"

openFoam_setFields::openFoam_setFields()
{
    finishedAddingValues = false;

    current_fieldName = 0;
    current_fieldRegion = 0;

    infValues = -999;
}

void openFoam_setFields::addField(std::string fieldName, std::string fieldType, std::string defaultValue, std::vector<std::string> regionModelTypes, std::vector<std::vector<std::vector<std::string> > > modelInputs)
{
    if(finishedAddingValues == false)
    {
        if(regionModelTypes.size() != modelInputs.size())
        {
            std::cout << "Error adding field to setFields! number of regions for input regionModelTypes " << regionModelTypes.size() << " does not match number of regions for input modelInputs (" << modelInputs.size() << ")!\n";
        } else
        {
            bool nameExists = false;
            for(size_t j = 0; j < fieldNames.size(); j++)
            {
                if(fieldNames[j] == fieldName)
                {
                    nameExists = true;
                    break;
                }
            }
            if(nameExists == true)
            {
                std::cout << "Error adding field to setFields! field name \"" << fieldName << "\" already exists!\n";
            } else
            {
                fieldNames.push_back(fieldName);
                fieldTypes.push_back(fieldType);
                fieldDefaultValues.push_back(defaultValue);
                // increase the size of this dimension for other vectors
                fieldRegions.push_back(std::vector<std::string>() );
                fieldRegionInfo.push_back(std::vector< std::vector<std::string> >() );
                fieldRegionValues.push_back(std::vector< std::vector<std::string> >() );
                // fill in info for the other vectors
                runModel(regionModelTypes,modelInputs);
                current_fieldName = current_fieldName + 1;
            }
        }
    } else
    {
        std::cout << "Error! Can't add values to setFields after running getValue type functions!\n";
    }
}

void openFoam_setFields::setupDesiredValues(std::string simulationType)
{
    // ugh, not an easy way to initialize all this stuff
    std::vector< std::vector < std::vector<std::string> > > regionInputs;
    std::vector< std::vector<std::string> > varSets;
    if(simulationType == "buoyantBoussinesqPimpleFoam")
    {
        for(size_t j = 0; j < 2; j++)
        {
            std::vector<std::string> modelVars;
            if(j == 0)
            {
                modelVars.push_back("patchName");
                modelVars.push_back(".*minZ");
            } else if(j == 1)
            {
                modelVars.push_back("value");
                modelVars.push_back("310");
            }
            varSets.push_back(modelVars);
        }
        regionInputs.push_back(varSets);
        std::vector<std::string> regions(1,"patchToFace");
        addField("T","volScalarFieldValue","300",regions,regionInputs);
    } else
    {
        // assume it is a myScalarTransportFoam solution. If it is simpleFoam, can just use this one
        for(size_t j = 0; j < 7; j++)
        {
            std::vector<std::string> modelVars;
            if(j == 0)
            {
                modelVars.push_back("xmin");
                modelVars.push_back("726019.742");
            } else if(j == 1)
            {
                modelVars.push_back("ymin");
                modelVars.push_back("5206748.293");
            } else if(j == 2)
            {
                modelVars.push_back("zmin");
                modelVars.push_back("1210");
            } else if(j == 3)
            {
                modelVars.push_back("xmax");
                modelVars.push_back("726283");
            } else if(j == 4)
            {
                modelVars.push_back("ymax");
                modelVars.push_back("5207018.3");
            } else if(j == 5)
            {
                modelVars.push_back("zmax");
                modelVars.push_back("1440");
            } else if(j == 6)
            {
                modelVars.push_back("value");
                modelVars.push_back("75");
            }
            varSets.push_back(modelVars);
        }
        regionInputs.push_back(varSets);
        std::vector<std::string> regions(1,"boxToCell");
        addField("source","volScalarFieldValue","0",regions,regionInputs);
        /*
        for(size_t j = 0; j < 6; j++)
        {
            std::vector<std::string> modelVars;
            if(j == 0)
            {
                modelVars.push_back("xpos");
                modelVars.push_back("726151.371");
            } else if(j == 1)
            {
                modelVars.push_back("ypos");
                modelVars.push_back("5206883.2965");
            } else if(j == 2)
            {
                modelVars.push_back("mass");
                // yeah, this has got to be off in cell size from the actual source we used
                // but it can't be that far off. Can find out exact mass needed by looking at source
                // in paraview, then looking at dimensions, and multiplying the value by the actual given
                // volume for the source. Wait, it's off because of the z dimension!!!
                // okay, the source dimensions I found in our latest Grant Creek Burn case are roughly
                // dx = 306, dy = 306, dz = 266. This gives volume = 24907176 m^3
                // so mass = 75 ug/m^3 * 24907176 m^3 = 1.8680382 kg.
                modelVars.push_back("7676802267.78");   // this is micrograms, so 7.6768 kg. Probably need to adjust this if the algorythm is improved
            } else if(j == 3)
            {
                modelVars.push_back("dx");
                modelVars.push_back("263.258");
            } else if(j == 4)
            {
                modelVars.push_back("dy");
                modelVars.push_back("270.007");
            } else if(j == 5)
            {
                modelVars.push_back("dz");
                modelVars.push_back("1440");
            }
            varSets.push_back(modelVars);
        }
        regionInputs.push_back(varSets);
        std::vector<std::string> regions(1,"plume");
        addField("source","volScalarFieldValue","0",regions,regionInputs);*/
    }
}

void openFoam_setFields::runModel(std::vector<std::string> regionModelTypes, std::vector<std::vector<std::vector<std::string> > > modelInputs)
{
    for(size_t i = 0; i < regionModelTypes.size(); i++)
    {
        if(regionModelTypes[i] == "boxToCell")
        {
            if(checkModel_boxToCell(modelInputs[i]) == false)
            {
                runModel_boxToCell(modelInputs[i]);
            }
        } else if(regionModelTypes[i] == "plume")
        {
            if(checkModel_plume(modelInputs[i]) == false)
            {
                runModel_plume(modelInputs[i]);
            }
        } else if(regionModelTypes[i] == "patchToFace")
        {
            if(checkModel_patchToFace(modelInputs[i]) == false)
            {
                runModel_patchToFace(modelInputs[i]);
            }
        } else
        {
            std::cout << "Error running setFields model! regionModelType[" << i << "] \"" << regionModelTypes[i] << "\" is not a valid type!\n";
        }
    }
}

std::vector<std::string> openFoam_setFields::get_fieldNames()
{
    if(finishedAddingValues == false)
    {
        finishedAddingValues = true;
    }
    return fieldNames;
}

std::vector<std::string> openFoam_setFields::get_fieldTypes()
{
    if(finishedAddingValues == false)
    {
        finishedAddingValues = true;
    }
    return fieldTypes;
}

std::vector<std::string> openFoam_setFields::get_fieldDefaultValues()
{
    if(finishedAddingValues == false)
    {
        finishedAddingValues = true;
    }
    return fieldDefaultValues;
}

std::vector< std::vector<std::string> > openFoam_setFields::get_fieldRegions()
{
    if(finishedAddingValues == false)
    {
        finishedAddingValues = true;
    }
    return fieldRegions;
}

std::vector< std::vector< std::vector<std::string> > > openFoam_setFields::get_fieldRegionInfo()
{
    if(finishedAddingValues == false)
    {
        finishedAddingValues = true;
    }
    return fieldRegionInfo;
}

std::vector< std::vector< std::vector<std::string> > > openFoam_setFields::get_fieldRegionValues()
{
    if(finishedAddingValues == false)
    {
        finishedAddingValues = true;
    }
    return fieldRegionValues;
}

bool openFoam_setFields::checkModelFill(std::vector<std::string> modelInput, std::string variableName, double size, bool shouldBeNumeric, bool canBeNegative, bool canBeZero)
{
    bool fail = false;
    if(modelInput.size() != 0)
    {
        if(modelInput[0] != variableName)
        {
            std::cout << "modelInput[0] = \"" << modelInput[0] << "\" which is not \"" << variableName << "\"!\n";
            fail = true;
        }
    }
    if(size == infValues)
    {
        if(modelInput.size() < 2)
        {
            std::cout << "modelInput size allowed to be infinite, but has no actual variables! Need at least one variable besides the variable name!\n";
            fail = true;
        } else
        {
            if(shouldBeNumeric == true)
            {
                for(size_t k = 1; k < modelInput.size(); k++)
                {
                    if(isNumeric(modelInput[k]) == false)
                    {
                        std::cout << "modelInput[" << k << "] \"" << modelInput[k] << "\" is not numeric but is supposed to be numeric!\n";
                        fail = true;
                    } else
                    {
                        if(canBeNegative == false)
                        {
                            if(isNegative(modelInput[k]) == true)
                            {
                                std::cout << "modelInput[" << k << "] \"" << modelInput[k] << "\" is negative when it isn't supposed to be negative!\n";
                                fail = true;
                            }
                        }
                        if(canBeZero == false)
                        {
                            if(isZero(modelInput[k]) == true)
                            {
                                std::cout << "modelInput[" << k << "] \"" << modelInput[k] << "\" is zero when it isn't supposed to be zero!\n";
                                fail = true;
                            }
                        }
                    }
                }
            }
        }
    } else
    {
        if(modelInput.size() != size)
        {
            std::cout << "modelInput.size() = " << modelInput.size() << ", which is not required size of " << size << "!\n";
            fail = true;
        } else
        {
            if(shouldBeNumeric == true)
            {
                if(isNumeric(modelInput[1]) == false)
                {
                    std::cout << "modelInput[1] \"" << modelInput[1] << "\" is not numeric but is supposed to be numeric!\n";
                    fail = true;
                } else
                {
                    if(canBeNegative == false)
                    {
                        if(isNegative(modelInput[1]) == true)
                        {
                            std::cout << "modelInput[1] \"" << modelInput[1] << "\" is negative when it isn't supposed to be negative!\n";
                            fail = true;
                        }
                    }
                    if(canBeZero == false)
                    {
                        if(isZero(modelInput[1]) == true)
                        {
                            std::cout << "modelInput[1] \"" << modelInput[1] << "\" is zero when it isn't supposed to be zero!\n";
                            fail = true;
                        }
                    }
                }
            }
        }
    }
    return fail;
}

bool openFoam_setFields::checkModel_boxToCell(std::vector<std::vector<std::string> > modelInputs)
{
    std::cout << "checking boxToCell setFields model\n";
    bool fail = false;
    if(modelInputs.size() != 7) // always one more than the array index
    {
        std::cout << "Error running boxToCell model! boxToCell model requires 7 inputs. Found " << modelInputs.size() << " inputs!\n";
        fail = true;
    } else
    {
        std::vector<bool> multiFail(7,false);
        multiFail[0] = checkModelFill(modelInputs[0],"xmin",2,true,true,true);
        multiFail[1] = checkModelFill(modelInputs[1],"ymin",2,true,true,true);
        multiFail[2] = checkModelFill(modelInputs[2],"zmin",2,true,true,true);
        multiFail[3] = checkModelFill(modelInputs[3],"xmax",2,true,true,true);
        multiFail[4] = checkModelFill(modelInputs[4],"ymax",2,true,true,true);
        multiFail[5] = checkModelFill(modelInputs[5],"zmax",2,true,true,true);
        multiFail[6] = checkModelFill(modelInputs[6],"value",2,true,true,true);

        // consolidate fails to a single fail check
        for(size_t j = 0; j < multiFail.size(); j++)
        {
            if(multiFail[j] == true)
            {
                fail = true;
            }
        }
        if(fail == true)
        {
            std::cout << "Error running boxToCell model! modelInputs were not correct!\n";
        }
    }
    return fail;
}

void openFoam_setFields::runModel_boxToCell(std::vector<std::vector<std::string> > modelInputs)
{
    std::cout << "running boxToCell model\n";
    // use the current position values to help with placement in vectors
    fieldRegions[current_fieldName].push_back("boxToCell");
    // increase the size of this dimension for other vectors
    fieldRegionInfo[current_fieldName].push_back(std::vector<std::string>() );
    fieldRegionValues[current_fieldName].push_back(std::vector<std::string>() );

    std::string regionInfo = "box (" + modelInputs[0][1] + " " + modelInputs[1][1] + " " + modelInputs[2][1]
            + ") (" + modelInputs[3][1] + " " + modelInputs[4][1] + " " + modelInputs[5][1] + ");";
    std::cout << "regionInfo = " << regionInfo << "\n";

    fieldRegionInfo[current_fieldName][current_fieldRegion].push_back(regionInfo);
    fieldRegionValues[current_fieldName][current_fieldRegion].push_back(modelInputs[6][1]);
    current_fieldRegion = current_fieldRegion + 1;
}

bool openFoam_setFields::checkModel_plume(std::vector<std::vector<std::string> > modelInputs)
{
    std::cout << "checking plume setFields model\n";
    bool fail = false;
    if(modelInputs.size() != 6) // always one more than the array index
    {
        std::cout << "Error running plume model! plume model requires 6 inputs. Found " << modelInputs.size() << " inputs!\n";
        fail = true;
    } else
    {
        std::vector<bool> multiFail(6,false);
        multiFail[0] = checkModelFill(modelInputs[0],"xpos",2,true,true,true);
        multiFail[1] = checkModelFill(modelInputs[1],"ypos",2,true,true,true);
        multiFail[2] = checkModelFill(modelInputs[2],"mass",2,true,false,true);
        multiFail[3] = checkModelFill(modelInputs[3],"dx",2,true,false,false);
        multiFail[4] = checkModelFill(modelInputs[4],"dy",2,true,false,false);
        multiFail[5] = checkModelFill(modelInputs[5],"dz",2,true,false,false);

        // consolidate fails to a single fail check
        for(size_t j = 0; j < multiFail.size(); j++)
        {
            if(multiFail[j] == true)
            {
                fail = true;
            }
        }
        if(fail == true)
        {
            std::cout << "Error running plume model! modelInputs were not correct!\n";
        }
    }
    return fail;
}

void openFoam_setFields::runModel_plume(std::vector<std::vector<std::string> > modelInputs)
{
    // still need to adjust this so that it actually grabs the right cells, because it will be off at differing resolutions
    // basically it will grab whatever cells have their center close enough, well we probably want something more exact than that
    // The other option would be to write an actual utility that uses openfoam stuff separate from all this, and set the source
    // using openfoam information directly.
    std::cout << "running plume model\n";
    // use the current position values to help with placement in vectors
    fieldRegions[current_fieldName].push_back("boxToCell");
    // increase the size of this dimension for other vectors
    fieldRegionInfo[current_fieldName].push_back(std::vector<std::string>() );
    fieldRegionValues[current_fieldName].push_back(std::vector<std::string>() );

    double xmin = stringToDbl(modelInputs[0][1]) - stringToDbl(modelInputs[3][1])/2;
    double ymin = stringToDbl(modelInputs[1][1]) - stringToDbl(modelInputs[4][1])/2;
    double zmin = 0;
    double xmax = stringToDbl(modelInputs[0][1]) + stringToDbl(modelInputs[3][1])/2;
    double ymax = stringToDbl(modelInputs[1][1]) + stringToDbl(modelInputs[4][1])/2;
    double zmax = zmin + stringToDbl(modelInputs[5][1]);
    double concentration = stringToDbl(modelInputs[2][1])/(stringToDbl(modelInputs[3][1])*stringToDbl(modelInputs[4][1])*stringToDbl(modelInputs[5][1]));

    std::string regionInfo = "box (" + dblToString(xmin) + " " + dblToString(ymin) + " " + dblToString(zmin)
            + ") (" + dblToString(xmax) + " " + dblToString(ymax) + " " + dblToString(zmax) + ");";
    std::cout << "regionInfo = " << regionInfo << "\n";

    fieldRegionInfo[current_fieldName][current_fieldRegion].push_back(regionInfo);
    fieldRegionValues[current_fieldName][current_fieldRegion].push_back(dblToString(concentration));
    current_fieldRegion = current_fieldRegion + 1;
}

bool openFoam_setFields::checkModel_patchToFace(std::vector<std::vector<std::string> > modelInputs)
{
    std::cout << "checking patchToFace setFields model\n";
    bool fail = false;
    if(modelInputs.size() != 2) // always one more than the array index
    {
        std::cout << "Error running plume model! plume model requires 2 inputs. Found " << modelInputs.size() << " inputs!\n";
        fail = true;
    } else
    {
        std::vector<bool> multiFail(2,false);
        multiFail[0] = checkModelFill(modelInputs[0],"patchName",2,false,false,false);
        multiFail[1] = checkModelFill(modelInputs[1],"value",2,true,true,true);

        // consolidate fails to a single fail check
        for(size_t j = 0; j < multiFail.size(); j++)
        {
            if(multiFail[j] == true)
            {
                fail = true;
            }
        }
        if(fail == true)
        {
            std::cout << "Error running plume model! modelInputs were not correct!\n";
        }
    }
    return fail;
}

void openFoam_setFields::runModel_patchToFace(std::vector<std::vector<std::string> > modelInputs)
{
    std::cout << "running patchToFace model\n";
    // use the current position values to help with placement in vectors
    fieldRegions[current_fieldName].push_back("patchToFace");
    // increase the size of this dimension for other vectors
    fieldRegionInfo[current_fieldName].push_back(std::vector<std::string>() );
    fieldRegionValues[current_fieldName].push_back(std::vector<std::string>() );

    std::string regionInfo = "name \"" + modelInputs[0][1] + "\";";
    std::cout << "regionInfo = " << regionInfo << "\n";

    fieldRegionInfo[current_fieldName][current_fieldRegion].push_back(regionInfo);
    fieldRegionValues[current_fieldName][current_fieldRegion].push_back(modelInputs[1][1]);
    current_fieldRegion = current_fieldRegion + 1;
}

bool openFoam_setFields::isNumeric(std::string s)
{
    bool isType = true;
    std::istringstream strm;
    strm.str(s);
    double n = 0;
    if((strm >> n).fail())
    {
        strm.clear();
        isType = false;
    }
    return isType;
}

bool openFoam_setFields::isNegative(std::string s)
{
    bool isNeg = false;
    std::istringstream strm;
    strm.str(s);
    double n = 0;
    if((strm >> n).fail())
    {
        std::cout << "Error converting string " << s << " to number, checking to see if it is negative!\n";
        isNeg = true;
        strm.clear();
    } else
    {
        if(n < 0)
        {
            isNeg = true;
        }
    }
    return isNeg;
}

bool openFoam_setFields::isZero(std::string s)
{
    bool isZiltch = false;
    std::istringstream strm;
    strm.str(s);
    double n = 0;
    if((strm >> n).fail())
    {
        std::cout << "Error converting string " << s << " to number, checking to see if it is zero!\n";
        isZiltch = true;
        strm.clear();
    } else
    {
        if(n == 0)
        {
            isZiltch = true;
        }
    }
    return isZiltch;
}

double openFoam_setFields::stringToDbl(std::string s)
{
    std::istringstream strm;
    strm.str(s);
    double n = 0;
    if((strm >> n).fail())
    {
        std::cout << "Error converting string " << s << " to double!\n";
        strm.clear();
    }
    return n;
}

std::string openFoam_setFields::dblToString(double n)
{
    std::ostringstream strm; // this is supposed to use o instead of i stringstream since creating a string
    if((strm << std::fixed << n).fail())
    {
        strm.clear();
        std::cout << "Error converting double to string!\n";
    }
    std::string strrr = strm.str();
    size_t precisionspot = 0;
    bool foundPrecisionSpot = false;
    size_t precision = 6;
    for(size_t i = 1; i < strrr.size(); i++)
    {
        //std::cout << "strrr.substr(strrr.size()-" << i << ",1) = \"" << strrr.substr(strrr.size()-i,1) << "\"\n";
        if(strrr.substr(strrr.size()-i,1) != "0")
        {
            if(foundPrecisionSpot == false)
            {
                // notice this catches it if it is zeroes up to the "."
                precisionspot = i;
                foundPrecisionSpot = true;
            }
        }
        if(strrr.substr(strrr.size()-i,1) == ".")
        {
            precision = i - precisionspot;
            break;
        }
    }
    strm.clear();
    strm.str("");
    if((strm << std::fixed << std::setprecision(precision) << n).fail())
    {
        strm.clear();
        std::cout << "Error converting double to string!\n";
    }
    return strm.str();
}
