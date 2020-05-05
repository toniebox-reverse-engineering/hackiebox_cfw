#ifndef WrapperWebServer_h
#define WrapperWebServer_h

#include "BaseHeader.h"

#include <WebServer.h>

class WrapperWebServer { 
    public:
    void
        begin(),
        handle(void);

    private:
        void
            handleNotFound(void),
            handleRoot(void),
            handleSse(void),
            handleAjax(void);

        WebServer* _server;

};

#endif
