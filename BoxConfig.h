#ifndef BoxConfig_h
#define BoxConfig_h

#include "BaseHeader.h"
#include "ConfigStructures.h"

#define BOXCONFIG_JSON_SIZE 1024

class BoxConfig {
    public:
        void
            begin(),
            read(),
            write();

        String getAsJson();
        void setFromJson(String);

        ConfigStruct* get();
    private:
        ConfigStruct _config;

        void _initializeConfig();
};

extern BoxConfig Config;

#endif