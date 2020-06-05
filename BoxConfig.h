#ifndef BoxConfig_h
#define BoxConfig_h

#include "BaseHeader.h"
#include "ConfigStructures.h"
#include "BoxSD.h"

#define BOXCONFIG_JSON_SIZE 382 //Size from https://arduinojson.org/v6/assistant/

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
        ConfigStruct _config;

        void _initializeConfig();
};

extern BoxConfig Config;

#endif