#ifndef NINJATOOLS_H
#define NINJATOOLS_H

#include "ninjaException.h"
#include "wxModelInitializationFactory.h"
#include "nomads_wx_init.h"
#include "gcp_wx_init.h"

class ninjaTools
{
public:
    ninjaTools();
    void fetchData();

private:
    int nNomadsCount;
    NomadsWxModel **papoNomads;

};

#endif // NINJATOOLS_H
