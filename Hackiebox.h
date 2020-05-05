#ifndef Hackiebox_h
#define Hackiebox_h

#include "BaseHeader.h"

#include "WrapperWiFi.h"
#include "WrapperWebServer.h"


class Hackiebox { 
    public:
    void
        setup(),
        loop();
    private:
        WrapperWiFi _wifi;
        WrapperWebServer _server;

};

#endif
