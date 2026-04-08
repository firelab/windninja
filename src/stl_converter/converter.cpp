#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <string.h>
#include <math.h>
#include "gdal_priv.h"
#include "ninja_conv.h"
#include "ninja_init.h"
#include "cpl_string.h"

#include "stl_create.h"

void Usage()
{
    printf("stl_converter [-r cellsize] [-o zoffset] input output\n");
    exit(1);
}

int main( int argc, char* argv[] )
{
    NinjaInitialize();
    /*  parse input arguments  */
    double dfRes = 0.0;
    double dfOffset = 0.0;
    const char *pszIn = NULL;
    const char *pszOut = NULL;
    int i = 1;
    while( i < argc )
    {
        if( strcmp( argv[i], "-r" ) == 0 && i + 1 < argc )
        {
            dfRes = atof( argv[++i] );
        }
        else if( strcmp( argv[i], "-o" ) == 0 && i + 1 < argc )
        {
            dfOffset = atof(argv[++i]);
        }
        else if( pszIn == NULL )
        {
            pszIn = argv[i];
        }
        else if( pszOut == NULL )
        {
            pszOut = argv[i];
        }
        else
        {
            Usage();
        }
        i++;
    }

    if( pszIn == NULL || pszOut == NULL )
    {
        Usage();
    }
    return NinjaElevationToStl( pszIn, pszOut, 1, dfRes, NinjaStlBinary, dfOffset, GDALTermProgress );
}

