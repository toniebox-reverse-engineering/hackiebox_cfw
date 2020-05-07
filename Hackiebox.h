#ifndef Hackiebox_h
#define Hackiebox_h

#include "BaseHeader.h"

#include "BoxBattery.h"
#include "BoxButtonEars.h"
#include "BoxLEDs.h"
#include "BoxPower.h"

#include "WrapperWiFi.h"
#include "WrapperWebServer.h"


class Hackiebox { 
    public:
    void
        setup(),
        loop();
    private:

        BoxBattery _boxBattery;
        BoxButtonEars _boxEars;
        BoxLEDs _boxLEDs;
        BoxPower _boxPower;

        WrapperWiFi _wifi;
        WrapperWebServer _server;
    
    void
        _buttonLoop();

};

#endif
