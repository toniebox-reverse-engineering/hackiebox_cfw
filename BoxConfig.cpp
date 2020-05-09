#include "BoxConfig.h"

void BoxConfig::begin() { 
    _initializeConfig();
    read();
}

void BoxConfig::read() {

}
void BoxConfig::write() { 

}

ConfigStruct* BoxConfig::getConfig() { 
    return &_config;
}

void BoxConfig::_initializeConfig() { 
    _config.version = CONFIG_ACTIVE_VERSION;

    //Battery
    _config.battery.voltageFactor = 67690;
    _config.battery.voltageChargerFactor = 71907;
    _config.battery.minimalAdc = 2400;

    //Button Ears
    _config.buttonEars.longPressMs = 1000;
    _config.buttonEars.veryLongPressMs = 10000;
}