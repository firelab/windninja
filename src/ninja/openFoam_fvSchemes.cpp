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

#include "openFoam_fvSchemes.h"

openFoam_fvSchemes::openFoam_fvSchemes()
{
    finishedAddingValues = false;
    biggestString = 0;
    minimumWhiteSpace = 4;  // the whitespace given to what is found to have the biggestString

    // setup the types of fvSchemes in the correct order used in OpenFOAM
    fvScheme_types.push_back("ddtSchemes");
    fvScheme_types.push_back("gradSchemes");
    fvScheme_types.push_back("divSchemes");
    fvScheme_types.push_back("laplacianSchemes");
    fvScheme_types.push_back("interpolationSchemes");
    fvScheme_types.push_back("SnGradSchemes");
    fvScheme_types.push_back("fluxRequired");

    // check to make sure it was setup correctly in the right order
    if(check_typePlacement() == true)
    {
        std::cout << "Error setting up openFoam_fvSchemes! the type order isn't correct!\n";
    }

    // get the vector sizes right for the vector of vectors
    fvScheme_nondefaultnames.resize(fvScheme_types.size());
    fvScheme_nondefaultvalues.resize(fvScheme_types.size());
    fvScheme_nondefaultwhitespace.resize(fvScheme_types.size());
}

bool openFoam_fvSchemes::check_typePlacement()
{
    bool fail = false;
    // if the style of setup changes, say new version of openfoam, need to change this to match the new format
    if(fvScheme_types.size() != 7)
    {
        std::cout << "Error! fvSchemes_types setup with size " << fvScheme_types.size() << " not size 7!"
                  << " Check openFoam_fvSchemes constructor!\n";
        fail = true;
    } else
    {
        if(fvScheme_types[0] != "ddtSchemes")
        {
            std::cout << "fvSchemes_types[0] != ddtSchemes!\n";
            fail = true;
        }
        if(fvScheme_types[1] != "gradSchemes")
        {
            std::cout << "fvSchemes_types[1] != gradSchemes!\n";
            fail = true;
        }
        if(fvScheme_types[2] != "divSchemes")
        {
            std::cout << "fvSchemes_types[2] != divSchemes!\n";
            fail = true;
        }
        if(fvScheme_types[3] != "laplacianSchemes")
        {
            std::cout << "fvSchemes_types[3] != laplacianSchemes!\n";
            fail = true;
        }
        if(fvScheme_types[4] != "interpolationSchemes")
        {
            std::cout << "fvSchemes_types[4] != interpolationSchemes!\n";
            fail = true;
        }
        if(fvScheme_types[5] != "SnGradSchemes")
        {
            std::cout << "fvSchemes_types[5] != SnGradSchemes!\n";
            fail = true;
        }
        if(fvScheme_types[6] != "fluxRequired")
        {
            std::cout << "fvSchemes_types[6] != fluxRequired!\n";
            fail = true;
        }
    }
    return fail;
}
void openFoam_fvSchemes::add_default(std::string type, std::string defaultvalue)
{
    if(finishedAddingValues == false)
    {
        bool foundType = false;
        for(size_t j = 0; j < fvScheme_types.size(); j++)
        {
            if(fvScheme_types[j] == type)
            {
                fvScheme_defaultvalues.push_back(defaultvalue);
                foundType = true;
                break;
            }
        }
        if(foundType == false)
        {
            std::cout << "Error adding fvScheme default value! Type " << type << " is not a valid type!\n";
        }
    } else
    {
        std::cout << "Error! Can't add values to fvSchemes after running getValue type functions!\n";
    }
}

void openFoam_fvSchemes::add_nondefault(std::string type, std::string name, std::string value)
{
    if(finishedAddingValues == false)
    {
        bool foundType = false;
        for(size_t j = 0; j < fvScheme_types.size(); j++)
        {
            if(fvScheme_types[j] == type)
            {
                fvScheme_nondefaultnames[j].push_back(name);
                fvScheme_nondefaultvalues[j].push_back(value);
                foundType = true;
                break;
            }
        }
        if(foundType == false)
        {
            std::cout << "Error adding fvScheme name and value! Type " << type << " is not a valid type!\n";
        }
    } else
    {
        std::cout << "Error! Can't add values to fvSchemes after running getValue type functions!\n";
    }
}

void openFoam_fvSchemes::setupDesiredValues(std::string simulationType)
{
    if(simulationType == "simpleFoam")
    {
        add_default("ddtSchemes","steadyState");
        add_default("gradSchemes","cellMDLimited leastSquares 0.5");
        add_default("divSchemes","none");
        add_nondefault("divSchemes","div(phi,U)","bounded Gauss linearUpwind grad(U)");
        add_nondefault("divSchemes","div(phi,k)","bounded Gauss upwind");
        add_nondefault("divSchemes","div(phi,epsilon)","bounded Gauss upwind");
        add_nondefault("divSchemes","div(phi,omega)","bounded Gauss 1.0");
        add_nondefault("divSchemes","div(phi,nuTilda)","bounded Gauss 1.0");
        add_nondefault("divSchemes","div((nuEff*dev(T(grad(U)))))","Gauss linear");
        add_nondefault("divSchemes","div((nuEff*dev(grad(U).T())))","Gauss linear");
        add_nondefault("divSchemes","div(phi,T)","bounded Gauss limitedLinear 1");
        add_default("laplacianSchemes","Gauss linear limited 0.333");
        add_default("interpolationSchemes","linear");
        add_nondefault("interpolationSchemes","interpolate(U)","linear");
        add_default("SnGradSchemes","corrected");
        add_nondefault("SnGradSchemes","snGrad(T)","limited 0.5");
        add_nondefault("SnGradSchemes","snGrad(k)","limited 0.5");
        add_nondefault("SnGradSchemes","snGrad(epsilon)","limited 0.5");
        add_nondefault("SnGradSchemes","snGrad(omega)","limited 0.5");
        add_nondefault("SnGradSchemes","snGrad(nuTilda)","limited 0.5");
        add_default("fluxRequired","no");
        add_nondefault("fluxRequired","p","");
    } else if(simulationType == "buoyantBoussinesqPimpleFoam")
    {
        // these are almost the same as a WindNinja simulation
        add_default("ddtSchemes","Euler");
        add_default("gradSchemes","cellMDLimited leastSquares 0.5");
        add_default("divSchemes","none");
        add_nondefault("divSchemes","div(phi,U)","bounded Gauss linearUpwind grad(U)");
        add_nondefault("divSchemes","div(phi,k)","bounded Gauss upwind");
        add_nondefault("divSchemes","div(phi,epsilon)","bounded Gauss upwind");
        add_nondefault("divSchemes","div(phi,omega)","bounded Gauss 1.0");
        add_nondefault("divSchemes","div(phi,nuTilda)","bounded Gauss 1.0");
        add_nondefault("divSchemes","div((nuEff*dev(T(grad(U)))))","Gauss linear");
        add_nondefault("divSchemes","div((nuEff*dev(grad(U).T())))","Gauss linear");
        add_nondefault("divSchemes","div(phi,T)","bounded Gauss limitedLinear 1");
        add_default("laplacianSchemes","Gauss linear limited 0.333");
        add_nondefault("laplacianSchemes","laplacian(DT,T)","Gauss linear corrected");
        add_default("interpolationSchemes","linear");
        add_nondefault("interpolationSchemes","interpolate(U)","linear");
        add_default("SnGradSchemes","corrected");
        add_nondefault("SnGradSchemes","snGrad(T)","limited 0.5");
        add_nondefault("SnGradSchemes","snGrad(k)","limited 0.5");
        add_nondefault("SnGradSchemes","snGrad(epsilon)","limited 0.5");
        add_nondefault("SnGradSchemes","snGrad(omega)","limited 0.5");
        add_nondefault("SnGradSchemes","snGrad(nuTilda)","limited 0.5");
        add_default("fluxRequired","no");
        add_nondefault("fluxRequired","p","");
        add_nondefault("fluxRequired","p_rgh","");
    } else
    {
        // assume it is a myScalarTransportFoam solution
        add_default("ddtSchemes","Euler");
        add_default("gradSchemes","Gauss linear");
        add_default("divSchemes","none");
        add_nondefault("divSchemes","div(phi,T)","bounded Gauss upwind");
        add_default("laplacianSchemes","Gauss linear limited 0.333");
        add_nondefault("laplacianSchemes","laplacian(DT,T)","Gauss linear corrected");
        add_default("interpolationSchemes","linear");
        add_default("SnGradSchemes","corrected");
        add_nondefault("SnGradSchemes","SnGrad(T)","limited 0.5");
        add_default("fluxRequired","no");
        add_nondefault("fluxRequired","T","");
    }
}

void openFoam_fvSchemes::calculateWhiteSpace()
{
    unsigned int neededWhiteSpace = 0;
    std::string createdWhiteSpace = "";

    for(size_t j = 0; j < fvScheme_types.size(); j++)
    {
        // do this for the default value. "default" has 7 chars
        neededWhiteSpace = biggestString - 7 + minimumWhiteSpace;
        createdWhiteSpace = "";
        for(size_t k = 0; k < neededWhiteSpace; k++)
        {
            createdWhiteSpace = createdWhiteSpace + " ";
        }
        fvScheme_defaultwhitespace.push_back(createdWhiteSpace);

        // now do the nondefault stuff. Might as well reuse the variables
        for(size_t i = 0; i < fvScheme_nondefaultnames[j].size(); i++)
        {
            if(fvScheme_types[j] == "fluxRequired")
            {
                createdWhiteSpace = "";
                fvScheme_nondefaultwhitespace[j].push_back(createdWhiteSpace);
            } else
            {
                neededWhiteSpace = biggestString - fvScheme_nondefaultnames[j][i].size() + minimumWhiteSpace;
                createdWhiteSpace = "";
                for(size_t k = 0; k < neededWhiteSpace; k++)
                {
                    createdWhiteSpace = createdWhiteSpace + " ";
                }
                fvScheme_nondefaultwhitespace[j].push_back(createdWhiteSpace);
            }
        }
    }
}

void openFoam_fvSchemes::calculateBiggestString()
{
    for(size_t j = 0; j < fvScheme_types.size(); j++)
    {
        for(size_t i = 0; i < fvScheme_nondefaultnames[j].size(); i++)
        {
            if(fvScheme_nondefaultnames[j][i].size() > biggestString)
            {
                biggestString = fvScheme_nondefaultnames[j][i].size();
            }
        }
    }
    // check to see if the word "default" is the biggestString. "default" has 7 chars
    if(7 > biggestString)
    {
        biggestString = 7;
    }
    calculateWhiteSpace();
}

std::vector<std::string> openFoam_fvSchemes::get_types()
{
    finishedAddingValues = true;
    return fvScheme_types;
}

std::vector<std::string> openFoam_fvSchemes::get_defaultwhitespace()
{
    finishedAddingValues = true;
    // this function runs the calculateWhiteSpace function after it finishes
    calculateBiggestString();
    return fvScheme_defaultwhitespace;
}

std::vector<std::string> openFoam_fvSchemes::get_defaultvalues()
{
    finishedAddingValues = true;
    return fvScheme_defaultvalues;
}

std::vector< std::vector<std::string> > openFoam_fvSchemes::get_nondefaultnames()
{
    finishedAddingValues = true;
    return fvScheme_nondefaultnames;
}

std::vector< std::vector<std::string> > openFoam_fvSchemes::get_nondefaultwhitespace()
{
    finishedAddingValues = true;
    // this function runs the calculateWhiteSpace function after it finishes
    calculateBiggestString();
    return fvScheme_nondefaultwhitespace;
}

std::vector< std::vector<std::string> > openFoam_fvSchemes::get_nondefaultvalues()
{
    finishedAddingValues = true;
    return fvScheme_nondefaultvalues;
}
