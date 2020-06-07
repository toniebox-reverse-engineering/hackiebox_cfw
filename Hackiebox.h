#ifndef Hackiebox_h
#define Hackiebox_h

#include "BaseHeader.h"

#include <ThreadController.h>

#include "BoxAccelerometer.h"
#include "BoxBattery.h"
#include "BoxButtonEars.h"
#include "BoxEvents.h"
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

        BoxAccelerometer boxAccel;
        BoxBattery boxBattery;
        BoxButtonEars boxEars;
        BoxLEDs boxLEDs;
        BoxPower boxPower;
        BoxSD boxSD;
        WrapperWiFi boxWiFi;
        WrapperWebServer webServer;
    private:


        ThreadController _threadController;
    void
        _buttonLoop();

};

extern Hackiebox Box;

#endif
