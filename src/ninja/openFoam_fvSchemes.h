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

    void add_default(std::string type, std::string defaultvalue);
    void add_nondefault(std::string type, std::string name, std::string value);

    size_t get_numOfTypes();
    std::string get_type(size_t typeIndex);
    std::string get_defaultwhitespace(size_t typeIndex);
    std::string get_defaultvalue(size_t typeIndex);
    size_t get_type_numOfVals(size_t typeIndex);
    std::string get_name(size_t typeIndex, size_t nameIndex);
    std::string get_whitespace(size_t typeIndex, size_t nameIndex);
    std::string get_value(size_t typeIndex, size_t nameIndex);

private:

    // the idea here is that while the user can fill in the types in any order,
    // the output order must always be the same, so the order is actually fixed
    // so this is checking to make sure some programmer kept the order fixed inside the program
    bool check_typePlacement();
    void calculateWhiteSpace();
    void calculateBiggestString();

    unsigned int biggestString;
    unsigned int minimumWhiteSpace;
    std::vector<std::string> fvScheme_types;
    std::vector<std::string> fvScheme_defaultvalues;
    std::vector<std::string> fvScheme_defaultwhitespace;
    std::vector< std::vector<std::string> > fvScheme_names;
    std::vector< std::vector<std::string> > fvScheme_values;
    std::vector< std::vector<std::string> > fvScheme_whitespace;

};

#endif // OPENFOAM_FVSCHEMES_H
