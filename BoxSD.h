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

        bool isInitialized();

        void webJsonListDir(WebServer* webServer, char* directory);
    private:
        bool _initialized = false;
};

#endif