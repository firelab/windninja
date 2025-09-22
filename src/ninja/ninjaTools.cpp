#include "ninjaTools.h"

ninjaTools::ninjaTools()
{
    nNomadsCount = 0;
    while( apszNomadsKeys[nNomadsCount][0] != NULL )
        nNomadsCount++;
    papoNomads = new NomadsWxModel*[nNomadsCount];
    int i = 0;
    while( apszNomadsKeys[i][0] != NULL )
    {
        papoNomads[i] = new NomadsWxModel( apszNomadsKeys[i][0] );
        i++;
    }
    CPLDebug( "WINDNINJA", "Loaded %d NOMADS models", nNomadsCount );
}

void ninjaTools::fetchData()
{
    wxModelInitialization *model;
    model = papoNomads[0];
    model->fetchForecast( "/home/mason/Downloads/testing/missoula_valley.tif", 36);
}
