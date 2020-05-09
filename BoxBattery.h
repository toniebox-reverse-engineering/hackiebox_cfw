#ifndef BoxBattery_h
#define BoxBattery_h

#include "BaseHeader.h"

class BoxBattery {
    public:
        void
            begin(),
            loop(),
            reloadConfig();
        
        bool isChargerConnected();
        uint16_t getBatteryAdcRaw();
        uint16_t getBatteryVoltage();
        bool isBatteryLow();

        void logBatteryStatus();

    private:
        uint32_t _batteryVoltageFactor;
        uint32_t _batteryVoltageChargerFactor;
        uint16_t _batteryMinimalAdc;
};

#endif