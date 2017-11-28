#include "openFoam_fvSchemes.h"

openFoam_fvSchemes::openFoam_fvSchemes()
{
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
    fvScheme_names.resize(fvScheme_types.size());
    fvScheme_values.resize(fvScheme_types.size());
    fvScheme_whitespace.resize(fvScheme_types.size());

    biggestString = 0;
    minimumWhiteSpace = 4;  // the whitespace given to what is found to have the biggestString
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
}

void openFoam_fvSchemes::add_nondefault(std::string type, std::string name, std::string value)
{
    bool foundType = false;
    for(size_t j = 0; j < fvScheme_types.size(); j++)
    {
        if(fvScheme_types[j] == type)
        {
            fvScheme_names[j].push_back(name);
            fvScheme_values[j].push_back(value);
            foundType = true;
            break;
        }
    }
    if(foundType == false)
    {
        std::cout << "Error adding fvScheme name and value! Type " << type << " is not a valid type!\n";
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
        for(size_t i = 0; i < fvScheme_names[j].size(); i++)
        {
            if(fvScheme_types[j] == "fluxRequired")
            {
                createdWhiteSpace = "";
                fvScheme_whitespace[j].push_back(createdWhiteSpace);
            } else
            {
                neededWhiteSpace = biggestString - fvScheme_names[j][i].size() + minimumWhiteSpace;
                createdWhiteSpace = "";
                for(size_t k = 0; k < neededWhiteSpace; k++)
                {
                    createdWhiteSpace = createdWhiteSpace + " ";
                }
                fvScheme_whitespace[j].push_back(createdWhiteSpace);
            }
        }
    }
}

void openFoam_fvSchemes::calculateBiggestString()
{
    for(size_t j = 0; j < fvScheme_types.size(); j++)
    {
        for(size_t i = 0; i < fvScheme_names[j].size(); i++)
        {
            if(fvScheme_names[j][i].size() > biggestString)
            {
                biggestString = fvScheme_names[j][i].size();
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

size_t openFoam_fvSchemes::get_numOfTypes()
{
    return fvScheme_types.size();
}

std::string openFoam_fvSchemes::get_type(size_t typeIndex)
{
    if(typeIndex >= fvScheme_types.size())
    {
        std::cout << "Error getting fvScheme type. typeIndex " << typeIndex << " greater than number of types!\n";
        return "";
    } else
    {
        return fvScheme_types[typeIndex];
    }
}

std::string openFoam_fvSchemes::get_defaultwhitespace(size_t typeIndex)
{
    if(typeIndex >= fvScheme_types.size())
    {
        std::cout << "Error getting fvScheme default whitespace. typeIndex " << typeIndex << " greater than number of types!\n";
        return "";
    } else
    {
        if(biggestString == 0)
        {
            // this function runs the calculateWhiteSpace function after it finishes
            calculateBiggestString();
        }
        return fvScheme_defaultwhitespace[typeIndex];
    }
}

std::string openFoam_fvSchemes::get_defaultvalue(size_t typeIndex)
{
    if(typeIndex >= fvScheme_types.size())
    {
        std::cout << "Error getting fvScheme default value. typeIndex " << typeIndex << " greater than number of types!\n";
        return "";
    } else
    {
        return fvScheme_defaultvalues[typeIndex];
    }
}

size_t openFoam_fvSchemes::get_type_numOfVals(size_t typeIndex)
{
    if(typeIndex >= fvScheme_types.size())
    {
        std::cout << "Error getting fvScheme type numberOfValues. typeIndex " << typeIndex << " greater than number of types!\n";
        return 0;
    } else
    {
        return fvScheme_names[typeIndex].size();
    }
}

std::string openFoam_fvSchemes::get_name(size_t typeIndex, size_t nameIndex)
{
    if(typeIndex >= fvScheme_types.size())
    {
        std::cout << "Error getting fvScheme name. typeIndex " << typeIndex << " greater than number of types!\n";
        return "";
    } else if(nameIndex >= fvScheme_names[typeIndex].size())
    {
        std::cout << "Error getting fvScheme name. nameIndex " << nameIndex << " greater than number of names!\n";
        return "";
    } else
    {
        return fvScheme_names[typeIndex][nameIndex];
    }
}

std::string openFoam_fvSchemes::get_whitespace(size_t typeIndex, size_t nameIndex)
{
    if(typeIndex >= fvScheme_types.size())
    {
        std::cout << "Error getting fvScheme whitespace. typeIndex " << typeIndex << " greater than number of types!\n";
        return "";
    } else if(nameIndex >= fvScheme_names[typeIndex].size())
    {
        std::cout << "Error getting fvScheme whitespace. nameIndex " << nameIndex << " greater than number of names!\n";
        return "";
    } else
    {
        if(biggestString == 0)
        {
            // this function runs the calculateWhiteSpace function after it finishes
            calculateBiggestString();
        }
        return fvScheme_whitespace[typeIndex][nameIndex];
    }
}

std::string openFoam_fvSchemes::get_value(size_t typeIndex, size_t nameIndex)
{
    if(typeIndex >= fvScheme_types.size())
    {
        std::cout << "Error getting fvScheme value. typeIndex " << typeIndex << " greater than number of types!\n";
        return "";
    } else if(nameIndex >= fvScheme_names[typeIndex].size())
    {
        std::cout << "Error getting fvScheme value. nameIndex " << nameIndex << " greater than number of names!\n";
        return "";
    } else
    {
        return fvScheme_values[typeIndex][nameIndex];
    }
}
