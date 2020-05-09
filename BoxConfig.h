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

        ConfigStruct* getConfig();
    private:
        ConfigStruct _config;

        void _initializeConfig();
};

extern BoxConfig Config;

#endif