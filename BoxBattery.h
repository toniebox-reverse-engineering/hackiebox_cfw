#ifndef BoxBattery_h
#define BoxBattery_h

#include "BaseHeader.h"

class BoxBattery {
    public:
        void
            begin(),
            loop(),
            reloadConfig();
        
        uint16_t getBatteryAdcRaw();
        uint16_t getBatteryVoltage();

        bool
            isChargerConnected(),
            isBatteryLow(),
            isBatteryCritical();

        void logBatteryStatus();

    private:
        uint32_t _batteryVoltageFactor;
        uint32_t _batteryVoltageChargerFactor;
        uint16_t _batteryLowAdc;
        uint16_t _batteryCriticalAdc;
};

#endif