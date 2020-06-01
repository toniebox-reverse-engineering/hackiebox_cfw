#ifndef BoxBattery_h
#define BoxBattery_h

#include "BaseHeader.h"
#include "JC_Button.h"

class BoxBattery {
    public:
        enum class BatteryEvent {
            BAT_LOW,
            BAT_CRITICAL,
            CHR_CONNECT,
            CHR_DISCONNECT
        };

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

        Button _charger = Button(8, 25, false, false);
        bool _wasLow;
        bool _wasCritical;
        uint16_t _batteryAdcRaw;
        uint16_t _batteryAdcLowRaw;
};

#endif