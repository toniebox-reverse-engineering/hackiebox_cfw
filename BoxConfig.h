#ifndef BoxConfig_h
#define BoxConfig_h

#include "BaseHeader.h"
#include "ConfigStructures.h"
#include "BoxSD.h"


#define BOXCONFIG_JSON_SIZE 560
//{"version":255,"battery":{"voltageFactor":4294967295,"voltageChargerFactor":4294967295,"lowAdc":65535,"criticalAdc":65535,"sleepMinutes":255},"buttonEars":{"longPressMs":65535,"veryLongPressMs":65535},"wifi":{"ssid":"12345678901234567890123456789012","password":"1234567890123456789012345678901234567890123456789012345678901234"},"log":{"sdLog":false},"misc":{"autodump":false,"swd":false,"watchdogSeconds":255}}
//Size from https://arduinojson.org/v6/assistant/

class BoxConfig {
    public:
        void
            begin(),
            read(),
            write();

        String getAsJson();
        bool setFromJson(String);

        ConfigStruct* get();
    private:
        String _json;
        ConfigStruct _config;

        void _initializeConfig();
};

extern BoxConfig Config;

#endif