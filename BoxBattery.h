#ifndef BoxBattery_h
#define BoxBattery_h

#include "BaseHeader.h"
#include "JC_Button.h"

#include <EnhancedThread.h>

class BoxBattery : public EnhancedThread {
    public:
        struct BatteryStats {
            bool charging;
            bool low;
            bool critical;
            uint16_t adcRaw;
            uint16_t voltage;
            bool testActive;
            uint16_t testActiveMinutes;
        };
        

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

        void startBatteryTest();
        void stopBatteryTest();
        bool batteryTestActive();
        
        EnhancedThread batteryTestThread;
        void doBatteryTestStep();

        BoxBattery::BatteryStats getBatteryStats();

    private:
        uint32_t _batteryVoltageFactor;
        uint16_t _batteryLowAdc;
        uint16_t _batteryCriticalAdc;

        Button _charger = Button(8, 25, false, false);
        bool _wasLow;
        bool _wasCritical;
        uint16_t _batteryAdcRaw;
        uint16_t _batteryAdcLowRaw;

        const char* _batteryTestFilename = "/revvox/batteryTest.csv";
        uint64_t _batteryTestStartMillis;

        const static uint8_t BATTERY_VOLTAGE_PIN = 60;
        void _readBatteryAdc();
};

#endif