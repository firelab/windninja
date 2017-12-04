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

#include "openFoam_fvSolutions.h"

openFoam_fvSolutions::openFoam_fvSolutions()
{
    finishedAddingValues = false;
    biggestString = 0;
    minimumWhiteSpace = 4;  // the whitespace given to what is found to have the biggestString

    std::cout << "setting up fvSolution sections\n";
    // setup the types of fvSchemes in the correct order used in OpenFOAM
    fvSolution_sections.push_back("solvers");
    fvSolution_sections.push_back("potentialFlow");
    fvSolution_sections.push_back("SIMPLE");
    fvSolution_sections.push_back("PIMPLE");
    fvSolution_sections.push_back("relaxationFactors");

    // check to make sure it was setup correctly in the right order
    if(check_sectionPlacement() == true)
    {
        std::cout << "Error setting up openFoam_fvSolutions! the solution order isn't correct!\n";
    }

    // get the vector sizes right for the outer vector of the vectors of vectors of vectors stuff
    fvSolution_headers.resize(fvSolution_sections.size());
    fvSolution_keyWords.resize(fvSolution_sections.size());
    fvSolution_keyWordValues.resize(fvSolution_sections.size());
    // probably vector[j].resize(size) is the way to resize the insides and needs done later
    // looks like pushing back an empty vector might be cleaner, depending on the situation
}

bool openFoam_fvSolutions::check_sectionPlacement()
{
    std::cout << "checking section placement\n";
    bool fail = false;
    // if the style of setup changes, say new version of openfoam, need to change this to match the new format
    if(fvSolution_sections.size() != 5)
    {
        std::cout << "Error! fvSolution_sections setup with size " << fvSolution_sections.size() << " not size 5!"
                  << " Check openFoam_fvSolutions constructor!\n";
        fail = true;
    } else
    {
        if(fvSolution_sections[0] != "solvers")
        {
            std::cout << "fvSolution_sections[0] != solvers!\n";
            fail = true;
        }
        if(fvSolution_sections[1] != "potentialFlow")
        {
            std::cout << "fvSolution_sections[1] != potentialFlow!\n";
            fail = true;
        }
        if(fvSolution_sections[2] != "SIMPLE")
        {
            std::cout << "fvSolution_sections[2] != SIMPLE!\n";
            fail = true;
        }
        if(fvSolution_sections[3] != "PIMPLE")
        {
            std::cout << "fvSolution_sections[3] != PIMPLE!\n";
            fail = true;
        }
        if(fvSolution_sections[4] != "relaxationFactors")
        {
            std::cout << "fvSolution_sections[4] != relaxationFactors!\n";
            fail = true;
        }
    }
    return fail;
}

void openFoam_fvSolutions::addToSection(std::string section, std::string header, std::string keyWord, std::string keyWordValue)
{
    //std::cout << "adding section \"" << section << "\", header \"" << header << "\", keyWord \"" << keyWord << "\", and keyWordValue \"" << keyWordValue << "\" to fvSolutions\n";
    if(finishedAddingValues == false)
    {
        bool foundSection = false;
        for(size_t j = 0; j < fvSolution_sections.size(); j++)
        {
            if(fvSolution_sections[j] == section)
            {
                bool foundHeader = false;
                // can't really do much error checking to see if it is a valid header, keyWord, or keyWordValue without adding all kinds of verification that is almost as big as probably building the header files in openfoam for this stuff
                // okay, the idea here is that if size = 0, going to pushback a header so I'm not accessing something nonexistent
                for(size_t i = 0; i < fvSolution_headers[j].size(); i++)
                {
                    if(fvSolution_headers[j][i] == header)
                    {
                        //std::cout << "header \"" << header << "\" exists, adding keyWord and keyWordvalue to header\n";
                        fvSolution_keyWords[j][i].push_back(keyWord);
                        fvSolution_keyWordValues[j][i].push_back(keyWordValue);
                        //std::cout << "fvSolution_headers[" << j << "].size() = " << fvSolution_headers[j].size() << "\n";
                        //std::cout << "fvSolution_keyWords[" << j << "][" << i << "].size() = " << fvSolution_keyWords[j][i].size() << "\n";
                        foundHeader = true;
                        break;
                    }
                }
                if(foundHeader == false)
                {
                    //std::cout << "header \"" << header << "\" not found, adding new header\n";
                    fvSolution_headers[j].push_back(header);
                    //fvSolution_keyWords[j].resize(fvSolution_headers[j].size());
                    //fvSolution_keyWordValues[j].resize(fvSolution_headers[j].size());
                    fvSolution_keyWords[j].push_back(std::vector<std::string>() );
                    fvSolution_keyWordValues[j].push_back(std::vector<std::string>() );
                    fvSolution_keyWords[j][fvSolution_headers[j].size()-1].push_back(keyWord);
                    fvSolution_keyWordValues[j][fvSolution_headers[j].size()-1].push_back(keyWordValue);
                    //std::cout << "fvSolution_headers[" << j << "].size() = " << fvSolution_headers[j].size() << "\n";
                    //std::cout << "fvSolution_keyWords[" << j << "][" << fvSolution_headers[j].size()-1 << "].size() = " << fvSolution_keyWords[j][fvSolution_headers[j].size()-1].size() << "\n";
                }
                foundSection = true;
                break;
            }
        }
        if(foundSection == false)
        {
            std::cout << "Error adding to fvSolutions section! section " << section << " is not a valid section!\n";
        }
    } else
    {
        std::cout << "Error! Can't add values to fvSolutions after running getValue type functions!\n";
    }
}

void openFoam_fvSolutions::setupDesiredValues(std::string simulationType)
{
    std::cout << "setting up fvSolution desired values\n";
    // just look at how many lines this is, and how tedious it is. Template files for each simulation type would be better
    if(simulationType == "simpleFoam")
    {
        addToSection("solvers","T","solver","PBiCG");
        addToSection("solvers","T","preconditioner","DILU");
        addToSection("solvers","T","tolerance","1e-06");
        addToSection("solvers","T","relTol","0");
        addToSection("solvers","p","solver","GAMG");
        addToSection("solvers","p","tolerance","1e-06");
        addToSection("solvers","p","relTol","0.001");
        addToSection("solvers","p","smoother","GaussSeidel");
        addToSection("solvers","p","nPreSweeps","0");
        addToSection("solvers","p","nPostSweeps","2");
        addToSection("solvers","p","cacheAgglomeration","false");
        addToSection("solvers","p","nCellsInCoarsestLevel","20");
        addToSection("solvers","p","agglomerator","faceAreaPair");
        addToSection("solvers","p","mergeLevels","1");
        addToSection("solvers","U","solver","smoothSolver");
        addToSection("solvers","U","smoother","GaussSeidel");
        addToSection("solvers","U","nSweeps","2");
        addToSection("solvers","U","tolerance","1e-06");
        addToSection("solvers","U","relTol","0.0");
        addToSection("solvers","k","solver","smoothSolver");
        addToSection("solvers","k","smoother","GaussSeidel");
        addToSection("solvers","k","nSweeps","2");
        addToSection("solvers","k","tolerance","1e-09");
        addToSection("solvers","k","relTol","0.001");
        addToSection("solvers","epsilon","solver","smoothSolver");
        addToSection("solvers","epsilon","smoother","GaussSeidel");
        addToSection("solvers","epsilon","nSweeps","2");
        addToSection("solvers","epsilon","tolerance","1e-09");
        addToSection("solvers","epsilon","relTol","0.001");
        addToSection("solvers","omega","solver","smoothSolver");
        addToSection("solvers","omega","smoother","GaussSeidel");
        addToSection("solvers","omega","nSweeps","2");
        addToSection("solvers","omega","tolerance","1.0");
        addToSection("solvers","omega","relTol","1.0");
        addToSection("solvers","nuTilda","solver","smoothSolver");
        addToSection("solvers","nuTilda","smoother","GaussSeidel");
        addToSection("solvers","nuTilda","nSweeps","2");
        addToSection("solvers","nuTilda","tolerance","1.0");
        addToSection("solvers","nuTilda","relTol","1.0");
        addToSection("solvers","cellDisplacement","solver","GAMG");
        addToSection("solvers","cellDisplacement","tolerance","1e-08");
        addToSection("solvers","cellDisplacement","relTol","0");
        addToSection("solvers","cellDisplacement","smoother","GaussSeidel");
        addToSection("solvers","cellDisplacement","cacheAgglomeration","true");
        addToSection("solvers","cellDisplacement","nCellsInCoarsestLevel","10");
        addToSection("solvers","cellDisplacement","agglomerator","faceAreaPair");
        addToSection("solvers","cellDisplacement","mergeLevels","1");
        addToSection("solvers","\"(cellMotionU|cellMotionUz)\"","$p","");
        addToSection("solvers","\"(cellMotionU|cellMotionUz)\"","tolerance","1e-08");
        addToSection("solvers","\"(cellMotionU|cellMotionUz)\"","relTol","0");
        addToSection("potentialFlow","noheader","nNonOrthogonalCorrectors","6");
        addToSection("potentialFlow","noheader","pRefCell","0");
        addToSection("potentialFlow","noheader","pRefValue","0");
        addToSection("SIMPLE","noheader","nNonOrthogonalCorrectors","0");
        addToSection("SIMPLE","noheader","pRefCell","0");
        addToSection("SIMPLE","noheader","pRefValue","0");
        addToSection("SIMPLE","residualControl","p","0.00001");
        addToSection("SIMPLE","residualControl","U","0.00001");
        addToSection("SIMPLE","residualControl","k","0.00001");
        addToSection("SIMPLE","residualControl","epsilon","0.00001");
        addToSection("SIMPLE","residualControl","omega","1.0");
        addToSection("SIMPLE","residualControl","nuTilda","1.0");
        addToSection("relaxationFactors","fields","p","0.5");
        addToSection("relaxationFactors","equations","U","0.5");
        addToSection("relaxationFactors","equations","k","0.6");
        addToSection("relaxationFactors","equations","epsilon","0.6");
        addToSection("relaxationFactors","equations","omega","1.0");
        addToSection("relaxationFactors","equations","nuTilda","1.0");
    } else if(simulationType == "buoyantBoussinesqPimpleFoam")
    {
        addToSection("solvers","T","solver","smoothSolver");
        addToSection("solvers","T","smoother","GaussSeidel");
        addToSection("solvers","T","nSweeps","2");
        addToSection("solvers","T","tolerance","1e-09");
        addToSection("solvers","T","relTol","0.001");
        addToSection("solvers","TFinal","$T","");
        addToSection("solvers","TFinal","relTol","0");
        addToSection("solvers","p","solver","GAMG");
        addToSection("solvers","p","tolerance","1e-06");
        addToSection("solvers","p","relTol","0.001");
        addToSection("solvers","p","smoother","GaussSeidel");
        addToSection("solvers","p","nPreSweeps","0");
        addToSection("solvers","p","nPostSweeps","2");
        addToSection("solvers","p","cacheAgglomeration","false");
        addToSection("solvers","p","nCellsInCoarsestLevel","20");
        addToSection("solvers","p","agglomerator","faceAreaPair");
        addToSection("solvers","p","mergeLevels","1");
        addToSection("solvers","pFinal","$p","");
        addToSection("solvers","pFinal","relTol","0");
        addToSection("solvers","p_rgh","solver","GAMG");
        addToSection("solvers","p_rgh","tolerance","1e-06");
        addToSection("solvers","p_rgh","relTol","0.001");
        addToSection("solvers","p_rgh","smoother","GaussSeidel");
        addToSection("solvers","p_rgh","nPreSweeps","0");
        addToSection("solvers","p_rgh","nPostSweeps","2");
        addToSection("solvers","p_rgh","cacheAgglomeration","false");
        addToSection("solvers","p_rgh","nCellsInCoarsestLevel","20");
        addToSection("solvers","p_rgh","agglomerator","faceAreaPair");
        addToSection("solvers","p_rgh","mergeLevels","1");
        addToSection("solvers","p_rghFinal","$p_rgh","");
        addToSection("solvers","p_rghFinal","relTol","0");
        addToSection("solvers","U","solver","smoothSolver");
        addToSection("solvers","U","smoother","GaussSeidel");
        addToSection("solvers","U","nSweeps","2");
        addToSection("solvers","U","tolerance","1e-06");
        addToSection("solvers","UFinal","$U","");
        addToSection("solvers","UFinal","relTol","0");
        addToSection("solvers","k","solver","smoothSolver");
        addToSection("solvers","k","smoother","GaussSeidel");
        addToSection("solvers","k","nSweeps","2");
        addToSection("solvers","k","tolerance","1e-09");
        addToSection("solvers","k","relTol","0.001");
        addToSection("solvers","kFinal","$k","");
        addToSection("solvers","kFinal","relTol","0");
        addToSection("solvers","epsilon","solver","smoothSolver");
        addToSection("solvers","epsilon","smoother","GaussSeidel");
        addToSection("solvers","epsilon","nSweeps","2");
        addToSection("solvers","epsilon","tolerance","1e-09");
        addToSection("solvers","epsilon","relTol","0.001");
        addToSection("solvers","epsilonFinal","$epsilon","");
        addToSection("solvers","epsilonFinal","relTol","0");
        addToSection("solvers","omega","solver","smoothSolver");
        addToSection("solvers","omega","smoother","GaussSeidel");
        addToSection("solvers","omega","nSweeps","2");
        addToSection("solvers","omega","tolerance","1.0");
        addToSection("solvers","omega","relTol","1.0");
        addToSection("solvers","omegaFinal","$omega","");
        addToSection("solvers","omegaFinal","relTol","0");
        addToSection("solvers","nuTilda","solver","smoothSolver");
        addToSection("solvers","nuTilda","smoother","GaussSeidel");
        addToSection("solvers","nuTilda","nSweeps","2");
        addToSection("solvers","nuTilda","tolerance","1.0");
        addToSection("solvers","nuTilda","relTol","1.0");
        addToSection("solvers","nuTildaFinal","$nuTilda","");
        addToSection("solvers","nuTildaFinal","relTol","0");
        addToSection("solvers","cellDisplacement","solver","GAMG");
        addToSection("solvers","cellDisplacement","tolerance","1e-08");
        addToSection("solvers","cellDisplacement","relTol","0");
        addToSection("solvers","cellDisplacement","smoother","GaussSeidel");
        addToSection("solvers","cellDisplacement","cacheAgglomeration","true");
        addToSection("solvers","cellDisplacement","nCellsInCoarsestLevel","10");
        addToSection("solvers","cellDisplacement","agglomerator","faceAreaPair");
        addToSection("solvers","cellDisplacement","mergeLevels","1");
        addToSection("solvers","\"(cellMotionU|cellMotionUz)\"","$p","");
        addToSection("solvers","\"(cellMotionU|cellMotionUz)\"","tolerance","1e-08");
        addToSection("solvers","\"(cellMotionU|cellMotionUz)\"","relTol","0");
        addToSection("potentialFlow","noheader","nNonOrthogonalCorrectors","6");
        addToSection("potentialFlow","noheader","pRefCell","0");
        addToSection("potentialFlow","noheader","pRefValue","0");
        addToSection("PIMPLE","noheader","momentumPredictor","no");
        addToSection("PIMPLE","noheader","nOuterCorrectors","1");
        addToSection("PIMPLE","noheader","nCorrectors","2");
        addToSection("PIMPLE","noheader","nNonOrthogonalCorrectors","0");
        addToSection("PIMPLE","noheader","pRefCell","0");
        addToSection("PIMPLE","noheader","pRefValue","0");
        // hmm, need to restructure it or do special stuff because this section goes one brace too many
        // just use PIMPLE to know it is wierd and assume it is residualControl
        // honestly, at first this seemed easier than replaceKeys, but look at how many lines of code are needed here! Easier to edit in a temp file
        addToSection("PIMPLE","p","tolerance","0.00001");
        addToSection("PIMPLE","p","relTol","0");
        addToSection("PIMPLE","p_rgh","tolerance","0.00001");
        addToSection("PIMPLE","p_rgh","relTol","0");
        addToSection("PIMPLE","T","tolerance","0.00001");
        addToSection("PIMPLE","T","relTol","0");
        addToSection("PIMPLE","U","tolerance","0.00001");
        addToSection("PIMPLE","U","relTol","0");
        addToSection("PIMPLE","k","tolerance","0.00001");
        addToSection("PIMPLE","k","relTol","0");
        addToSection("PIMPLE","epsilon","tolerance","0.00001");
        addToSection("PIMPLE","epsilon","relTol","0");
        addToSection("PIMPLE","omega","tolerance","1.0");
        addToSection("PIMPLE","omega","relTol","0");
        addToSection("PIMPLE","nuTilda","tolerance","1.0");
        addToSection("PIMPLE","nuTilda","relTol","0");
        addToSection("relaxationFactors","fields","p","0.5");
        addToSection("relaxationFactors","fields","p_rgh","0.5");
        addToSection("relaxationFactors","equations","T","0.5");
        addToSection("relaxationFactors","equations","U","0.5");
        addToSection("relaxationFactors","equations","k","0.6");
        addToSection("relaxationFactors","equations","epsilon","0.6");
        addToSection("relaxationFactors","equations","omega","1.0");
        addToSection("relaxationFactors","equations","nuTilda","1.0");
    } else
    {
        // assume it is a myScalarTransportFoam solution
        addToSection("solvers","T","solver","PBiCG");
        addToSection("solvers","T","preconditioner","DILU");
        addToSection("solvers","T","tolerance","1e-06");
        addToSection("solvers","T","relTol","0.2");
        addToSection("SIMPLE","noheader","nNonOrthogonalCorrectors","0");
    }
}

bool openFoam_fvSolutions::validateHeaders()
{
    std::cout << "validating headers\n";
    bool fail = false;
    for(size_t j = 0; j < fvSolution_sections.size(); j++)
    {
        //std::cout << "fvSolution_sections[" << j  << "] = \"" << fvSolution_sections[j] << "\"\n";
        for(size_t i = 0; i < fvSolution_headers[j].size(); i++)
        {
            //std::cout << "fvSolution_headers[" << j << "][" << i << "] = \"" << fvSolution_headers[j][i] << "\"\n";
            if(fvSolution_headers[j][i] == "")
            {
                std::cout << "Error in fvSolutions in section \"" << fvSolution_sections[j] << "\"! Found specified blank header! Headers cannot be specified as empty strings!\n";
                fail = true;
            }
            if(fvSolution_sections[j] == "PIMPLE")
            {
                if(fvSolution_headers[j].size() != 0 && fvSolution_headers[j][0] != "noheader")
                {
                    std::cout << "Error in fvSolutions! Using section \"PIMPLE\" without specifying first header type as \"noheader\"!\n";
                    fail = true;
                }
            }
            if(i != 0 && fvSolution_headers[j][i] == "noheader")
            {
                std::cout << "Error in fvSolutions in section \"" << fvSolution_sections[j] << "\"! Header type \"noheader\" found after first header!\n";
                fail = true;
            }
            if(fvSolution_headers[j][i].size() > 5)
            {
                //std::cout << "calculating substring for fvSolution_headers[" << j << "][" << i << "] \"" << fvSolution_headers[j][i] << "\"\n";
                size_t substrSpot = fvSolution_headers[j][i].size()-5+1;    // add one because looking at first char of "Final"
                if(fvSolution_headers[j][i].substr(substrSpot,5) == "Final")
                {
                    std::string pastHeader = fvSolution_headers[j][i].substr(0,substrSpot);
                    //std::cout << "pastHeader = " << pastHeader << "\n";
                    bool foundPastHeader = false;
                    for(size_t ii = 0; ii < i; ii++)
                    {
                        if(fvSolution_headers[j][ii] == pastHeader)
                        {
                            foundPastHeader = true;
                        }
                    }
                    if(foundPastHeader == false)
                    {
                        std::cout << "Error in fvSolutions in section \"" << fvSolution_sections[j] << "\"! \"" << fvSolution_headers[j][i] << "\" header found before \"" << pastHeader << "\"!\n";
                        fail = true;
                    }
                }
            }
        }
    }
    return fail;
}

bool openFoam_fvSolutions::validateKeyWords()
{
    std::cout << "validating keywords\n";
    bool fail = false;
    for(size_t j = 0; j < fvSolution_sections.size(); j++)
    {
        //std::cout << "fvSolution_sections[" << j << "] = \"" << fvSolution_sections[j] << "\"\n";
        for(size_t i = 0; i < fvSolution_headers[j].size(); i++)
        {
            //std::cout << "fvSolution_headers[" << j << "][" << i << "] = \"" << fvSolution_headers[j][i] << "\"\n";
            for(size_t k = 0; k < fvSolution_keyWords[j][i].size(); k++)
            {
                //std::cout << "fvSolution_keyWords[" << j << "][" << i << "][" << k << "] = \"" << fvSolution_keyWords[j][i][k] << "\", fvSolution_keyWordValues[j][i][k] = " << fvSolution_keyWordValues[j][i][k] << "\"\n";
                if(fvSolution_keyWords[j][i][k] == "")
                {
                    std::cout << "Error in fvSolutions in section \"" << fvSolution_sections[j] << "\" in header \"" << fvSolution_headers[j][i] << "\"! Keywords cannot be specified as empty strings!\n";
                    fail = true;
                }
                if(k > 0 && fvSolution_keyWords[j][i][k].substr(0,1) == "$")
                {
                    std::cout << "Error in fvSolutions in section \"" << fvSolution_sections[j] << "\" in header \"" << fvSolution_headers[j][i] << "\"! Found keyword with first char $ after first keyword!\n";
                    fail = true;
                }
                //std::cout << "calculating substring to see if keyword has $ in it\n";
                if(k == 0 && fvSolution_keyWords[j][i][k].substr(0,1) == "$")
                {
                    if(fvSolution_keyWords[j][i][k].size() == 1)
                    {
                        std::cout << "Error in fvSolutions in section \"" << fvSolution_sections[j] << "\" in header \"" << fvSolution_headers[j][i] << "\"! Found keyword char \"$\" without the rest of the keyword!\n";
                        fail = true;
                    } else
                    {
                        std::string pastHeader = fvSolution_keyWords[j][i][k].substr(1,fvSolution_keyWords[j][i][k].size()-1);
                        bool foundPastHeader = false;
                        for(size_t ii = 0; ii < i; ii++)
                        {
                            if(fvSolution_headers[j][ii] == pastHeader)
                            {
                                foundPastHeader = true;
                            }
                        }
                        if(foundPastHeader == false)
                        {
                            std::cout << "Error in fvSolutions in section \"" << fvSolution_sections[j] << "\" in header \"" << fvSolution_headers[j][i] << "\"! keyword \"" << fvSolution_keyWords[j][i][k] << "\" specified before \"" << pastHeader << "\" header!\n";
                            fail = true;
                        }
                    }
                }
                if(fvSolution_keyWordValues[j][i][k] == "")
                {
                    if(fvSolution_keyWords[j][i][k].substr(0,1) != "$")
                    {
                        std::cout << "Error in fvSolutions in section \"" << fvSolution_sections[j] << "\" in header \"" << fvSolution_headers[j][i] << "\" in keyword \"" << fvSolution_keyWords[j][i][k] << "\"! Keywordvalues cannot be specified as empty strings unless keyword first char is the $ char!\n";
                        fail = true;
                    }
                }
            }
        }
    }
    return fail;
}

void openFoam_fvSolutions::resizeForOutput()
{
    std::cout << "resizing fvSolutionsInfo for output\n";
    bool fail1 = validateHeaders();
    bool fail2 = validateKeyWords();
    if(fail1 == true || fail2 == true)
    {
        // I would almost prefer to end the program now, or if I find empty strings before calling substrings
        // but I don't want to sacrifice the regular windninja output just because of an error in my stuff
        std::cout << "Found errors in headers and/or keywords! Resizing vectors for output may produce errors!\n";
    }

    std::cout << "filling replacement vectors\n";
    std::vector<std::string> replacement_fvSolution_sections;
    std::vector< std::vector<std::string> > replacement_fvSolution_headers;
    std::vector< std::vector< std::vector<std::string> > > replacement_fvSolution_keyWords;
    std::vector< std::vector< std::vector<std::string> > > replacement_fvSolution_keyWordValues;
    // fill replacement vectors in the same order as old stuff, but dropping all empty/unused information
    size_t replaceSectionCounter = 0;
    size_t replaceHeaderCounter = 0;
    for(size_t j = 0; j < fvSolution_sections.size(); j++)
    {
        //std::cout << "fvSolution_sections[" << j << "] = " << fvSolution_sections[j] << "\n";
        //std::cout << "fvSolution_headers[" << j << "].size() = " << fvSolution_headers[j].size() << "\n";
        if(fvSolution_headers[j].size() > 0)
        {
            replacement_fvSolution_sections.push_back(fvSolution_sections[j]);
            // increase the size of this dimension for other vectors. Looks prettier to pushback an empty vector than to resize
            replacement_fvSolution_headers.push_back(std::vector<std::string>() );
            replacement_fvSolution_keyWords.push_back(std::vector< std::vector<std::string> >() );
            replacement_fvSolution_keyWordValues.push_back(std::vector< std::vector<std::string> >() );
            for(size_t i = 0; i < fvSolution_headers[j].size(); i++)
            {
                //std::cout << "fvSolution_headers[" << j << "][" << i << "] = " << fvSolution_headers[j][i] << "\n";
                //std::cout << "fvSolution_keyWords[" << j << "][" << i << "].size() = " << fvSolution_keyWords[j][i].size() << "\n";
                if(fvSolution_keyWords[j][i].size() > 0)
                {
                    replacement_fvSolution_headers[replaceSectionCounter].push_back(fvSolution_headers[j][i]);
                    // increase the size of this dimension for other vectors
                    replacement_fvSolution_keyWords[replaceSectionCounter].push_back(std::vector<std::string>() );
                    replacement_fvSolution_keyWordValues[replaceSectionCounter].push_back(std::vector<std::string>() );
                    for(size_t k = 0; k < fvSolution_keyWords[j][i].size(); k++)
                    {
                        //std::cout << "fvSolution_keyWords[" << j << "][" << i << "][" << k << "] = " << fvSolution_keyWords[j][i][k] << "\n";
                        //std::cout << "fvSolution_keyValues[" << j << "][" << i << "][" << k << "] = " << fvSolution_keyWordValues[j][i][k] << "\n";
                        //std::cout << "replacement_fvSolution_keyWords.size() = " << replacement_fvSolution_sections.size() << "\n";
                        //std::cout << "replacement_fvSolution_keyWords[" << replaceSectionCounter << "].size() = " << replacement_fvSolution_keyWords[replaceSectionCounter].size() << "\n";
                        //std::cout << "replacement_fvSolution_keyWords[" << replaceSectionCounter << "][" << replaceHeaderCounter << "].size() = " << replacement_fvSolution_keyWords[replaceSectionCounter][replaceHeaderCounter].size() << ", before adding one\n";
                        replacement_fvSolution_keyWords[replaceSectionCounter][replaceHeaderCounter].push_back(fvSolution_keyWords[j][i][k]);
                        replacement_fvSolution_keyWordValues[replaceSectionCounter][replaceHeaderCounter].push_back(fvSolution_keyWordValues[j][i][k]);
                    }
                    replaceHeaderCounter = replaceHeaderCounter + 1;
                }
            }
            replaceSectionCounter = replaceSectionCounter + 1;
            // reset replaceHeaderCounter
            replaceHeaderCounter = 0;
        }
    }

    // now replace old vector stuff with the new. I think the easiest way to do this is to resize the old
    // replacing values, going value by value
    std::cout << "resizing old vectors to fill with new vectors\n";
    fvSolution_sections.resize(replacement_fvSolution_sections.size());
    fvSolution_headers.resize(replacement_fvSolution_sections.size());
    fvSolution_keyWords.resize(replacement_fvSolution_sections.size());
    fvSolution_keyWordValues.resize(replacement_fvSolution_sections.size());
    //std::cout << "replacement_fvSolution_sections.size() = " << replacement_fvSolution_sections.size() << "\n";
    for(size_t j = 0; j < replacement_fvSolution_sections.size(); j++)
    {
        fvSolution_sections[j] = replacement_fvSolution_sections[j];
        // will this work? Do I need to do resizing completely separate from filling values?
        fvSolution_headers[j].resize(replacement_fvSolution_headers[j].size());
        // change the size of this dimension for other vectors
        fvSolution_keyWords[j].resize(replacement_fvSolution_keyWords[j].size());
        fvSolution_keyWordValues[j].resize(replacement_fvSolution_keyWordValues[j].size());
        //std::cout << "replacement_fvSolution_headers[" << j << "].size() = " << replacement_fvSolution_headers[j].size() << "\n";
        for(size_t i = 0; i < replacement_fvSolution_headers[j].size(); i++)
        {
            fvSolution_headers[j][i] = replacement_fvSolution_headers[j][i];
            fvSolution_keyWords[j][i].resize(replacement_fvSolution_keyWords[j][i].size());
            fvSolution_keyWordValues[j][i].resize(replacement_fvSolution_keyWordValues[j][i].size());
            //std::cout << "replacement_fvSolution_keyWords[" << j << "][" << i << "].size() = " << replacement_fvSolution_keyWords[j][i].size() << "\n";
            for(size_t k = 0; k < replacement_fvSolution_keyWords[j][i].size(); k++)
            {
                fvSolution_keyWords[j][i][k] = replacement_fvSolution_keyWords[j][i][k];
                fvSolution_keyWordValues[j][i][k] = replacement_fvSolution_keyWordValues[j][i][k];
            }
        }
    }

    calculateBiggestString();
    calculateWhiteSpace();
    // now with all the vectors nice in a nice order for printing, and the whitespace generated, ready to get output
    std::cout << "finished resizing vectors for output\n";
}

void openFoam_fvSolutions::calculateWhiteSpace()
{
    std::cout << "calculating whitespace\n";
    unsigned int neededWhiteSpace = 0;
    std::string createdWhiteSpace = "";

    // probably should just resize the whitespace here. It hasn't been filled at all yet
    fvSolution_whitespace.resize(fvSolution_sections.size());
    //std::cout << "fvSolution_whitespace.size() = " << fvSolution_whitespace.size() << "\n";
    for(size_t j = 0; j < fvSolution_sections.size(); j++)
    {
        // will this work? need to do resizing completely separate from filling values? Looks like it did work
        fvSolution_whitespace[j].resize(fvSolution_headers[j].size());
        //std::cout << "fvSolution_whitespace[" << j << "].size() = " << fvSolution_whitespace[j].size() << "\n";
        for(size_t i = 0; i < fvSolution_headers[j].size(); i++)
        {
            for(size_t k = 0; k < fvSolution_keyWords[j][i].size(); k++)
            {
                //std::cout << "fvSolution_keyWords[" << j << "][" << i << "][" << k << "] = " << fvSolution_keyWords[j][i][k] << "\n";
                if(fvSolution_keyWords[j][i][k].substr(0,1) == "$") // assumes it has to be at least one char
                {
                    createdWhiteSpace = "";
                    //std::cout << "createdWhitespace = \"" << createdWhiteSpace << "\"\n";
                    fvSolution_whitespace[j][i].push_back(createdWhiteSpace);
                } else
                {
                    neededWhiteSpace = biggestString - fvSolution_keyWords[j][i][k].size() + minimumWhiteSpace;
                    createdWhiteSpace = "";
                    for(size_t m = 0; m < neededWhiteSpace; m++)
                    {
                        createdWhiteSpace = createdWhiteSpace + " ";
                    }
                    //std::cout << "createdWhitespace = \"" << createdWhiteSpace << "\"\n";
                    fvSolution_whitespace[j][i].push_back(createdWhiteSpace);
                }
            }
        }
    }
}

void openFoam_fvSolutions::calculateBiggestString()
{
    // I keep wondering if I should make separate biggestStrings for given sections/headers
    // but it looks like it will be more complex than that
    std::cout << "calculating biggest string\n";
    for(size_t j = 0; j < fvSolution_sections.size(); j++)
    {
        for(size_t i = 0; i < fvSolution_headers[j].size(); i++)
        {
            for(size_t k = 0; k < fvSolution_keyWords[j][i].size(); k++)
            {
                if(fvSolution_keyWords[j][i][k].size() > biggestString)
                {
                    biggestString = fvSolution_keyWords[j][i][k].size();
                }
            }
        }
    }
}

std::vector<std::string> openFoam_fvSolutions::get_sections()
{
    if(finishedAddingValues == false)
    {
        resizeForOutput();
        finishedAddingValues = true;
    }
    return fvSolution_sections;
}

std::vector< std::vector<std::string> > openFoam_fvSolutions::get_headers()
{
    if(finishedAddingValues == false)
    {
        resizeForOutput();
        finishedAddingValues = true;
    }
    return fvSolution_headers;
}

std::vector< std::vector< std::vector<std::string> > > openFoam_fvSolutions::get_keyWords()
{
    if(finishedAddingValues == false)
    {
        resizeForOutput();
        finishedAddingValues = true;
    }
    return fvSolution_keyWords;
}

std::vector< std::vector< std::vector<std::string> > > openFoam_fvSolutions::get_whitespace()
{
    if(finishedAddingValues == false)
    {
        resizeForOutput();
        finishedAddingValues = true;
    }
    return fvSolution_whitespace;
}

std::vector< std::vector< std::vector<std::string> > > openFoam_fvSolutions::get_keyWordValues()
{
    if(finishedAddingValues == false)
    {
        resizeForOutput();
        finishedAddingValues = true;
    }
    return fvSolution_keyWordValues;
}
