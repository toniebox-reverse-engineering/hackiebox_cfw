#ifndef BoxConfig_h
#define BoxConfig_h

#include "BaseHeader.h"
#include "ConfigStructures.h"

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