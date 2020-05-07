#ifndef Hackiebox_h
#define Hackiebox_h

#include "BaseHeader.h"

#include "BoxButtonEars.h"

#include "WrapperWiFi.h"
#include "WrapperWebServer.h"


class Hackiebox { 
    public:
    void
        setup(),
        loop();
    private:

        BoxButtonEars _boxEars;

        WrapperWiFi _wifi;
        WrapperWebServer _server;
    
    void
        _buttonLoop();

};

#endif
