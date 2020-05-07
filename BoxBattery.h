#ifndef BoxBattery_h
#define BoxBattery_h

#include "BaseHeader.h"

class BoxBattery {
    public:
        void
            begin(),
            loop();
        
        bool isChargerConnected();
        int getBatteryAdcRaw();
        int getBatteryVoltage();
        bool isBatteryLow();

        void logBatteryStatus();

    private:
        int _batteryVoltageFactor = 67690;
        int _batteryVoltageChargerFactor = 71907;
        int _batteryMinimalAdc = 2400;
};

#endif