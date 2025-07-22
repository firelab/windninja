#ifndef SERVERBRIDGE_H
#define SERVERBRIDGE_H

#include <QMessageBox>
#include "ninja_version.h"
#include <sstream>
#include <cpl_conv.h>
#include <cpl_http.h>

class ServerBridge
{
public:
    ServerBridge();
    void checkMessages(void);

private:
    QString version;

    bool NinjaCheckVersions(char * mostrecentversion, char * localversion);
    char * NinjaQueryServerMessages(bool checkAbort);
};

#endif // SERVERBRIDGE_H
