#ifndef BoxSD_h
#define BoxSD_h

#include "BaseHeader.h"
#include <FatFs.h>
#include <WebServer.h>

class BoxSD {
    public:
        void
            begin(),
            loop();

        void webJsonListDir(WebServer* webServer, char* directory);
    private:
};

#endif