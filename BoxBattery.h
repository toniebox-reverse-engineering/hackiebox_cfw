#ifndef BoxBattery_h
#define BoxBattery_h

#include "BaseHeader.h"

class BoxBattery {
    public:
        void
            begin(),
            loop();
        
        bool isChargerConnected();
        uint16_t getBatteryAdcRaw();
        uint16_t getBatteryVoltage();
        bool isBatteryLow();

        void logBatteryStatus();

    private:
        uint32_t _batteryVoltageFactor = 67690;
        uint32_t _batteryVoltageChargerFactor = 71907;
        uint16_t _batteryMinimalAdc = 2400;
};

#endif