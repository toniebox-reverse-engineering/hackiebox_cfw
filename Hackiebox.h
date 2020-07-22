#ifndef Hackiebox_h
#define Hackiebox_h

#include "BaseHeader.h"

#include <ThreadController.h>

#include <hw_ints.h>
#include <driverlib/wdt.h>
#include <driverlib/rom_map.h>
#include <driverlib/prcm.h>

#include "BoxAccelerometer.h"
#include "BoxBattery.h"
#include "BoxButtonEars.h"
#include "BoxCLI.h"
#include "BoxDAC.h"
#include "BoxEvents.h"
#include "BoxI2C.h"
#include "BoxLEDs.h"
#include "BoxPower.h"
#include "BoxRFID.h"
#include "BoxSD.h"

#include "WrapperWiFi.h"
#include "WrapperWebServer.h"

#include "LogStreamMulti.h"
#include "LogStreamSd.h"
#include "LogStreamSse.h"

class Hackiebox { 
    public:

        void
            setup(),
            loop();
        
        bool
            watchdog_start(),
            watchdog_isFed();
        void
            watchdog_stop(),
            watchdog_feed(),
            watchdog_unfeed();

        bool inDelayTask;
        void delayTask(uint16_t millis);
        void delayTaskWork(uint16_t millis);


        ThreadController threadController;
        
        BoxAccelerometer boxAccel;
        BoxBattery boxBattery;
        BoxButtonEars boxEars; 
        BoxCLI boxCLI; 
        BoxDAC boxDAC;
        BoxI2C boxI2C;
        BoxLEDs boxLEDs;
        BoxPower boxPower;
        BoxRFID boxRFID;
        BoxSD boxSD;
        WrapperWiFi boxWiFi;
        WrapperWebServer webServer;

        LogStreamMulti logStreamMulti;
        LogStreamSd logStreamSd;
        LogStreamSse logStreamSse;

    private:/*
        typedef void (*fAPPWDTDevCallbk)();
        void 
            watchdog_handler();*/
        
        bool _watchdog_fed;
};
extern Hackiebox Box;

#endif
