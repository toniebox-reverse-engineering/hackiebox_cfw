#ifndef Hackiebox_h
#define Hackiebox_h

#include "BaseHeader.h"

#include "BoxBattery.h"
#include "BoxButtonEars.h"
#include "BoxLEDs.h"
#include "BoxPower.h"
#include "BoxSD.h"

#include "WrapperWiFi.h"
#include "WrapperWebServer.h"


class Hackiebox { 
    public:
    void
        setup(),
        loop();

        BoxBattery boxBattery;
        BoxButtonEars boxEars;
        BoxLEDs boxLEDs;
        BoxPower boxPower;
        BoxSD boxSD;
    private:

        WrapperWiFi _wifi;
        WrapperWebServer _server;
    
    void
        _buttonLoop();

};

extern Hackiebox Box;

#endif
